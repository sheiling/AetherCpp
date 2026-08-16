#include "crossitem.h"

#include <QPainter>
#include <QDebug>

CrossItem::CrossItem(const QLineF &lineH, const QLineF &lineV, QGraphicsItem *parent)
	:QGraphicsItem(parent),
	m_lineH(lineH),
	m_lineV(lineV)
{
	this->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
	this->setFlag(QGraphicsItem::ItemIsSelectable, false);
}

CrossItem::~CrossItem()
{
}

QRectF CrossItem::boundingRect() const
{
	return QRectF(-CROSS_SIZE, -CROSS_SIZE, 2 * CROSS_SIZE, 2 * CROSS_SIZE);
}

void CrossItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	painter->save();

	painter->setPen(QPen(Qt::green, 1.0, Qt::SolidLine));
	painter->drawLine(m_lineH);
	painter->drawLine(m_lineV);

	painter->restore();
}