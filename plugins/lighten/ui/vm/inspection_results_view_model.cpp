/**
 * @file inspection_results_view_model.cpp
 * @brief InspectionResultsViewModel 查询、分页与导出实现
 */

#include "inspection_results_view_model.h"

#include <QMetaType>

/** @brief InspectionResultsViewModel 成员实现 */
namespace lighten {

InspectionResultsViewModel::InspectionResultsViewModel(QObject *parent)
    : ViewModel(parent)
{
    qRegisterMetaType<InspectionFilter>("InspectionFilter");
    qRegisterMetaType<InspectionStats>("InspectionStats");
    qRegisterMetaType<InspectionPage>("InspectionPage");

    registerProperty(PROP_FILTER, QVariant::fromValue(InspectionFilter::defaultToday()));
    registerProperty(PROP_STATS, QVariant::fromValue(InspectionStats{}));
    registerProperty(PROP_PAGE, QVariant::fromValue(InspectionPage{}));
    registerProperty(PROP_PAGE_INDEX, 0);
    registerProperty(PROP_PAGE_SIZE, kDefaultPageSize);
    registerProperty(PROP_STATUS, QStringLiteral("就绪"));

    m_queryCommand = new FunctionalCommand([this]() { slot_query(); }, nullptr, this);
    m_queryCommand->setName(QStringLiteral("inspection.query"));

    m_resetFilterCommand = new FunctionalCommand([this]() { slot_resetFilter(); }, nullptr, this);
    m_resetFilterCommand->setName(QStringLiteral("inspection.resetFilter"));

    m_exportCommand = new FunctionalCommand([this]() { emit sig_exportRequested(); }, nullptr, this);
    m_exportCommand->setName(QStringLiteral("inspection.export"));

    m_prevPageCommand = new FunctionalCommand(
        [this]() {
            slot_goToPage(pageIndex() - 1);
        },
        [this]() { return pageIndex() > 0; }, this);
    m_prevPageCommand->setName(QStringLiteral("inspection.prevPage"));

    m_nextPageCommand = new FunctionalCommand(
        [this]() {
            slot_goToPage(pageIndex() + 1);
        },
        [this]() {
            return pageIndex() + 1 < page().totalPages;
        },
        this);
    m_nextPageCommand->setName(QStringLiteral("inspection.nextPage"));
}

/** 按当前筛选条件刷新 KPI 与分页（重置到第 1 页） */
void InspectionResultsViewModel::slot_query()
{
    setPageIndex(0);
    setStats(InspectionResultsRepository::statsFor(filter()));
    refreshPage();
    setStatus(QStringLiteral("查询完成，共 %1 条").arg(page().totalCount));
}

/** 恢复今日默认筛选并重新查询 */
void InspectionResultsViewModel::slot_resetFilter()
{
    setFilter(InspectionFilter::defaultToday());
    slot_query();
}

void InspectionResultsViewModel::slot_goToPage(int index)
{
    setPageIndex(qMax(0, index));
    refreshPage();
}

/** 导出当前筛选结果为 CSV（filePath 为空时忽略，通常由 View 在保存对话框确认后传入） */
void InspectionResultsViewModel::slot_exportTable(const QString &filePath)
{
    if (filePath.isEmpty())
        return;

    if (InspectionResultsRepository::exportToCsv(filter(), filePath)) {
        emit sig_exportSucceeded();
    } else {
        setStatus(QStringLiteral("导出失败"));
    }
}

void InspectionResultsViewModel::refreshPage()
{
    setPage(InspectionResultsRepository::query(filter(), pageIndex(), pageSize()));
}

} // namespace lighten
