#pragma once

/**
 * @file system_config_helpers.h
 * @brief 系统配置三子页共用的展示辅助函数
 *
 * 当前提供缺陷项 displayName → 界面可读文本的翻译/兼容逻辑。
 */

#include "uibase/languagemanager.h"

#include <QString>

namespace aoi {

/**
 * @brief 将缺陷项 displayName 转为界面可读文本
 *
 * `AoiSystemConfig.defects[*].displayName` 可能存翻译键（以 `aoi.` 开头）或用户自定义明文。
 * - 以 `aoi.` 开头：走 LanguageManager 翻译
 * - 否则：原样显示（兼容旧数据或现场改名）
 */
inline QString defectItemDisplayText(const QString &displayName)
{
    if (displayName.isEmpty())
        return {};
    if (displayName.startsWith(QLatin1String("aoi.")))
        return TR(displayName.toUtf8().constData());
    return displayName;
}

} // namespace aoi
