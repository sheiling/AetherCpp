#pragma once

/**
 * @file defect_map_canvas.h
 * @brief 缺陷 MAP 画布：晶粒网格、缺陷点绘制、缩放平移与命中选中
 */

#include "vm/review/defect_review_types.h"

#include "inspection_types.h"

#include <optional>

#include <QHash>
#include <QVector>
#include <QWidget>

namespace aoi::review {

class DefectReviewViewModel;

/** 画布绘制状态快照 */
struct DefectMapCanvasState
{
    DefectReviewDieGrid                  dieGrid;
    inspection::InspectionDefectTypeMask visibleTypeMask = 0;
    bool                                 showOkFilter    = false;
    QHash<LampKey, DefectReviewLampRecord> records;
    QVector<LampKey>                     visibleKeys;
    QHash<LampKey, DefectDieChipInfo>        dieChipLayout;
    std::optional<LampKey>               selectedKey;
    bool                                 gridValid = false;
};

class DefectMapCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit DefectMapCanvas(QWidget *parent = nullptr);

    void applyState(const DefectMapCanvasState &state);
    void centerOnLamp(const LampKey &key);

signals:
    void lampClicked(const aoi::review::LampKey &key);

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void zoomIn();
    void zoomOut();
    void fitView();
    void resetOneToOne();

private:
    QRectF  mapContentRect() const;
    QRectF  visibleMapRect() const;
    QRectF  cellRect(int dieRow, int dieCol) const;
    QPointF mapToWidget(const QPointF &mapPt) const;
    QPointF widgetToMap(const QPointF &widgetPt) const;
    std::optional<LampKey> hitTest(const QPoint &pos) const;
    void applyZoomAtViewportCenter(double factor);
    void    clampPan();
    QVector<inspection::InspectionDefectType> visibleTypesForRecord(
        const DefectReviewLampRecord &rec) const;
    void drawMarkerSquares(QPainter &p, const QRectF &pxRect,
                           const QVector<inspection::InspectionDefectType> &types,
                           bool okOnly) const;
    void drawSelectionFrame(QPainter &p, const QRectF &pxRect) const;
    QColor mapBackgroundColor() const;
    QColor mapGridLineColor() const;
    bool   isOkCell(const LampKey &key) const;
    std::optional<int> chipTypeForCell(const LampKey &key) const;
    QString okCellTooltip(const LampKey &key) const;
    void   updateOkTooltip(const QPoint &pos);

    DefectMapCanvasState m_state;
    double               m_zoom         = 1.0;
    QPointF              m_pan          = QPointF(0.5, 0.5);
    bool                 m_fitPending   = true;
    bool                 m_panDragging  = false;
    QPoint               m_lastDragPos;
};

} // namespace aoi::review
