#pragma once

/**
 * @file motion_catalog_dialogs.h
 * @brief 运控目录（轴/IO）新增弹窗：ThemedPanelDialog + 名称/模块双字段输入
 */

#include <QString>

class QWidget;

namespace motion_control_plugin {

/**
 * @brief 名称 + 模块名双字段弹窗
 * @return 用户点击确定且名称非空时 true；nameOut/moduleOut 写入 trim 后文本
 */
bool promptNameModuleDialog(QWidget *parent,
                            const QString &titleKey,
                            const QString &nameLabelKey,
                            const QString &moduleLabelKey,
                            const QString &namePlaceholderKey,
                            const QString &modulePlaceholderKey,
                            const QString &defaultName,
                            const QString &defaultModuleName,
                            QString *nameOut,
                            QString *moduleOut);

} // namespace motion_control_plugin
