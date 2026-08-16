#include "circleitem.h"
#include "handlesize.h"
#include "crossitem.h"

#include <QPainter>
#include <QDebug>

CircleItem::CircleItem(const QRectF &rect, QGraphicsItem *parent)
	:QGraphicsEllipseItem(rect, parent),
	m_boundingRect(rect)
{
	this->setFlag(QGraphicsItem::ItemIsMovable, true);
	this->setFlag(QGraphicsItem::ItemIsSelectable, true);

	QRectF handleRect = QRectF(0, 0, HANDLE_SHAPE_SIZE, HANDLE_SHAPE_SIZE);
	handleRect.moveCenter(QPointF(0, 0));
	HandleSize *handleSize = new HandleSize(E_HANDLE_TOP, handleRect, this);
	handleSize->setPos(0, -rect.height() / 2);

	handleSize = new HandleSize(E_HANDLE_RIGHT, handleRect, this);
	handleSize->setPos(rect.width() / 2, 0);

	handleSize = new HandleSize(E_HANDLE_BOTTOM, handleRect, this);
	handleSize->setPos(0, rect.height() / 2);

	handleSize = new HandleSize(E_HANDLE_LEFT, handleRect, this);
	handleSize->setPos(-rect.width() / 2, 0);

	QLineF lineH(-CROSS_SIZE, 0, CROSS_SIZE, 0);
	QLineF lineV(0, -CROSS_SIZE, 0, CROSS_SIZE);
	CrossItem *crossItem = new CrossItem(lineH, lineV, this);
	crossItem->setPos(0, 0);
}

CircleItem::~CircleItem()
{
}

void CircleItem::SetBoundingRect(const QRectF &rect)
{
	m_boundingRect = rect;
	this->setRect(rect);

	//更新item位置
	QList<QGraphicsItem *> children = this->childItems();
	for (QGraphicsItem *child : children)
	{
		HandleSize *handleSize = qgraphicsitem_cast<HandleSize*>(child);
		if (handleSize)
		{
			qreal radius = rect.width() / 2;
			switch (handleSize->m_type)
			{
			case E_HANDLE_TOP:
				handleSize->setPos(0, -radius);
				break;
			case E_HANDLE_RIGHT:
				handleSize->setPos(radius, 0);
				break;
			case E_HANDLE_BOTTOM:
				handleSize->setPos(0, radius);
				break;
			case E_HANDLE_LEFT:
				handleSize->setPos(-radius, 0);
				break;
			default:
				break;
			}
		}
	}
}

QPainterPath CircleItem::shape() const
{
	QPainterPath path;
	path.addEllipse(boundingRect());
	return path;
}

QRectF CircleItem::boundingRect() const
{
	return m_boundingRect;
}

void CircleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	painter->save();

	painter->setPen(QPen(Qt::blue, 1.0 / parentItem()->scale(), Qt::SolidLine));
	painter->drawEllipse(this->boundingRect());

	painter->restore();
}

QVariant CircleItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if (QGraphicsItem::ItemSelectedHasChanged == change)
	{
		QList<QGraphicsItem *> chidren = this->childItems();
		for (QGraphicsItem *child : chidren)
		{
			child->setVisible(value.toBool());
		}
	}
	return QGraphicsEllipseItem::itemChange(change, value);
}
