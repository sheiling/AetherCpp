#include "camera_callback_manager.h"

#include "cam_async_api.h"
#include "cam_async_config.h"
#include "camera_callback_handler.h"

#include <common/core/device/camera/camera_base.h>
#include <common/core/device/deviceManager/device_manager.h>
#include <messagecenter/message_center.h>

#include <QDebug>
#include <QFile>
#include <QFileInfo>

#include <thread>
#include <vector>

namespace cam_async {

namespace {

enum class SaveImageFormat { Jpg, Bmp, Raw, Unknown };

SaveImageFormat saveFormatFromFileName(const QString &fileName)
{
    const QString ext = QFileInfo(fileName).suffix().toLower();
    if (ext == QLatin1String("jpg") || ext == QLatin1String("jpeg")) {
        return SaveImageFormat::Jpg;
    }
    if (ext == QLatin1String("bmp")) {
        return SaveImageFormat::Bmp;
    }
    if (ext == QLatin1String("raw")) {
        return SaveImageFormat::Raw;
    }
    return SaveImageFormat::Unknown;
}

struct SyncGrabFrame {
    int height = 0;
    int width = 0;
    std::vector<unsigned char> buffer;
    common::imgproc::ImageFormat imageFormat = common::imgproc::ImageFormat::UNKNOWN;
};

std::shared_ptr<common::device::CameraBase> cameraForName(const QString &cameraName)
{
    const auto dm = tryGetDeviceManager();
    if (!dm) {
        qWarning() << "[CamAsync::CameraCallbackManager] DeviceManager unavailable, camera:"
                   << cameraName;
        return nullptr;
    }

    const auto camera = dm->CameraHandler(cameraName.toStdString());
    if (!camera) {
        qWarning() << "[CamAsync::CameraCallbackManager] camera not found:" << cameraName;
    }
    return camera;
}

bool syncGrabOneFrame(const QString &cameraName, SyncGrabFrame &out)
{
    const auto camera = cameraForName(cameraName);
    if (!camera) {
        return false;
    }

    int height = 0;
    int width = 0;
    unsigned char *dataBuffer = nullptr;
    int dataLen = 0;
    if (const auto err = camera->SyncGrab(height, width, dataBuffer, dataLen)) {
        qWarning() << "[CamAsync::CameraCallbackManager] SyncGrab failed:" << cameraName
                   << QString::fromStdString(err->Message());
        return false;
    }
    if (!dataBuffer || dataLen <= 0 || height <= 0 || width <= 0) {
        qWarning() << "[CamAsync::CameraCallbackManager] SyncGrab returned empty buffer:"
                   << cameraName;
        return false;
    }

    out.height = height;
    out.width = width;
    out.buffer.assign(dataBuffer, dataBuffer + dataLen);
    out.imageFormat = camera->GetCameraInfo().imageFormat;
    return true;
}

QImage frameToQImage(const SyncGrabFrame &frame)
{
    return ConvertCameraBufferToQImage(
        frame.height, frame.width, frame.buffer.data(), static_cast<int>(frame.buffer.size()),
        frame.imageFormat);
}

bool saveFrameToFile(const SyncGrabFrame &frame, const QString &filePath, SaveImageFormat format)
{
    if (format == SaveImageFormat::Raw) {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "[CamAsync::CameraCallbackManager] failed to open raw file:" << filePath;
            return false;
        }
        const qint64 written =
            file.write(reinterpret_cast<const char *>(frame.buffer.data()),
                       static_cast<qint64>(frame.buffer.size()));
        return written == static_cast<qint64>(frame.buffer.size());
    }

    const QImage image = frameToQImage(frame);
    if (image.isNull()) {
        qWarning() << "[CamAsync::CameraCallbackManager] failed to convert frame for save:"
                   << filePath;
        return false;
    }

