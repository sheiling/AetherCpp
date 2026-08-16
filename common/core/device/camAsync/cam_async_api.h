#pragma once

#include "cam_async_global.h"

#include <QString>

// 相机图像推送事件键（MessageCenter::Center::Notify / Attach / Detach）；参数：QVariantList[0] = 窗口ID（int），QVariantList[1] = 相机名称（QString），QVariantList[2] = 图像（QImage）
#define CAM_ASYNC_EVENT_CAMERA_IMG "event-camera-img"
// 相机单帧保存结果事件键；参数：QVariantList[0] = 文件名（QString），QVariantList[1] = 是否成功（bool）
#define CAM_ASYNC_EVENT_CAMERA_SAVE "event-camera-save"

namespace cam_async {

/** 按 device_config.json 创建各相机 Handler（会先调用 Exit） */
CAM_ASYNC_EXPORT bool Init();
/** 销毁所有 Handler 并注销采集回调 */
CAM_ASYNC_EXPORT bool Exit();

/** 采集单帧并在后台线程推送到指定预览窗口，提交失败返回 false */
CAM_ASYNC_EXPORT bool GrabOneImage(const QString &cameraName, int winId);
/** 采集单帧并在后台线程按文件名后缀（jpg/bmp/raw）保存，完成后通过 CAM_ASYNC_EVENT_CAMERA_SAVE 通知（fileName, success）；提交失败返回 false */
CAM_ASYNC_EXPORT bool GrabOneImageToFile(const QString &cameraName, const QString &fileName);
/** 启动相机异步采集（会先绑定 winId），失败返回 false */
CAM_ASYNC_EXPORT bool StartAsyncGrab(const QString &cameraName, int winId);
/** 停止相机采集，失败返回 false */
CAM_ASYNC_EXPORT bool StopAsyncGrab(QString &cameraName);
/** 更新所有相机 Handler 的预览窗口 ID */
CAM_ASYNC_EXPORT void SetWinId(int winId);

} // namespace cam_async
