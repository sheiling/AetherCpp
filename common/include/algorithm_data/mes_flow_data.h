#pragma once

/**
 * @file mes_flow_data.h
 * @brief MES 工步 FlowContext 运行时键定义。
 *
 * FlowContext 运行时键：
 *   - KEY_MES_STEP_CONFIG                  MES 连接与设备身份（GLOBAL；HomeView「初始化」从 config/mes_config.json 写入）
 *   - KEY_MES_ELECTRICAL_PARA_STEP_CONFIG  电气/阈值上报参数（GLOBAL；HomeView「初始化」从系统配置 + 配方写入）
 *   - KEY_MES_EQUIPMENT_STATUS             设备状态上报参数（FLOW；可选，缺省 SJX003 空闲）
 *   - KEY_MES_FAIL_REASON                  检测失败原因（FLOW；可选，供检测结果上报 dieResult 编码）
 *   - KEY_DEDUP_DEFECTS_OUT                去重后缺陷点（FLOW；result_summary 写入）
 *   - KEY_LINE_DEFECT_SUMMARY_OUT          线性不良汇总（FLOW；result_summary 写入）
 *   - KEY_BLOCK_DEFECT_SUMMARY_OUT         区块不良汇总（FLOW；result_summary 写入）
 */

#include <common/utility/json_tool/reflector.h>

#include <string>

// MES 连接与设备身份（GLOBAL scope；JSON 字段同 mes::MesConfig）
#define KEY_MES_STEP_CONFIG "key-mes-step-config"

// 电气/阈值配置（GLOBAL scope；JSON 字段同 mes::MesElectricalPara）
#define KEY_MES_ELECTRICAL_PARA_STEP_CONFIG "key-mes-electrical-para-step-config"

// 设备状态上报（FLOW scope；JSON: { "upperLevelCode", "currentCode" }）
#define KEY_MES_EQUIPMENT_STATUS "key-mes-equipment-status"

// 检测失败原因（FLOW scope；如「大面积灭点」「线性不良」）
#define KEY_MES_FAIL_REASON "key-mes-fail-reason"