    const char *const fmt = (format == SaveImageFormat::Bmp) ? "BMP" : "JPG";
    if (!image.save(filePath, fmt)) {
        qWarning() << "[CamAsync::CameraCallbackManager] failed to save image:" << filePath;
        return false;
    }
    return true;
}

void grabOneImageWorker(const QString &cameraName, int winId)
{
    QImage image;
    SyncGrabFrame frame;
    if (syncGrabOneFrame(cameraName, frame)) {
        image = frameToQImage(frame);
        if (image.isNull()) {
            qWarning() << "[CamAsync::CameraCallbackManager] failed to convert SyncGrab buffer:"
                       << cameraName;
        }
    }

    MessageCenter::Center::Notify(CAM_ASYNC_EVENT_CAMERA_IMG, winId, cameraName, image);
}

void grabOneImageToFileWorker(const QString &cameraName,
                              const QString &fileName,
                              SaveImageFormat format)
{
    SyncGrabFrame frame;
    const bool success =
        syncGrabOneFrame(cameraName, frame) && saveFrameToFile(frame, fileName, format);
    MessageCenter::Center::Notify(CAM_ASYNC_EVENT_CAMERA_SAVE, fileName, success);
}

} // namespace

CameraCallbackManager *CameraCallbackManager::instance()
{
    static CameraCallbackManager manager;
    return &manager;
}

CameraCallbackManager::CameraCallbackManager(QObject *parent)
    : QObject(parent)
{
}

bool CameraCallbackManager::Exit()
{
    for (auto it = m_handlers.begin(); it != m_handlers.end(); ++it) {
        delete it.value();
    }
    m_handlers.clear();
    qDebug() << "[CamAsync::CameraCallbackManager] exited.";
    return true;
}

bool CameraCallbackManager::Init()
{
    Exit();

    const std::vector<std::string> cameraKeys = DeviceConfigReader::loadCameraKeys();
    if (cameraKeys.empty()) {
        qWarning() << "[CamAsync::CameraCallbackManager] No cameras in device_config.json.";
        return false;
    }

    if (!tryGetDeviceManager()) {
        qWarning() << "[CamAsync::CameraCallbackManager] DeviceManager not registered. "
                      "Enable bootstrap deviceManager or register DeviceManager in IoC.";
        return false;
    }

    for (const std::string &key : cameraKeys) {
        const QString cameraName = QString::fromStdString(key);
        auto *handler = new CameraCallbackHandler(cameraName, this);
        m_handlers.insert(cameraName, handler);
    }

    qDebug() << "[CamAsync::CameraCallbackManager] initialized handlers:" << m_handlers.size();
    return true;
}

bool CameraCallbackManager::grabOneImage(const QString &cameraName, int winId)
{
    std::thread(grabOneImageWorker, cameraName, winId).detach();
    return true;
}

bool CameraCallbackManager::grabOneImageToFile(const QString &cameraName, const QString &fileName)
{
    if (fileName.isEmpty()) {
        qWarning() << "[CamAsync::CameraCallbackManager] empty file name:" << cameraName;
        return false;
    }
    const SaveImageFormat format = saveFormatFromFileName(fileName);
    if (format == SaveImageFormat::Unknown) {
        qWarning() << "[CamAsync::CameraCallbackManager] unsupported save file type:" << fileName;
        return false;
    }

    std::thread(grabOneImageToFileWorker, cameraName, fileName, format).detach();
    return true;
}

bool CameraCallbackManager::startCameraAsyncGrab(const QString &cameraName, int winId)
{
    CameraCallbackHandler *handler = m_handlers.value(cameraName, nullptr);
    if (!handler) {
        qWarning() << "[CamAsync::CameraCallbackManager] unknown camera:" << cameraName;
        return false;
    }
    handler->SetWinId(winId);

    const auto camera = cameraForName(cameraName);
    if (!camera) {
        return false;
    }

    if (const auto err = camera->AsyncGrab()) {
        qWarning() << "[CamAsync::CameraCallbackManager] AsyncGrab failed:" << cameraName
                   << QString::fromStdString(err->Message());
        return false;
    }

    return true;
}

bool CameraCallbackManager::stopCameraAsyncGrab(const QString &cameraName)
{
    const auto camera = cameraForName(cameraName);
    if (!camera) {
        return false;
    }

    if (const auto err = camera->StopGrab()) {
        qWarning() << "[CamAsync::CameraCallbackManager] StopGrab failed:" << cameraName
                   << QString::fromStdString(err->Message());
        return false;
    }

    return true;
}

void CameraCallbackManager::setWinId(int winId)
{
    for (auto it = m_handlers.begin(); it != m_handlers.end(); ++it) {
        it.value()->SetWinId(winId);
    }
}

} // namespace cam_async
