/**
 * @file defect_review_image_crop.cpp
 * @brief 复判区：分区图 + ROI 裁切实现
 */

#include "defect_review_image_crop.h"

#include <QImageReader>
#include <QLoggingCategory>
#include <QRect>

namespace aoi::review {
namespace {

Q_LOGGING_CATEGORY(lcDefectReviewImage, "aoi.review.image")

bool isValidRoi(const aoi::Rect &roi)
{
    return roi.w > 0 && roi.h > 0;
}

} // namespace

DefectChipImageSource resolveChipImageSource(const inspection::DefectInfoList &workingCopy,
                                             int partitionIndex, int itemIndex)
{
    DefectChipImageSource source;
    if (partitionIndex < 0 || partitionIndex >= workingCopy.defectPartitions.size())
        return source;
    const inspection::DefectInfoPartition &part = workingCopy.defectPartitions[partitionIndex];
    if (itemIndex < 0 || itemIndex >= part.defectItems.size())
        return source;

    source.originPartitionPath = part.partitionImageOriginName.trimmed();
    source.detectPartitionPath = part.partitionImageDetectName.trimmed();
    source.roi                 = part.defectItems[itemIndex].roi;
    return source;
}

QImage cropPartitionImage(const QString &path, const aoi::Rect &roi)
{
    const QString imagePath = path.trimmed();
    if (imagePath.isEmpty()) {
        qWarning(lcDefectReviewImage) << "partition image path is empty";
        return {};
    }
    if (!isValidRoi(roi)) {
        qWarning(lcDefectReviewImage) << "invalid roi for" << imagePath << "x=" << roi.x << "y=" << roi.y
                                      << "w=" << roi.w << "h=" << roi.h;
        return {};
    }

    QImageReader reader(imagePath);
    if (!reader.canRead()) {
        qWarning(lcDefectReviewImage) << "cannot read partition image" << imagePath << reader.errorString();
        return {};
    }

    const QSize imageSize = reader.size();
    if (!imageSize.isValid()) {
        qWarning(lcDefectReviewImage) << "invalid partition image size" << imagePath;
        return {};
    }

    const QRect imageRect(0, 0, imageSize.width(), imageSize.height());
    const QRect clipRect(roi.x, roi.y, roi.w, roi.h);
    if (!imageRect.contains(clipRect)) {
        qWarning(lcDefectReviewImage) << "roi out of partition image bounds" << imagePath << clipRect
                                      << "image" << imageSize;
        return {};
    }

    reader.setClipRect(clipRect);
    QImage clipped = reader.read();
    if (clipped.isNull()) {
        qWarning(lcDefectReviewImage) << "failed to crop partition image" << imagePath << clipRect
                                        << reader.errorString();
        return {};
    }
    return clipped;
}

QImage cropDetectPartitionImage(const QString &path, const aoi::Rect &roi)
{
    if (path.trimmed().isEmpty()) {
        qWarning(lcDefectReviewImage) << "detect partition image path is empty";
        return {};
    }
    return cropPartitionImage(path, roi);
}

} // namespace aoi::review
