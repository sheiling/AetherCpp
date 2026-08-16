#pragma once

/**
 * @file defect_review_view.h
 * @brief 复判页：薄壳 PageWidget，内嵌 DefectReviewWidget
 */

#include "pagewidget.h"

namespace aoi::review {

class DefectReviewWidget;

class DefectReviewView : public PageWidget
{
    Q_OBJECT

public:
    explicit DefectReviewView(QWidget *parent = nullptr);

    void onLoad() override;

private:
    void retranslateUi();

    DefectReviewWidget *m_reviewWidget = nullptr;
};

} // namespace aoi::review
