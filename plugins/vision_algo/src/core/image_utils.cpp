#include "image_utils.h"

#include <vision_algo_plugin_global.h>

#include <common/base/imgproc/image_type_map.h>

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>

namespace vision_algo {
namespace {

bool algoMatToCvRgb8(const ::AlgoMat *mat, cv::Mat &rgb8)
{
    if (!mat || AlgoMatIsEmpty(mat) || mat->type != ALGO_8UC3)
        return false;

    cv::Mat bgr(mat->rows, mat->cols, CV_8UC3, mat->data);
    cv::cvtColor(bgr, rgb8, cv::COLOR_BGR2RGB);
    return !rgb8.empty();
}

::AlgoMat rgbCvMatToOwnedAlgoMat(const cv::Mat &rgb8)
{
    if (rgb8.empty() || rgb8.type() != CV_8UC3)
        return {};

    cv::Mat continuous = rgb8.isContinuous() ? rgb8 : rgb8.clone();
    const unsigned int len =
        static_cast<unsigned int>(static_cast<size_t>(continuous.rows) * continuous.cols * 3);
    return AlgoMatCreateFromMem(continuous.rows, continuous.cols, ALGO_8UC3,
                                reinterpret_cast<const char *>(continuous.data), len);
}

} // namespace

QVector<char> packBgr888FromRgbMat(const ::AlgoMat *rgbMat)
{
    if (!rgbMat || AlgoMatIsEmpty(rgbMat) || rgbMat->type != ALGO_8UC3)
        return {};

    cv::Mat rgb(rgbMat->rows, rgbMat->cols, CV_8UC3, rgbMat->data);
    cv::Mat bgr;
    cv::cvtColor(rgb, bgr, cv::COLOR_RGB2BGR);

    cv::Mat continuous = bgr.isContinuous() ? bgr : bgr.clone();
    const int len      = continuous.cols * continuous.rows * 3;
    if (len <= 0)
        return {};

    QVector<char> buffer(len);
    std::memcpy(buffer.data(), continuous.data, static_cast<size_t>(len));
    return buffer;
}

std::shared_ptr<common::device::ImageInfo> cloneAlgoMatToRgbImageInfo(const ::AlgoMat *mat)
{
    if (!mat || AlgoMatIsEmpty(mat))
        return nullptr;

    const int w = mat->cols;
    const int h = mat->rows;
    if (w <= 0 || h <= 0)
        return nullptr;

    if (mat->type == ALGO_8UC1) {
        const size_t len = static_cast<size_t>(w) * h;
        ::AlgoMat owned =
            AlgoMatCreateFromMem(h, w, ALGO_8UC1, reinterpret_cast<const char *>(mat->data), len);
        if (AlgoMatIsEmpty(&owned))
            return nullptr;
        auto out         = std::make_shared<common::device::ImageInfo>();
        out->imageMat    = common::device::adoptAlgoMat(std::move(owned));
        out->imageFormat = common::imgproc::ImageFormat::MONO8;
        out->bayerType   = COLOR_BayerNone;
        return out;
    }

    if (mat->type != ALGO_8UC3)
        return nullptr;

    cv::Mat rgb8;
    if (!algoMatToCvRgb8(mat, rgb8))
        return nullptr;

    ::AlgoMat rgbOwned = rgbCvMatToOwnedAlgoMat(rgb8);
    if (AlgoMatIsEmpty(&rgbOwned))
        return nullptr;

    auto out         = std::make_shared<common::device::ImageInfo>();
    out->imageMat    = common::device::adoptAlgoMat(std::move(rgbOwned));
    out->imageFormat = common::imgproc::ImageFormat::RGB24;
    out->bayerType   = COLOR_BayerNone;
    return out;
}

bool isValidRgbImageInfo(const std::shared_ptr<common::device::ImageInfo> &info)
{
    return info && info->imageMat && !AlgoMatIsEmpty(info->imageMat.get())
           && info->imageFormat == common::imgproc::ImageFormat::RGB24;
}

QString normalizeModelPath(QString path)
{
    path = path.trimmed();
    if (path.isEmpty())
        return path;
    if (!path.endsWith(QStringLiteral(".model"), Qt::CaseInsensitive))
        path += QStringLiteral(".model");
    return path;
}

QString defaultModelDir()
{
    return QCoreApplication::applicationDirPath() + QStringLiteral("/model");
}

QString resolveModelFilePath(const QString &templateFile)
{
    const QString trimmed = templateFile.trimmed();
    if (trimmed.isEmpty())
        return {};

    QFileInfo info(trimmed);
    if (info.isAbsolute())
        return normalizeModelPath(info.absoluteFilePath());

    QString fileName = info.fileName();
    if (fileName.endsWith(QStringLiteral(".tpl"), Qt::CaseInsensitive)) {
        fileName = QFileInfo(fileName).completeBaseName() + QStringLiteral(".model");
    } else {
        fileName = normalizeModelPath(fileName);
    }
    return QDir(defaultModelDir()).absoluteFilePath(fileName);
}

VISION_ALGO_EXPORT std::string resolveModelFilePath(const std::string &templateFile)
{
    return resolveModelFilePath(QString::fromStdString(templateFile)).toStdString();
}

QByteArray toNativePathBytes(const QString &path)
{
    return QDir::toNativeSeparators(QFileInfo(path).absoluteFilePath()).toLocal8Bit();
}

QByteArray toLocalNameBytes(const QString &text)
{
    return text.trimmed().toLocal8Bit();
}

std::shared_ptr<common::device::ImageInfo> ensureRgbProcessImage(
    const std::shared_ptr<common::device::ImageInfo> &processImage,
    common::device::ImageCapture                     *imageCapture)
{
    if (!processImage || !processImage->imageMat || AlgoMatIsEmpty(processImage->imageMat.get())) {
        return nullptr;
    }

    if (isValidRgbImageInfo(processImage)) {
        return processImage;
    }

    if (!imageCapture) {
        return nullptr;
    }

    if (!imageCapture->needsRgbConversion(*processImage, imageCapture->cameraInfo())) {
        return processImage;
    }

    return imageCapture->makeRgbForProcess(processImage);
}

QVector<char> packBgrFromRgbImageInfo(const std::shared_ptr<common::device::ImageInfo> &rgbImage)
{
    if (!isValidRgbImageInfo(rgbImage)) {
        return {};
    }
    return packBgr888FromRgbMat(rgbImage->imageMat.get());
}

} // namespace vision_algo
