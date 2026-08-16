/**
 * @file image_preview.cpp
 * @brief ImagePreview 实现：场景构建、滚轮缩放、ROI/focus 叠加与主题刷新
 *
 * ThemeTokens 用量（叠加层）：accentBorder/accent（ROI pen）、focus/accent（focus pen）。
 * 视图背景与边框见 image_preview_style.h。
 */

#include "image/image_preview.h"

#include "image/image_preview_style.h"
#include "theme/theme_refresh.h"

#include <QFrame>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QPen>
#include <QResizeEvent>
#include <QWheelEvent>

namespace ui {

namespace {

/** 创建 zValue=10 的矩形叠加项并加入场景 */
QGraphicsRectItem *makeOverlayRect(QGraphicsScene *scene)
{
    auto *item = new QGraphicsRectItem();
    item->setZValue(10.0);
    scene->addItem(item);
    return item;
}

} // namespace

ImagePreview::ImagePreview(QWidget *parent)
    : QGraphicsView(parent)
{
    setObjectName(QStringLiteral("imagePreviewView"));
    setFrameShape(QFrame::NoFrame);
    setRenderHint(QPainter::Antialiasing, true);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);

    m_scene = new QGraphicsScene(this);
    setScene(m_scene);

    m_pixmapItem = m_scene->addPixmap(QPixmap());
    m_roiItem    = makeOverlayRect(m_scene);
    m_focusItem  = makeOverlayRect(m_scene);
    m_roiItem->setVisible(false);
    m_focusItem->setVisible(false);

    bindTheme();
    refreshStyle();
}

void ImagePreview::setWheelZoomEnabled(bool enabled)
{
    m_wheelZoomEnabled = enabled;
    setDragMode(enabled ? QGraphicsView::ScrollHandDrag : QGraphicsView::NoDrag);
}

void ImagePreview::setImage(const QImage &image)
{
    m_image = image;
    rebuildPixmap();
    m_fitPending = true;
    fitToWindow();
}

void ImagePreview::clearImage()
{
    m_image = QImage();
    rebuildPixmap();
    resetView();
}

void ImagePreview::setRoiRect(const QRectF &roiInImageCoords)
{
    m_roiRect = roiInImageCoords;
    m_hasRoi  = m_roiRect.isValid() && !m_roiRect.isEmpty();
    updateOverlayItems();
}

void ImagePreview::clearRoi()
{
    m_hasRoi = false;
    updateOverlayItems();
}

void ImagePreview::setFocusRect(const QRectF &rectInImageCoords)
{
    m_focusRect = rectInImageCoords;
    m_hasFocus  = m_focusRect.isValid() && !m_focusRect.isEmpty();
    updateOverlayItems();
}

void ImagePreview::clearFocusRect()
{
    m_hasFocus = false;
    updateOverlayItems();
}

void ImagePreview::fitToWindow()
{
    if (!m_pixmapItem || m_pixmapItem->pixmap().isNull()) {
        resetTransform();
        m_userScale = 1.0;
        return;
    }

    fitInView(m_pixmapItem, Qt::KeepAspectRatio);
    m_userScale = transform().m11();
    m_fitPending = false;
}

void ImagePreview::resetView()
{
    resetTransform();
    m_userScale = 1.0;
    centerOn(m_pixmapItem);
}

void ImagePreview::wheelEvent(QWheelEvent *event)
{
    if (!m_wheelZoomEnabled || !m_pixmapItem || m_pixmapItem->pixmap().isNull()) {
        QGraphicsView::wheelEvent(event);
        return;
    }

    const double factor = event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15;
    m_userScale = qBound(0.05, m_userScale * factor, 32.0);
    applyViewTransform();
    event->accept();
}

void ImagePreview::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    if (m_fitPending)
        fitToWindow();
}

void ImagePreview::bindTheme()
{
    bindThemeRefresh(this, [this]() { refreshStyle(); });
}

void ImagePreview::refreshStyle()
{
    setStyleSheet(buildImagePreviewStylesheet(ThemeTokens::resolve()));
    updateOverlayItems();
}

void ImagePreview::rebuildPixmap()
{
    if (!m_pixmapItem)
        return;

    if (m_image.isNull()) {
        m_pixmapItem->setPixmap(QPixmap());
        m_scene->setSceneRect(QRectF());
        return;
    }

    m_pixmapItem->setPixmap(QPixmap::fromImage(m_image));
    m_scene->setSceneRect(m_pixmapItem->boundingRect());
    updateOverlayItems();
}

void ImagePreview::updateOverlayItems()
{
    const ThemeTokens t = ThemeTokens::resolve();

    if (m_roiItem) {
        if (m_hasRoi) {
            QPen pen(t.accentBorder.isValid() ? t.accentBorder : t.accent);
            pen.setWidthF(2.0);
            pen.setCosmetic(true);
            m_roiItem->setPen(pen);
            m_roiItem->setBrush(Qt::NoBrush);
            m_roiItem->setRect(m_roiRect);
            m_roiItem->setVisible(true);
        } else {
            m_roiItem->setVisible(false);
        }
    }

    if (m_focusItem) {
        if (m_hasFocus) {
            QPen pen(t.focus.isValid() ? t.focus : t.accent);
            pen.setStyle(Qt::DashLine);
            pen.setWidthF(2.0);
            pen.setCosmetic(true);
            m_focusItem->setPen(pen);
            m_focusItem->setBrush(Qt::NoBrush);
            m_focusItem->setRect(m_focusRect);
            m_focusItem->setVisible(true);
        } else {
            m_focusItem->setVisible(false);
        }
    }
}

void ImagePreview::applyViewTransform()
{
    resetTransform();
    scale(m_userScale, m_userScale);
}

} // namespace ui
