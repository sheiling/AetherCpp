#pragma once

#include <QGraphicsItem>
#include <QImage>
#include <QList>
#include <QPointF>

class QGraphicsItemGroup;
#include "graphicsviewdef.h"

#define EVENT_IMAGE_INFO	"event-image-info"

class ImageItem : public QGraphicsItem
{
public:
	ImageItem(int winId);
	~ImageItem();
	enum { Type = UserType + E_TYPE_IMG };

	void ResetItem();
	bool SetItem(QImage image);
	void SetRoiType(int type);
	void SetRoi(E_ItemType itemType, const QList<double> &roi);
	void DrawPolygon(const QVector<QPointF> &points, E_DrawStep eStep);
	void SetGrid(bool bGrid);
	bool GetRoi(E_ItemType &itemType, QList<double> &roi) const;
	QImage currentImage() const;
	QRectF currentRoiRect() const;
	void SetDefectCrosshair(double imageX, double imageY);
	void ClearDefectCrosshair();

public:
	virtual QRectF boundingRect() const;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);
	virtual int type() const { return Type; }

protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
	virtual void wheelEvent(QGraphicsSceneWheelEvent *event);
	virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);

private:
	void drawRect(const QPointF &prePos, const QPointF &curPos);
	void drawRotRect(const QPointF &prePos, const QPointF &curPos);
	void drawCircle(const QPointF &prePos, const QPointF &curPos);
	void drawRing(const QPointF &prePos, const QPointF &curPos);

private:
	/** 按 m_scale 重建缺陷十字线（滚轮缩放后保持屏幕像素大小） */
	void rebuildDefectCrosshair();

private:
	int m_winId;
	int m_roiType;
	QImage m_image;
	qreal m_scale;
	bool m_bGrid;

	QGraphicsItem *m_roiItem;
	QGraphicsItemGroup *m_crosshairGroup = nullptr;
	double m_crosshairImageX = -1.0;
	double m_crosshairImageY = -1.0;
	bool m_bMove;
	QPointF m_scenePos;
};
