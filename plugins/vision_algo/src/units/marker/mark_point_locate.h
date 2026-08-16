#pragma once

/**
 * @file mark_point_locate.h
 * @brief Mark 点定位算法单元（CISDK marker_detector）
 */

#include <vision_algo/units/marker/mark_point_locate_data.h>

#include <common/base/algorithm/algorithm_base.h>
#include <common/utility/errors/include/error.h>

#include <QString>
#include <QVector>

namespace vision_algo {

class MarkPointLocate final : public common::algorithm::AlgorithmBase
{
public:
    static bool sdkReady();
    static QString sdkLoadDetail();

    std::string Name() const override;
    common::errors::ErrorPtr Execute(common::algorithm::AlgorithmExecuteData &data) override;

private:
    common::errors::ErrorPtr markDetect(const QVector<char> &bgrBuffer, uint64_t imageWidth,
                                        uint64_t imageHeight);

    MarkerDetectInfo m_markerInfo;
    float            point_x_pixel = 0.0f;
    float            point_y_pixel = 0.0f;
    std::string      m_imagePath;
    bool             m_saveImageFlag = false;
};

} // namespace vision_algo
