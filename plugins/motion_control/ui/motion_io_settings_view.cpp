/**
 * @file motion_io_settings_view.cpp
 * @brief IO 设置页实现：左导航 + 双栏 Section + 自适应列数网格卡片
 */

#include "motion_io_settings_view.h"

#include "motion_action_keys.h"
#include "dialogs/motion_catalog_dialogs.h"

#include "widgets/io/io_param_form_widget.h"

#include "extensionsystem/pluginmanager.h"
#include "imotion_control_service.h"
#include "component/controls/button/push_button_style.h"
#include "component/controls/button/themed_push_button.h"
#include "component/dialog/themed_message_box.h"
#include "component/group/themed_section_panel.h"
#include "component/list/themed_list_widget.h"
#include "uibase/languagemanager.h"
#include "uibase/permission/permservice.h"
#include "uibase/thememanager.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidgetItem>
#include <QScrollArea>
#include <QVBoxLayout>

namespace motion_control_plugin {

using ExtensionSystem::PluginManager;

namespace {

constexpr int kIoGridColumnsWithNav    = 2;
constexpr int kIoGridColumnsFullWidth  = 3;
constexpr int kIoGridColumnsMax        = 3;

QString trKey(const char *key)
{
    return LanguageManager::instance()->t(QString::fromLatin1(key));
}

QScrollArea *makeIoGridScrollHost(QWidget **outHost, QGridLayout **outGrid, QWidget *parent)
{
    auto *scroll = new QScrollArea(parent);
    scroll->setObjectName(QStringLiteral("motionIoParamScroll"));
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setAttribute(Qt::WA_StyledBackground, true);
    scroll->setStyleSheet(QStringLiteral("QScrollArea#motionIoParamScroll { background: transparent; border: none; }"));
    scroll->viewport()->setAttribute(Qt::WA_StyledBackground, true);
    scroll->viewport()->setAutoFillBackground(false);
    scroll->viewport()->setStyleSheet(QStringLiteral("background: transparent;"));

    auto *host = new QWidget(scroll);
    host->setObjectName(QStringLiteral("motionIoParamHost"));
    host->setAttribute(Qt::WA_StyledBackground, false);
    host->setAutoFillBackground(false);
    auto *grid = new QGridLayout(host);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setHorizontalSpacing(8);
    grid->setVerticalSpacing(8);
    grid->setAlignment(Qt::AlignTop);

    scroll->setWidget(host);
    *outHost = host;
    *outGrid = grid;
    return scroll;
}

bool showModuleNavigator(const IoModuleMap &groups)
{
    return groups.size() > 1;
}

void addFormToGrid(QGridLayout *grid, IoParamFormWidget *form, int index, int columnCount)
{
    if (!grid || !form || columnCount <= 0)
        return;
    const int row = index / columnCount;
    const int col = index % columnCount;
    form->setMinimumWidth(180);
    form->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    grid->addWidget(form, row, col, Qt::AlignTop);
}

void syncIoGridStretch(QGridLayout *grid, int itemCount, int columnCount)
{
    if (!grid || itemCount <= 0 || columnCount <= 0)
        return;

    for (int c = 0; c < columnCount; ++c)
        grid->setColumnStretch(c, 1);
    for (int c = columnCount; c < kIoGridColumnsMax; ++c)
        grid->setColumnStretch(c, 0);
}

} // namespace

MotionIoSettingsView::MotionIoSettingsView(QWidget *parent)
    : PageWidget(parent)
{
    setPageId(QStringLiteral("motion.io_settings"));
}

void MotionIoSettingsView::onLoad()
{
    m_service = PluginManager::getObject<IMotionControlService>();
    buildLayout();
    bindPermissions();
    applyTheme();
    retranslateUi();
    applyPermissions();
    reloadModuleList();

    if (m_service) {
        connect(m_service, &IMotionControlService::ioStatesUpdated, this,
                &MotionIoSettingsView::refreshIoFormStates);
        connect(m_service, &IMotionControlService::catalogChanged, this,
                &MotionIoSettingsView::reloadModuleList);
    }
}

void MotionIoSettingsView::buildLayout()
{
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(12);

    m_leftNav = new QWidget(this);
    m_leftNav->setObjectName(QStringLiteral("motionIoLeftNav"));
    auto *leftLay = new QVBoxLayout(m_leftNav);
    leftLay->setContentsMargins(0, 0, 0, 0);
    leftLay->setSpacing(8);

    m_moduleList = ui::makeListWidget(m_leftNav);
    m_moduleList->setObjectName(QStringLiteral("motionIoModuleList"));
    m_moduleList->setMinimumWidth(180);
    m_moduleList->setMaximumWidth(240);

    leftLay->addWidget(m_moduleList, 1);

    m_rightPanel = new QWidget(this);
    m_rightPanel->setObjectName(QStringLiteral("motionIoRightPanel"));
    m_rightPanel->setAttribute(Qt::WA_StyledBackground, false);
    m_rightPanel->setAutoFillBackground(false);
    auto *rightLay = new QVBoxLayout(m_rightPanel);
    rightLay->setContentsMargins(0, 0, 0, 0);
    rightLay->setSpacing(10);

    auto *columns = new QHBoxLayout();
    columns->setSpacing(12);

    m_inputPanel = new ui::ThemedSectionPanel(QStringLiteral("motion.io.input_section"), m_rightPanel);
    m_inputPanel->setContentMargins(8, 8, 8, 8);
    m_inputPanel->setContentSpacing(8);
    auto *inputScroll = makeIoGridScrollHost(&m_inputHost, &m_inputGrid, m_inputPanel->contentHost());
    m_inputPanel->contentLayout()->addWidget(inputScroll, 1);

    m_outputPanel = new ui::ThemedSectionPanel(QStringLiteral("motion.io.output_section"), m_rightPanel);
    m_outputPanel->setContentMargins(8, 8, 8, 8);
    m_outputPanel->setContentSpacing(8);
    auto *outputScroll = makeIoGridScrollHost(&m_outputHost, &m_outputGrid, m_outputPanel->contentHost());
    m_outputPanel->contentLayout()->addWidget(outputScroll, 1);

    columns->addWidget(m_inputPanel, 1);
    columns->addWidget(m_outputPanel, 1);
    rightLay->addLayout(columns, 1);

    auto *btnRow = new QHBoxLayout();
    btnRow->setSpacing(8);
    btnRow->addStretch();
    m_addIoBtn = ui::makePushButton(QString(), ui::PushButtonVariant::Secondary, m_rightPanel);
    m_addIoBtn->setProperty("trKey", QStringLiteral("motion.io.add_btn"));
    m_deleteIoBtn = ui::makePushButton(QString(), ui::PushButtonVariant::Secondary, m_rightPanel);
    m_deleteIoBtn->setProperty("trKey", QStringLiteral("motion.io.delete_btn"));
    m_saveAllBtn = ui::makePushButton(QString(), ui::PushButtonVariant::Primary, m_rightPanel);
    connect(m_addIoBtn, &QPushButton::clicked, this, &MotionIoSettingsView::onAddIoClicked);
    connect(m_deleteIoBtn, &QPushButton::clicked, this, &MotionIoSettingsView::onDeleteIoClicked);
    connect(m_saveAllBtn, &QPushButton::clicked, this, &MotionIoSettingsView::onSaveAllClicked);
    btnRow->addWidget(m_addIoBtn);
    btnRow->addWidget(m_deleteIoBtn);
    btnRow->addWidget(m_saveAllBtn);
    rightLay->addLayout(btnRow);

    m_emptyPanel = new QWidget(this);
    m_emptyPanel->setObjectName(QStringLiteral("motionIoEmptyPanel"));
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
    m_emptyAddBtn->setProperty("trKey", QStringLiteral("motion.io.add_btn"));
    emptyBtnRow->addWidget(m_emptyAddBtn);
    emptyBtnRow->addStretch();
    emptyLay->addLayout(emptyBtnRow);
    emptyLay->addStretch();
    connect(m_emptyAddBtn, &QPushButton::clicked, this, &MotionIoSettingsView::onAddIoClicked);

    root->addWidget(m_leftNav);
    root->addWidget(m_rightPanel, 1);
    root->addWidget(m_emptyPanel, 1);

    connect(m_moduleList, &QListWidget::currentItemChanged, this,
            &MotionIoSettingsView::onModuleSelected);
}

void MotionIoSettingsView::bindPermissions()
{
    connect(PermService::instance(), &PermService::permissionsChanged, this,
            &MotionIoSettingsView::applyPermissions);
}

void MotionIoSettingsView::applyPermissions()
{
    const bool canEdit   = PermService::instance()->canAction(MotionActionKeys::IoEdit);
    const bool canWrite  = PermService::instance()->canAction(MotionActionKeys::IoWrite);
    const bool canAdd    = PermService::instance()->canAction(MotionActionKeys::IoAdd);
    const bool canDelete = PermService::instance()->canAction(MotionActionKeys::IoDelete);

    if (m_saveAllBtn)
        m_saveAllBtn->setEnabled(canEdit);
    if (m_deleteIoBtn) {
        m_deleteIoBtn->setVisible(canDelete);
        m_deleteIoBtn->setEnabled(!m_selectedIoName.isEmpty());
    }
    if (m_addIoBtn)
        m_addIoBtn->setVisible(canAdd);
    if (m_emptyAddBtn)
        m_emptyAddBtn->setVisible(canAdd);
    if (m_leftNav && m_moduleList && m_rightPanel) {
        const bool hasIo         = m_rightPanel->isVisible();
        const bool showModuleNav = m_moduleList->isVisible();
        m_leftNav->setVisible(hasIo && showModuleNav);
    }

    for (IoParamFormWidget *form : std::as_const(m_ioForms)) {
        if (!form)
            continue;
        form->setDetailsVisible(canEdit);
        form->setOutputClickEnabled(canWrite);
    }
}

void MotionIoSettingsView::applyTheme()
{
    setObjectName(QStringLiteral("motionIoSettingsRoot"));
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QStringLiteral("#motionIoSettingsRoot { background: transparent; }"));
}

