#include "camera_service_impl.h"

#include "display_image_utils.h"

#include <common/utility/diagnostics/timing_debug.h>

#include <common/3rdparty/cisdk/measurement/include/algo_mat.h>

#include <QMetaObject>
#include <QThread>
#include <QTimer>

namespace camera_plugin {

namespace {

QString toQString(const std::string &s) { return QString::fromStdString(s); }

} // namespace

CameraServiceImpl::CameraServiceImpl(QObject *parent)
    : ICameraService(parent)
    , m_frameDispatcher(std::make_unique<FrameDispatcher>(this))
{
    wireFrameEmitter();
}

CameraServiceImpl::~CameraServiceImpl() { shutdown(); }

void CameraServiceImpl::wireFrameEmitter()
{
    m_frameDispatcher->setFrameHandler([this](CameraFrame frame) {
        // RawCapture 不应进入预览 Handler；仅 LivePreview / SingleShot 上抛 frameReady
        if (frame.kind != CameraImageKind::LivePreview && frame.kind != CameraImageKind::SingleShot) {
            return;
        }
        if (frame.displayImage.isNull()) {
            return;
        }
        const QString kindDetail =
            (frame.kind == CameraImageKind::LivePreview ? QStringLiteral("实时预览")
                                                        : QStringLiteral("单帧"))
            + QLatin1Char(' ') + frame.captureName;
        common::utility::TimingDebugScope emitTiming(
            common::utility::TimingDebugKeys::CaptureTiming, "相机服务.发送frameReady",
            kindDetail.toStdString());
        emit frameReady(std::move(frame.displayImage));
    });
}

bool CameraServiceImpl::refreshCaptures(QString *errorOut)
{
    std::string loadError;
    if (!m_captureManager.refreshFromDeviceManager(&loadError)) {
        if (errorOut) {
            *errorOut = QString::fromStdString(loadError);
        }
        return false;
    }

    wireCaptureCallbacks();
    m_initialized = true;
    return true;
}

void CameraServiceImpl::wireCaptureCallbacks()
{
    for (const auto &name : m_captureManager.captureNames()) {
        auto *capture = m_captureManager.capture(name);
        if (!capture) {
            continue;
        }
        // DispCB 在 ImageCapture debayer 完成后触发（execDispFuncCB=true 路径）
        capture->RegisterDispCB(
            [this, captureName = name, cameraName = capture->getCameraName()](
                std::string /*dispName*/, std::shared_ptr<common::device::ImageInfo> info) {
                CameraImageKind kind = CameraImageKind::LivePreview;
                {
                    QMutexLocker lock(&m_mutex);
                    const auto it = m_captureKindByName.find(captureName);
                    if (it != m_captureKindByName.end()) {
                        kind = it->second;
                    }
                }
                const auto *cap = m_captureManager.capture(captureName);
                if (!cap) {
                    return;
                }
                m_frameDispatcher->dispatchFromCapture(captureName, cameraName, kind, info,
                                                       cap->cameraInfo());
            });
    }
}

QStringList CameraServiceImpl::captureNames() const
{
    QStringList names;
    for (const auto &n : m_captureManager.captureNames()) {
        names.append(toQString(n));
    }
    return names;
}

QStringList CameraServiceImpl::captureAxisNames(const QString &captureName) const
{
    const auto *capture = m_captureManager.capture(captureName.toStdString());
    if (!capture) {
        return {};
    }
    QStringList names;
    for (const auto &axisName : capture->axisNames()) {
        names.append(toQString(axisName));
    }
    return names;
}

bool CameraServiceImpl::isCaptureReady(const QString &captureName) const
{
    const auto *capture = m_captureManager.capture(captureName.toStdString());
    return capture && capture->isReady();
}

void CameraServiceImpl::setExposure(const QString &captureName, int exposureUs)
{
    auto *capture = m_captureManager.capture(captureName.toStdString());
    if (!capture) {
        return;
    }
    capture->setExposureTime(exposureUs);
}

void CameraServiceImpl::cancelDeferredStop()
{
    m_deferredStopGeneration.fetch_add(1, std::memory_order_relaxed);
}

void CameraServiceImpl::setPreviewActiveState(bool active)
{
    if (QThread::currentThread() == thread()) {
        emit previewActiveChanged(active);
        return;
    }
    QMetaObject::invokeMethod(
        this, [this, active]() { emit previewActiveChanged(active); }, Qt::QueuedConnection);
}

bool CameraServiceImpl::snapOnce(const QString &captureName, int exposureUs)
{
    auto *capture = m_captureManager.capture(captureName.toStdString());
    if (!capture || !capture->isReady()) {
        emit captureError(QStringLiteral("capture not ready: %1").arg(captureName));
        return false;
    }

    cancelDeferredStop();

    QString activePreview;
    {
        QMutexLocker lock(&m_mutex);
        activePreview = m_activePreviewCaptureName;
    }
    // 单帧与预览互斥：同采集器先停连续采
    if (activePreview == captureName) {
        stopCaptureInternal(captureName, false);
        setPreviewActiveState(false);
    }

    if (exposureUs > 0) {
        capture->setExposureTime(exposureUs);
    }

    {
        QMutexLocker lock(&m_mutex);
        m_captureKindByName[captureName.toStdString()] = CameraImageKind::SingleShot;
    }
    capture->captureImage(true);
    return true;
}

void CameraServiceImpl::publishRoleImageFromInfo(
    const QString &captureName, const QString &imageRole,
    const std::shared_ptr<common::device::ImageInfo> &rgb, const QString &cameraName,
    int maxDisplayEdge)
{
    if (!rgb) {
        return;
    }

    common::utility::TimingDebugScope totalTiming(common::utility::TimingDebugKeys::CaptureTiming,
                                                  "相机服务.角色图像上抛",
                                                  captureName.toStdString() + " 角色="
                                                      + imageRole.toStdString());

    QImage fullImage;
    {
        common::utility::TimingDebugScope qimageTiming(
            common::utility::TimingDebugKeys::CaptureTiming, "相机服务.转QImage",
            captureName.toStdString());
        // 工步已 debayer；此处为插件内 ImageInfo → QImage 边界
        fullImage = FrameDispatcher::toDisplayImage(rgb);
    }
    if (fullImage.isNull()) {
        return;
    }
    publishRoleImage(captureName, imageRole, std::move(fullImage), cameraName, maxDisplayEdge);
}

void CameraServiceImpl::publishRoleImage(const QString &captureName,
                                         const QString &imageRole,
                                         QImage image,
                                         const QString &cameraName,
                                         int maxDisplayEdge)
{
    if (image.isNull() || imageRole.isEmpty()) {
        return;
    }

    common::utility::TimingDebugScope totalTiming(
        common::utility::TimingDebugKeys::CaptureTiming, "相机服务.角色图像上抛",
        captureName.toStdString() + " 角色=" + imageRole.toStdString());

    QString resolvedCamera = cameraName;
    if (resolvedCamera.isEmpty()) {
        if (const auto *capture = m_captureManager.capture(captureName.toStdString())) {
            resolvedCamera = QString::fromStdString(capture->getCameraName());
        }
    }

    QImage display;
    {
        common::utility::TimingDebugScope downscaleTiming(
            common::utility::TimingDebugKeys::CaptureTiming, "相机服务.预览降采样",
            imageRole.toStdString());
        display = downscaleForDisplay(image, maxDisplayEdge);
    }
    {
        common::utility::TimingDebugScope emitTiming(
            common::utility::TimingDebugKeys::CaptureTiming, "相机服务.发送roleImageReady",
            imageRole.toStdString());
        emit roleImageReady(captureName, resolvedCamera, imageRole, std::move(display));
    }
}

bool CameraServiceImpl::startPreview(const QString &captureName, int exposureUs)
{
    auto *capture = m_captureManager.capture(captureName.toStdString());
    if (!capture || !capture->isReady()) {
        emit captureError(QStringLiteral("capture not ready: %1").arg(captureName));
        return false;
    }

    cancelDeferredStop();
    m_frameDispatcher->clearPendingFrames();

    QString activePreview;
    {
        QMutexLocker lock(&m_mutex);
        activePreview = m_activePreviewCaptureName;
    }
    if (!activePreview.isEmpty() && activePreview != captureName) {
        stopCaptureInternal(activePreview, false);
    }

    if (exposureUs > 0) {
        capture->setExposureTime(exposureUs);
    }

    {
        QMutexLocker lock(&m_mutex);
        m_captureKindByName[captureName.toStdString()] = CameraImageKind::LivePreview;
        m_activePreviewCaptureName                     = captureName;
    }

    if (!capture->continuousCapture(true)) {
        QMutexLocker lock(&m_mutex);
        if (m_activePreviewCaptureName == captureName) {
            m_activePreviewCaptureName.clear();
        }
        m_captureKindByName.erase(captureName.toStdString());
        emit captureError(QStringLiteral("start preview failed: %1").arg(captureName));
        return false;
    }

    setPreviewActiveState(true);
    return true;
}

bool CameraServiceImpl::stopCaptureInternal(const QString &captureName, bool waitDrain)
{
    auto *capture = m_captureManager.capture(captureName.toStdString());
    if (!capture) {
        return false;
    }

    {
        QMutexLocker lock(&m_mutex);
        if (m_activePreviewCaptureName == captureName) {
            m_activePreviewCaptureName.clear();
        }
        m_captureKindByName.erase(captureName.toStdString());
    }

    return capture->stopCapture(waitDrain);
}

void CameraServiceImpl::stopPreview(bool deferHardware)
{
    QString captureName;
    {
        QMutexLocker lock(&m_mutex);
        captureName = m_activePreviewCaptureName;
    }
    if (captureName.isEmpty()) {
        return;
    }

    if (auto *capture = m_captureManager.capture(captureName.toStdString())) {
        capture->suppressFrameDispatch();
    }
    m_frameDispatcher->clearPendingFrames();

    if (deferHardware) {
        const quint64 generation =
            m_deferredStopGeneration.fetch_add(1, std::memory_order_relaxed) + 1;

        {
            QMutexLocker lock(&m_mutex);
            m_activePreviewCaptureName.clear();
            m_captureKindByName.erase(captureName.toStdString());
        }

        setPreviewActiveState(false);

        // 下一事件循环停硬件，避免页面切换阻塞 UI
        QTimer::singleShot(0, this, [this, captureName, generation]() {
            if (generation != m_deferredStopGeneration.load(std::memory_order_relaxed)) {
                return;
            }
            stopCaptureInternal(captureName, false);
        });
        return;
    }

    {
        QMutexLocker lock(&m_mutex);
        m_activePreviewCaptureName.clear();
        m_captureKindByName.erase(captureName.toStdString());
    }
    setPreviewActiveState(false);

    const QString name = captureName;
    QMetaObject::invokeMethod(
        this, [this, name]() { stopCaptureInternal(name, false); }, Qt::QueuedConnection);
}

void CameraServiceImpl::onDisplayDetached()
{
    stopPreview(true);
}

bool CameraServiceImpl::isPreviewActive() const
{
    QMutexLocker lock(&m_mutex);
    return !m_activePreviewCaptureName.isEmpty();
}

QString CameraServiceImpl::activeCaptureName() const
{
    QMutexLocker lock(&m_mutex);
    return m_activePreviewCaptureName;
}

void CameraServiceImpl::shutdown()
{
    QString activePreview;
    {
        QMutexLocker lock(&m_mutex);
        activePreview = m_activePreviewCaptureName;
        m_activePreviewCaptureName.clear();
        m_captureKindByName.clear();
        m_initialized = false;
    }

    m_frameDispatcher->clearPendingFrames();

    if (!activePreview.isEmpty()) {
        stopCaptureInternal(activePreview, false);
    }

    if (m_frameDispatcher) {
        m_frameDispatcher->setFrameHandler(nullptr);
    }
    m_captureManager.shutdown();
}

} // namespace camera_plugin
