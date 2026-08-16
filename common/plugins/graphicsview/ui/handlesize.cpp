#include "handlesize.h"
#include "crossitem.h"

#include <QPainter>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QVector2D>

HandleSize::HandleSize(int type, const QRectF &rect, QGraphicsItem *parent)
	:QGraphicsRectItem(rect, parent),
	m_type(type),
	m_bMove(false),
	m_bHReverse(false),
	m_bVReverse(false)
{
	this->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
}

HandleSize::~HandleSize()
{
}

void HandleSize::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	painter->save();

	if(m_type < E_HANDLE_ROT)
	{
		painter->setPen(QPen(Qt::red, 1.0, Qt::SolidLine));
		painter->drawRect(this->boundingRect());
	}
	else if (E_HANDLE_ROT == m_type)
	{
		painter->setPen(QPen(Qt::green, 1.0, Qt::SolidLine));
		painter->drawEllipse(this->boundingRect());
	}
	else if (E_HANDLE_ROT_BEGIN == m_type)
	{
		painter->setPen(QPen(Qt::green, 1.0, Qt::SolidLine));
		painter->drawEllipse(this->boundingRect());
	}
	else if (E_HANDLE_ROT_END == m_type)
	{
		painter->setPen(QPen(Qt::red, 1.0, Qt::SolidLine));
		painter->drawEllipse(this->boundingRect());
	}
	else
	{
		painter->setPen(QPen(Qt::red, 1.0, Qt::SolidLine));
		painter->drawEllipse(this->boundingRect());
	}

	painter->restore();
}

void HandleSize::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if (event->button() != Qt::LeftButton)
	{
		return;
	}

	m_bMove = true;
	m_scenePos = event->scenePos();
}

void HandleSize::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if (!m_bMove)
	{
		return;
	}

	if (!parentItem())
	{
		return;
	}

	if (!parentItem()->parentItem())
	{
		return;
	}

	//偏移
	QPointF scenePos = event->scenePos();
	QPointF prePos = parentItem()->mapFromScene(m_scenePos);
	QPointF curPos = parentItem()->mapFromScene(scenePos);
	QPointF offset = curPos - prePos;
	QLineF preLine(0, 0, prePos.x(), prePos.y());
	QLineF curLine(0, 0, curPos.x(), curPos.y());
	m_scenePos = scenePos;

	//矩形
	RectItem *rectItem = qgraphicsitem_cast<RectItem*>(parentItem());
	if(rectItem)
	{
		moveRect(rectItem, offset);
	}

	//圆
	CircleItem *circleItem = qgraphicsitem_cast<CircleItem*>(parentItem());
	if (circleItem)
	{		
		moveCircle(circleItem, curPos);
	}

	//旋转矩形
	RotRectItem *rotRectItem = qgraphicsitem_cast<RotRectItem*>(parentItem());
	if (rotRectItem)
	{
		QPointF pt0 = rotRectItem->pos();
		QPointF pt1 = rotRectItem->parentItem()->mapFromScene(scenePos);
		QLineF line(pt0, pt1);
		moveRotRect(rotRectItem, -line.angle(), offset);
	}

	//同心圆
	RingItem *ringItem = qgraphicsitem_cast<RingItem*>(parentItem());
	if (ringItem)
	{
		moveRing(ringItem, curPos);
	}

	//多边形
	PolygonItem *polygonItem = qgraphicsitem_cast<PolygonItem*>(parentItem());
	if (polygonItem)
	{
		movePolygon(polygonItem, curPos);
	}
}

void HandleSize::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	m_bMove = false;
	m_bVReverse = false;
	m_bHReverse = false;
}

