#pragma once

/**
 * @file defect_map_panel.h
 * @brief 缺陷 MAP 面板：原型侧栏类型筛选 + 工具栏 + 画布
 */

#include "vm/review/defect_review_types.h"

#include <QWidget>

class QFrame;
class QPushButton;

namespace aoi::review {

class DefectMapCanvas;
class DefectReviewViewModel;
class DefectTypeFilterRow;

class DefectMapPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DefectMapPanel(DefectReviewViewModel *viewModel, QWidget *parent = nullptr);

    void retranslateUi();
    void refreshFromViewModel();
    void centerOnVisibleRow(int visibleRowIndex);

private:
    void buildUi();
    void wireViewModel();
    void rebuildTypeFilters();
    void syncTypeFilters();
    void applyFilterFrameStyle();

    DefectReviewViewModel    *m_viewModel   = nullptr;
    DefectMapCanvas          *m_canvas      = nullptr;
    QFrame                   *m_filterFrame = nullptr;
    QWidget                  *m_filterHost  = nullptr;
    DefectTypeFilterRow      *m_selectAllRow        = nullptr;
    DefectTypeFilterRow      *m_selectAllDefectsRow = nullptr;
    DefectTypeFilterRow      *m_okFilterRow         = nullptr;
    QVector<DefectTypeFilterRow *> m_typeRows;
    QPushButton              *m_zoomInBtn   = nullptr;
    QPushButton              *m_zoomOutBtn  = nullptr;
    QPushButton              *m_fitBtn      = nullptr;
    QPushButton              *m_oneToOneBtn = nullptr;
};

} // namespace aoi::review
