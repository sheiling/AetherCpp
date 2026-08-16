#pragma once

/**
 * @file defect_review_widget.h
 * @brief 缺陷复判独立 Widget：MAP + 复判区（含确认按钮）
 */

#include "inspection_types.h"

#include <QWidget>

namespace aoi::review {

class DefectMapPanel;
class DefectReviewPanel;
class DefectReviewViewModel;

class DefectReviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DefectReviewWidget(QWidget *parent = nullptr, bool bindBridge = true);

    DefectReviewViewModel *viewModel() const { return m_viewModel; }

    void setHistoryRecordId(qint64 id);

    void setDieGrid(int dieRowCount, int dieColCount);
    void loadReviewSession(const inspection::DefectInfoList &data);

    inspection::DefectInfoList workingDefectData() const;
    bool                       isDirty() const;

signals:
    void reviewConfirmed(const inspection::DefectInfoList &data);

public:
    void retranslateUi();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void buildUi();
    void wireViewModel();

    DefectReviewViewModel *m_viewModel   = nullptr;
    DefectMapPanel        *m_mapPanel    = nullptr;
    DefectReviewPanel     *m_reviewPanel = nullptr;
};

} // namespace aoi::review
