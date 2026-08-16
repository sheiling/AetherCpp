#pragma once

/**
 * @file capture_path_types.h
 * @brief 采集路径规划类型：FOV 访问顺序、逐点路径、双轴插补条带
 *
 * 本模块**仅产出规划数据**，不发起运动、不采图、不写设备。
 * 输入为 `fov_planner::FovGridPlan` 中的 FOV 物理中心；
 * 输出供检测工步 / 运控网关后续执行。
 */

#include "service/fov_planner/fov_grid_planner.h"

#include <string>
#include <vector>

namespace aoi::path_planner {

/**
 * @brief FOV 访问几何模式（与配方 `panelCalib.pathModeIndex` 一一对应）
 *
 * | 索引 | 模式 | 说明 |
 * |------|------|------|
 * | 0 | Serpentine | S 型：奇数行反向，减少空程 |
 * | 1 | RowZPattern | Z 型：各行同向，行间快速回程 |
 * | 2 | RowMajor | 逐行：按 gridRow 升序、gridCol 升序 |
 * | 3 | ColumnMajor | 逐列：按 gridCol 升序、gridRow 升序 |
 */
enum class FovVisitPattern
{
    Serpentine   = 0,
    RowZPattern  = 1,
    RowMajor     = 2,
    ColumnMajor  = 3,
};

/**
 * @brief 路径执行方式（运动层策略，默认逐点）
 *
 * - PointToPoint：每个 FOV 独立定位到中心后触发采图（默认，兼容现有采图服务）
 * - LinearScanTrigger：双轴直线插补 + 位置比较硬件触发（条带扫描，减少加减速）
 */
enum class CaptureExecutionMode
{
    PointToPoint      = 0,
    LinearScanTrigger = 1,
};

/** 由配方 pathModeIndex 解析访问模式，越界时回退 Serpentine */
FovVisitPattern fovVisitPatternFromRecipeIndex(int pathModeIndex);

/** 双轴插补条带的扫描轴 */
enum class ScanAxis
{
    X = 0,
    Y = 1,
};

/**
 * @brief 逐点采集路径点
 *
 * position 为 FOV 规划给出的物理中心 (mm)；fovIndex 与 FovGridCell::index 一致。
 */
struct CapturePathWaypoint
{
    int    fovIndex  = -1;
    int    gridRow   = 0;
    int    gridCol   = 0;
    double xMm       = 0.0;
    double yMm       = 0.0;
};

/**
 * @brief 双轴插补 + 硬件触发条带
 *
 * 固定轴坐标 `fixedCoordMm` 不变，沿 `scanAxis` 从 startPosMm 插补到 endPosMm；
 * `triggerPositionsMm` 与 `fovIndices` 一一对应，供位置比较触发采图。
 */
struct CaptureLinearScanStrip
{
    ScanAxis            scanAxis          = ScanAxis::X;
    double              fixedCoordMm      = 0.0;
    double              startPosMm        = 0.0;
    double              endPosMm          = 0.0;
    std::vector<double> triggerPositionsMm;
    std::vector<int>    fovIndices;
};

/**
 * @brief 采集路径规划结果（抽象层，不含轴名）
 */
struct CapturePathPlan
{
    FovVisitPattern     visitPattern   = FovVisitPattern::Serpentine;
    CaptureExecutionMode executionMode = CaptureExecutionMode::PointToPoint;

    /** 全局 FOV 访问顺序（FovGridCell::index） */
    std::vector<int> visitOrder;

    /** executionMode == PointToPoint 时有效，与 visitOrder 一致 */
    std::vector<CapturePathWaypoint> waypoints;

    /** executionMode == LinearScanTrigger 时有效 */
    std::vector<CaptureLinearScanStrip> scanStrips;
};

/**
 * @brief 逐点运动任务（规划数据，供运控层消费）
 */
struct CapturePointMotionTask
{
    std::string axisXName;
    std::string axisYName;
    double      xMm     = 0.0;
    double      yMm     = 0.0;
    int         fovIndex = -1;
    double      speed   = 0.0;
    int         timeoutMs = 30000;
};

/**
 * @brief 双轴插补条带运动任务（规划数据，供运控层消费）
 */
struct CaptureLinearMotionTask
{
    std::string         axisXName;
    std::string         axisYName;
    double              startXmm = 0.0;
    double              startYmm = 0.0;
    double              endXmm   = 0.0;
    double              endYmm   = 0.0;
    bool                compareOnX = true;
    std::vector<double> comparePositionsMm;
    std::vector<int>    fovIndices;
    double              speed     = 0.0;
    int                 timeoutMs = 30000;
};

/**
 * @brief 绑定轴名后的完整采集运动规划（Session 缓存类型）
 */
struct CaptureMotionPlan
{
    CaptureExecutionMode executionMode = CaptureExecutionMode::PointToPoint;
    std::vector<int>     imageIndexOrder;

    std::vector<CapturePointMotionTask>  pointTasks;
    std::vector<CaptureLinearMotionTask> linearTasks;
};

struct CapturePathGenerateResult
{
    bool            ok = false;
    std::string     error;
    CapturePathPlan path;
};

} // namespace aoi::path_planner
