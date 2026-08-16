#pragma once

/**
 * @file affine_transform_2d.h
 * @brief 灯板逻辑坐标 (mm) 与机台物理坐标 (mm) 之间的二维仿射变换
 *
 * 矩阵形式：p_phys = M * p_log + t，其中 M 为 2×2，t 为平移 (tx_, ty_)。
 * FOV 规划模块通过双 Mark 点相似变换建立逻辑↔物理映射。
 */

#include <common/include/geometry.h>

#include <optional>
#include <string>
#include <vector>

namespace aoi::geometry {

using Vec2d  = common::geometry::Vec2d;
using Rect2d = common::geometry::Rect2d;

/**
 * @brief 二维仿射变换（逻辑 → 物理）
 */
class AffineTransform2D
{
public:
    AffineTransform2D() = default;

    /** 逻辑坐标 → 物理坐标 */
    Vec2d transform(const Vec2d &logical) const;
    /** 物理坐标 → 逻辑坐标；行列式接近 0 时返回 (0,0) */
    Vec2d inverseTransform(const Vec2d &physical) const;

    bool isValid() const { return valid_; }
    const std::string &lastError() const { return lastError_; }

    /**
     * @brief 由一对 Mark 点建立相似变换（等比缩放 + 旋转 + 平移）
     *
     * 逻辑对 (logicalA, logicalB) 映射到物理对 (physicalA, physicalB)。
     * 两点距离过近时 valid_=false。
     */
    static AffineTransform2D fromTwoPointSimilarity(const Vec2d &logicalA,
                                                    const Vec2d &logicalB,
                                                    const Vec2d &physicalA,
                                                    const Vec2d &physicalB);

    /**
     * @brief 多对对应点最小二乘仿射（至少 3 对）
     *
     * 用于 Mark 多于 2 个时的过定标定；点集退化时 valid_=false。
     */
    static AffineTransform2D fromPointPairsLeastSquares(
        const std::vector<Vec2d> &logicalPoints,
        const std::vector<Vec2d> &physicalPoints);

private:
    double m00_ = 1.0; ///< 仿射矩阵 [0,0]
    double m01_ = 0.0; ///< 仿射矩阵 [0,1]
    double m10_ = 0.0; ///< 仿射矩阵 [1,0]
    double m11_ = 1.0; ///< 仿射矩阵 [1,1]
    double tx_  = 0.0; ///< X 平移 (mm)
    double ty_  = 0.0; ///< Y 平移 (mm)

    bool        valid_     = true;
    std::string lastError_;
};

/** 逻辑灯板四角（由 BL/TR Mark 推算矩形四角） */
struct PanelLogicalCorners
{
    Vec2d bottomLeft;
    Vec2d bottomRight;
    Vec2d topLeft;
    Vec2d topRight;

    /** 由左下、右上 Mark 构造轴对齐矩形四角 */
    static PanelLogicalCorners fromBottomLeftAndTopRight(const Vec2d &bl, const Vec2d &tr);
};

} // namespace aoi::geometry
