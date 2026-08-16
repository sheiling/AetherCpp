#pragma once

#include <QGraphicsEllipseItem>
#include "graphicsviewdef.h"

class CircleItem : public QGraphicsEllipseItem
{
public:
	CircleItem(const QRectF &rect, QGraphicsItem *parent = nullptr);
	~CircleItem();
	enum { Type = UserType + E_TYPE_CIRCLE};
	void SetBoundingRect(const QRectF &rect);

public:
	virtual QPainterPath shape() const;
	virtual QRectF boundingRect() const;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);
	virtual int type() const { return Type; }

protected:
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
	QRectF m_boundingRect;
};
