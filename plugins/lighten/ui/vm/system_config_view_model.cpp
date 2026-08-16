/**
 * @file system_config_view_model.cpp
 * @brief SystemConfigViewModel 配置读写与命令实现
 */

#include "system_config_view_model.h"

#include "lighten_param_persist.h"
#include "lightenplugin.h"

#include "messagecenter/message_center.h"

#include <QMetaType>

/** @brief SystemConfigViewModel 成员实现 */
namespace lighten {

namespace {

void notifyGeneralSettingsUseCountChanged(const GeneralSettings &gs)
{
    MessageCenter::Center::Notify(
        EVENT_LIGHTEN_GENERAL_SETTINGS_CHANGED,
        QVariantList{gs.currentUseCount, gs.maxUseCount});
}

} // namespace

SystemConfigViewModel::SystemConfigViewModel(QObject *parent)
    : ViewModel(parent)
{
    qRegisterMetaType<SystemConfig>("SystemConfig");

    registerProperty(PROP_CONFIG, QVariant::fromValue(SystemConfig::load()));
    registerProperty(PROP_STATUS, QStringLiteral("就绪"));
    registerProperty(PROP_NAV_INDEX, 0);

    m_saveCommand = new FunctionalCommand([this]() { slot_save(); }, nullptr, this);
    m_saveCommand->setName(QStringLiteral("systemConfig.save"));

    m_resetCommand = new FunctionalCommand([this]() { slot_resetToDefaults(); }, nullptr, this);
    m_resetCommand->setName(QStringLiteral("systemConfig.reset"));

    m_clearUseCountCommand =
        new FunctionalCommand([this]() { slot_clearCurrentUseCount(); }, nullptr, this);
    m_clearUseCountCommand->setName(QStringLiteral("systemConfig.clearUseCount"));

    slot_load();
}

void SystemConfigViewModel::slot_load()
{
    setConfig(SystemConfigRepository::load());
    setStatus(QStringLiteral("配置已加载"));
}

void SystemConfigViewModel::slot_save()
{
    const SystemConfig before = SystemConfigRepository::load();
    const SystemConfig after  = config();
    if (SystemConfigRepository::save(after)) {
        ParameterChangeLogger::logSystemConfigSave(before, after);
        if (status() != QStringLiteral("配置已保存"))
            setStatus(QStringLiteral("配置已保存"));
        notifyGeneralSettingsUseCountChanged(after.generalSettings);
        MessageCenter::Center::Notify(EVENT_LIGHTEN_SYSTEM_CONFIG_SAVED, QVariantList{});
        emit sig_saveSucceeded();
    } else {
        setStatus(QStringLiteral("保存配置失败"));
        emit sig_saveFailed();
    }
}

void SystemConfigViewModel::slot_resetToDefaults()
{
    setConfig(SystemConfig::defaultDemo());
    setStatus(QStringLiteral("已恢复默认"));
}

void SystemConfigViewModel::slot_clearCurrentUseCount()
{
    modifyConfig([](SystemConfig &c) { c.generalSettings.currentUseCount = 0; });
    setStatus(QStringLiteral("使用次数已清零"));
}

} // namespace lighten
