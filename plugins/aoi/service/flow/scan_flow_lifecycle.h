#pragma once

/**
 * @file scan_flow_lifecycle.h
 * @brief AOI 扫描流程生命周期：Session 注入、abort 急停、结束文案
 *
 * 挂接到 flow_host::FlowHostCoordinator，在 FlowDefinition onStart/onAbort 等回调中执行。
 */

#include <flow_host/flow_lifecycle_handler.h>

#include <scheduler/context/flow_context.h>

#include <memory>

namespace aoi::flow_host {
class FlowHostCoordinator;
} // namespace aoi::flow_host

namespace aoi::flow {

/**
 * @brief AOI 单板扫描流程生命周期实现
 *
 * onFlowStart：创建 InspectionSession，绑定 ScanFlowProgressBridge，写入 kCtxSession。
 * onFlowAbort：置 abortFlag，调用 CaptureMotionGateway 急停。
 */
class AoiScanFlowLifecycleHandler final : public flow_host::FlowLifecycleHandler
{
public:
    void onFlowStart(scheduler::FlowContext &ctx) override;
    QString completeMessage(const scheduler::FlowContext &ctx) const override;
    QString failedMessage(const scheduler::FlowContext &ctx) const override;
    QString abortMessage() const override;
    void onFlowAbort(scheduler::FlowContext &ctx) override;
    QString startedLogMessage() const override;
};

/** 向全局 FlowHostCoordinator 注册 AOI 扫描业务生命周期 */
void bindAoiScanFlowHost(flow_host::FlowHostCoordinator &coordinator);

} // namespace aoi::flow
