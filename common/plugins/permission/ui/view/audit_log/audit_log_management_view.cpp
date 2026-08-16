#include "ui/view/audit_log/audit_log_management_view.h"

#include "ui/view/audit_log/audit_log_result_delegate.h"
#include "ui/view/audit_log/audit_log_table_model.h"
#include "ui/view/message_popup.h"
#include "ui/view/perm_combobox.h"
#include "ui/view/perm_datetime_edit.h"
#include "ui/view/perm_datetime_utils.h"
#include "ui/view/perm_text.h"
#include "ui/view/permission_ui_utils.h"
#include "ui/vm/audit_log_management_view_model.h"

#include "service/perm_action_keys.h"
#include "uibase/permission/permservice.h"

#include "databinding.h"

#include "controls/button/themed_push_button.h"
#include "controls/input/themed_line_edit.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>

namespace permission {
namespace {

enum RangePresetId {
    PresetLast30Days = 0,
    PresetLast7Days,
    PresetToday,
    PresetYesterday,
    PresetUnlimited,
};

} // namespace

AuditLogManagementView::AuditLogManagementView(QWidget *parent)
    : PageWidget(parent)
{
}

void AuditLogManagementView::applyTheme()
{
    permission_ui::applyPermissionTheme(this);
}

void AuditLogManagementView::onLoad()
{
    permission_ui::applyPermissionTheme(this);

    m_viewModel = new AuditLogManagementViewModel(nullptr, this);

    buildLayout();
    setupBindings();

    retranslateUi();
    syncDraftFiltersToView();
    m_viewModel->refreshList();
}

void AuditLogManagementView::onShow()
{
    if (m_viewModel)
        m_viewModel->refreshList();
}

void AuditLogManagementView::buildLayout()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(12);

    m_pageTitle = new QLabel(this);
    permission_ui::initPageTitle(m_pageTitle, true);

    auto *filterBar = new QFrame(this);
    permission_ui::initFilterToolbar(filterBar);
    auto *filterRow = new QHBoxLayout(filterBar);
    filterRow->setContentsMargins(0, 2, 0, 4);
    filterRow->setSpacing(8);
    filterRow->setAlignment(Qt::AlignVCenter);

    m_operatorEdit = new ui::ThemedLineEdit(filterBar);
    m_operatorEdit->setMinimumWidth(120);
    permission_ui::initSearchField(m_operatorEdit);

    m_actionFilter = new permission_ui::PermComboBox(filterBar);
    m_actionFilter->setMinimumWidth(118);
    permission_ui::initComboBox(m_actionFilter);

    m_rangePreset = new permission_ui::PermComboBox(filterBar);
    m_rangePreset->setMinimumWidth(100);
    permission_ui::initComboBox(m_rangePreset);

    m_timeFrom = new permission_ui::PermDateTimeEdit(filterBar);
    m_lblDateSep = new QLabel(QStringLiteral("→"), filterBar);
    m_lblDateSep->setObjectName(QStringLiteral("permAuditDateSep"));
    m_lblDateSep->setAlignment(Qt::AlignCenter);
    m_timeTo = new permission_ui::PermDateTimeEdit(filterBar);
    permission_ui::initPermDateTimeEdit(m_timeFrom);
    permission_ui::initPermDateTimeEdit(m_timeTo);
    m_timeFrom->setNullable(true);
    m_timeTo->setNullable(true);
    m_timeFrom->setEndOfRange(false);
    m_timeTo->setEndOfRange(true);
    m_timeFrom->setResolution(permission_ui::PermDateTimeResolution::Hour);
    m_timeTo->setResolution(permission_ui::PermDateTimeResolution::Hour);
    m_timeFrom->setPlaceholderWhenEmpty(PTXT("perm.common.any"));
    m_timeTo->setPlaceholderWhenEmpty(PTXT("perm.common.any"));

    m_btnSearch = permission_ui::makePrimaryButton(QString(), filterBar);
    m_btnReset  = permission_ui::makeSecondaryButton(QString(), filterBar);
    m_btnExport = permission_ui::makeSecondaryButton(QString(), filterBar);

    m_timeFrom->setFixedWidth(158);
    m_timeTo->setFixedWidth(158);

