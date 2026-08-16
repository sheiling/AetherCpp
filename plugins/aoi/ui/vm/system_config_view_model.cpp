/**
 * @file system_config_view_model.cpp
 * @brief 系统配置 ViewModel：属性注册、`system.save` Command 与 load/save 实现
 */

#include "system_config_view_model.h"

#include "service/param_change/param_change_audit_manager.h"

#include "uibase/permission/permservice.h"
#include "uibase/languagemanager.h"

#include <QMetaType>

namespace aoi {

SystemConfigViewModel::SystemConfigViewModel(const QString &pageKey, QObject *parent)
    : ViewModel(parent)
    , m_pageKey(pageKey)
{
    qRegisterMetaType<AoiSystemConfig>("AoiSystemConfig");

    registerProperty(PROP_CONFIG, QVariant::fromValue(AoiSystemConfig::defaultDemo()));

    m_saveCommand = new FunctionalCommand(
        [this]() {
            const AoiSystemConfig before   = SystemConfigRepository::load();
            const bool            hadChanges = (before != config());
            if (!save())
                return;
            if (hadChanges)
                emit saveSucceeded();
        },
        nullptr, this);
    m_saveCommand->setName(QStringLiteral("system.save"));

    if (!m_pageKey.isEmpty()) {
        PermService::instance()->registerAction(QStringLiteral("system.save"), m_pageKey,
                                                QStringLiteral("aoi.system.action.save"));
    }

    load();
}

void SystemConfigViewModel::load()
{
    m_suppressPersist = true;
    setConfig(SystemConfigRepository::load());
    m_suppressPersist = false;
}

bool SystemConfigViewModel::save()
{
    const AoiSystemConfig before = SystemConfigRepository::load();
    const AoiSystemConfig after  = config();
    if (before == after)
        return true;
    if (SystemConfigRepository::save(after)) {
        ParamAuditContext ctx;
        ctx.moduleCode = QStringLiteral("aoi.system_config");
        ctx.sourcePage = QStringLiteral("系统配置");
        ParamChangeAuditManager::instance().audit(before, after, ctx);
        return true;
    }
    emit saveFailed();
    return false;
}

} // namespace aoi
