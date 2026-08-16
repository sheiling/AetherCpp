#include "ui/vm/audit_log_management_view_model.h"

#include "command.h"
#include "service/perm_action_keys.h"
#include "model/audit_log_detail_text.h"
#include "ui/view/perm_datetime_utils.h"
#include "ui/view/perm_text.h"

#include <QDate>
#include <QTime>

AuditLogManagementViewModel::FilterValues AuditLogManagementViewModel::defaultFilterValues()
{
    const QDate today = QDate::currentDate();
    FilterValues  v;
    v.operatorName = QString();
    v.actionType   = QString();
    v.timeFrom     = QDateTime(today.addDays(-30), QTime(0, 0, 0));
    v.timeTo       = QDateTime(today, QTime(23, 59, 59));
    return v;
}

AuditLogManagementViewModel::AuditLogManagementViewModel(permission::AuditLogModel *model,
                                                           QObject *parent)
    : ViewModel(parent)
{
    if (model) {
        m_model = model;
    } else {
        m_ownedModel = std::make_unique<permission::AuditLogModel>(nullptr);
        m_model      = m_ownedModel.get();
    }

    const FilterValues defaults = defaultFilterValues();
    registerProperty(PROP_DRAFT_OPERATOR_NAME, defaults.operatorName);
    registerProperty(PROP_DRAFT_ACTION_TYPE, defaults.actionType);
    registerProperty(PROP_DRAFT_TIME_FROM, defaults.timeFrom);
    registerProperty(PROP_DRAFT_TIME_TO, defaults.timeTo);
    registerProperty(PROP_PAGE_INDEX, 0);
    registerProperty(PROP_TOTAL_COUNT, 0);
    registerProperty(PROP_PAGE_SIZE, 50);

    m_applied = defaults;

    bindCommands();
    connect(this, &ViewModel::propertyChanged, this, [this]() { updatePagerCommands(); });
}

void AuditLogManagementViewModel::applyFilterValues(const FilterValues &values, bool toDraft,
                                                    bool toApplied)
{
    if (toDraft) {
        setDraftOperatorName(values.operatorName);
        setDraftActionType(values.actionType);
        setDraftTimeFrom(values.timeFrom);
        setDraftTimeTo(values.timeTo);
    }
    if (toApplied)
        m_applied = values;
}

void AuditLogManagementViewModel::copyDraftToApplied()
{
    m_applied.operatorName = draftOperatorName();
    m_applied.actionType   = draftActionType();
    m_applied.timeFrom     = draftTimeFrom();
    m_applied.timeTo       = draftTimeTo();
}

bool AuditLogManagementViewModel::validateDraftTimeRange()
{
    const bool fromValid = draftTimeFrom().isValid();
    const bool toValid   = draftTimeTo().isValid();

    if (!fromValid || !toValid)
        return true;

    const QDateTime boundFrom = permission_ui::permDateTimeFilterBound(
        draftTimeFrom(), permission_ui::PermDateTimeResolution::Hour, false);
    const QDateTime boundTo = permission_ui::permDateTimeFilterBound(
        draftTimeTo(), permission_ui::PermDateTimeResolution::Hour, true);
    if (boundFrom > boundTo) {
        emit showWarning(PTXT("perm.common.notice"), PTXT("perm.audit.start_after_end"));
        return false;
    }
    return true;
}

void AuditLogManagementViewModel::bindCommands()
{
    m_searchCommand = new FunctionalCommand([this]() { submitSearch(); }, nullptr, this);
    m_searchCommand->setName(permission::PermKeys::AuditQuery);

    m_resetFilterCommand = new FunctionalCommand([this]() { resetFilters(); }, nullptr, this);
    m_resetFilterCommand->setName(permission::PermKeys::AuditResetFilter);

    m_prevPageCommand =
        new FunctionalCommand([this]() { goPrevPage(); }, [this]() { return canGoPrevPage(); }, this);
    m_prevPageCommand->setName(permission::PermKeys::AuditPrevPage);

    m_nextPageCommand =
        new FunctionalCommand([this]() { goNextPage(); }, [this]() { return canGoNextPage(); }, this);
    m_nextPageCommand->setName(permission::PermKeys::AuditNextPage);

    m_exportCommand = new FunctionalCommand([this]() { exportPlaceholder(); }, nullptr, this);
    m_exportCommand->setName(permission::PermKeys::AuditExport);

    updatePagerCommands();
}