    filterRow->addWidget(m_operatorEdit);
    filterRow->addWidget(m_actionFilter);
    filterRow->addWidget(m_rangePreset);
    filterRow->addWidget(m_timeFrom);
    filterRow->addWidget(m_lblDateSep);
    filterRow->addWidget(m_timeTo);
    filterRow->addStretch();
    filterRow->addWidget(m_btnSearch);
    filterRow->addWidget(m_btnReset);
    filterRow->addWidget(m_btnExport);

    m_tableModel = new AuditLogTableModel(this);
    m_tableView  = new QTableView(this);
    m_tableView->setModel(m_tableModel);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    permission_ui::initAuditLogTableView(m_tableView);
    m_tableView->setItemDelegateForColumn(AuditLogTableModel::ColResult,
                                          new AuditLogResultDelegate(m_tableView));

    auto *footer = new QWidget(this);
    permission_ui::initFooter(footer);
    auto *footerLay = new QHBoxLayout(footer);
    footerLay->setSpacing(10);

    m_totalLabel  = new QLabel(footer);
    m_pageLabel   = new QLabel(footer);
    permission_ui::initMutedLabel(m_totalLabel);
    m_btnPrevPage = permission_ui::makeSecondaryButton(QString(), footer);
    m_btnNextPage = permission_ui::makeSecondaryButton(QString(), footer);

    footerLay->addWidget(m_totalLabel);
    footerLay->addStretch();
    footerLay->addWidget(m_btnPrevPage);
    footerLay->addWidget(m_pageLabel);
    footerLay->addWidget(m_btnNextPage);

    root->addWidget(m_pageTitle);
    root->addWidget(filterBar);
    root->addWidget(m_tableView, 1);
    root->addWidget(footer);
}

void AuditLogManagementView::setupBindings()
{
    DataBinding::bind(m_viewModel, AuditLogManagementViewModel::PROP_DRAFT_OPERATOR_NAME,
                      m_operatorEdit);
    DataBinding::bindCommand(m_viewModel->searchCommand(), m_btnSearch);
    DataBinding::bindCommand(m_viewModel->resetFilterCommand(), m_btnReset);
    DataBinding::bindCommand(m_viewModel->exportCommand(), m_btnExport);
    DataBinding::bindCommand(m_viewModel->prevPageCommand(), m_btnPrevPage);
    DataBinding::bindCommand(m_viewModel->nextPageCommand(), m_btnNextPage);

    connect(m_actionFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) {
                if (!m_viewModel || m_syncingPickers)
                    return;
                m_viewModel->setDraftActionType(m_actionFilter->itemData(index).toString());
            });

    connect(m_rangePreset, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &AuditLogManagementView::applyRangePreset);

    connect(m_timeFrom, &permission_ui::PermDateTimeEdit::dateTimeChanged, this,
            &AuditLogManagementView::onPickerTimeFromChanged);
    connect(m_timeTo, &permission_ui::PermDateTimeEdit::dateTimeChanged, this,
            &AuditLogManagementView::onPickerTimeToChanged);

    connect(m_viewModel, &AuditLogManagementViewModel::draftTimeFromChanged, this,
            &AuditLogManagementView::syncDraftTimePickersToView);
    connect(m_viewModel, &AuditLogManagementViewModel::draftTimeToChanged, this,
            &AuditLogManagementView::syncDraftTimePickersToView);
    connect(m_viewModel, &AuditLogManagementViewModel::draftActionTypeChanged, this,
            &AuditLogManagementView::syncActionTypeCombo);
    connect(m_viewModel, &AuditLogManagementViewModel::filtersReset, this,
            &AuditLogManagementView::syncDraftFiltersToView);

    connect(m_viewModel, &AuditLogManagementViewModel::logListChanged, this,
            &AuditLogManagementView::syncTable);
    connect(m_viewModel, &AuditLogManagementViewModel::totalCountChanged, this,
            &AuditLogManagementView::syncPagerLabel);
    connect(m_viewModel, &AuditLogManagementViewModel::pageIndexChanged, this,
            &AuditLogManagementView::syncPagerLabel);

    connect(m_viewModel, &AuditLogManagementViewModel::showInfo, this,
            [this](const QString &title, const QString &message) {
                permission_ui::MessagePopup::show(this, title, message, QDialogButtonBox::Ok);
            });
    connect(m_viewModel, &AuditLogManagementViewModel::showWarning, this,
            [this](const QString &title, const QString &message) {
                permission_ui::MessagePopup::show(this, title.isEmpty() ? PTXT("perm.common.notice") : title,
                                                 message, QDialogButtonBox::Ok);
            });

    connect(m_tableView, &QTableView::doubleClicked, this, [this](const QModelIndex &index) {
        if (!index.isValid() || !m_tableModel)
            return;
        if (!PS->canAction(permission::PermKeys::AuditView))
            return;
        showLogDetail(m_tableModel->logIdAt(index.row()));
    });

    syncActionTypeCombo();
    syncRangePresetCombo();
}

