/**
 * @file inspection_history_view_model.cpp
 */

#include "inspection_history_view_model.h"

#if COMMON_ORM_ENABLED
#include "repository/inspection_history/inspection_history_repository.h"
#include "service/inspection/history/inspection_history_mapper.h"
#endif

#include <QDate>
#include <QDebug>
#include <QTime>

namespace aoi {

inspection::InspectionHistoryQuery InspectionHistoryFilterState::toQuery(int pageIndex,
                                                                         int pageSize) const
{
    inspection::InspectionHistoryQuery q;
    q.productName       = productModel.trimmed();
    q.sn                = boardSerial.trimmed();
    q.resultFilter      = resultFilter;
    q.defectTypeFilter  = defectTypeFilter;
    q.timeFrom          = timeFrom;
    q.timeTo            = timeTo;
    q.pageIndex         = pageIndex;
    q.pageSize          = pageSize;
    return q;
}

InspectionHistoryViewModel::InspectionHistoryViewModel(QObject *parent)
    : ViewModel(parent)
{
    const QDate today = QDate::currentDate();
    m_filter.timeFrom = QDateTime(today, QTime(0, 0, 0));
    m_filter.timeTo   = QDateTime(today, QTime(23, 59, 59));
    m_page.pageSize   = 20;
}

void InspectionHistoryViewModel::query()
{
    m_page.pageIndex = 0;
    runQuery();
}

void InspectionHistoryViewModel::resetFilter()
{
    m_filter.productModel.clear();
    m_filter.boardSerial.clear();
    m_filter.resultFilter      = QStringLiteral("全部");
    m_filter.defectTypeFilter  = QStringLiteral("全部");
    const QDate today          = QDate::currentDate();
    m_filter.timeFrom          = QDateTime(today, QTime(0, 0, 0));
    m_filter.timeTo            = QDateTime(today, QTime(23, 59, 59));
    emit filterChanged();
    query();
}

void InspectionHistoryViewModel::goToPage(int pageIndex)
{
    m_page.pageIndex = qMax(0, pageIndex);
    runQuery();
}

void InspectionHistoryViewModel::setPageSize(int pageSize)
{
    m_page.pageSize  = qMax(1, pageSize);
    m_page.pageIndex = 0;
    runQuery();
}

void InspectionHistoryViewModel::setProductModel(const QString &v)
{
    m_filter.productModel = v;
    emit filterChanged();
}

void InspectionHistoryViewModel::setBoardSerial(const QString &v)
{
    m_filter.boardSerial = v;
    emit filterChanged();
}

void InspectionHistoryViewModel::setResultFilter(const QString &v)
{
    m_filter.resultFilter = v;
    emit filterChanged();
}

void InspectionHistoryViewModel::setDefectTypeFilter(const QString &v)
{
    m_filter.defectTypeFilter = v;
    emit filterChanged();
}

void InspectionHistoryViewModel::setTimeFrom(const QDateTime &v)
{
    m_filter.timeFrom = v;
    emit filterChanged();
}

void InspectionHistoryViewModel::setTimeTo(const QDateTime &v)
{
    m_filter.timeTo = v;
    emit filterChanged();
}

void InspectionHistoryViewModel::refresh()
{
    runQuery();
}

bool InspectionHistoryViewModel::exportCurrentFilter(const QString &filePath, int *rowCountOut,
                                                     QString *errorOut)
{
#if COMMON_ORM_ENABLED
    QString error;
    const inspection::InspectionHistoryQuery q = m_filter.toQuery(0, 1000000);
    const inspection::InspectionHistoryPage  page =
        inspection::InspectionHistoryRepository::instance().queryPage(q, &error);
    if (!error.isEmpty()) {
        if (errorOut)
            *errorOut = error;
        if (rowCountOut)
            *rowCountOut = 0;
        return false;
    }
    if (!inspection::writeInspectionHistoryCsv(page.rows, filePath, errorOut)) {
        if (rowCountOut)
            *rowCountOut = 0;
        return false;
    }
    if (rowCountOut)
        *rowCountOut = page.rows.size();
    return true;
#else
    Q_UNUSED(filePath)
    if (rowCountOut) *rowCountOut = 0;
    if (errorOut) *errorOut = QStringLiteral("ORM disabled");
    return false;
#endif
}

void InspectionHistoryViewModel::runQuery()
{
    m_lastQueryError.clear();
#if COMMON_ORM_ENABLED
    QString error;
    const inspection::InspectionHistoryQuery q =
        m_filter.toQuery(m_page.pageIndex, m_page.pageSize);
    m_page  = inspection::InspectionHistoryRepository::instance().queryPage(q, &error);
    m_stats = inspection::InspectionHistoryRepository::instance().queryStats(q, &error);
    if (!error.isEmpty()) {
        m_lastQueryError = error;
        qWarning() << "[InspectionHistoryViewModel]" << error;
    }
#endif
    emit pageChanged();
    emit statsChanged();
}

} // namespace aoi
