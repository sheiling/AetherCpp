#pragma once

/**
 * @file inspection_results_view_model.h
 * @brief 检测结果历史 ViewModel（筛选、分页、统计、导出）
 */

#include "lighten_viewmodel_macros.h"
#include "command.h"
#include "inspection_results_model.h"

namespace lighten {

/**
 * @brief 检测结果历史 ViewModel
 *
 * 绑定 InspectionResultsView：筛选、统计 KPI、分页表格、导出。
 */
class InspectionResultsViewModel : public ViewModel
{
    Q_OBJECT

public:
    static constexpr const char *PROP_FILTER     = "filter";
    static constexpr const char *PROP_STATS      = "stats";
    static constexpr const char *PROP_PAGE       = "page";
    static constexpr const char *PROP_PAGE_INDEX = "pageIndex";
    static constexpr const char *PROP_STATUS     = "status";

    static constexpr const char *PROP_PAGE_SIZE  = "pageSize";

    LIGHTEN_VIEWMODEL_STRUCT_PROPERTY(InspectionFilter, filter, Filter)
    LIGHTEN_VIEWMODEL_STRUCT_PROPERTY(InspectionStats, stats, Stats)
    LIGHTEN_VIEWMODEL_STRUCT_PROPERTY(InspectionPage, page, Page)
    LIGHTEN_VIEWMODEL_PROPERTY(int, pageIndex, PageIndex)
    LIGHTEN_VIEWMODEL_PROPERTY(int, pageSize, PageSize)
    LIGHTEN_VIEWMODEL_PROPERTY(QString, status, Status)

signals:
    void sig_filterChanged();
    void sig_statsChanged();
    void sig_pageChanged();
    void sig_pageIndexChanged();
    void sig_pageSizeChanged();
    void sig_statusChanged();
    /** 由 exportCommand 触发，由 View 弹出保存对话框后调用 slot_exportTable */
    void sig_exportRequested();
    /** CSV 写入成功后由 ViewModel 发出，View 弹出成功提示 */
    void sig_exportSucceeded();

public:
    explicit InspectionResultsViewModel(QObject *parent = nullptr);

    /** 执行查询命令 */
    Command *queryCommand() const { return m_queryCommand; }
    /** 重置筛选条件命令 */
    Command *resetFilterCommand() const { return m_resetFilterCommand; }
    /** 导出表格命令 */
    Command *exportCommand() const { return m_exportCommand; }
    /** 上一页命令 */
    Command *prevPageCommand() const { return m_prevPageCommand; }
    /** 下一页命令 */
    Command *nextPageCommand() const { return m_nextPageCommand; }

public slots:
    /** 按当前筛选条件查询并刷新分页数据 */
    void slot_query();
    /** 重置筛选条件为今日默认 */
    void slot_resetFilter();
    /** 跳转到指定页码（0-based） */
    void slot_goToPage(int index);
    /** 将当前筛选结果导出到 CSV 文件 */
    void slot_exportTable(const QString &filePath);

private:
    void refreshPage();

    static constexpr int kDefaultPageSize = 20;

    FunctionalCommand *m_queryCommand       = nullptr;
    FunctionalCommand *m_resetFilterCommand = nullptr;
    FunctionalCommand *m_exportCommand      = nullptr;
    FunctionalCommand *m_prevPageCommand    = nullptr;
    FunctionalCommand *m_nextPageCommand    = nullptr;
};

} // namespace lighten
