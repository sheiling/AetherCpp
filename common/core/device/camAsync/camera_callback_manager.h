#pragma once

#include <QMap>
#include <QObject>
#include <QString>

namespace cam_async {

class CameraCallbackHandler;

/**
 * @brief 管理所有 CameraCallbackHandler 实例（单例）。
 */
class CameraCallbackManager : public QObject
{
    Q_OBJECT

public:
    static CameraCallbackManager *instance();

    /** 按 device_config.json 创建各相机 Handler（会先调用 Exit） */
    bool Init();
    /** 销毁所有 Handler 并注销采集回调 */
    bool Exit();

    /** 采集单帧并在后台线程推送到指定预览窗口，提交失败返回 false */
    bool grabOneImage(const QString &cameraName, int winId);
    /** 采集单帧并在后台线程保存到文件，完成后通知 CAM_ASYNC_EVENT_CAMERA_SAVE（fileName, success）；提交失败返回 false */
    bool grabOneImageToFile(const QString &cameraName, const QString &fileName);
    /** 启动相机异步采集（会先绑定 winId），失败返回 false */
    bool startCameraAsyncGrab(const QString &cameraName, int winId);
    /** 停止相机采集，失败返回 false */
    bool stopCameraAsyncGrab(const QString &cameraName);
    /** 更新所有 Handler 的预览窗口 ID */
    void setWinId(int winId);

private:
    explicit CameraCallbackManager(QObject *parent = nullptr);
    Q_DISABLE_COPY(CameraCallbackManager)

    QMap<QString, CameraCallbackHandler *> m_handlers;
};

} // namespace cam_async