void MotionIoSettingsView::retranslateUi()
{
    if (m_saveAllBtn)
        m_saveAllBtn->setText(trKey("motion.io.save_all"));
    auto applyBtnText = [](QPushButton *btn) {
        if (!btn)
            return;
        const QVariant key = btn->property("trKey");
        if (key.isValid())
            btn->setText(trKey(key.toString().toLatin1().constData()));
    };
    applyBtnText(m_addIoBtn);
    applyBtnText(m_emptyAddBtn);
    applyBtnText(m_deleteIoBtn);
    if (m_emptyLabel)
        m_emptyLabel->setText(trKey("motion.io.empty_hint"));
}

void MotionIoSettingsView::onHide()
{
    if (m_service)
        m_service->setAutoRefreshEnabled(false);
}

QString MotionIoSettingsView::moduleDisplayName(const QString &moduleKey)
{
    if (moduleKey.isEmpty() || moduleKey == QStringLiteral("_default"))
        return trKey("motion.module.default");
    return moduleKey;
}

void MotionIoSettingsView::reloadModuleList()
{
    if (!m_service || !m_moduleList)
        return;

    const QString previousModule = m_currentModuleKey;
    m_service->reloadCatalog();
    m_cardNames = m_service->motionCardNames();

    const IoModuleMap groups = m_service->iosByModule();
    const bool hasIo         = !groups.isEmpty();

    if (m_leftNav)
        m_leftNav->setVisible(hasIo);
    if (m_rightPanel)
        m_rightPanel->setVisible(hasIo);
    if (m_emptyPanel)
        m_emptyPanel->setVisible(!hasIo);

    if (!hasIo) {
        m_currentModuleKey.clear();
        clearIoForms();
        applyPermissions();
        return;
    }

    m_moduleList->blockSignals(true);
    m_moduleList->clear();

    const bool showModuleNav = showModuleNavigator(groups);
    m_moduleList->setVisible(showModuleNav);

    for (auto it = groups.constBegin(); it != groups.constEnd(); ++it) {
        auto *item = new QListWidgetItem(moduleDisplayName(it.key()), m_moduleList);
        item->setData(Qt::UserRole, it.key());
    }

    int selectModuleRow = 0;
    if (!previousModule.isEmpty()) {
        for (int i = 0; i < m_moduleList->count(); ++i) {
            if (m_moduleList->item(i)->data(Qt::UserRole).toString() == previousModule) {
                selectModuleRow = i;
                break;
            }
        }
    }

    QString moduleKey;
    if (showModuleNav) {
        m_moduleList->setCurrentRow(selectModuleRow);
        moduleKey = m_moduleList->currentItem()
                        ? m_moduleList->currentItem()->data(Qt::UserRole).toString()
                        : groups.constBegin().key();
    } else {
        moduleKey = groups.constBegin().key();
    }

    m_moduleList->blockSignals(false);

    m_currentModuleKey = moduleKey;
    loadModule(moduleKey);
    applyPermissions();

    if (showModuleNav)
        m_moduleList->refreshAllRows();
}

