/**
 * @file defect_review_dialog.cpp
 */

#include "review/defect_review_dialog.h"

#include "review/defect_review_widget.h"
#include "vm/review/defect_review_view_model.h"
#include "service/inspection/history/inspection_history_mapper.h"

#include "component/dialog/themed_message_box.h"

#include <QCloseEvent>
#include <QSize>

namespace aoi::review {

DefectReviewDialog::DefectReviewDialog(QWidget *parent)
    : ui::ThemedPanelDialog(parent)
{
    buildUi();
    setMinimumPanelWidth(960);
    enableResizableWindow(QSize(1024, 640));
    resize(1400, 900);
}

void DefectReviewDialog::buildUi()
{
    m_reviewWidget = new DefectReviewWidget(this, /*bindBridge=*/false);
    m_reviewWidget->viewModel()->setRequireFilePathForConfirm(false);
    setBodyWidget(m_reviewWidget);

    connect(m_reviewWidget, &DefectReviewWidget::reviewConfirmed, this, [this](const inspection::DefectInfoList &) {
        emit recordUpdated(m_recordId);
        accept();
    });
}

QString DefectReviewDialog::resultLabel(inspection::InspectionHistoryResult result)
{
    switch (result) {
    case inspection::InspectionHistoryResult::Ok:
        return QStringLiteral("OK");
    case inspection::InspectionHistoryResult::Ng:
        return QStringLiteral("NG");
    case inspection::InspectionHistoryResult::Rw:
        return QStringLiteral("RW");
    }
    return QStringLiteral("—");
}

QString DefectReviewDialog::titleForRow(const inspection::InspectionHistoryRow &row) const
{
    return QStringLiteral("检测详情 · %1 / %2 · %3")
        .arg(row.productName, row.sn, resultLabel(row.result));
}

void DefectReviewDialog::openForRecord(const inspection::InspectionHistoryRow &row)
{
    m_recordId = row.id;
    setTitleText(titleForRow(row));
    if (m_reviewWidget) {
        m_reviewWidget->setHistoryRecordId(row.id);
        m_reviewWidget->loadReviewSession(inspection::defectInfoListFromRow(row));
        m_reviewWidget->setFocus();
    }
    showMaximized();
    raise();
    activateWindow();
}

void DefectReviewDialog::closeEvent(QCloseEvent *event)
{
    if (m_reviewWidget && m_reviewWidget->isDirty()) {
        const bool ok = ui::ThemedMessageBox::ask(
            this, QStringLiteral("关闭"), QStringLiteral("有未保存的复判修改，确定关闭吗？"),
            QStringLiteral("确定"), QStringLiteral("取消"));
        if (!ok) {
            event->ignore();
            return;
        }
    }
    ui::ThemedPanelDialog::closeEvent(event);
}

} // namespace aoi::review
