/**
 * @file defect_review_widget.cpp
 */

#include "review/defect_review_widget.h"

#include "review/defect_map_panel.h"
#include "review/defect_review_panel.h"
#include "vm/review/defect_review_view_model.h"

#include "component/group/themed_section_panel.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QVBoxLayout>

namespace aoi::review {

DefectReviewWidget::DefectReviewWidget(QWidget *parent, bool bindBridge)
    : QWidget(parent)
{
    m_viewModel = new DefectReviewViewModel(bindBridge, this);
    buildUi();
    wireViewModel();
    installEventFilter(this);
    setFocusPolicy(Qt::StrongFocus);
    retranslateUi();
}

void DefectReviewWidget::buildUi()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(8);

    auto *body = new QHBoxLayout();
    body->setSpacing(8);

    auto *mapSection = new ui::ThemedSectionPanel(QStringLiteral("aoi.review.defect_map"), this);
    mapSection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_mapPanel       = new DefectMapPanel(m_viewModel, mapSection->contentHost());
    mapSection->setContentWidget(m_mapPanel);

    auto *reviewSection = new ui::ThemedSectionPanel(QStringLiteral("aoi.review.panel_title"), this);
    reviewSection->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_reviewPanel       = new DefectReviewPanel(m_viewModel, reviewSection->contentHost());
    reviewSection->setContentWidget(m_reviewPanel);

    body->addWidget(mapSection, 65);
    body->addWidget(reviewSection, 35);
    root->addLayout(body, 1);
}

void DefectReviewWidget::wireViewModel()
{
    connect(m_viewModel, &DefectReviewViewModel::reviewConfirmed, this, &DefectReviewWidget::reviewConfirmed);

    connect(m_viewModel, &DefectReviewViewModel::selectionCenterRequested, this, [this]() {
        if (!m_viewModel || !m_mapPanel)
            return;
        const int idx = m_viewModel->selectedVisibleIndex();
        if (idx >= 0)
            m_mapPanel->centerOnVisibleRow(idx);
    });
}

void DefectReviewWidget::setDieGrid(int dieRowCount, int dieColCount)
{
    if (!m_viewModel)
        return;
    DefectReviewDieGrid grid;
    grid.rowCount = dieRowCount;
    grid.colCount = dieColCount;
    m_viewModel->setDieGrid(grid);
}

void DefectReviewWidget::loadReviewSession(const inspection::DefectInfoList &data)
{
    if (m_viewModel)
        m_viewModel->loadReviewSession(data);
}

inspection::DefectInfoList DefectReviewWidget::workingDefectData() const
{
    return m_viewModel ? m_viewModel->workingDefectData() : inspection::DefectInfoList{};
}

bool DefectReviewWidget::isDirty() const
{
    return m_viewModel && m_viewModel->isDirty();
}

void DefectReviewWidget::setHistoryRecordId(qint64 id)
{
    if (m_viewModel)
        m_viewModel->setHistoryRecordId(id);
}

void DefectReviewWidget::retranslateUi()
{
    if (m_mapPanel)
        m_mapPanel->retranslateUi();
    if (m_reviewPanel)
        m_reviewPanel->retranslateUi();
}

bool DefectReviewWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this && event->type() == QEvent::KeyPress && m_viewModel) {
        auto *ke = static_cast<QKeyEvent *>(event);
        if (!ke->isAutoRepeat()) {
            m_viewModel->handleKey(static_cast<Qt::Key>(ke->key()));
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

} // namespace aoi::review
