/**
 * @file affine_transform_2d.cpp
 * @brief AffineTransform2D 与 PanelLogicalCorners 实现
 */

#include "affine_transform_2d.h"

#include <cmath>

namespace aoi::geometry {

Vec2d AffineTransform2D::transform(const Vec2d &logical) const
{
    return {m00_ * logical.x + m01_ * logical.y + tx_,
            m10_ * logical.x + m11_ * logical.y + ty_};
}

Vec2d AffineTransform2D::inverseTransform(const Vec2d &physical) const
{
    const double det = m00_ * m11_ - m01_ * m10_;
    if (std::abs(det) <= 1e-12)
        return {};

    const double invDet = 1.0 / det;
    const double px     = physical.x - tx_;
    const double py     = physical.y - ty_;
    return {(m11_ * px - m01_ * py) * invDet, (-m10_ * px + m00_ * py) * invDet};
}

AffineTransform2D AffineTransform2D::fromTwoPointSimilarity(const Vec2d &logicalA,
                                                            const Vec2d &logicalB,
                                                            const Vec2d &physicalA,
                                                            const Vec2d &physicalB)
{
    AffineTransform2D result;
    const Vec2d  vL   = logicalB - logicalA;
    const Vec2d  vP   = physicalB - physicalA;
    const double lenL = vL.length();
    const double lenP = vP.length();
    if (lenL <= 1e-9 || lenP <= 1e-9) {
        result.valid_     = false;
        result.lastError_ = "marker pair distance too small";
        return result;
    }

    // 相似变换：统一缩放 + 旋转角差
    const double scale = lenP / lenL;
    const double angle = std::atan2(vP.y, vP.x) - std::atan2(vL.y, vL.x);
    const double cosA  = std::cos(angle);
    const double sinA  = std::sin(angle);

    result.m00_ = scale * cosA;
    result.m01_ = -scale * sinA;
    result.m10_ = scale * sinA;
    result.m11_ = scale * cosA;
    // 平移使 logicalA 精确映射到 physicalA
    result.tx_  = physicalA.x - (scale * cosA * logicalA.x - scale * sinA * logicalA.y);
    result.ty_  = physicalA.y - (scale * sinA * logicalA.x + scale * cosA * logicalA.y);
    return result;
}

AffineTransform2D AffineTransform2D::fromPointPairsLeastSquares(
    const std::vector<Vec2d> &logicalPoints,
    const std::vector<Vec2d> &physicalPoints)
{
    AffineTransform2D result;
    if (logicalPoints.size() != physicalPoints.size() || logicalPoints.size() < 3) {
        result.valid_     = false;
        result.lastError_ = "need at least 3 point pairs";
        return result;
    }

    const size_t n    = logicalPoints.size();
    double       sx   = 0, sy = 0, spx = 0, spy = 0;
    for (size_t i = 0; i < n; ++i) {
        sx += logicalPoints[i].x;
        sy += logicalPoints[i].y;
        spx += physicalPoints[i].x;
        spy += physicalPoints[i].y;
    }
    const double invN   = 1.0 / static_cast<double>(n);
    const double meanLx = sx * invN;
    const double meanLy = sy * invN;
    const double meanPx = spx * invN;
    const double meanPy = spy * invN;

    // 去中心化后累加协方差项
    double sxx = 0, syy = 0, sxy = 0;
    double spxx = 0, spyy = 0, spxy = 0;
    for (size_t i = 0; i < n; ++i) {
        const double lx = logicalPoints[i].x - meanLx;
        const double ly = logicalPoints[i].y - meanLy;
        const double px = physicalPoints[i].x - meanPx;
        const double py = physicalPoints[i].y - meanPy;
        sxx += lx * lx;
        syy += ly * ly;
        sxy += lx * ly;
        spxx += lx * px;
        spyy += ly * py;
        spxy += lx * py;
    }

    const double det = sxx * syy - sxy * sxy;
    if (std::abs(det) <= 1e-12) {
        result.valid_     = false;
        result.lastError_ = "degenerate point distribution";
        return result;
    }

    result.m00_ = (spxx * syy - spxy * sxy) / det;
    result.m01_ = (spxy * sxx - spxx * sxy) / det;
    result.m10_ = (spyy * sxy - spxy * syy) / det;
    result.m11_ = (spyy * sxx - spxy * sxy) / det;
    result.tx_  = meanPx - (result.m00_ * meanLx + result.m01_ * meanLy);
    result.ty_  = meanPy - (result.m10_ * meanLx + result.m11_ * meanLy);
    return result;
}

PanelLogicalCorners PanelLogicalCorners::fromBottomLeftAndTopRight(const Vec2d &bl,
                                                                   const Vec2d &tr)
{
    PanelLogicalCorners corners;
    corners.bottomLeft  = bl;
    corners.topRight    = tr;
    corners.bottomRight = {tr.x, bl.y};
    corners.topLeft     = {bl.x, tr.y};
    return corners;
}

} // namespace aoi::geometry
