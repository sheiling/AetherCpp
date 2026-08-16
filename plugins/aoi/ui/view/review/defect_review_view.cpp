/**
 * @file defect_review_view.cpp
 */

#include "review/defect_review_view.h"

#include "review/defect_review_widget.h"

#include <QVBoxLayout>

namespace aoi::review {

DefectReviewView::DefectReviewView(QWidget *parent)
    : PageWidget(parent)
{
    setPageId(QStringLiteral("aoi.review"));
}

void DefectReviewView::onLoad()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_reviewWidget = new DefectReviewWidget(this, /*bindBridge=*/true);
    layout->addWidget(m_reviewWidget);

    // 正式业务链路由 ScanFlowProgressBridge 上送真实待复判会话；
    // PageWidget 这里不再主动灌 mock，避免覆盖刚接入的检测结果。
    m_reviewWidget->setFocus();
    retranslateUi();
}

void DefectReviewView::retranslateUi()
{
    if (m_reviewWidget)
        m_reviewWidget->retranslateUi();
}

} // namespace aoi::review
