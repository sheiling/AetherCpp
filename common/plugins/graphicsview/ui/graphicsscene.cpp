#include "graphicsscene.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QGraphicsView>
#include <QKeyEvent>

GraphicsScene::GraphicsScene(QObject *parent)
	: QGraphicsScene(parent),
	m_roiType(E_TYPE_NONE), 
	m_imageItem(nullptr),
	m_bMove(false)
{
}

GraphicsScene::~GraphicsScene()
{
}

void GraphicsScene::SetImageItem(ImageItem *imageItem)
{
	m_imageItem = imageItem;
}

void GraphicsScene::SetRoiType(int type)
{
	m_roiType = type;
	m_bMove = false;
	m_points.clear();
}

QList<QGraphicsItem *> GraphicsScene::selectedRoiItems() const
{
	QList<QGraphicsItem *> roiItems;
	for (QGraphicsItem *item : selectedItems()) {
		const int type = item->type() - QGraphicsItem::UserType;
		if (type >= E_TYPE_RECT && type <= E_TYPE_POLYGON) {
			roiItems.append(item);
		}
	}
	return roiItems;
}

void GraphicsScene::moveSelectedRoiItems(qreal dx, qreal dy)
{
	for (QGraphicsItem *item : selectedRoiItems()) {
		item->moveBy(dx, dy);
	}
}

void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	if (!m_imageItem) {
		return;
	}

	if (m_roiType != E_TYPE_POLYGON) {
		if (mouseEvent->button() == Qt::RightButton) {
			m_imageItem->ResetItem();
		}
		QGraphicsScene::mousePressEvent(mouseEvent);
		return;
	}

	if (mouseEvent->button() == Qt::LeftButton) {
		m_bMove = true;
		m_points.append(m_imageItem->mapFromScene(mouseEvent->scenePos()));
		m_imageItem->DrawPolygon(m_points, E_STEP_START);
		mouseEvent->accept();
	} else if (mouseEvent->button() == Qt::RightButton) {
		m_bMove = false;
		m_roiType = E_TYPE_NONE;
		m_imageItem->DrawPolygon(m_points, E_STEP_END);
		mouseEvent->accept();
	}
}

void GraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
	QGraphicsScene::mouseMoveEvent(mouseEvent);

	if (!m_imageItem || !m_bMove) {
		return;
	}

	QVector<QPointF> points = m_points;
	points.append(m_imageItem->mapFromScene(mouseEvent->scenePos()));
	m_imageItem->DrawPolygon(points, E_STEP_MID);
}

void GraphicsScene::keyPressEvent(QKeyEvent *keyEvent)
{
	QGraphicsScene::keyPressEvent(keyEvent);

	if (!m_imageItem) {
		return;
	}

	const QList<QGraphicsItem *> roiItems = selectedRoiItems();
	if (keyEvent->key() == Qt::Key_Delete) {
		if (!roiItems.isEmpty()) {
			SetRoiType(E_TYPE_NONE);
			m_imageItem->SetRoiType(E_TYPE_NONE);
		}
		return;
	}

	switch (keyEvent->key()) {
	case Qt::Key_Left:
		moveSelectedRoiItems(-1, 0);
		break;
	case Qt::Key_Up:
		moveSelectedRoiItems(0, -1);
		break;
	case Qt::Key_Right:
		moveSelectedRoiItems(1, 0);
		break;
	case Qt::Key_Down:
		moveSelectedRoiItems(0, 1);
		break;
	default:
		break;
	}
}
