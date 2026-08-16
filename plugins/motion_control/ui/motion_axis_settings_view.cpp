/**
 * @file motion_axis_settings_view.cpp
 * @brief 轴设置页：左导航 + 右 Splitter（参数表单 / 调试面板）
 *
 * 导航策略与 IO 页一致：模块数 > 1 显示分组树，否则隐藏树、用轴名列表。
 */

#include "motion_axis_settings_view.h"

#include "motion_action_keys.h"
#include "dialogs/motion_catalog_dialogs.h"

#include "widgets/axis/axis_param_form_widget.h"
#include "widgets/axis/axis_debug_panel.h"

#include "extensionsystem/pluginmanager.h"
#include "imotion_control_service.h"
#include "component/controls/button/push_button_style.h"
#include "component/controls/button/themed_push_button.h"
#include "component/dialog/themed_message_box.h"
#include "component/list/themed_list_widget.h"
#include "component/tree/themed_tree_widget.h"
#include "uibase/languagemanager.h"
#include "uibase/permission/permservice.h"
#include "uibase/thememanager.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QSplitter>
#include <QVBoxLayout>

namespace motion_control_plugin {

using ExtensionSystem::PluginManager;

namespace {

QString trKey(const char *key)
{
    return LanguageManager::instance()->t(QString::fromLatin1(key));
}

/** 多模块（>1）时用树形模块导航，否则用轴名列表 */
bool useModuleTreeNav(const AxisModuleMap &groups)
{
    return groups.size() > 1;
}

} // namespace

MotionAxisSettingsView::MotionAxisSettingsView(QWidget *parent)
    : PageWidget(parent)
{
    setPageId(QStringLiteral("motion.axis_settings"));
}

void MotionAxisSettingsView::onLoad()
{
    m_service = PluginManager::getObject<IMotionControlService>();
    buildLayout();
    bindPermissions();
    applyTheme();
    retranslateUi();
    applyPermissions();
    reloadAxisNav();

    if (m_service) {
        connect(m_service, &IMotionControlService::catalogChanged, this,
                &MotionAxisSettingsView::reloadAxisNav);
    }
}

void MotionAxisSettingsView::buildLayout()
{
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(12);

    m_leftNav = new QWidget(this);
    m_leftNav->setObjectName(QStringLiteral("motionAxisLeftNav"));
    auto *leftLay = new QVBoxLayout(m_leftNav);
    leftLay->setContentsMargins(0, 0, 0, 0);
    leftLay->setSpacing(8);

    m_tree = new ui::ThemedTreeWidget(m_leftNav);
    m_tree->setObjectName(QStringLiteral("motionAxisModuleTree"));
    m_tree->setMinimumWidth(200);
    m_tree->setMaximumWidth(280);

    m_axisList = ui::makeListWidget(m_leftNav);
    m_axisList->setObjectName(QStringLiteral("motionAxisList"));
    m_axisList->setMinimumWidth(200);
    m_axisList->setMaximumWidth(280);

    leftLay->addWidget(m_tree, 1);
    leftLay->addWidget(m_axisList, 1);

    auto *navBtnRow = new QHBoxLayout();
    navBtnRow->setSpacing(8);
    m_navBtnRow = new QWidget(m_leftNav);
    m_navBtnRow->setLayout(navBtnRow);
    m_addAxisBtn = ui::makePushButton(QString(), ui::PushButtonVariant::Secondary, m_navBtnRow);
    m_addAxisBtn->setProperty("trKey", QStringLiteral("motion.axis.add_btn"));
    m_deleteAxisBtn = ui::makePushButton(QString(), ui::PushButtonVariant::Secondary, m_navBtnRow);
    m_deleteAxisBtn->setProperty("trKey", QStringLiteral("motion.axis.delete_btn"));
    navBtnRow->addWidget(m_addAxisBtn);
    navBtnRow->addWidget(m_deleteAxisBtn);
    leftLay->addWidget(m_navBtnRow);

    m_rightSplitter = new QSplitter(Qt::Vertical, this);
    m_rightSplitter->setObjectName(QStringLiteral("motionAxisSettingsSplitter"));
    m_rightSplitter->setChildrenCollapsible(false);

    m_paramForm  = new AxisParamFormWidget(m_service, m_rightSplitter);
    m_debugPanel = new AxisDebugPanel(m_service, m_rightSplitter);
    m_debugPanel->setMinimumHeight(340);
    m_rightSplitter->addWidget(m_paramForm);
    m_rightSplitter->addWidget(m_debugPanel);
    m_rightSplitter->setStretchFactor(0, 3);
    m_rightSplitter->setStretchFactor(1, 2);
    m_rightSplitter->setSizes({450, 340});

    m_emptyPanel = new QWidget(this);
    m_emptyPanel->setObjectName(QStringLiteral("motionAxisEmptyPanel"));
    auto *emptyLay = new QVBoxLayout(m_emptyPanel);
    emptyLay->setContentsMargins(0, 0, 0, 0);
    emptyLay->addStretch();
    m_emptyLabel = new QLabel(m_emptyPanel);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setWordWrap(true);
    emptyLay->addWidget(m_emptyLabel);
    auto *emptyBtnRow = new QHBoxLayout();
    emptyBtnRow->addStretch();
    m_emptyAddBtn = ui::makePushButton(QString(), ui::PushButtonVariant::Primary, m_emptyPanel);
    m_emptyAddBtn->setProperty("trKey", QStringLiteral("motion.axis.add_btn"));
    emptyBtnRow->addWidget(m_emptyAddBtn);
    emptyBtnRow->addStretch();
    emptyLay->addLayout(emptyBtnRow);
    emptyLay->addStretch();
    connect(m_emptyAddBtn, &QPushButton::clicked, this, &MotionAxisSettingsView::onAddAxisClicked);

    root->addWidget(m_leftNav);
    root->addWidget(m_rightSplitter, 1);
    root->addWidget(m_emptyPanel, 1);

    connect(m_tree, &ui::ThemedTreeWidget::itemSelected, this,
            &MotionAxisSettingsView::onAxisSelected);
    connect(m_axisList, &QListWidget::currentItemChanged, this,
            &MotionAxisSettingsView::onAxisListSelected);
    connect(m_addAxisBtn, &QPushButton::clicked, this, &MotionAxisSettingsView::onAddAxisClicked);
    connect(m_deleteAxisBtn, &QPushButton::clicked, this,
            &MotionAxisSettingsView::onDeleteAxisClicked);
    connect(m_paramForm, &AxisParamFormWidget::configSaved, m_debugPanel,
            &AxisDebugPanel::refreshState);
    if (m_service) {
        connect(m_service, &IMotionControlService::axisStatesUpdated, m_debugPanel,
                &AxisDebugPanel::refreshState, Qt::UniqueConnection);
    }
}

void MotionAxisSettingsView::bindPermissions()
{
    connect(PermService::instance(), &PermService::permissionsChanged, this,
            &MotionAxisSettingsView::applyPermissions);
}

void MotionAxisSettingsView::applyPermissions()
{
    const bool canAdd    = PermService::instance()->canAction(MotionActionKeys::AxisAdd);
    const bool canDelete = PermService::instance()->canAction(MotionActionKeys::AxisDelete);
    const bool showDelete = canDelete && !m_currentAxisName.isEmpty();

    if (m_addAxisBtn)
        m_addAxisBtn->setVisible(canAdd);
    if (m_deleteAxisBtn)
        m_deleteAxisBtn->setVisible(showDelete);
    if (m_navBtnRow)
        m_navBtnRow->setVisible(canAdd || showDelete);
    if (m_emptyAddBtn)
        m_emptyAddBtn->setVisible(canAdd);
}

void MotionAxisSettingsView::applyTheme()
{
    setObjectName(QStringLiteral("motionAxisSettingsRoot"));
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QStringLiteral("#motionAxisSettingsRoot { background: transparent; }"));
}

