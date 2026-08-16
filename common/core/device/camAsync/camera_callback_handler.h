#pragma once

#include "cam_async_global.h"

#include <common/base/imgproc/image_type_map.h>

#include <QImage>
#include <QObject>
#include <QString>

#include <memory>

namespace common::device {
class CameraBase;
}

namespace cam_async {

/** 将相机原始帧缓冲转为 QImage（支持 MONO/RGB/BGR 及 Bayer 格式） */
CAM_ASYNC_EXPORT QImage ConvertCameraBufferToQImage(int height,
                                   int width,
                                   const unsigned char *dataBuffer,
                                   int dataLen,
                                   common::imgproc::ImageFormat imageFormat);

/**
 * @brief 单路相机的采集回调：注册 GrabCB，将原始帧转为 QImage 并通过 MessageCenter 分发。
 */
class CameraCallbackHandler : public QObject
{
public:
    explicit CameraCallbackHandler(const QString &cameraName, QObject *parent = nullptr);
    ~CameraCallbackHandler() override;

    void SetWinId(int winId);

private:
    QString m_cameraName;
    int m_currentId = -1;
    std::shared_ptr<common::device::CameraBase> m_camera;
};

} // namespace cam_async
