#pragma once

/**
 * @file scan_flow_pause.h
 * @brief 扫描工步与整机 MachineFSM 协同：暂停等待、abort 感知
 *
 * 背景：FlowInstance 仅在工步边界检查 pause；FOV 采图/检测等长循环须在循环内
 * 调用 waitWhileMachinePaused，响应操作员「暂停」按钮。
 */

#include "service/inspection/session/inspection_session.h"

#include <memory>

namespace aoi::flow {

/**
 * @brief 若整机主态为 Paused，阻塞直至恢复 Running 或 session 请求 abort
 * @param session 单板扫描会话（abortFlag 由 AoiScanFlowLifecycleHandler::onFlowAbort 置位）
 * @return true 可继续工步；false 已 abort 或主态不可继续（Alarm/NotReset 等）
 */
bool waitWhileMachinePaused(const std::shared_ptr<inspection::InspectionSession> &session);

} // namespace aoi::flow
