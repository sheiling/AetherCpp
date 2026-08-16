#include "frame_dispatcher.h"

#include <common/utility/diagnostics/timing_debug.h>

#include <Logger.h>

#include <QFutureWatcher>
#include <QMetaObject>
#include <QMutexLocker>
#include <QtConcurrent>

#include <cstring>
#include <utility>

namespace camera_plugin {
namespace {

QString previewKindLabel(CameraImageKind kind)
{
    switch (kind) {
    case CameraImageKind::LivePreview:
        return QStringLiteral("实时预览");
    case CameraImageKind::SingleShot:
        return QStringLiteral("单帧");
    default:
        return QStringLiteral("其它");
    }
}

/** RGB24 AlgoMat 行拷贝为 QImage::Format_RGB888 */
QImage rgbMatToQImage(const AlgoMat &mat)
{
    if (AlgoMatIsEmpty(&mat) || mat.cols <= 0 || mat.rows <= 0 || !mat.data) {
        return {};
    }

    const int rowBytes = mat.cols * 3;
    const size_t needBytes = static_cast<size_t>(rowBytes) * static_cast<size_t>(mat.rows);
    if (mat.len < needBytes) {
        LOG_WARN("[FrameDispatcher] RGB buffer too small: len={} need={} {}x{}", mat.len, needBytes,
                 mat.cols, mat.rows);
        return {};
    }

    QImage image(mat.cols, mat.rows, QImage::Format_RGB888);
    for (int y = 0; y < mat.rows; ++y) {
        std::memcpy(image.scanLine(y), mat.data + static_cast<size_t>(y) * rowBytes,
                    static_cast<size_t>(rowBytes));
    }
    return image;
}

QImage monoMatToQImage(const AlgoMat &mat)
{
    if (AlgoMatIsEmpty(&mat) || mat.cols <= 0 || mat.rows <= 0 || !mat.data) {
        return {};
    }

    const int rowBytes = mat.cols;
    const size_t needBytes = static_cast<size_t>(rowBytes) * static_cast<size_t>(mat.rows);
    if (mat.len < needBytes) {
        LOG_WARN("[FrameDispatcher] mono buffer too small: len={} need={} {}x{}", mat.len, needBytes,
                 mat.cols, mat.rows);
        return {};
    }

    QImage image(mat.cols, mat.rows, QImage::Format_Grayscale8);
    for (int y = 0; y < mat.rows; ++y) {
        std::memcpy(image.scanLine(y), mat.data + static_cast<size_t>(y) * rowBytes,
                    static_cast<size_t>(rowBytes));
    }
    return image;
}

struct ConvertJob {
    std::shared_ptr<common::device::ImageInfo> info;
    QString                                    kindDetail;
};

QImage convertJobToQImage(const ConvertJob &job)
{
    common::utility::TimingDebugScope qimageTiming(
        common::utility::TimingDebugKeys::CaptureTiming, "帧分发.转QImage",
        job.kindDetail.toStdString());
    return FrameDispatcher::toDisplayImage(job.info);
}

} // namespace

FrameDispatcher::FrameDispatcher(QObject *parent)
    : QObject(parent)
{
}

void FrameDispatcher::setFrameHandler(FrameHandler handler)
{
    m_frameHandler = std::move(handler);
}

void FrameDispatcher::clearPendingFrames()
{
    QMutexLocker lock(&m_publishMutex);
    m_pendingPublish  = CameraFrame();
    m_publishPosted   = false;
    m_convertingFrame = CameraFrame();
    m_convertInFlight.store(false, std::memory_order_release);
}

void FrameDispatcher::publishFrame(CameraFrame frame)
{
    if (!m_frameHandler) {
        return;
    }

    {
        QMutexLocker lock(&m_publishMutex);
        m_pendingPublish = std::move(frame);
        if (m_convertInFlight.load(std::memory_order_acquire)) {
            // 仅合并为最新帧；转图完成后由 trySchedulePendingDeliver 再派发
            return;
        }
        if (m_publishPosted) {
            return;
        }
        m_publishPosted = true;
    }

    scheduleDeliver();
}

void FrameDispatcher::scheduleDeliver()
{
    QMetaObject::invokeMethod(this, &FrameDispatcher::deliverPendingFrame, Qt::QueuedConnection);
}

void FrameDispatcher::trySchedulePendingDeliver()
{
    bool schedule = false;
    {
        QMutexLocker lock(&m_publishMutex);
        if ((m_pendingPublish.algoInfo || !m_pendingPublish.displayImage.isNull())
            && !m_convertInFlight.load(std::memory_order_acquire)) {
            m_publishPosted = true;
            schedule          = true;
        }
    }
    if (schedule) {
        scheduleDeliver();
    }
}

void FrameDispatcher::deliverPendingFrame()
{
    if (m_convertInFlight.load(std::memory_order_acquire)) {
        return;
    }

    CameraFrame frame;
    {
        QMutexLocker lock(&m_publishMutex);
        frame            = std::move(m_pendingPublish);
        m_pendingPublish = CameraFrame();
        m_publishPosted  = false;
    }

    if (frame.displayImage.isNull() && !frame.algoInfo) {
        return;
    }

    if (!frame.displayImage.isNull()) {
        finishDeliver(std::move(frame));
        return;
    }

    startConvert(std::move(frame));
}

void FrameDispatcher::startConvert(CameraFrame frame)
{
    if (!frame.algoInfo) {
        return;
    }

    ConvertJob job;
    job.info       = frame.algoInfo;
    job.kindDetail = previewKindLabel(frame.kind) + QLatin1Char(' ') + frame.captureName;

    frame.algoInfo.reset();
    m_convertingFrame = std::move(frame);
    m_convertInFlight.store(true, std::memory_order_release);

    auto *watcher = new QFutureWatcher<QImage>(this);
    connect(watcher, &QFutureWatcher<QImage>::finished, this, [this, watcher]() {
        const QImage image = watcher->result();
        watcher->deleteLater();
        onConvertFinished(image);
    });
    watcher->setFuture(QtConcurrent::run(convertJobToQImage, job));
}

void FrameDispatcher::onConvertFinished(QImage image)
{
    m_convertInFlight.store(false, std::memory_order_release);

    // 转 QImage 完成后必须投递；若因 pending 非空丢弃本次结果，实时预览下几乎永远不上屏
    CameraFrame frame;
    {
        QMutexLocker lock(&m_publishMutex);
        frame              = std::move(m_convertingFrame);
        frame.displayImage = std::move(image);
        m_convertingFrame  = CameraFrame();
    }

    if (!frame.displayImage.isNull()) {
        finishDeliver(std::move(frame));
    } else {
        trySchedulePendingDeliver();
    }
}

void FrameDispatcher::finishDeliver(CameraFrame frame)
{
    const QString kindDetail =
        previewKindLabel(frame.kind) + QLatin1Char(' ') + frame.captureName;
    common::utility::TimingDebugScope deliverTiming(
        common::utility::TimingDebugKeys::CaptureTiming, "帧分发.投递预览帧",
        kindDetail.toStdString());

    const FrameHandler handler = m_frameHandler;
    if (handler && !frame.displayImage.isNull()) {
        common::utility::TimingDebugScope handlerTiming(
            common::utility::TimingDebugKeys::CaptureTiming, "帧分发.调用Handler",
            kindDetail.toStdString());
        handler(std::move(frame));
    }

    trySchedulePendingDeliver();
}

void FrameDispatcher::dispatchFromCapture(const std::string &captureName,
                                          const std::string &cameraName,
                                          CameraImageKind kind,
                                          const std::shared_ptr<common::device::ImageInfo> &info,
                                          const common::device::CameraInfo &cameraInfo)
{
    Q_UNUSED(cameraInfo);

    if (!info || !info->imageMat || AlgoMatIsEmpty(info->imageMat.get())) {
        return;
    }

    CameraFrame frame;
    frame.captureName = QString::fromStdString(captureName);
    frame.cameraName  = QString::fromStdString(cameraName);
    frame.kind        = kind;
    frame.frameIndex  = info->frameIndex;
    frame.algoInfo    = info;

    publishFrame(std::move(frame));
}

QImage FrameDispatcher::toDisplayImage(const std::shared_ptr<common::device::ImageInfo> &info)
{
    if (!info || !info->imageMat || AlgoMatIsEmpty(info->imageMat.get())) {
        return {};
    }

    const AlgoMat &mat = *info->imageMat;
    switch (info->imageFormat) {
    case common::imgproc::ImageFormat::RGB24:
        return rgbMatToQImage(mat);
    case common::imgproc::ImageFormat::MONO8:
    case common::imgproc::ImageFormat::GRAY8:
        return monoMatToQImage(mat);
    default:
        LOG_WARN("[FrameDispatcher] unsupported display format {} for UI conversion",
                 static_cast<int>(info->imageFormat));
        return {};
    }
}

} // namespace camera_plugin
