#include "polygonitem.h"
#include "handlesize.h"

#include <QPainter>
#include <QDebug>

PolygonItem::PolygonItem(const QPolygonF &polygon, QGraphicsItem *parent)
	: QGraphicsPolygonItem(polygon, parent),
	m_bDraw(false)
{
	this->setFlag(QGraphicsItem::ItemIsMovable, true);
	this->setFlag(QGraphicsItem::ItemIsSelectable, true);
}

PolygonItem::~PolygonItem()
{
}

void PolygonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	painter->save();

	painter->setPen(QPen(Qt::blue, 1.0 / parentItem()->scale(), Qt::SolidLine));
	painter->drawPolygon(this->polygon());

	painter->restore();
}

QVariant PolygonItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if (QGraphicsItem::ItemSelectedHasChanged == change)
	{
		QList<QGraphicsItem *> chidren = this->childItems();
		for (QGraphicsItem *child : chidren)
		{
			if(m_bDraw)
			{
				child->setVisible(true);
			}
			else
			{
				child->setVisible(value.toBool());
			}
		}
	}
	return QGraphicsPolygonItem::itemChange(change, value);
}
