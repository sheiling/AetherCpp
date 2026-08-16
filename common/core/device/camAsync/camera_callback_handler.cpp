#include "camera_callback_handler.h"

#include "cam_async_api.h"
#include "cam_async_config.h"

#include <common/base/algorithm/algorithm_packet.h>
#include <common/base/imgproc/image_type_map.h>
#include <common/base/imgproc/imgproc.h>
#include <common/core/device/camera/camera_base.h>
#include <common/core/device/deviceManager/device_manager.h>
#include <messagecenter/message_center.h>

#include <QDebug>

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cstring>
#include <vector>

namespace cam_async {

namespace {

using common::imgproc::ImageFormat;
using common::imgproc::ImageFormatToPixelDepth;
using common::imgproc::IsBayerFormat;
using common::imgproc::PixelDepth;

std::shared_ptr<common::algorithm::ImageBuffer> buildBayerImageBuffer(
    int width,
    int height,
    const unsigned char *dataBuffer,
    int dataLen,
    int elemSize)
{
    const int minRowBytes = width * elemSize;
    if (!dataBuffer || width <= 0 || height <= 0 || dataLen < minRowBytes * height)
        return nullptr;

    const int rowBytes = (dataLen % height == 0) ? (dataLen / height) : minRowBytes;
    if (rowBytes < minRowBytes)
        return nullptr;

    auto src = common::algorithm::ImageBuffer::Create(width, height, 1, elemSize, rowBytes);
    if (rowBytes == minRowBytes) {
        std::memcpy(src->data.get(), dataBuffer, src->ByteSize());
        return src;
    }

    for (int y = 0; y < height; ++y) {
        std::memcpy(src->data.get() + static_cast<size_t>(y) * static_cast<size_t>(rowBytes),
                    dataBuffer + static_cast<size_t>(y) * static_cast<size_t>(rowBytes),
                    static_cast<size_t>(minRowBytes));
    }
    return src;
}

QImage matBgrToQImage(const cv::Mat &bgr)
{
    if (bgr.empty())
        return QImage();

    cv::Mat rgb;
    if (bgr.type() == CV_16UC3) {
        cv::Mat bgr8;
        if (auto err = common::imgproc::ConvertMat12To8(bgr, bgr8)) {
            qWarning() << "[CamAsync::CameraCallbackHandler] ConvertMat12To8 failed:"
                       << QString::fromStdString(err->Message());
            return QImage();
        }
        cv::cvtColor(bgr8, rgb, cv::COLOR_BGR2RGB);
    } else if (bgr.type() == CV_16UC1) {
        cv::Mat gray8;
        if (auto err = common::imgproc::ConvertMat12To8(bgr, gray8)) {
            qWarning() << "[CamAsync::CameraCallbackHandler] ConvertMat12To8 failed:"
                       << QString::fromStdString(err->Message());
            return QImage();
        }
        return QImage(gray8.data, gray8.cols, gray8.rows, static_cast<int>(gray8.step),
                      QImage::Format_Grayscale8)
            .copy();
    } else if (bgr.type() == CV_8UC3) {
        cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB);
    } else if (bgr.type() == CV_8UC1) {
        return QImage(bgr.data, bgr.cols, bgr.rows, static_cast<int>(bgr.step),
                      QImage::Format_Grayscale8)
            .copy();
    } else {
        return QImage();
    }

    return QImage(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step),
                  QImage::Format_RGB888)
        .copy();
}

