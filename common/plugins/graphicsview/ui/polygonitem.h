#pragma once

#include <QGraphicsPolygonItem>
#include "graphicsviewdef.h"

class PolygonItem : public QGraphicsPolygonItem
{
public:
	PolygonItem(const QPolygonF &polygon, QGraphicsItem *parent = nullptr);
	~PolygonItem();
	enum { Type = UserType + E_TYPE_POLYGON};
	void SetDraw(bool bDraw) { m_bDraw = bDraw; }

public:
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);
	virtual int type() const { return Type; }

protected:
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
	bool m_bDraw;
};
