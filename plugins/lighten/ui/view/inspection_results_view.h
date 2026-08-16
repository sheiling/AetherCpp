#pragma once

/**
 * @file inspection_results_view.h
 * @brief 检测结果历史视图（筛选、统计 KPI、分页宽表）
 */

#include "pagewidget.h"
#include "inspection_results_model.h"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QLabel>
#include <QVector>
#include <QPointer>

class QComboBox;
class QDialog;
class QLineEdit;
class QPushButton;

namespace lighten {

class InspectionResultsViewModel;

/**
 * @brief 检测结果历史视图
 */
class InspectionResultsView : public PageWidget
{
    Q_OBJECT

public:
    explicit InspectionResultsView(QWidget *parent = nullptr);

protected:
    void onLoad() override;

private:
    void buildFilterCard(QVBoxLayout *root);
    void buildStatsRow(QVBoxLayout *root);
    void buildDataTable(QVBoxLayout *root);
    void setupBindings();
    /** 同步筛选条件并弹出 CSV 保存对话框 */
    void promptExportTable();
    /** 导出成功提示（样式对齐配方页「保存配方」） */
    void showExportSuccessTip();
    void refreshTable();
    void refreshStats();
    void refreshPager();
    InspectionFilter readFilterFromUi() const;
    void applyFilterToUi(const InspectionFilter &filter);

    InspectionResultsViewModel *m_viewModel = nullptr;

    QComboBox  *m_modelSel   = nullptr;
    QLineEdit  *m_snEdit     = nullptr;
    QComboBox  *m_resultSel  = nullptr;
    QComboBox  *m_defectSel  = nullptr;
    QComboBox  *m_flashSel   = nullptr;
    QWidget    *m_fromDt     = nullptr;
    QWidget    *m_toDt       = nullptr;
    QPushButton *m_btnQuery  = nullptr;
    QPushButton *m_btnReset  = nullptr;
    QPushButton *m_btnExport = nullptr;

    QPointer<QDialog> m_exportSuccessTipDlg;

    QTableWidget *m_table       = nullptr;
    QVector<QLabel *> m_stats;
    QLabel       *m_totalLbl     = nullptr;
    QWidget      *m_pageNumsWrap = nullptr;
    QPushButton  *m_prevBtn      = nullptr;
    QPushButton  *m_nextBtn      = nullptr;
    QComboBox    *m_pageSizeSel  = nullptr;
};

} // namespace lighten