void HandleSize::moveRect(RectItem *rectItem, const QPointF &offset)
{
	qreal width = rectItem->boundingRect().width();
	qreal height = rectItem->boundingRect().height();
	QRectF leftTopRect(offset.x(), offset.y(), width - offset.x(), height - offset.y());
	QRectF topRect(0, offset.y(), width, height - offset.y());
	QRectF rightTopRect(0, offset.y(), width + offset.x(), height - offset.y());
	QRectF rightRect(0, 0, width + offset.x(), height);
	QRectF rightBottomRect(0, 0, width + offset.x(), height + offset.y());
	QRectF bottomRect(0, 0, width, height + offset.y());
	QRectF leftBottomRect(offset.x(), 0, width - offset.x(), height + offset.y());
	QRectF leftRect(offset.x(), 0, width - offset.x(), height);

	//获取矩形
	QRectF rect;
	switch (m_type)
	{
	case E_HANDLE_LEFT_TOP:
		if (m_bHReverse && m_bVReverse)
		{
			rect = rightBottomRect;
		}
		else if (m_bHReverse)
		{
			rect = rightTopRect;
		}
		else if (m_bVReverse)
		{
			rect = leftBottomRect;
		}
		else
		{
			rect = leftTopRect;
		}
		break;
	case E_HANDLE_TOP:
		if (m_bHReverse && m_bVReverse)
		{
			rect = bottomRect;
		}
		else if (m_bVReverse)
		{
			rect = bottomRect;
		}
		else
		{
			rect = topRect;
		}
		break;
	case E_HANDLE_RIGHT_TOP:
		if (m_bHReverse && m_bVReverse)
		{
			rect = leftBottomRect;
		}
		else if (m_bHReverse)
		{
			rect = leftTopRect;
		}
		else if (m_bVReverse)
		{
			rect = rightBottomRect;
		}
		else
		{
			rect = rightTopRect;
		}
		break;
	case E_HANDLE_RIGHT:
		if (m_bHReverse && m_bVReverse)
		{
			rect = leftRect;
		}
		else if (m_bHReverse)
		{
			rect = leftRect;
		}
		else
		{
			rect = rightRect;
		}
		break;
	case E_HANDLE_RIGHT_BOTTOM:
		if (m_bHReverse && m_bVReverse)
		{
			rect = leftTopRect;
		}
		else if (m_bHReverse)
		{
			rect = leftBottomRect;
		}
		else if (m_bVReverse)
		{
			rect = rightTopRect;
		}
		else
		{
			rect = rightBottomRect;
		}
		break;
	case E_HANDLE_BOTTOM:
		if (m_bHReverse && m_bVReverse)
		{
			rect = topRect;
		}
		else if (m_bVReverse)
		{
			rect = topRect;
		}
		else
		{
			rect = bottomRect;
		}
		break;
	case E_HANDLE_LEFT_BOTTOM:
		if (m_bHReverse && m_bVReverse)
		{
			rect = rightTopRect;
		}
		else if (m_bHReverse)
		{
			rect = rightBottomRect;
		}
		else if (m_bVReverse)
		{
			rect = leftTopRect;
		}
		else
		{
			rect = leftBottomRect;
		}
		break;
	case E_HANDLE_LEFT:
		if (m_bHReverse && m_bVReverse)
		{
			rect = rightRect;
		}
		else if (m_bHReverse)
		{
			rect = rightRect;
		}
		else
		{
			rect = leftRect;
		}
		break;
	default:
		break;
	}

	//更新水平+竖直反转
	if (rect.width() < 0)
	{
		m_bHReverse = !m_bHReverse;
	}
	if (rect.height() < 0)
	{
		m_bVReverse = !m_bVReverse;
	}

	//标准化
	QRectF normalRect = rect.normalized();
	QRectF adjustRect = normalRect;
	adjustRect.moveTo(0, 0);

	//调整矩形
	rectItem->SetBoundingRect(adjustRect);
	rectItem->moveBy(normalRect.x(), normalRect.y());
}