void MotionIoSettingsView::onModuleSelected(QListWidgetItem *current, QListWidgetItem *previous)
{
    Q_UNUSED(previous);
    if (!current)
        return;
    m_currentModuleKey = current->data(Qt::UserRole).toString();
    loadModule(m_currentModuleKey);
    applyPermissions();
}

void MotionIoSettingsView::loadModule(const QString &moduleKey)
{
    if (!m_service || moduleKey.isEmpty() || !m_inputGrid || !m_outputGrid)
        return;

    m_currentModuleKey = moduleKey;
    clearIoForms();

    const QStringList ioNames = m_service->iosByModule().value(moduleKey);
    if (ioNames.isEmpty())
        return;

    const int columnCount = ioGridColumnCount();

    QStringList sorted = ioNames;
    sorted.sort(Qt::CaseInsensitive);

    const bool canEdit  = PermService::instance()->canAction(MotionActionKeys::IoEdit);
    const bool canWrite = PermService::instance()->canAction(MotionActionKeys::IoWrite);

    int inputIndex  = 0;
    int outputIndex = 0;

    for (const QString &ioName : sorted) {
        const IoConfigData cfg = m_service->ioConfig(ioName);
        QWidget *parentHost = cfg.isOutput ? m_outputHost : m_inputHost;
        auto    *form       = new IoParamFormWidget(m_service, parentHost);
        form->setCardNameOptions(m_cardNames);
        form->loadConfig(cfg);
        form->setDetailsVisible(canEdit);
        form->setOutputClickEnabled(canWrite);
        connect(form, &IoParamFormWidget::cardSelected, this,
                [this, form]() { setSelectedIoForm(form); });

        if (cfg.isOutput) {
            addFormToGrid(m_outputGrid, form, outputIndex++, columnCount);
        } else {
            addFormToGrid(m_inputGrid, form, inputIndex++, columnCount);
        }
        m_ioForms.append(form);
    }

    syncIoGridStretch(m_inputGrid, inputIndex, columnCount);
    syncIoGridStretch(m_outputGrid, outputIndex, columnCount);

    if (!m_ioForms.isEmpty())
        setSelectedIoForm(m_ioForms.first());

    refreshIoFormStates();
}

