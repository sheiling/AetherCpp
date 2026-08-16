#pragma once

#include <QGraphicsRectItem>
#include "graphicsviewdef.h"

class RotRectItem : public QGraphicsRectItem
{
public:
	RotRectItem(const QRectF &rect, QGraphicsItem *parent = nullptr);
	~RotRectItem();
	enum { Type = UserType + E_TYPE_ROT_RECT};
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