void AuditLogManagementView::onPickerTimeFromChanged(const QDateTime &value)
{
    if (!m_viewModel || m_syncingPickers)
        return;
    m_viewModel->setDraftTimeFrom(value);
}

void AuditLogManagementView::onPickerTimeToChanged(const QDateTime &value)
{
    if (!m_viewModel || m_syncingPickers)
        return;
    m_viewModel->setDraftTimeTo(value);
}

void AuditLogManagementView::syncDraftTimePickersToView()
{
    if (!m_viewModel)
        return;

    m_syncingPickers = true;

    if (m_timeFrom) {
        if (m_viewModel->draftTimeFrom().isValid())
            m_timeFrom->setDateTime(m_viewModel->draftTimeFrom());
        else
            m_timeFrom->clearDateTime();
    }

    if (m_timeTo) {
        if (m_viewModel->draftTimeTo().isValid())
            m_timeTo->setDateTime(m_viewModel->draftTimeTo());
        else
            m_timeTo->clearDateTime();
    }

    m_syncingPickers = false;
}

void AuditLogManagementView::applyRangePreset(int presetIndex)
{
    if (!m_viewModel || m_syncingPickers)
        return;

    const int preset = m_rangePreset->itemData(presetIndex).toInt();
    permission_ui::PermDateTimeRange range;
    switch (preset) {
    case PresetLast7Days:
        range = permission_ui::permDateTimeRangeLast7Days();
        break;
    case PresetToday:
        range = permission_ui::permDateTimeRangeToday();
        break;
    case PresetYesterday:
        range = permission_ui::permDateTimeRangeYesterday();
        break;
    case PresetUnlimited:
        range = permission_ui::permDateTimeRangeUnlimited();
        break;
    case PresetLast30Days:
    default:
        range = permission_ui::permDateTimeRangeLast30Days();
        break;
    }

    if (range.unlimited) {
        m_viewModel->setDraftTimeFrom(QDateTime());
        m_viewModel->setDraftTimeTo(QDateTime());
        return;
    }

    m_viewModel->setDraftTimeFrom(range.from);
    m_viewModel->setDraftTimeTo(range.to);
}

void AuditLogManagementView::syncDraftFiltersToView()
{
    syncDraftTimePickersToView();
    syncActionTypeCombo();

    if (m_rangePreset) {
        m_rangePreset->blockSignals(true);
        m_rangePreset->setCurrentIndex(0);
        m_rangePreset->blockSignals(false);
    }
}

void AuditLogManagementView::syncTable()
{
    if (!m_viewModel || !m_tableModel)
        return;
    m_tableModel->setItems(m_viewModel->logItems());
    m_tableView->resizeColumnsToContents();
    syncPagerLabel();
}

void AuditLogManagementView::syncPagerLabel()
{
    if (!m_viewModel)
        return;
    if (m_totalLabel)
        m_totalLabel->setText(PTXT("perm.common.total_count").arg(m_viewModel->totalCount()));
    if (m_pageLabel) {
        const int pages = qMax(1, m_viewModel->totalPages());
        m_pageLabel->setText(
            PTXT("perm.common.page_indicator").arg(m_viewModel->pageIndex() + 1).arg(pages));
    }
}

