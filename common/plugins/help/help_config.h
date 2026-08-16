#pragma once

/**
 * @file help_config.h
 * @brief 帮助页配置 — 从 config/app.toml 的 [help] 段读取（Bootstrap 已 load app.toml）
 *
 * 版本字段优先读 help.version.*，为空则回退 [app].name / version / build_date；
 * 文档路径读 help.documents.*，相对 exe 目录。
 */

#include <QString>

namespace help {

/** 帮助页「版本信息」展示项 */
struct HelpVersionInfo
{
    QString appName;
    QString currentVersion;
    QString buildTime;
    QString updateTime;
};

/** 三份 PDF 相对或绝对路径（配置原值，未拼 exe 目录） */
struct HelpDocuments
{
    QString userManual;
    QString maintenance;
    QString helpDoc;
};

/** 从 ConfigManager 加载版本信息（help.version → app 回退） */
HelpVersionInfo loadHelpVersionInfo();

/** 从 ConfigManager 加载 PDF 路径（help.documents） */
HelpDocuments loadHelpDocuments();

/** 将配置路径解析为绝对路径（已是绝对路径则原样返回） */
QString resolveRuntimePath(const QString &path);

} // namespace help