int MotionIoSettingsView::ioGridColumnCount() const
{
    if (m_leftNav && m_leftNav->isVisible())
        return kIoGridColumnsWithNav;
    return kIoGridColumnsFullWidth;
}

void MotionIoSettingsView::clearIoForms()
{
    m_selectedIoName.clear();
    for (IoParamFormWidget *form : std::as_const(m_ioForms)) {
        if (form)
            form->deleteLater();
    }
    m_ioForms.clear();
}

void MotionIoSettingsView::setSelectedIoForm(IoParamFormWidget *form)
{
    m_selectedIoName = form ? form->ioName() : QString();
    for (IoParamFormWidget *item : std::as_const(m_ioForms)) {
        if (!item)
            continue;
        item->setSelected(item == form);
    }
    applyPermissions();
}

void MotionIoSettingsView::refreshIoFormStates()
{
    if (!m_service)
        return;

    for (IoParamFormWidget *form : std::as_const(m_ioForms)) {
        if (!form)
            continue;
        const IoRuntimeState state = m_service->ioState(form->ioName());
        form->refreshRuntimeState(state.value);
    }
}

void MotionIoSettingsView::onSaveAllClicked()
{
    if (!PermService::instance()->canAction(MotionActionKeys::IoEdit))
        return;
    if (!m_service || m_currentModuleKey.isEmpty())
        return;

    for (IoParamFormWidget *form : std::as_const(m_ioForms)) {
        if (!form)
            continue;
        if (!m_service->setIoConfig(form->ioName(), form->collectConfig()))
            return;
    }
    m_service->saveDeviceConfig();
    reloadModuleList();
}