void AuditLogManagementViewModel::updatePagerCommands()
{
    if (m_searchCommand)
        m_searchCommand->updateCanExecute();
    if (m_resetFilterCommand)
        m_resetFilterCommand->updateCanExecute();
    if (m_exportCommand)
        m_exportCommand->updateCanExecute();
    if (m_prevPageCommand)
        m_prevPageCommand->updateCanExecute();
    if (m_nextPageCommand)
        m_nextPageCommand->updateCanExecute();
}

permission::AuditLogQueryFilter AuditLogManagementViewModel::appliedFilter() const
{
    permission::AuditLogQueryFilter filter;
    filter.operatorName = m_applied.operatorName.trimmed();
    filter.actionType   = m_applied.actionType;
    if (m_applied.timeFrom.isValid()) {
        filter.timeFrom = permission_ui::permDateTimeFilterBound(
            m_applied.timeFrom, permission_ui::PermDateTimeResolution::Hour, false);
    }
    if (m_applied.timeTo.isValid()) {
        filter.timeTo = permission_ui::permDateTimeFilterBound(
            m_applied.timeTo, permission_ui::PermDateTimeResolution::Hour, true);
    }
    filter.offset = pageIndex() * pageSize();
    filter.limit  = pageSize();
    return filter;
}

bool AuditLogManagementViewModel::submitSearch()
{
    if (!validateDraftTimeRange())
        return false;

    copyDraftToApplied();
    setPageIndex(0);
    refreshList();
    return true;
}

void AuditLogManagementViewModel::refreshList()
{
    const permission::AuditLogQueryFilter filter = appliedFilter();

    permission::AuditLogQueryFilter countFilter = filter;
    countFilter.offset = 0;
    countFilter.limit  = 0;

    m_logItems = m_model->list(filter);
    setTotalCount(static_cast<int>(m_model->count(countFilter)));

    const int pages = totalPages();
    if (pageIndex() >= pages && pages > 0)
        setPageIndex(pages - 1);

    emit logListChanged();
    updatePagerCommands();
}

void AuditLogManagementViewModel::resetFilters()
{
    const FilterValues defaults = defaultFilterValues();

    beginBatchUpdate();
    applyFilterValues(defaults, true, true);
    setPageIndex(0);
    endBatchUpdate();

    emit filtersReset();
    refreshList();
}

void AuditLogManagementViewModel::goPrevPage()
{
    if (!canGoPrevPage())
        return;
    setPageIndex(pageIndex() - 1);
    refreshList();
}

void AuditLogManagementViewModel::goNextPage()
{
    if (!canGoNextPage())
        return;
    setPageIndex(pageIndex() + 1);
    refreshList();
}

void AuditLogManagementViewModel::exportPlaceholder()
{
    emit showInfo(PTXT("perm.common.notice"), PTXT("perm.audit.export_pending"));
}

int AuditLogManagementViewModel::totalPages() const
{
    if (totalCount() <= 0 || pageSize() <= 0)
        return 1;
    return (totalCount() + pageSize() - 1) / pageSize();
}

bool AuditLogManagementViewModel::canGoPrevPage() const { return pageIndex() > 0; }

bool AuditLogManagementViewModel::canGoNextPage() const
{
    return pageIndex() + 1 < totalPages();
}

QString AuditLogManagementViewModel::formatLogDetail(qint64 logId) const
{
    for (const auto &item : m_logItems) {
        if (item.id == logId)
            return permission::auditDetailFullText(item);
    }

    permission::AuditLogRecord rec;
    if (!m_model->findById(logId, rec))
        return PTXT("perm.audit.record_not_found");

    return permission::auditDetailFullText(permission::AuditLogModel::toListItem(rec));
}
