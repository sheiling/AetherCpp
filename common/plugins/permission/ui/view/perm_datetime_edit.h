#pragma once

/**
 * @file perm_datetime_edit.h
 * @brief 权限插件日期时间输入（复用 common/ui ThemedDateTimeEdit）
 */

#include "controls/datetime/themed_datetime_edit.h"

namespace permission_ui {

using PermDateTimeResolution = ui::DateTimeResolution;
using PermDateTimeEdit       = ui::ThemedDateTimeEdit;
using PermDateTimePickerTexts = ui::DateTimePickerTexts;

} // namespace permission_ui
