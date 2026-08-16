#pragma once

/**
 * @file motion_action_keys.h
 * @brief MotionControl 插件权限 action key（与 PermService::registerAction 第一参一致）
 */

namespace motion_control_plugin {

namespace MotionActionKeys {

inline constexpr const char *IoEdit  = "motion.io.edit";
inline constexpr const char *IoWrite = "motion.io.write";
inline constexpr const char *IoAdd   = "motion.io.add";
inline constexpr const char *IoDelete = "motion.io.delete";
inline constexpr const char *AxisSave = "motion.axis.save";
inline constexpr const char *AxisAdd = "motion.axis.add";
inline constexpr const char *AxisDelete = "motion.axis.delete";

} // namespace MotionActionKeys

} // namespace motion_control_plugin
