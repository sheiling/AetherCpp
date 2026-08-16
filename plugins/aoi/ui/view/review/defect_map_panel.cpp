/**
 * @file defect_map_panel.cpp
 */

#include "review/defect_map_panel.h"

#include "review/defect_map_canvas.h"
#include "review/defect_type_filter_row.h"
#include "vm/review/defect_review_view_model.h"

#include "component/controls/button/themed_push_button.h"
#include "component/theme/theme_refresh.h"
#include "component/theme/theme_tokens.h"
#include "uibase/languagemanager.h"

#include <QHBoxLayout>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QVBoxLayout>

namespace aoi::review {

DefectMapPanel::DefectMapPanel(DefectReviewViewModel *viewModel, QWidget *parent)
    : QWidget(parent)
    , m_viewModel(viewModel)
{
    setFocusPolicy(Qt::StrongFocus);
    buildUi();
    wireViewModel();
    ui::bindThemeRefresh(this, [this]() {
        applyFilterFrameStyle();
        retranslateUi();
    });
    retranslateUi();
    refreshFromViewModel();
}

void DefectMapPanel::buildUi()
{
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(8);

    auto *filterScroll = new QScrollArea(this);
    filterScroll->setWidgetResizable(true);
    filterScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    filterScroll->setFrameShape(QFrame::NoFrame);
    filterScroll->setFixedWidth(168);

    m_filterFrame = new QFrame(filterScroll);
    m_filterFrame->setObjectName(QStringLiteral("defectReviewTypeFilterFrame"));

    m_filterHost = new QWidget(m_filterFrame);
    auto *filterLayout = new QVBoxLayout(m_filterHost);
    filterLayout->setContentsMargins(6, 8, 6, 8);
    filterLayout->setSpacing(2);

    m_selectAllRow = new DefectTypeFilterRow(QColor(), m_filterHost, false);
    filterLayout->addWidget(m_selectAllRow);

    m_selectAllDefectsRow = new DefectTypeFilterRow(QColor(), m_filterHost, false);
    filterLayout->addWidget(m_selectAllDefectsRow);

    filterLayout->addSpacing(24);

    m_okFilterRow = new DefectTypeFilterRow(okSwatchColor(), m_filterHost);
    filterLayout->addWidget(m_okFilterRow);

    rebuildTypeFilters();
    filterLayout->addStretch(1);

    auto *frameLayout = new QVBoxLayout(m_filterFrame);
    frameLayout->setContentsMargins(0, 0, 0, 0);
    frameLayout->addWidget(m_filterHost);
    filterScroll->setWidget(m_filterFrame);
    root->addWidget(filterScroll);

    auto *mapCol = new QVBoxLayout();
    mapCol->setSpacing(4);

    auto *toolbar = new QHBoxLayout();
    m_zoomInBtn   = new ui::ThemedPushButton(this);
    m_zoomOutBtn  = new ui::ThemedPushButton(this);
    m_fitBtn      = new ui::ThemedPushButton(this);
    m_oneToOneBtn = new ui::ThemedPushButton(this);
    toolbar->addWidget(m_zoomInBtn);
    toolbar->addWidget(m_zoomOutBtn);
    toolbar->addWidget(m_fitBtn);
    toolbar->addWidget(m_oneToOneBtn);
    toolbar->addStretch(1);
    mapCol->addLayout(toolbar);

    m_canvas = new DefectMapCanvas(this);
    mapCol->addWidget(m_canvas, 1);

    root->addLayout(mapCol, 1);

    connect(m_zoomInBtn, &QPushButton::clicked, m_canvas, &DefectMapCanvas::zoomIn);
    connect(m_zoomOutBtn, &QPushButton::clicked, m_canvas, &DefectMapCanvas::zoomOut);
    connect(m_fitBtn, &QPushButton::clicked, m_canvas, &DefectMapCanvas::fitView);
    connect(m_oneToOneBtn, &QPushButton::clicked, m_canvas, &DefectMapCanvas::resetOneToOne);

    connect(m_selectAllRow, &DefectTypeFilterRow::toggled, this, [this](bool on) {
        if (m_viewModel)
            m_viewModel->setAllFiltersChecked(on);
    });
    connect(m_selectAllDefectsRow, &DefectTypeFilterRow::toggled, this, [this](bool on) {
        if (m_viewModel)
            m_viewModel->setAllDefectTypesChecked(on);
    });
    connect(m_okFilterRow, &DefectTypeFilterRow::toggled, this, [this](bool on) {
        if (m_viewModel)
            m_viewModel->setOkFilterChecked(on);
    });

    applyFilterFrameStyle();
}

void DefectMapPanel::applyFilterFrameStyle()
{
    if (!m_filterFrame)
        return;
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    m_filterFrame->setStyleSheet(QStringLiteral(
        "#defectReviewTypeFilterFrame {"
        "  background: %1;"
        "  border: 1px solid %2;"
        "  border-radius: 8px;"
        "}")
                                     .arg(t.surface.name(QColor::HexArgb), t.surfaceBorder.name(QColor::HexArgb)));
}

void DefectMapPanel::rebuildTypeFilters()
{
    if (!m_filterHost)
        return;

    auto *layout = qobject_cast<QVBoxLayout *>(m_filterHost->layout());
    if (!layout)
        return;

    for (DefectTypeFilterRow *row : m_typeRows) {
        layout->removeWidget(row);
        row->deleteLater();
    }
    m_typeRows.clear();

    for (int i = 0; i < inspection::kInspectionDefectTypeCount; ++i) {
        const auto type = static_cast<inspection::InspectionDefectType>(i);
        auto      *row  = new DefectTypeFilterRow(defectTypeColor(type), m_filterHost);
        row->setProperty("defectType", static_cast<int>(type));
        const int insertIndex = layout->indexOf(m_okFilterRow) + 1 + m_typeRows.size();
        layout->insertWidget(insertIndex, row);
        m_typeRows.append(row);

        connect(row, &DefectTypeFilterRow::toggled, this, [this, type](bool on) {
            if (m_viewModel)
                m_viewModel->setTypeChecked(type, on);
        });
    }
}

void DefectMapPanel::wireViewModel()
{
    if (!m_viewModel)
        return;

    connect(m_canvas, &DefectMapCanvas::lampClicked, this, [this](const LampKey &key) {
        if (m_viewModel)
            m_viewModel->selectLamp(key, false);
    });
    connect(m_viewModel, &DefectReviewViewModel::sessionChanged, this, &DefectMapPanel::refreshFromViewModel);
    connect(m_viewModel, &DefectReviewViewModel::selectionChanged, this, &DefectMapPanel::refreshFromViewModel);
}

void DefectMapPanel::refreshFromViewModel()
{
    if (!m_viewModel || !m_canvas)
        return;

    syncTypeFilters();

    DefectMapCanvasState state;
    state.dieGrid         = m_viewModel->dieGrid();
    state.gridValid       = state.dieGrid.isValid();
    state.visibleTypeMask = m_viewModel->visibleTypeMask();
    state.showOkFilter    = m_viewModel->isOkFilterChecked();
    state.records         = m_viewModel->lampRecords();
    state.dieChipLayout   = m_viewModel->dieChipLayout();
    state.visibleKeys     = m_viewModel->visibleLampKeys();
    state.selectedKey     = m_viewModel->selectedKey();
    m_canvas->applyState(state);
}

void DefectMapPanel::centerOnVisibleRow(int visibleRowIndex)
{
    if (!m_viewModel || !m_canvas)
        return;

    const QVector<LampKey> &keys = m_viewModel->visibleLampKeys();
    if (visibleRowIndex < 0 || visibleRowIndex >= keys.size())
        return;

    m_canvas->centerOnLamp(keys[visibleRowIndex]);
}

void DefectMapPanel::syncTypeFilters()
{
    if (!m_viewModel)
        return;

    if (m_selectAllRow) {
        QSignalBlocker blocker(m_selectAllRow);
        m_selectAllRow->setCheckState(m_viewModel->allFiltersCheckState());
        m_selectAllRow->setLabelText(TR("aoi.review.filter.select_all"));
    }

    if (m_selectAllDefectsRow) {
        QSignalBlocker blocker(m_selectAllDefectsRow);
        m_selectAllDefectsRow->setCheckState(m_viewModel->allDefectTypesCheckState());
        m_selectAllDefectsRow->setLabelText(TR("aoi.review.filter.select_all_defects"));
    }

    if (m_okFilterRow) {
        QSignalBlocker blocker(m_okFilterRow);
        m_okFilterRow->setChecked(m_viewModel->isOkFilterChecked());
        m_okFilterRow->setLabelText(TR("aoi.review.result.ok"));
    }

    for (DefectTypeFilterRow *row : m_typeRows) {
        const auto type    = static_cast<inspection::InspectionDefectType>(row->property("defectType").toInt());
        const bool enabled = m_viewModel->isTypeEnabled(type);
        QSignalBlocker blocker(row);
        row->setEnabled(enabled);
        row->setChecked(enabled && m_viewModel->isTypeChecked(type));
        row->setLabelText(defectTypeFilterLabel(type));
    }
}

void DefectMapPanel::retranslateUi()
{
    m_zoomInBtn->setText(TR("aoi.review.map.zoom_in"));
    m_zoomOutBtn->setText(TR("aoi.review.map.zoom_out"));
    m_fitBtn->setText(TR("aoi.review.map.fit"));
    m_oneToOneBtn->setText(TR("aoi.review.map.one_to_one"));
    syncTypeFilters();
}

} // namespace aoi::review
