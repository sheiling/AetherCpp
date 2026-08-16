#pragma once

/**
 * @file capture_profile.h
 * @brief 采图参数快照：从系统配置与配方解析曝光、光源及 Z 高度等
 *
 * 流程位置：Marker 定位 / FOV 采图 之前，由 load_recipe 或各工步入场时写入
 * `InspectionSession::captureProfile`。
 */

#include "data/dataModel/recipe_config_model.h"
#include "data/dataModel/system_config_model.h"

#include <QString>

namespace aoi::capture {

/**
 * @brief 单次运动采图前的设备参数快照
 *
 * 由 `buildCaptureProfile` 从 `AoiSystemConfig` + `RecipeDocument` 填充；
 * Marker 工步用 `markCaptureZmm`，FOV 工步用 `fovCaptureZmm`。
 */
struct CaptureProfile
{
    /** 运控 Z 轴名称（来自 common.axisZName）；空表示无 Z 轴（仅 XY 运动） */
    QString axisZName;
    /** Mark 采图高度 (mm)，见 resolveMarkCaptureZ */
    double markCaptureZmm = 0.0;
    /** FOV 采图高度 (mm)，见 resolveFovCaptureZ */
    double fovCaptureZmm = 0.0;
    /** 曝光时间 (μs)，来自 recipe.camera.exposureUs */
    int exposureUs = 0;
    /** 光源亮度，来自 recipe.camera.lightBrightness */
    int lightBrightness = 0;
    /** 像素当量 (mm/px)，来自 recipe.camera.captureAccuracy */
    double mmPerPixel = 1.0;
    /** 是否配置了可用 Z 轴（axisZName 非空） */
    bool hasZAxis = false;
};

/**
 * @brief 从系统配置与配方构建采图参数快照
 * @param sysConfig 系统配置（轴名、相机名等 CommonParams）
 * @param recipe    当前配方（camera / positionCalib / marks）
 */
CaptureProfile buildCaptureProfile(const AoiSystemConfig &sysConfig, const RecipeDocument &recipe);

/**
 * @brief 解析指定 Mark 槽位的采图 Z (mm)
 * @param recipe   配方文档
 * @param markSlot Mark 槽位 0..3
 * @return 优先 marks[slot].capturePos.z，否则 positionCalib.scanPosition.z
 */
double resolveMarkCaptureZ(const RecipeDocument &recipe, int markSlot);

/**
 * @brief 解析 FOV 网格采图 Z (mm)
 * @param recipe 配方文档
 * @return 优先 scanPosition.z，否则 marks[0].capturePos.z
 */
double resolveFovCaptureZ(const RecipeDocument &recipe);

} // namespace aoi::capture
