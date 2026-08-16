/**
 * @file motion_axis_debug_view.cpp
 * @brief 轴调试页：模块列表 + 2 列 AxisDebugPanel 网格（可滚动）
 */

#include "motion_axis_debug_view.h"

#include "widgets/axis/axis_debug_panel.h"

#include "extensionsystem/pluginmanager.h"
#include "imotion_control_service.h"
#include "component/list/themed_list_widget.h"
#include "uibase/languagemanager.h"
#include "uibase/thememanager.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QFrame>
#include <QScrollArea>
#include <QVBoxLayout>

namespace motion_control_plugin {

using ExtensionSystem::PluginManager;

namespace {

constexpr int kDebugGridColumns = 2;

QString trKey(const char *key)
{
    return LanguageManager::instance()->t(QString::fromLatin1(key));
}

/** 多模块（>1）时才展示模块导航 */
bool showModuleNavigator(const AxisModuleMap &groups)
{
    return groups.size() > 1;
}

QScrollArea *makeDebugGridScrollHost(QWidget **outHost, QGridLayout **outGrid, QWidget *parent)
{
    auto *scroll = new QScrollArea(parent);
    scroll->setObjectName(QStringLiteral("motionAxisDebugScroll"));
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setAttribute(Qt::WA_StyledBackground, true);
    scroll->setStyleSheet(
        QStringLiteral("QScrollArea#motionAxisDebugScroll { background: transparent; border: none; }"));
    scroll->viewport()->setAttribute(Qt::WA_StyledBackground, true);
    scroll->viewport()->setAutoFillBackground(false);
    scroll->viewport()->setStyleSheet(QStringLiteral("background: transparent;"));

    auto *host = new QWidget(scroll);
    host->setObjectName(QStringLiteral("motionAxisDebugHost"));
    host->setAttribute(Qt::WA_StyledBackground, false);
    host->setAutoFillBackground(false);
    auto *grid = new QGridLayout(host);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(12);
    grid->setAlignment(Qt::AlignTop);

    scroll->setWidget(host);
    *outHost = host;
    *outGrid = grid;
    return scroll;
}

void syncDebugGridStretch(QGridLayout *grid)
{
    if (!grid)
        return;
    for (int c = 0; c < kDebugGridColumns; ++c)
        grid->setColumnStretch(c, 1);
}

} // namespace

MotionAxisDebugView::MotionAxisDebugView(QWidget *parent)
    : PageWidget(parent)
{
    setPageId(QStringLiteral("motion.axis_debug"));
}

void MotionAxisDebugView::onLoad()
{
    m_service = PluginManager::getObject<IMotionControlService>();
    buildLayout();
    applyTheme();
    reloadModuleList();

    if (m_service) {
        connect(m_service, &IMotionControlService::axisStatesUpdated, this,
                &MotionAxisDebugView::refreshDebugPanels);
    }
}

void MotionAxisDebugView::buildLayout()
{
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(12);

    m_moduleList = ui::makeListWidget(this);
    m_moduleList->setObjectName(QStringLiteral("motionAxisDebugModuleList"));
    m_moduleList->setMinimumWidth(180);
    m_moduleList->setMaximumWidth(240);

    auto *scroll = makeDebugGridScrollHost(&m_panelHost, &m_panelGrid, this);

    root->addWidget(m_moduleList);
    root->addWidget(scroll, 1);

    connect(m_moduleList, &QListWidget::currentItemChanged, this,
            &MotionAxisDebugView::onModuleSelected);
}

void MotionAxisDebugView::applyTheme()
{
    setObjectName(QStringLiteral("motionAxisDebugRoot"));
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QStringLiteral("#motionAxisDebugRoot { background: transparent; }"));
}

QString MotionAxisDebugView::moduleDisplayName(const QString &moduleKey)
{
    if (moduleKey.isEmpty() || moduleKey == QStringLiteral("_default"))
        return trKey("motion.module.default");
    return moduleKey;
}

void MotionAxisDebugView::reloadModuleList()
{
    if (!m_service || !m_moduleList)
        return;

    const QString previousKey = m_currentModuleKey;
    m_service->reloadCatalog();

    m_moduleList->blockSignals(true);
    m_moduleList->clear();

    const AxisModuleMap groups = m_service->axesByModule();
    const bool showModuleNav   = showModuleNavigator(groups);
    m_moduleList->setVisible(showModuleNav);

    for (auto it = groups.constBegin(); it != groups.constEnd(); ++it) {
        auto *item = new QListWidgetItem(moduleDisplayName(it.key()), m_moduleList);
        item->setData(Qt::UserRole, it.key());
    }

    int selectRow = 0;
    if (!previousKey.isEmpty()) {
        for (int i = 0; i < m_moduleList->count(); ++i) {
            if (m_moduleList->item(i)->data(Qt::UserRole).toString() == previousKey) {
                selectRow = i;
                break;
            }
        }
    }

    if (groups.isEmpty()) {
        clearDebugPanels();
    } else if (showModuleNav) {
        m_moduleList->setCurrentRow(selectRow);
        onModuleSelected(m_moduleList->currentItem(), nullptr);
    } else {
        loadModule(groups.constBegin().key());
    }

    if (showModuleNav)
        m_moduleList->refreshAllRows();

    m_moduleList->blockSignals(false);
}

void MotionAxisDebugView::onModuleSelected(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    if (!current)
        return;
    loadModule(current->data(Qt::UserRole).toString());
}

void MotionAxisDebugView::loadModule(const QString &moduleKey)
{
    if (!m_service || moduleKey.isEmpty() || !m_panelGrid || !m_panelHost)
        return;

    m_currentModuleKey = moduleKey;
    clearDebugPanels();

    QStringList axes = m_service->axesByModule().value(moduleKey);
    if (axes.isEmpty())
        return;

    axes.sort(Qt::CaseInsensitive);

    int index = 0;
    for (const QString &axisName : axes) {
        auto *panel = new AxisDebugPanel(m_service, m_panelHost);
        panel->setAxisName(axisName);
        panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        panel->setMinimumWidth(320);
        m_panelGrid->addWidget(panel, index / kDebugGridColumns, index % kDebugGridColumns,
                               Qt::AlignTop);
        m_debugPanels.append(panel);
        ++index;
    }

    syncDebugGridStretch(m_panelGrid);
    refreshDebugPanels();
}

void MotionAxisDebugView::clearDebugPanels()
{
    for (AxisDebugPanel *panel : std::as_const(m_debugPanels)) {
        if (!panel)
            continue;
        m_panelGrid->removeWidget(panel);
        panel->deleteLater();
    }
    m_debugPanels.clear();
}

void MotionAxisDebugView::refreshDebugPanels()
{
    if (!isVisible())
        return;

    for (AxisDebugPanel *panel : std::as_const(m_debugPanels)) {
        if (panel)
            panel->refreshState();
    }
}

void MotionAxisDebugView::onShow()
{
    if (m_service)
        m_service->setAutoRefreshEnabled(true);
    reloadModuleList();
    refreshDebugPanels();
}

void MotionAxisDebugView::onHide()
{
    if (m_service)
        m_service->setAutoRefreshEnabled(false);
}

} // namespace motion_control_plugin
