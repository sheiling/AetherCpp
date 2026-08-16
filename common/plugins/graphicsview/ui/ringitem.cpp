#include "ringitem.h"
#include "handlesize.h"
#include "crossitem.h"

#include <QPainter>
#include <QDebug>

RingItem::RingItem(const QRectF &rect, QGraphicsItem *parent)
	:QGraphicsEllipseItem(rect, parent),
	m_outerRect(rect)
{
	this->setFlag(QGraphicsItem::ItemIsMovable, true);
	this->setFlag(QGraphicsItem::ItemIsSelectable, true);

	QRectF innerRect(0, 0, rect.width() * RING_RATIO, rect.height() * RING_RATIO);
	innerRect.moveCenter(QPointF(0, 0));
	m_innerRect = innerRect;

	//外圆
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

	//内圆
	handleSize = new HandleSize(E_HANDLE_INNER_TOP, handleRect, this);
	handleSize->setPos(0, -rect.height() / 2 * RING_RATIO);

	handleSize = new HandleSize(E_HANDLE_INNER_RIGHT, handleRect, this);
	handleSize->setPos(rect.width() / 2 * RING_RATIO, 0);

	handleSize = new HandleSize(E_HANDLE_INNER_BOTTOM, handleRect, this);
	handleSize->setPos(0, rect.height() / 2 * RING_RATIO);

	handleSize = new HandleSize(E_HANDLE_INNER_LEFT, handleRect, this);
	handleSize->setPos(-rect.width() / 2 * RING_RATIO, 0);

	//中心+
	QLineF lineH(-CROSS_SIZE, 0, CROSS_SIZE, 0);
	QLineF lineV(0, -CROSS_SIZE, 0, CROSS_SIZE);
	CrossItem *crossItem = new CrossItem(lineH, lineV, this);
	crossItem->setPos(0, 0);
}

RingItem::~RingItem()
{
}

void RingItem::SetInnerRect(const QRectF &innerRect)
{
	m_innerRect = innerRect;

	if (m_outerRect.contains(m_innerRect))
	{
		this->setRect(m_outerRect);
		this->update();
	}
	else
	{
		this->setRect(m_innerRect);
	}

	//更新item位置
	QList<QGraphicsItem *> children = this->childItems();
	for (QGraphicsItem *child : children)
	{
		HandleSize *handleSize = qgraphicsitem_cast<HandleSize*>(child);
		if (handleSize)
		{
			switch (handleSize->m_type)
			{
			case E_HANDLE_INNER_TOP:
				handleSize->setPos(0, -innerRect.height() / 2);
				break;
			case E_HANDLE_INNER_RIGHT:
				handleSize->setPos(innerRect.width() / 2, 0);
				break;
			case E_HANDLE_INNER_BOTTOM:
				handleSize->setPos(0, innerRect.height() / 2);
				break;
			case E_HANDLE_INNER_LEFT:
				handleSize->setPos(-innerRect.width() / 2, 0);
				break;
			default:
				break;
			}
		}
	}
}

void RingItem::SetOuterRect(const QRectF &outerRect)
{
	m_outerRect = outerRect;

	if (m_outerRect.contains(m_innerRect))
	{
		this->setRect(m_outerRect);
	}
	else
	{
		this->setRect(m_innerRect);
		this->update();
	}

	//更新item位置
	QList<QGraphicsItem *> children = this->childItems();
	for (QGraphicsItem *child : children)
	{
		CrossItem *crossItem = qgraphicsitem_cast<CrossItem*>(child);
		if (crossItem)
		{
			crossItem->setPos(outerRect.center());
			continue;
		}

		HandleSize *handleSize = qgraphicsitem_cast<HandleSize*>(child);
		if (handleSize)
		{
			switch (handleSize->m_type)
			{
			case E_HANDLE_TOP:
				handleSize->setPos(0, -outerRect.height() / 2);
				break;
			case E_HANDLE_RIGHT:
				handleSize->setPos(outerRect.width() / 2, 0);
				break;
			case E_HANDLE_BOTTOM:
				handleSize->setPos(0, outerRect.height() / 2);
				break;
			case E_HANDLE_LEFT:
				handleSize->setPos(-outerRect.width() / 2, 0);
				break;
			default:
				break;
			}
		}
	}
}

QPainterPath RingItem::shape() const
{
	QPainterPath path;
	path.addEllipse(m_innerRect);
	path.addEllipse(m_outerRect);
	return path;
}

QRectF RingItem::boundingRect() const
{
	if (m_outerRect.contains(m_innerRect))
	{
		return m_outerRect;
	}
	else
	{
		return m_innerRect;
	}
}

void RingItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	painter->save();

	painter->setPen(QPen(Qt::blue, 1.0 / parentItem()->scale(), Qt::SolidLine));
	painter->drawEllipse(m_outerRect);
	painter->drawEllipse(m_innerRect);

	painter->restore();
}

QVariant RingItem::itemChange(GraphicsItemChange change, const QVariant &value)
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