void MotionIoSettingsView::onAddIoClicked()
{
    if (!PermService::instance()->canAction(MotionActionKeys::IoAdd) || !m_service)
        return;

    QString ioName;
    QString moduleName;
    if (!promptNameModuleDialog(this,
                                QStringLiteral("motion.io.add_dialog.title"),
                                QStringLiteral("motion.io.add_dialog.name_label"),
                                QStringLiteral("motion.io.add_dialog.module_label"),
                                QStringLiteral("motion.io.add_dialog.name_placeholder"),
                                QStringLiteral("motion.io.add_dialog.module_placeholder"),
                                QString(),
                                m_currentModuleKey == QStringLiteral("_default") ? QString()
                                                                                   : m_currentModuleKey,
                                &ioName,
                                &moduleName)) {
        return;
    }

    QString errorKey;
    if (!m_service->addIo(ioName, moduleName, &errorKey)) {
        if (!errorKey.isEmpty()) {
            ui::ThemedMessageBox::information(this, trKey("motion.io.add_dialog.title"),
                                              trKey(errorKey.toLatin1().constData()));
        }
        return;
    }

    m_currentModuleKey = moduleName.isEmpty() ? QStringLiteral("_default") : moduleName;
    reloadModuleList();
}

void MotionIoSettingsView::onDeleteIoClicked()
{
    if (m_selectedIoName.isEmpty())
        return;
    onDeleteIoRequested(m_selectedIoName);
}

void MotionIoSettingsView::onDeleteIoRequested(const QString &ioName)
{
    if (!PermService::instance()->canAction(MotionActionKeys::IoDelete) || !m_service)
        return;

    if (ioName.isEmpty())
        return;

    const QString text = trKey("motion.io.delete_confirm").arg(ioName);
    if (!ui::ThemedMessageBox::ask(this, trKey("motion.io.delete_btn"), text,
                                   trKey("motion.dialog.confirm"),
                                   trKey("motion.dialog.cancel"))) {
        return;
    }

    QString errorKey;
    if (!m_service->removeIo(ioName, &errorKey)) {
        if (!errorKey.isEmpty()) {
            ui::ThemedMessageBox::information(this, trKey("motion.io.delete_btn"),
                                              trKey(errorKey.toLatin1().constData()));
        }
        return;
    }

    reloadModuleList();
}

void MotionIoSettingsView::onShow()
{
    if (m_service)
        m_service->setAutoRefreshEnabled(true);
    reloadModuleList();
    applyPermissions();
    refreshIoFormStates();
}

} // namespace motion_control_plugin