void MotionAxisSettingsView::retranslateUi()
{
    auto applyBtnText = [](QPushButton *btn) {
        if (!btn)
            return;
        const QVariant key = btn->property("trKey");
        if (key.isValid())
            btn->setText(trKey(key.toString().toLatin1().constData()));
    };
    applyBtnText(m_addAxisBtn);
    applyBtnText(m_deleteAxisBtn);
    applyBtnText(m_emptyAddBtn);
    if (m_emptyLabel)
        m_emptyLabel->setText(trKey("motion.axis.empty_hint"));
}

void MotionAxisSettingsView::reloadAxisNav()
{
    if (!m_service || !m_tree || !m_axisList)
        return;

    m_service->reloadCatalog();
    const AxisModuleMap groups = m_service->axesByModule();

    const bool hasAxis = !groups.isEmpty();
    if (m_leftNav)
        m_leftNav->setVisible(hasAxis);
    if (m_rightSplitter)
        m_rightSplitter->setVisible(hasAxis);
    if (m_emptyPanel)
        m_emptyPanel->setVisible(!hasAxis);

    if (!hasAxis) {
        m_currentAxisName.clear();
        applyPermissions();
        return;
    }

    if (useModuleTreeNav(groups)) {
        m_tree->setVisible(true);
        m_axisList->setVisible(false);
        m_tree->setGroupedItems(groups);

        const QStringList allAxes = m_service->axisNames();
        if (!m_currentAxisName.isEmpty() && allAxes.contains(m_currentAxisName, Qt::CaseInsensitive)) {
            m_tree->selectItem(m_currentAxisName);
        } else if (!allAxes.isEmpty()) {
            onAxisSelected(QString(), allAxes.first());
        }
        applyPermissions();
        return;
    }

    m_tree->setVisible(false);
    m_axisList->setVisible(true);

    const QString moduleKey = groups.constBegin().key();
    const QStringList axes  = groups.constBegin().value();

    m_axisList->blockSignals(true);
    m_axisList->clear();
    for (const QString &axisName : axes) {
        auto *item = new QListWidgetItem(axisName, m_axisList);
        item->setData(Qt::UserRole, moduleKey);
    }

    int selectRow = 0;
    if (!m_currentAxisName.isEmpty()) {
        for (int i = 0; i < m_axisList->count(); ++i) {
            if (m_axisList->item(i)->text().compare(m_currentAxisName, Qt::CaseInsensitive) == 0) {
                selectRow = i;
                break;
            }
        }
    }

    if (m_axisList->count() > 0) {
        m_axisList->setCurrentRow(selectRow);
        onAxisListSelected(m_axisList->currentItem(), nullptr);
    }

    m_axisList->refreshAllRows();
    m_axisList->blockSignals(false);
    applyPermissions();
}

