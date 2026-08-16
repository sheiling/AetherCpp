#pragma once

/**
 * @file inspection_detect_service.h
 * @brief FOV 缺陷检测引擎（产线 Planned + 任意图 AdHoc 统一 detectFov）
 */

#include "fov_detect_request.h"
#include "inspection_session.h"

#include <vision_algo/units/die_bonding/die_bonding_chip_detect_data.h>

#include <common/core/device/imageCapture/image_capture.h>

#include <scheduler/context/flow_context.h>
#include <scheduler/core/flow_types.h>

#include <future>

namespace aoi::inspection {

class InspectionDetectEngine
{
public:
    InspectionDetectEngine();
    ~InspectionDetectEngine();

    bool initialize(const RecipeDocument &recipe, QString *errorOut = nullptr,
                    size_t poolSize = vision_algo::kDefaultDieBondingPoolSize,
                    int dlInferenceEngineOverride = -1);

    bool isReady() const { return ready_; }

    FovDetectRecord detectFov(const FovDetectRequest &request);

    std::future<FovDetectRecord> detectFovAsync(FovDetectRequest request);

    std::future<FovDetectRecord> detectAsync(std::shared_ptr<common::device::ImageInfo> processImage,
                                             int fovIndex, const RecipeDocument &recipe);

    FovDetectRecord detectSync(std::shared_ptr<common::device::ImageInfo> processImage,
                               int fovIndex, const RecipeDocument &recipe,
                               bool dlCropLocate = true);

    bool syncDetectParam(const RecipeDocument &recipe, QString *errorOut = nullptr);

private:
    bool ready_ = false;
    vision_algo::DieBondingChipDetectParam param_{};
};

scheduler::StepResult runFovDetectService(const std::shared_ptr<InspectionSession> &session,
                                          scheduler::FlowContext &ctx);

class InspectionDetectService
{
public:
    static InspectionDetectService &instance();

    void warmUp();

    bool ensureInitialized(const RecipeDocument &recipe, QString *errorOut = nullptr);

    bool isReady() const { return engine_.isReady(); }

    InspectionDetectEngine &engine() { return engine_; }
    const InspectionDetectEngine &engine() const { return engine_; }

private:
    InspectionDetectService() = default;

    InspectionDetectEngine engine_;
};

} // namespace aoi::inspection
