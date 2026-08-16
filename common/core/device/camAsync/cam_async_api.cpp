#include "cam_async_api.h"

#include "camera_callback_manager.h"

namespace cam_async {

bool Init()
{
    return CameraCallbackManager::instance()->Init();
}

bool Exit()
{
    return CameraCallbackManager::instance()->Exit();
}

bool GrabOneImage(const QString &cameraName, int winId)
{
    return CameraCallbackManager::instance()->grabOneImage(cameraName, winId);
}

bool GrabOneImageToFile(const QString &cameraName, const QString &fileName)
{
    return CameraCallbackManager::instance()->grabOneImageToFile(cameraName, fileName);
}

bool StartAsyncGrab(const QString &cameraName, int winId)
{
    return CameraCallbackManager::instance()->startCameraAsyncGrab(cameraName, winId);
}

bool StopAsyncGrab(QString &cameraName)
{
    return CameraCallbackManager::instance()->stopCameraAsyncGrab(cameraName);
}

void SetWinId(int winId)
{
    CameraCallbackManager::instance()->setWinId(winId);
}

} // namespace cam_async