void MotionAxisSettingsView::onAxisListSelected(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    if (!current)
        return;
    onAxisSelected(current->data(Qt::UserRole).toString(), current->text());
}

void MotionAxisSettingsView::onAxisSelected(const QString &moduleName, const QString &axisName)
{
    Q_UNUSED(moduleName);
    m_currentAxisName = axisName;
    if (m_paramForm)
        m_paramForm->loadAxis(axisName);
    if (m_debugPanel)
        m_debugPanel->setAxisName(axisName);
    applyPermissions();
}

QString MotionAxisSettingsView::currentAxisName() const
{
    return m_currentAxisName;
}

void MotionAxisSettingsView::onAddAxisClicked()
{
    if (!PermService::instance()->canAction(MotionActionKeys::AxisAdd) || !m_service)
        return;

    QString axisName;
    QString moduleName;
    if (!promptNameModuleDialog(this,
                                QStringLiteral("motion.axis.add_dialog.title"),
                                QStringLiteral("motion.axis.add_dialog.name_label"),
                                QStringLiteral("motion.axis.add_dialog.module_label"),
                                QStringLiteral("motion.axis.add_dialog.name_placeholder"),
                                QStringLiteral("motion.axis.add_dialog.module_placeholder"),
                                QString(),
                                QString(),
                                &axisName,
                                &moduleName)) {
        return;
    }

    QString errorKey;
    if (!m_service->addAxis(axisName, moduleName, &errorKey)) {
        if (!errorKey.isEmpty()) {
            ui::ThemedMessageBox::information(this, trKey("motion.axis.add_dialog.title"),
                                              trKey(errorKey.toLatin1().constData()));
        }
        return;
    }

    m_currentAxisName = axisName;
    reloadAxisNav();
}

void MotionAxisSettingsView::onDeleteAxisClicked()
{
    if (!PermService::instance()->canAction(MotionActionKeys::AxisDelete) || !m_service)
        return;

    const QString axisName = currentAxisName();
    if (axisName.isEmpty())
        return;

    const QString text = trKey("motion.axis.delete_confirm").arg(axisName);
    if (!ui::ThemedMessageBox::ask(this, trKey("motion.axis.delete_btn"), text,
                                   trKey("motion.dialog.confirm"),
                                   trKey("motion.dialog.cancel"))) {
        return;
    }

    QString errorKey;
    if (!m_service->removeAxis(axisName, &errorKey)) {
        if (!errorKey.isEmpty()) {
            ui::ThemedMessageBox::information(this, trKey("motion.axis.delete_btn"),
                                              trKey(errorKey.toLatin1().constData()));
        }
        return;
    }

    m_currentAxisName.clear();
    reloadAxisNav();
}

void MotionAxisSettingsView::onShow()
{
    if (m_service)
        m_service->setAutoRefreshEnabled(true);
    reloadAxisNav();
    applyPermissions();
    if (m_debugPanel)
        m_debugPanel->refreshState();
}

void MotionAxisSettingsView::onHide()
{
    if (m_service)
        m_service->setAutoRefreshEnabled(false);
}

} // namespace motion_control_plugin
