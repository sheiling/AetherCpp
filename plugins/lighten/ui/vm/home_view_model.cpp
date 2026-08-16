/**
 * @file home_view_model.cpp
 * @brief HomeViewModel 属性注册与运行时状态更新实现
 */

#include "home_view_model.h"

#include <QDateTime>
#include <QDebug>
#include <QMetaType>

#include <extensionsystem/pluginmanager.h>
#include <interface/ilighten_production_kpi_service.h>

/** @brief HomeViewModel 成员实现 */
namespace lighten {

HomeViewModel::HomeViewModel(QObject *parent)
    : ViewModel(parent)
{
    qRegisterMetaType<HomeRuntimeState>("HomeRuntimeState");

    registerProperty(PROP_RUNTIME, QVariant::fromValue(HomeRuntimeState::defaultDemo()));
    registerProperty(PROP_STATUS, QStringLiteral("就绪"));
    registerProperty(PROP_SELECTED_COLOR, static_cast<int>(ColorChannel::Red));
    registerProperty(PROP_SELECTED_CAMERA, static_cast<int>(CameraIndex::Main));
    registerProperty(PROP_SHOW_RESULT_IMAGE, false);

    slot_refreshFromModel();
}

void HomeViewModel::slot_refreshFromModel()
{
    setRuntime(HomeRuntimeState::defaultDemo());
}

void HomeViewModel::slot_updateCurrentRecipe(const QString &recipeId)
{
    if (recipeId.isEmpty())
        return;
    HomeRuntimeState rt = runtime();
    if (rt.kpi.currentRecipe == recipeId)
        return;
    rt.kpi.currentRecipe = recipeId;
    setRuntime(rt);
}

void HomeViewModel::slot_updateMaxUseCount(int maxUseCount)
{
    HomeRuntimeState rt = runtime();
    if (rt.kpi.maxUseCount == maxUseCount)
        return;
    rt.kpi.maxUseCount = maxUseCount;
    setRuntime(rt);
}

void HomeViewModel::slot_refreshProductionKpiFromService()
{
    auto *service = ExtensionSystem::PluginManager::getInterface<ILightenProductionKpiService>();
    if (!service || !service->isReady())
        return;

    LightenProductionKpiRecord record;
    QString error;
    if (!service->loadKpi(&record, &error)) {
        qWarning() << "[HomeViewModel] loadKpi failed:" << error;
        return;
    }

    HomeRuntimeState rt = runtime();
    rt.kpi.totalPanels     = record.totalLightenedPanels;
    rt.kpi.okCount         = record.okCount;
    rt.kpi.ngCount         = record.ngCount;
    rt.kpi.rwCount         = record.rwCount;
    rt.kpi.passRate        = record.passRate;
    rt.kpi.currentUseCount = record.insertionCount;
    rt.kpi.syncMaxUseCountFromConfig();
    setRuntime(rt);
}

void HomeViewModel::slot_updateProgressStatus(int slotIndex, const QString &statusText)
{
    HomeRuntimeState rt = runtime();
    if (slotIndex < 0 || slotIndex >= rt.progress.size()) {
        return;
    }
    if (rt.progress[slotIndex].statusText == statusText) {
        return;
    }
    rt.progress[slotIndex].statusText = statusText;
    setRuntime(rt);
}

void HomeViewModel::slot_appendLog(const QString &message, const QString &level)
{
    HomeRuntimeState rt = runtime();
    RunLogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.level     = level;
    entry.message   = message;
    rt.logs.append(entry);
    setRuntime(rt);
}

} // namespace lighten
