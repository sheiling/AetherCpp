#pragma once

/**
 * @file recipe_step_debug_string_light.h
 * @brief 产品配方单步调试「串灯检测」：参数组装、采图/读图、算法执行。
 */

#include "recipe_step_debug_common.h"

#include <common/utility/errors/include/error.h>

namespace lighten {

/** 单步调试串灯检测请求（由 UI 在主线程采集上下文后传入）。 */
struct RecipeStepDebugStringLightRequest : RecipeStepDebugImageContext
{
    ColorChannel                     colorChannel = ColorChannel::Red;
    QString                          leftObliqueRawPath;
    QString                          rightObliqueRawPath;
    RecipeStepDebugSoftDisplayParams softDisplay;
};

struct RecipeStepDebugStringLightResult
{
    common::errors::ErrorPtr err;
    QString                  outputDir;
};

RecipeStepDebugStringLightResult runRecipeStepDebugStringLight(
    const RecipeStepDebugStringLightRequest& request);

/** exe/sample/{相机路径}/{颜色}.contour，供检测前校验与算法复用。 */
QString stepDebugPureContourFilePath(const QString& cameraPath, ColorChannel colorChannel);

} // namespace lighten
