#pragma once

/**
 * @file inspection_history_view.h
 * @brief 检测结果导航页（§3.1.5）
 */

#include "inspection_types.h"
#include "repository/inspection_history/inspection_history_types.h"

#include "pagewidget.h"

class QLabel;
class QResizeEvent;
class QVBoxLayout;

namespace ui {
class CursorToast;
class KpiMetricCard;
class ThemedComboBox;
class ThemedDateTimeEdit;
class ThemedLineEdit;
class ThemedPushButton;
class ThemedSectionPanel;
class ThemedTableWidget;
} // namespace ui

namespace aoi {

class InspectionHistoryViewModel;
namespace review {
class DefectReviewDialog;
}

class InspectionHistoryView : public PageWidget
{
    Q_OBJECT

public:
    explicit InspectionHistoryView(QWidget *parent = nullptr);

protected:
    void onLoad() override;
    void onShow() override;
    void retranslateUi() override;
    void applyTheme() override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void buildLayout();
    void buildFilterBar(QWidget *parent, QVBoxLayout *root);
    void buildKpiRow(QWidget *parent, QVBoxLayout *root);
    void buildTableSection(QWidget *parent, QVBoxLayout *root);
    void buildPager(ui::ThemedSectionPanel *section);
    void wireViewModel();
    void refreshTable();
    void refreshKpi();
    void refreshPager();
    void applyTableColumnStretch();
    void applyPageDecorStyles();
    void applyFilterFromEditors();
    void syncFilterEditorsFromViewModel();
    void openDetailForRow(const inspection::InspectionHistoryRow &row);
    void showQueryResultToast();
    void showExportResultToast(bool ok, int rowCount, const QString &error);
    static QString formatDetectTime(const QString &iso);

    InspectionHistoryViewModel *m_viewModel = nullptr;
    review::DefectReviewDialog *m_detailDialog = nullptr;
    ui::CursorToast            *m_statusToast = nullptr;

    ui::ThemedLineEdit    *m_productEdit    = nullptr;
    ui::ThemedLineEdit    *m_snEdit         = nullptr;
    ui::ThemedComboBox    *m_resultCombo    = nullptr;
    ui::ThemedComboBox       *m_defectCombo    = nullptr;
    ui::ThemedDateTimeEdit   *m_timeFromEdit   = nullptr;
    ui::ThemedDateTimeEdit   *m_timeToEdit     = nullptr;
    QVector<ui::KpiMetricCard *> m_kpiCards;
    ui::ThemedTableWidget *m_table          = nullptr;
    QLabel                *m_pagerInfo      = nullptr;
    ui::ThemedPushButton  *m_prevBtn        = nullptr;
    ui::ThemedPushButton  *m_nextBtn        = nullptr;
    ui::ThemedComboBox    *m_pageSizeCombo  = nullptr;
};

} // namespace aoi
