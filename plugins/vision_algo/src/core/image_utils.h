#pragma once

/**
 * @file image_utils.h
 * @brief VisionAlgo 插件内部图像与模板路径工具（不对外暴露）
 *
 * - RGB 校验、CISDK ::AlgoMat 克隆、RGB↔BGR 打包
 * - debayer 委派 ImageCapture
 * - 模板 .model 路径规范化
 */

#include <algo_mat.h>

#include <common/core/device/imageCapture/image_capture.h>

#include <QByteArray>
#include <QString>
#include <QVector>

#include <memory>

namespace vision_algo {

QString normalizeModelPath(QString path);
QString defaultModelDir();
QString resolveModelFilePath(const QString &templateFile);
QByteArray toNativePathBytes(const QString &path);
QByteArray toLocalNameBytes(const QString &text);

QVector<char> packBgr888FromRgbMat(const ::AlgoMat *rgbMat);

std::shared_ptr<common::device::ImageInfo> cloneAlgoMatToRgbImageInfo(const ::AlgoMat *mat);

bool isValidRgbImageInfo(const std::shared_ptr<common::device::ImageInfo> &info);

/**
 * @brief 若 processImage 需 RGB 转换且 capture 非空，调用 makeRgbForProcess
 * @return 可直接用于 ZXing / CISDK 的 RGB24 ImageInfo；失败返回 nullptr
 */
std::shared_ptr<common::device::ImageInfo> ensureRgbProcessImage(
    const std::shared_ptr<common::device::ImageInfo> &processImage,
    common::device::ImageCapture                     *imageCapture);

/**
 * @brief 从 RGB24 ImageInfo 打包 CISDK 检测所需的 BGR 交错缓冲
 * @note 输入须已通过 ensureRgbProcessImage / isValidRgbImageInfo 校验
 */
QVector<char> packBgrFromRgbImageInfo(const std::shared_ptr<common::device::ImageInfo> &rgbImage);

} // namespace vision_algo
