#pragma once

// MessageCenter 事件键（MessageCenter::Center::Attach / Detach / Notify）

// 启动画面；参数：QVariantList[0] = 插件信息（QString）
#define EVENT_SPLASH_MESSAGE "event-splash-message"

// 相机状态变更；参数：QVariantList[0] = 相机 sn（QString），QVariantList[1] = 相机状态（int）
#define EVENT_CAMERA_STATE "event-camera-state"

// 相机连接状态变更；参数：QVariantList[0] = camera_name（QString），QVariantList[1] = 连接状态（bool）
#define EVENT_CAMERA_CONN_STATE "event-camera-conn-state"

// Flash 检测状态变更；参数：QVariantList[0] = flash_detect_key（QString），QVariantList[1] = 连接状态（bool）
#define EVENT_FLASH_DETECT_STATE "event-flash-detect-state"

// 运控连接状态变更；参数：QVariantList[0] = motion_card_name（QString），QVariantList[1] = 连接状态（bool）
#define EVENT_MOTION_CONN_STATE "event-motion-conn-state"

// 扫码枪连接状态变更；参数：QVariantList[0] = scanner_name（QString），QVariantList[1] = 连接状态（bool）
#define EVENT_SCANNER_CONN_STATE "event-scanner-conn-state"

// 运行日志上报；参数：QVariantList[0] = 级别（QString，如 INFO/WARN/ERROR），QVariantList[1] = 消息正文（QString）
#define EVENT_RUNTIME_LOG "event-runtime-log"
