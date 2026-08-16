#pragma once

#include "model/audit_log_management_model.h"
#include "model/audit_log_model.h"
#include "command.h"
#include "viewmodel.h"

#include <QDateTime>
#include <QVector>

#include <memory>

class AuditLogManagementViewModel : public ViewModel
{
    Q_OBJECT

public:
    static constexpr const char *PROP_DRAFT_OPERATOR_NAME = "draftOperatorName";
    static constexpr const char *PROP_DRAFT_ACTION_TYPE   = "draftActionType";
    static constexpr const char *PROP_DRAFT_TIME_FROM     = "draftTimeFrom";
    static constexpr const char *PROP_DRAFT_TIME_TO       = "draftTimeTo";
    static constexpr const char *PROP_PAGE_INDEX          = "pageIndex";
    static constexpr const char *PROP_TOTAL_COUNT         = "totalCount";
    static constexpr const char *PROP_PAGE_SIZE           = "pageSize";

    VIEWMODEL_PROPERTY(QString, draftOperatorName, DraftOperatorName)
    VIEWMODEL_PROPERTY(QString, draftActionType, DraftActionType)
    VIEWMODEL_PROPERTY(QDateTime, draftTimeFrom, DraftTimeFrom)
    VIEWMODEL_PROPERTY(QDateTime, draftTimeTo, DraftTimeTo)
    VIEWMODEL_PROPERTY(int, pageIndex, PageIndex)
    VIEWMODEL_PROPERTY(int, totalCount, TotalCount)
    VIEWMODEL_PROPERTY(int, pageSize, PageSize)

signals:
    void draftOperatorNameChanged();
    void draftActionTypeChanged();
    void draftTimeFromChanged();
    void draftTimeToChanged();
    void pageIndexChanged();
    void totalCountChanged();
    void pageSizeChanged();

    void logListChanged();
    /// 草稿与已应用筛选均已恢复默认，View 应整栏同步控件
    void filtersReset();
    void showInfo(const QString &title, const QString &message);
    void showWarning(const QString &title, const QString &message);

public:
    explicit AuditLogManagementViewModel(permission::AuditLogModel *model = nullptr,
                                         QObject *parent                   = nullptr);

    const QVector<permission::AuditLogListItem> &logItems() const { return m_logItems; }

    int  totalPages() const;
    bool canGoPrevPage() const;
    bool canGoNextPage() const;

    QString formatLogDetail(qint64 logId) const;

    Command *searchCommand() const { return m_searchCommand; }
    Command *resetFilterCommand() const { return m_resetFilterCommand; }
    Command *prevPageCommand() const { return m_prevPageCommand; }
    Command *nextPageCommand() const { return m_nextPageCommand; }
    Command *exportCommand() const { return m_exportCommand; }

    /// 校验草稿时间范围，提交为已应用筛选并刷新列表
    bool submitSearch();

public slots:
    void refreshList();
    void resetFilters();
    void goPrevPage();
    void goNextPage();
    void exportPlaceholder();

private:
    struct FilterValues {
        QString   operatorName;
        QString   actionType;
        QDateTime timeFrom;
        QDateTime timeTo;
    };

    static FilterValues defaultFilterValues();
    void                applyFilterValues(const FilterValues &values, bool toDraft, bool toApplied);
    void                copyDraftToApplied();
    bool                validateDraftTimeRange();

    permission::AuditLogQueryFilter appliedFilter() const;
    void                            bindCommands();
    void                            updatePagerCommands();

    std::unique_ptr<permission::AuditLogModel> m_ownedModel;
    permission::AuditLogModel                 *m_model = nullptr;

    QVector<permission::AuditLogListItem> m_logItems;

    FilterValues m_applied;

    FunctionalCommand *m_searchCommand      = nullptr;
    FunctionalCommand *m_resetFilterCommand = nullptr;
    FunctionalCommand *m_prevPageCommand    = nullptr;
    FunctionalCommand *m_nextPageCommand    = nullptr;
    FunctionalCommand *m_exportCommand      = nullptr;
};
