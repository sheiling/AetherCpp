#pragma once

/**
 * @file recipe_step_debug_pure_color.h
 * @brief 产品配方单步调试「纯色检测」：参数组装、采图/读图、算法执行。
 */

#include "recipe_step_debug_common.h"

#include <common/utility/errors/include/error.h>

namespace lighten {

/** 单步调试纯色检测请求（由 UI 在主线程采集上下文后传入）。 */
struct RecipeStepDebugPureColorRequest : RecipeStepDebugImageContext
{
    ColorChannel                   colorChannel = ColorChannel::Red;
    RecipeStepDebugSoftDisplayParams softDisplay;
};

struct RecipeStepDebugPureColorResult
{
    common::errors::ErrorPtr err;
    QString                  outputDir;
};

RecipeStepDebugPureColorResult runRecipeStepDebugPureColor(
    const RecipeStepDebugPureColorRequest& request);

} // namespace lighten
