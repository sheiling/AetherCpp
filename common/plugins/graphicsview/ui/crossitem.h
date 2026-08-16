#pragma once

#include <QGraphicsItem>
#include "graphicsviewdef.h"

class CrossItem : public QGraphicsItem
{
public:
	CrossItem(const QLineF &lineH, const QLineF &lineV, QGraphicsItem *parent = Q_NULLPTR);
	~CrossItem();
	enum { Type = UserType + E_TYPE_CROSS};

public:
	virtual QRectF boundingRect() const;
	virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);
	virtual int type() const { return Type; }

private:
	QLineF m_lineH;
	QLineF m_lineV;
};
