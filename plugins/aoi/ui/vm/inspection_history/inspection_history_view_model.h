#pragma once

/**
 * @file inspection_history_view_model.h
 * @brief 检测历史页 ViewModel：筛选、分页、KPI、导出
 */

#include "repository/inspection_history/inspection_history_types.h"

#include "viewmodel.h"

#include <QDateTime>
#include <QString>

namespace aoi {

struct InspectionHistoryFilterState
{
    QString   productModel;
    QString   boardSerial;
    QString   resultFilter = QStringLiteral("全部");
    QString   defectTypeFilter = QStringLiteral("全部");
    QDateTime timeFrom;
    QDateTime timeTo;

    inspection::InspectionHistoryQuery toQuery(int pageIndex, int pageSize) const;
};

class InspectionHistoryViewModel : public ViewModel
{
    Q_OBJECT

public:
    explicit InspectionHistoryViewModel(QObject *parent = nullptr);

    const InspectionHistoryFilterState &filter() const { return m_filter; }
    const inspection::InspectionHistoryPage &page() const { return m_page; }
    const inspection::InspectionHistoryStats &stats() const { return m_stats; }
    int pageIndex() const { return m_page.pageIndex; }
    int pageSize() const { return m_page.pageSize; }

public slots:
    void query();
    void resetFilter();
    void goToPage(int pageIndex);
    void setPageSize(int pageSize);
    void setProductModel(const QString &v);
    void setBoardSerial(const QString &v);
    void setResultFilter(const QString &v);
    void setDefectTypeFilter(const QString &v);
    void setTimeFrom(const QDateTime &v);
    void setTimeTo(const QDateTime &v);
    void refresh();
    bool exportCurrentFilter(const QString &filePath, int *rowCountOut = nullptr,
                             QString *errorOut = nullptr);

    /** 最近一次 runQuery 的数据库错误；空表示成功 */
    QString lastQueryError() const { return m_lastQueryError; }

signals:
    void pageChanged();
    void statsChanged();
    void filterChanged();

private:
    void runQuery();

    InspectionHistoryFilterState       m_filter;
    inspection::InspectionHistoryPage  m_page;
    inspection::InspectionHistoryStats m_stats;
    QString                            m_lastQueryError;
};

} // namespace aoi
