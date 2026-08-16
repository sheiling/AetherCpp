#pragma once

/**
 * @file defect_review_dialog.h
 * @brief 检测历史详情弹窗：内嵌 DefectReviewWidget（§3.1.5）
 */

#include "repository/inspection_history/inspection_history_types.h"

#include "component/dialog/themed_panel_dialog.h"

namespace aoi::review {

class DefectReviewWidget;

/** 历史记录「查看」弹窗；不绑定 ScanFlowProgressBridge */
class DefectReviewDialog final : public ui::ThemedPanelDialog
{
    Q_OBJECT

public:
    explicit DefectReviewDialog(QWidget *parent = nullptr);

    /** 加载行数据并最大化展示 */
    void openForRecord(const inspection::InspectionHistoryRow &row);

signals:
    /** 复判确认且历史库 UPDATE 成功后发出，供历史页刷新 */
    void recordUpdated(qint64 recordId);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void buildUi();
    QString titleForRow(const inspection::InspectionHistoryRow &row) const;
    static QString resultLabel(inspection::InspectionHistoryResult result);

    DefectReviewWidget *m_reviewWidget = nullptr;
    qint64              m_recordId     = -1;
};

} // namespace aoi::review
