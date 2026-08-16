#pragma once

/**
 * @file image_preview.h
 * @brief 图像预览控件：缩放、平移、ROI 与焦点矩形叠加
 *
 * 基于 QGraphicsView，支持滚轮缩放（可关闭）、加载后 fitToWindow、
 * 在图像坐标系中绘制 ROI（实线）与 focus（虚线）矩形。
 * 样式由 image_preview_style.h 生成 QSS，叠加层 pen 由 ThemeTokens 驱动。
 */

#include <QGraphicsView>
#include <QImage>
#include <QRectF>
#include "component_global.h"

class QGraphicsPixmapItem;
class QGraphicsRectItem;
class QGraphicsScene;

namespace ui {

/**
 * 可交互图像预览视图。
 *
 * 坐标约定：setRoiRect / setFocusRect 传入的 QRectF 均为**图像像素坐标**，
 * 与 m_image 左上角为 (0,0) 对齐，不受视图 transform 影响。
 */
class UI_COMPONENT_EXPORT ImagePreview : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ImagePreview(QWidget *parent = nullptr);

    /** 启用/禁用滚轮缩放；禁用时 dragMode 为 NoDrag */
    void setWheelZoomEnabled(bool enabled);
    bool wheelZoomEnabled() const { return m_wheelZoomEnabled; }

    /** 加载图像并 fitToWindow；空图像则清空场景 */
    void setImage(const QImage &image);
    void clearImage();

    /** 在图像坐标系中设置 ROI 实线框；无效或空矩形则隐藏 */
    void setRoiRect(const QRectF &roiInImageCoords);
    void clearRoi();

    /** 在图像坐标系中设置焦点虚线框 */
    void setFocusRect(const QRectF &rectInImageCoords);
    void clearFocusRect();

    /** 等比缩放使整图可见，并重置 m_userScale 为当前 transform 比例 */
    void fitToWindow();
    /** 重置 transform 并将视图中心对准 pixmap */
    void resetView();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void bindTheme();
    void refreshStyle();
    void rebuildPixmap();
    void updateOverlayItems();
    void applyViewTransform();

    QGraphicsScene       *m_scene       = nullptr;
    QGraphicsPixmapItem  *m_pixmapItem = nullptr;
    QGraphicsRectItem    *m_roiItem    = nullptr;   ///< ROI 实线叠加层
    QGraphicsRectItem    *m_focusItem  = nullptr;   ///< 焦点虚线叠加层

    QImage   m_image;
    QRectF   m_roiRect;
    QRectF   m_focusRect;
    bool     m_hasRoi    = false;
    bool     m_hasFocus  = false;
    bool     m_wheelZoomEnabled = true;
    double   m_userScale = 1.0;      ///< 相对 fit 基准的用户缩放倍率
    bool     m_fitPending = false;   ///< resize 后待执行 fitToWindow
};

} // namespace ui
