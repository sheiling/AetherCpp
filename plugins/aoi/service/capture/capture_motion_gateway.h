#pragma once

/**
 * @file capture_motion_gateway.h
 * @brief 运动 + 采图编排：运控同步到位、曝光/光源、原图采集
 *
 * 流程位置：
 * - Marker 工步：运动至 mark.capturePos → captureRawAt
 * - FOV 工步：按路径规划逐格 centerPhysical → captureRawAt
 * - load_recipe：checkReady 校验相机/运控就绪
 *
 * 角色 UI 预览上抛仍经 ICameraService::publishRoleImageFromInfo（本类不负责）。
 */

#include "capture_profile.h"
#include "data/dataModel/recipe_config_model.h"
#include "data/dataModel/system_config_model.h"

#include <QString>

#include <memory>

namespace common::device {
struct ImageInfo;
}

namespace aoi::capture {

/** 采图前设备就绪检查结果 */
struct CaptureReadyStatus
{
    /** true 表示相机与运控均可用 */
    bool ok = false;
    /** 失败时的现场可读说明 */
    QString message;
};

/**
 * @brief 运动与采图同步网关（ImageCapture + IMotionControlService）
 *
 * 典型调用链：checkReady → moveXYZAndWait → applyCaptureProfile → snapRawImage
 * 或一步 captureRawAt（Z→XY→曝光→原图）。
 */
class CaptureMotionGateway
{
public:
    /**
     * @brief 校验 common 中配置的相机名、XY 轴及设备就绪状态
     * @param common 系统 CommonParams（captureName / axisXName / axisYName / axisZName）
     */
    static CaptureReadyStatus checkReady(const CommonParams &common);

    /**
     * @brief 单轴绝对运动并等待到位
     * @param common     运控速度/超时
     * @param axisName   轴名
     * @param targetPos  目标位置 (mm 或脉冲，与运控配置一致)
     * @param errorOut   失败原因
     */
    static bool moveAxisAndWait(const CommonParams &common, const QString &axisName,
                                double targetPos, QString *errorOut = nullptr);

    /**
     * @brief XY 顺序运动并等待（先 X 后 Y）
     * @param xMm 目标 X (mm)
     * @param yMm 目标 Y (mm)
     */
    static bool moveXYAndWait(const CommonParams &common, double xMm, double yMm,
                              QString *errorOut = nullptr);

    /**
     * @brief Z（若有）→ XY 运动并等待
     * @param zMm     目标 Z (mm)
     * @param profile 含 axisZName / hasZAxis
     */
    static bool moveXYZAndWait(const CommonParams &common, double xMm, double yMm, double zMm,
                               const CaptureProfile &profile, QString *errorOut = nullptr);

    /**
     * @brief 下发曝光与光源（配方 camera 段）
     * @param profile 含 exposureUs / lightBrightness
     */
    static bool applyCaptureProfile(const CommonParams &common, const CaptureProfile &profile,
                                    QString *errorOut = nullptr);

    /**
     * @brief 当前位置单帧原图（停预览 → snapRawFromCapture）
     * @param exposureUs 曝光；0 表示沿用采集器当前值
     * @param outRaw     原图输出
     */
    static bool snapRawImage(const CommonParams &common, int exposureUs,
                             std::shared_ptr<common::device::ImageInfo> *outRaw,
                             QString *errorOut = nullptr);

    /**
     * @brief 运动至 (x,y,z) 并采集一帧原图（Marker / FOV 共用）
     * @param xMm/yMm/zMm 物理坐标 (mm)
     */
    static bool captureRawAt(const CommonParams &common, const CaptureProfile &profile,
                             double xMm, double yMm, double zMm,
                             std::shared_ptr<common::device::ImageInfo> *outRaw,
                             QString *errorOut = nullptr);

    /** 流程中止时急停全部运控轴 */
    static void emergencyStop(const CommonParams &common);
};

} // namespace aoi::capture
