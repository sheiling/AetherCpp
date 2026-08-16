/**
 * @file recipe_virtual_inspect_view_model.cpp
 */

#include "recipe_virtual_inspect_view_model.h"
#include "recipe_config_view_model.h"

#include "flow_host/flow_host_coordinator.h"
#include "service/inspection/virtual_inspect/virtual_inspect_runner.h"

#include <icamera_service.h>
#include <extensionsystem/pluginmanager.h>
#include <Logger.h>
#include <uibase/languagemanager.h>

#include <QMetaObject>
#include <QThread>
#include <QTimer>

using namespace ExtensionSystem;

namespace aoi {

RecipeVirtualInspectViewModel::RecipeVirtualInspectViewModel(RecipeConfigViewModel *host,
                                                             QObject *parent)
    : QObject(parent)
    , m_host(host)
{
    m_detectingTimer = new QTimer(this);
    m_detectingTimer->setInterval(400);
    connect(m_detectingTimer, &QTimer::timeout, this,
            &RecipeVirtualInspectViewModel::tickDetectingAnimation);

    m_runnerThread = new QThread(this);
    m_runner       = new inspection::VirtualInspectRunner();
    m_runner->moveToThread(m_runnerThread);
    m_runnerThread->start();

    connect(m_runner, &inspection::VirtualInspectRunner::progressChanged, this,
            &RecipeVirtualInspectViewModel::onRunnerProgress, Qt::QueuedConnection);
    connect(m_runner, &inspection::VirtualInspectRunner::finished, this,
            &RecipeVirtualInspectViewModel::finishVirtualInspect, Qt::QueuedConnection);
}

RecipeVirtualInspectViewModel::~RecipeVirtualInspectViewModel()
{
    if (!m_runnerThread)
        return;

    if (m_runner)
        QMetaObject::invokeMethod(m_runner, "cancel", Qt::BlockingQueuedConnection);

    m_runnerThread->quit();
    m_runnerThread->wait();

    delete m_runner;
    m_runner = nullptr;
}

void RecipeVirtualInspectViewModel::setVirtualInspecting(bool inspecting)
{
    if (m_virtualInspecting == inspecting)
        return;
    m_virtualInspecting = inspecting;
    emit virtualInspectingChanged();
}

bool RecipeVirtualInspectViewModel::canVirtualInspect(QString *blockedReason) const
{
    if (flow_host::flowHostCoordinator().isProductionRunActive()) {
        if (blockedReason)
            *blockedReason = QStringLiteral("aoi.recipe.inspect.blocked_while_running");
        return false;
    }
    return true;
}

void RecipeVirtualInspectViewModel::virtualInspectFromImage(const QImage &image)
{
    if (!m_host || m_virtualInspecting)
        return;

    QString blockedReason;
    if (!canVirtualInspect(&blockedReason)) {
        m_host->notifyStatus(TR(blockedReason.toUtf8().constData()));
        return;
    }

    if (m_host->isLivePreviewActive()) {
        if (auto *camSvc = PluginManager::getInterface<ICameraService>())
            camSvc->stopPreview(false);
    }

    if (image.isNull()) {
        m_host->notifyStatus(TR("aoi.recipe.inspect.no_preview"));
        return;
    }

    const QString detectSourcePath =
        m_host->previewImageFromFile() ? m_host->previewImageSourcePath() : QString();
    LOG_INFO("[VirtualInspect] click: frame {}x{} detectSource={}", image.width(), image.height(),
             detectSourcePath.isEmpty() ? "(temp)" : detectSourcePath.toStdString());

    const RecipeDocument recipeSnapshot = m_host->document();
    const QImage         frameCopy      = image.copy();

    setVirtualInspecting(true);
    startDetectingAnimation();

    LOG_INFO("[VirtualInspect] adhoc detectFov begin");
    QMetaObject::invokeMethod(m_runner, "start", Qt::QueuedConnection,
                              Q_ARG(RecipeDocument, recipeSnapshot), Q_ARG(QImage, frameCopy),
                              Q_ARG(QString, detectSourcePath));
}

void RecipeVirtualInspectViewModel::cancelVirtualInspect()
{
    if (!m_host || !m_virtualInspecting || !m_runner)
        return;
    LOG_INFO("[VirtualInspect] cancel requested");
    QMetaObject::invokeMethod(m_runner, "cancel", Qt::QueuedConnection);
}

void RecipeVirtualInspectViewModel::onRunnerProgress(int percent, const QString &stageKey)
{
    emit virtualInspectProgress(percent, stageKey);
    if (m_host) {
        m_host->setStatus(TR("aoi.recipe.inspect.detecting") + QStringLiteral(" %1% — ").arg(percent)
                          + TR(stageKey));
    }
}

void RecipeVirtualInspectViewModel::startDetectingAnimation()
{
    if (!m_host)
        return;
    m_detectingDotCount = 0;
    m_host->setStatus(TR("aoi.recipe.inspect.detecting"));
    m_detectingTimer->start();
}

void RecipeVirtualInspectViewModel::stopDetectingAnimation()
{
    m_detectingTimer->stop();
    m_detectingDotCount = 0;
}

void RecipeVirtualInspectViewModel::tickDetectingAnimation()
{
    if (!m_host)
        return;
    m_detectingDotCount = (m_detectingDotCount + 1) % 4;
    const QString dots  = QString(m_detectingDotCount, QChar(0x3002));
    m_host->setStatus(TR("aoi.recipe.inspect.detecting") + dots);
}

void RecipeVirtualInspectViewModel::finishVirtualInspect(
    aoi::CurrentInspectionSnapshot snapshot, const QString &error)
{
    LOG_INFO("[VirtualInspect] adhoc detectFov end error={} sn={} overall={}", error.toStdString(),
             snapshot.serialNumber.toStdString(),
             static_cast<int>(snapshot.overallResult));
    stopDetectingAnimation();
    emit virtualInspectProgress(-1, QString());

    setVirtualInspecting(false);

    if (error.isEmpty() && !snapshot.serialNumber.isEmpty()) {
        m_inspectSnapshot = std::move(snapshot);
        emit virtualInspectSnapshotChanged();
    }

    if (!m_host)
        return;

    if (!error.isEmpty()) {
        if (error == QStringLiteral("aoi.recipe.inspect.cancelled"))
            m_host->notifyStatus(TR("aoi.recipe.inspect.cancelled"));
        else if (error == QStringLiteral("aoi.recipe.inspect.blocked_while_running"))
            m_host->notifyStatus(TR("aoi.recipe.inspect.blocked_while_running"));
        else
            m_host->notifyStatus(error.startsWith(QStringLiteral("aoi."))
                                     ? TR(error.toUtf8().constData())
                                     : error);
        return;
    }

    if (snapshot.overallResult == aoi::InspectionOverallResult::Ok) {
        m_host->notifyStatus(TR("aoi.recipe.inspect.done_ok"));
        return;
    }

    int totalDefects = 0;
    for (const inspection::InspectionSnapshotRow &row : snapshot.rows) {
        if (row.total > 0)
            totalDefects += row.total;
    }
    m_host->notifyStatus(TR("aoi.recipe.inspect.done_ng").arg(totalDefects));
}

} // namespace aoi