void AuditLogManagementView::syncActionTypeCombo()
{
    if (!m_actionFilter)
        return;

    const QString current = m_viewModel ? m_viewModel->draftActionType() : QString();

    m_actionFilter->blockSignals(true);
    m_actionFilter->clear();
    m_actionFilter->addItem(PTXT("perm.audit.filter_all_actions"), QString());
    m_actionFilter->addItem(PTXT("perm.audit.filter_login"), QStringLiteral("login"));
    m_actionFilter->addItem(PTXT("perm.audit.filter_recipe_edit"), QStringLiteral("recipe_edit"));
    m_actionFilter->addItem(PTXT("perm.audit.filter_permission_change_legacy"), QStringLiteral("permission_change"));
    m_actionFilter->addItem(PTXT("perm.audit.filter_role_permission_save"), QStringLiteral("role_permission"));
    m_actionFilter->addItem(PTXT("perm.audit.filter_system_config"), QStringLiteral("system_config"));

    const int idx = m_actionFilter->findData(current);
    m_actionFilter->setCurrentIndex(idx >= 0 ? idx : 0);
    m_actionFilter->blockSignals(false);
}

void AuditLogManagementView::syncRangePresetCombo()
{
    if (!m_rangePreset)
        return;

    m_rangePreset->blockSignals(true);
    m_rangePreset->clear();
    m_rangePreset->addItem(PTXT("perm.audit.preset_last_30_days"), PresetLast30Days);
    m_rangePreset->addItem(PTXT("perm.audit.preset_last_7_days"), PresetLast7Days);
    m_rangePreset->addItem(PTXT("perm.audit.preset_today"), PresetToday);
    m_rangePreset->addItem(PTXT("perm.audit.preset_yesterday"), PresetYesterday);
    m_rangePreset->addItem(PTXT("perm.audit.filter_any_time"), PresetUnlimited);
    m_rangePreset->setCurrentIndex(0);
    m_rangePreset->blockSignals(false);
}

void AuditLogManagementView::showLogDetail(qint64 logId)
{
    if (!m_viewModel || logId <= 0)
        return;

    const QString title = PTXT("perm.audit.details_title");
    const QString body  = m_viewModel->formatLogDetail(logId);
    permission_ui::MessagePopup::showScrollable(this, title, body, QDialogButtonBox::Ok);
}

void AuditLogManagementView::syncDateTimePickerTexts()
{
    permission_ui::PermDateTimePickerTexts texts;
    texts.hourLabel            = PTXT("perm.datetime.hour_label");
    texts.hourRangeHint        = PTXT("perm.datetime.hour_range_hint");
    texts.hourTooltipPattern   = PTXT("perm.datetime.hour_tooltip");
    texts.monthHeaderPattern   = PTXT("perm.datetime.month_header");
    texts.nowLabel             = PTXT("perm.datetime.now");
    texts.clearLabel           = PTXT("perm.common.any");
    texts.confirmLabel         = PTXT("perm.common.ok");
    if (m_timeFrom)
        m_timeFrom->setPickerTexts(texts);
    if (m_timeTo)
        m_timeTo->setPickerTexts(texts);
}

void AuditLogManagementView::retranslateUi()
{
    if (m_pageTitle)
        m_pageTitle->setText(PTXT("perm.audit.page_title"));
    if (m_operatorEdit)
        m_operatorEdit->setPlaceholderText(PTXT("perm.audit.search_operator_placeholder"));
    if (m_timeFrom)
        m_timeFrom->setPlaceholderWhenEmpty(PTXT("perm.common.any"));
    if (m_timeTo)
        m_timeTo->setPlaceholderWhenEmpty(PTXT("perm.common.any"));
    syncDateTimePickerTexts();
    if (m_btnSearch)
        m_btnSearch->setText(PTXT("perm.common.search"));
    if (m_btnReset)
        m_btnReset->setText(PTXT("perm.common.reset"));
    if (m_btnExport)
        m_btnExport->setText(PTXT("perm.common.export"));
    if (m_btnPrevPage)
        m_btnPrevPage->setText(PTXT("perm.common.prev_page"));
    if (m_btnNextPage)
        m_btnNextPage->setText(PTXT("perm.common.next_page"));

    if (m_tableModel)
        emit m_tableModel->headerDataChanged(Qt::Horizontal, 0, AuditLogTableModel::ColCount - 1);

    syncActionTypeCombo();
    syncRangePresetCombo();
    syncPagerLabel();
}

} // namespace permission
