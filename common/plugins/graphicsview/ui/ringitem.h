#pragma once

#include <QGraphicsEllipseItem>
#include "graphicsviewdef.h"

class RingItem : public QGraphicsEllipseItem
{
public:
	RingItem(const QRectF &rect, QGraphicsItem *parent = nullptr);
	~RingItem();
	enum { Type = UserType + E_TYPE_RING};
	void SetInnerRect(const QRectF &innerRect);
	void SetOuterRect(const QRectF &outerRect);
	QRectF innerRect() const { return m_innerRect; }
	QRectF outerRect() const { return m_outerRect; }

public:
	virtual QPainterPath shape() const;
	virtual QRectF boundingRect() const;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);
	virtual int type() const { return Type; }

protected:
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
	QRectF m_outerRect;
	QRectF m_innerRect;
};
