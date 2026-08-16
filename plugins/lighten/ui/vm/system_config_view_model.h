#pragma once

/**
 * @file system_config_view_model.h
 * @brief 系统配置 ViewModel（配置加载/保存/重置与命令绑定）
 */

#include "lighten_viewmodel_macros.h"
#include "command.h"
#include "system_config_model.h"

namespace lighten {

/**
 * @brief 系统配置 ViewModel
 *
 * 绑定 SystemConfigView：开关、检测项分色、通用路径/延时、判定阈值。
 */
class SystemConfigViewModel : public ViewModel
{
    Q_OBJECT

public:
    static constexpr const char *PROP_CONFIG  = "config";
    static constexpr const char *PROP_STATUS  = "status";
    static constexpr const char *PROP_NAV_INDEX = "navIndex";

    LIGHTEN_VIEWMODEL_STRUCT_PROPERTY(SystemConfig, config, Config)
    LIGHTEN_VIEWMODEL_PROPERTY(QString, status, Status)
    LIGHTEN_VIEWMODEL_PROPERTY(int, navIndex, NavIndex)

signals:
    void sig_configChanged();
    void sig_statusChanged();
    void sig_navIndexChanged();
    /** 每次保存配置写入成功时发出（与 status 是否变化无关） */
    void sig_saveSucceeded();
    /** 保存写入失败时发出 */
    void sig_saveFailed();

public:
    explicit SystemConfigViewModel(QObject *parent = nullptr);

    /** 保存配置命令 */
    Command *saveCommand() const { return m_saveCommand; }
    /** 恢复默认配置命令 */
    Command *resetCommand() const { return m_resetCommand; }
    /** 清零当前使用次数命令 */
    Command *clearUseCountCommand() const { return m_clearUseCountCommand; }

public slots:
    /** 从持久化层加载系统配置 */
    void slot_load();
    /** 保存当前配置到持久化层 */
    void slot_save();
    /** 恢复为内置默认配置 */
    void slot_resetToDefaults();
    /** 将当前使用次数清零 */
    void slot_clearCurrentUseCount();

private:
    FunctionalCommand *m_saveCommand          = nullptr;
    FunctionalCommand *m_resetCommand         = nullptr;
    FunctionalCommand *m_clearUseCountCommand = nullptr;
};

} // namespace lighten