void HandleSize::moveRotRect(RotRectItem *rotRectItem, qreal angle, const QPointF &offset)
{
	//旋转
	if (E_HANDLE_ROT == m_type)
	{
		rotRectItem->setRotation(angle);
		return;
	}

	qreal width = rotRectItem->boundingRect().width();
	qreal height = rotRectItem->boundingRect().height();
	QRectF leftTopRect(-width / 2 + offset.x(), -height / 2 + offset.y(), width - 2 * offset.x(), height - 2 * offset.y());
	QRectF topRect(-width / 2, -height / 2 + offset.y(), width, height - 2 * offset.y());
	QRectF rightTopRect(-width / 2- offset.x(), -height / 2 + offset.y(), width + 2 * offset.x(), height - 2 * offset.y());
	QRectF rightRect(-width / 2 - offset.x(), -height / 2, width + 2 * offset.x(), height);
	QRectF rightBottomRect(-width / 2 - offset.x(), -height / 2 - offset.y(), width + 2 * offset.x(), height + 2 * offset.y());
	QRectF bottomRect(-width / 2, -height / 2 - offset.y(), width, height + 2 * offset.y());
	QRectF leftBottomRect(-width / 2 + offset.x(), -height / 2 - offset.y(), width - 2 * offset.x(), height + 2 * offset.y());
	QRectF leftRect(-width / 2 + offset.x(), -height / 2, width - 2 * offset.x(), height);

	//获取矩形
	QRectF rect;
	switch (m_type)
	{
	case E_HANDLE_LEFT_TOP:
		if (m_bHReverse && m_bVReverse)
		{
			rect = rightBottomRect;
		}
		else if (m_bHReverse)
		{
			rect = rightTopRect;
		}
		else if (m_bVReverse)
		{
			rect = leftBottomRect;
		}
		else
		{
			rect = leftTopRect;
		}
		break;
	case E_HANDLE_TOP:
		if (m_bHReverse && m_bVReverse)
		{
			rect = bottomRect;
		}
		else if (m_bVReverse)
		{
			rect = bottomRect;
		}
		else
		{
			rect = topRect;
		}
		break;
	case E_HANDLE_RIGHT_TOP:
		if (m_bHReverse && m_bVReverse)
		{
			rect = leftBottomRect;
		}
		else if (m_bHReverse)
		{
			rect = leftTopRect;
		}
		else if (m_bVReverse)
		{
			rect = rightBottomRect;
		}
		else
		{
			rect = rightTopRect;
		}
		break;
	case E_HANDLE_RIGHT:
		if (m_bHReverse && m_bVReverse)
		{
			rect = leftRect;
		}
		else if (m_bHReverse)
		{
			rect = leftRect;
		}
		else
		{
			rect = rightRect;
		}
		break;
	case E_HANDLE_RIGHT_BOTTOM:
		if (m_bHReverse && m_bVReverse)
		{
			rect = leftTopRect;
		}
		else if (m_bHReverse)
		{
			rect = leftBottomRect;
		}
		else if (m_bVReverse)
		{
			rect = rightTopRect;
		}
		else
		{
			rect = rightBottomRect;
		}
		break;
	case E_HANDLE_BOTTOM:
		if (m_bHReverse && m_bVReverse)
		{
			rect = topRect;
		}
		else if (m_bVReverse)
		{
			rect = topRect;
		}
		else
		{
			rect = bottomRect;
		}
		break;
	case E_HANDLE_LEFT_BOTTOM:
		if (m_bHReverse && m_bVReverse)
		{
			rect = rightTopRect;
		}
		else if (m_bHReverse)
		{
			rect = rightBottomRect;
		}
		else if (m_bVReverse)
		{
			rect = leftTopRect;
		}
		else
		{
			rect = leftBottomRect;
		}
		break;
	case E_HANDLE_LEFT:
		if (m_bHReverse && m_bVReverse)
		{
			rect = rightRect;
		}
		else if (m_bHReverse)
		{
			rect = rightRect;
		}
		else
		{
			rect = leftRect;
		}
		break;
	default:
		break;
	}

	//更新水平+竖直反转
	if (rect.width() < 0)
	{
		m_bHReverse = !m_bHReverse;
	}
	if (rect.height() < 0)
	{
		m_bVReverse = !m_bVReverse;
	}

	// 调整矩形
	QRectF normalRect = rect.normalized();
	rotRectItem->SetBoundingRect(normalRect);
}

void HandleSize::moveCircle(CircleItem *circleItem, const QPointF &curPos)
{
	//更新外接矩形
	QVector2D vector(curPos.x(), curPos.y());
	qreal radius = vector.length();
	QRectF rect(-radius, -radius, 2 * radius, 2 * radius);

	//调整矩形
	circleItem->SetBoundingRect(rect);
}

void HandleSize::moveRing(RingItem *ringItem, const QPointF &curPos)
{
	QVector2D vector(curPos.x(), curPos.y());
	qreal radius = vector.length();
	QRectF rect(-radius, -radius, 2 * radius, 2 * radius);

	//调整矩形
	switch (m_type)
	{
	case E_HANDLE_INNER_TOP:
	case E_HANDLE_INNER_RIGHT:
	case E_HANDLE_INNER_BOTTOM:
	case E_HANDLE_INNER_LEFT:
		ringItem->SetInnerRect(rect);
		break;
	case E_HANDLE_TOP:
	case E_HANDLE_RIGHT:
	case E_HANDLE_BOTTOM:
	case E_HANDLE_LEFT:
		ringItem->SetOuterRect(rect);
		break;
	default:
		break;
	}
}

void HandleSize::movePolygon(PolygonItem *polygonItem, const QPointF &curPos)
{
	if (m_type < E_HANDLE_CONTROL)
	{
		return;
	}

	QPolygonF polygon = polygonItem->polygon();
	polygon[m_type- E_HANDLE_CONTROL] = curPos;
	this->setPos(curPos);
	polygonItem->setPolygon(polygon);
}