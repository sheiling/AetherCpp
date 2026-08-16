#pragma once

/**
 * @file motion_debug_action_keys.h
 * @brief 运控调试页权限 action key（与 PermService::registerAction 第一参一致）
 */

namespace motion {

namespace MotionDebugActionKeys {

inline constexpr const char *IoRead  = "motionDebug.ioRead";
inline constexpr const char *IoWrite = "motionDebug.ioWrite";

} // namespace MotionDebugActionKeys

} // namespace motion