QImage bufferToQImage(int height,
                      int width,
                      const unsigned char *dataBuffer,
                      int dataLen,
                      ImageFormat imageFormat)
{
    if (!dataBuffer || dataLen <= 0 || height <= 0 || width <= 0)
        return QImage();

    switch (imageFormat) {
    case ImageFormat::MONO8:
    case ImageFormat::GRAY8: {
        QImage image(width, height, QImage::Format_Grayscale8);
        const int rowBytes = width;
        for (int y = 0; y < height; ++y) {
            const int offset = y * rowBytes;
            if (offset + rowBytes > dataLen)
                break;
            std::memcpy(image.scanLine(y), dataBuffer + offset, static_cast<size_t>(rowBytes));
        }
        return image;
    }
    case ImageFormat::RGB24: {
        QImage image(width, height, QImage::Format_RGB888);
        const int rowBytes = width * 3;
        for (int y = 0; y < height; ++y) {
            const int offset = y * rowBytes;
            if (offset + rowBytes > dataLen)
                break;
            std::memcpy(image.scanLine(y), dataBuffer + offset, static_cast<size_t>(rowBytes));
        }
        return image;
    }
    case ImageFormat::BGR24: {
        QImage image(width, height, QImage::Format_RGB888);
        const int rowBytes = width * 3;
        for (int y = 0; y < height; ++y) {
            const int offset = y * rowBytes;
            if (offset + rowBytes > dataLen)
                break;
            const unsigned char *src = dataBuffer + offset;
            unsigned char *dst = image.scanLine(y);
            for (int x = 0; x < width; ++x) {
                dst[x * 3 + 0] = src[x * 3 + 2];
                dst[x * 3 + 1] = src[x * 3 + 1];
                dst[x * 3 + 2] = src[x * 3 + 0];
            }
        }
        return image;
    }
    default:
        break;
    }

    if (!IsBayerFormat(imageFormat))
        return QImage();

    const int elemSize = (ImageFormatToPixelDepth(imageFormat) == PixelDepth::UINT16) ? 2 : 1;
    auto src = buildBayerImageBuffer(width, height, dataBuffer, dataLen, elemSize);
    if (!src)
        return QImage();

    cv::Mat bgr;
    if (auto err = common::imgproc::Debayer(src, imageFormat, bgr)) {
        qWarning() << "[CamAsync::CameraCallbackHandler] Debayer failed:"
                   << QString::fromStdString(err->Message());
        return QImage();
    }
    QImage image = matBgrToQImage(bgr);
    image = image.rgbSwapped();
    return image.copy();
}

} // namespace

QImage ConvertCameraBufferToQImage(int height,
                                   int width,
                                   const unsigned char *dataBuffer,
                                   int dataLen,
                                   common::imgproc::ImageFormat imageFormat)
{
    return bufferToQImage(height, width, dataBuffer, dataLen, imageFormat);
}

CameraCallbackHandler::~CameraCallbackHandler()
{
    if (m_camera) {
        m_camera->RegisterGrabCB(nullptr);
    }
}

CameraCallbackHandler::CameraCallbackHandler(const QString &cameraName, QObject *parent)
    : QObject(parent)
    , m_cameraName(cameraName)
{
    const auto dm = tryGetDeviceManager();
    if (!dm) {
        qWarning() << "[CamAsync::CameraCallbackHandler] DeviceManager unavailable, camera:"
                   << cameraName;
        return;
    }

    m_camera = dm->CameraHandler(cameraName.toStdString());
    if (!m_camera) {
        qWarning() << "[CamAsync::CameraCallbackHandler] camera not found:" << cameraName;
        return;
    }

    m_camera->RegisterGrabCB(
        [this](int height, int width, unsigned char *dataBuffer, int dataLen,
               common::imgproc::ImageFormat imageFormat) {
            if (!dataBuffer || dataLen <= 0 || height <= 0 || width <= 0)
                return;

            std::vector<unsigned char> bufferCopy(static_cast<size_t>(dataLen));
            std::memcpy(bufferCopy.data(), dataBuffer, bufferCopy.size());

            const QImage image = ConvertCameraBufferToQImage(
                height, width, bufferCopy.data(), dataLen, imageFormat);
            if (image.isNull())
                return;

            MessageCenter::Center::Notify(
                CAM_ASYNC_EVENT_CAMERA_IMG, m_currentId, m_cameraName, image);
        });
}

void CameraCallbackHandler::SetWinId(int winId)
{
    m_currentId = winId;
}

} // namespace cam_async
