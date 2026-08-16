#pragma once

#include <QGraphicsRectItem>
#include "rectitem.h"
#include "circleitem.h"
#include "rotrectitem.h"
#include "ringitem.h"
#include "polygonitem.h"
#include "graphicsviewdef.h"

class HandleSize : public QGraphicsRectItem
{
public:
	HandleSize(int type, const QRectF &rect, QGraphicsItem *parent = nullptr);
	~HandleSize();
	int m_type;
	enum { Type = UserType + E_TYPE_CTRL};

public:
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);
	virtual int type() const { return Type; }

protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
	void moveRect(RectItem *rectItem, const QPointF &offset);
	void moveRotRect(RotRectItem *rotRectItem, qreal angle, const QPointF &offset);
	void moveCircle(CircleItem *circleItem, const QPointF &curPos);
	void moveRing(RingItem *ringItem, const QPointF &curPos);
	void movePolygon(PolygonItem *polygonItem, const QPointF &curPos);

private:
	bool m_bMove;
	QPointF m_scenePos;

	bool m_bHReverse;
	bool m_bVReverse;
};
