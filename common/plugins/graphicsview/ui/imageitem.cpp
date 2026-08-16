#include "imageitem.h"
#include <QPainter>
#include <QGraphicsView>
#include <cstring>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QLineF>
#include <QVector2D>
#include <QTransform>
#include <QtGlobal>
#include "graphicsviewdef.h"
#include "rectitem.h"
#include "circleitem.h"
#include "rotrectitem.h"
#include "ringitem.h"
#include "polygonitem.h"
#include "handlesize.h"
#include "messagecenter/message_center.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsItemGroup>
#include <QGraphicsLineItem>

ImageItem::ImageItem(int winId)
	:m_winId(winId),
	m_roiItem(nullptr),
	m_bMove(false),
	m_roiType(E_TYPE_NONE),
	m_bGrid(false),
	m_scale(1.0)
{
	//可选择，可移动，鼠标悬浮事件
	this->setFlag(QGraphicsItem::ItemIsSelectable, true);
	this->setFlag(QGraphicsItem::ItemIsMovable, true);
	this->setAcceptHoverEvents(this);
}

ImageItem::~ImageItem()
{
	ClearDefectCrosshair();
}

void ImageItem::ResetItem()
{
	if (this->scene()->views().size() < 1)
	{
		return;
	}

	if (m_image.isNull())
	{
		return;
	}

	//缩放
	QGraphicsView *view = this->scene()->views().at(0);
	qreal scale = qMin(view->width() * 1.0 / m_image.width(), view->height() * 1.0 / m_image.height());
	this->setScale(scale);
	this->setPos(0, 0);
	m_scale = scale;

	//调整位置
	view->setSceneRect(QRectF(0, 0, m_image.width() * scale, m_image.height() * scale));
	view->setFocus();

	if (m_crosshairImageX >= 0.0)
		rebuildDefectCrosshair();
}

bool ImageItem::SetItem(QImage image)
{
	if (image.isNull()) {
		const bool sizeChanged = !m_image.isNull();
		m_image = QImage();
		if (sizeChanged) {
			prepareGeometryChange();
		}
		update();
		return sizeChanged;
	}

	const bool sizeChanged = m_image.isNull() || m_image.size() != image.size();
	m_image = std::move(image);
	if (sizeChanged) {
		prepareGeometryChange();
	}

	update();
	return sizeChanged;
}

void ImageItem::ClearDefectCrosshair()
{
	m_crosshairImageX = -1.0;
	m_crosshairImageY = -1.0;
	if (!m_crosshairGroup)
		return;
	delete m_crosshairGroup;
	m_crosshairGroup = nullptr;
	update();
}

void ImageItem::rebuildDefectCrosshair()
{
	if (m_crosshairImageX < 0.0 || m_image.isNull())
		return;

	if (m_crosshairGroup) {
		delete m_crosshairGroup;
		m_crosshairGroup = nullptr;
	}

	const double imageX = m_crosshairImageX;
	const double imageY = m_crosshairImageY;

	m_crosshairGroup = new QGraphicsItemGroup(this);
	m_crosshairGroup->setPos(imageX, imageY);
	m_crosshairGroup->setZValue(100.0);

	const qreal invScale = 1.0 / qMax(m_scale, 1e-6);
	const qreal arm      = 48.0 * invScale;
	const qreal dotR     = 5.0 * invScale;
	const qreal penW     = 2.0 * invScale;

	QPen pen(QColor(255, 64, 64), penW);
	pen.setCosmetic(false);

	auto *hLine = new QGraphicsLineItem(-arm, 0.0, arm, 0.0, m_crosshairGroup);
	hLine->setPen(pen);
	auto *vLine = new QGraphicsLineItem(0.0, -arm, 0.0, arm, m_crosshairGroup);
	vLine->setPen(pen);

	auto *dot = new QGraphicsEllipseItem(-dotR, -dotR, dotR * 2.0, dotR * 2.0, m_crosshairGroup);
	dot->setPen(pen);
	dot->setBrush(QColor(255, 64, 64));

	update();
}

void ImageItem::SetDefectCrosshair(double imageX, double imageY)
{
	if (m_image.isNull()) {
		ClearDefectCrosshair();
		return;
	}

	m_crosshairImageX = imageX;
	m_crosshairImageY = imageY;
	rebuildDefectCrosshair();
}

QImage ImageItem::currentImage() const
{
	return m_image;
}

QRectF ImageItem::currentRoiRect() const
{
	if (!m_roiItem)
		return {};

	const QRectF raw = m_roiItem->boundingRect();
	if (raw.isNull())
		return {};

	const QRectF normalized = raw.normalized();
	if (normalized.width() < 0.5 || normalized.height() < 0.5)
		return {};

	const QPointF origin = m_roiItem->pos();
	return normalized.translated(origin);
}

void ImageItem::SetRoiType(int type)
{
	m_roiType = type;
	m_bMove = false;

	//删除已有ROI
	if (m_roiItem)
	{
		delete m_roiItem;
		m_roiItem = nullptr;
	}
}

namespace {

QVector<QPointF> roiToPoints(const QList<double> &roi)
{
	QVector<QPointF> points;
	points.reserve(roi.size() / 2);
	for (int i = 0; i + 1 < roi.size(); i += 2)
		points.append(QPointF(roi.at(i), roi.at(i + 1)));
	return points;
}

void createPolygonHandles(PolygonItem *polygonItem)
{
	if (!polygonItem)
		return;

	polygonItem->SetDraw(false);
	const QPolygonF polygon = polygonItem->polygon();
	int handleCount = 0;
	for (QGraphicsItem *child : polygonItem->childItems()) {
		if (qgraphicsitem_cast<HandleSize *>(child))
			++handleCount;
	}

	QRectF handleRect(0, 0, HANDLE_SHAPE_SIZE, HANDLE_SHAPE_SIZE);
	handleRect.moveCenter(QPointF(0, 0));
	for (int i = handleCount; i < polygon.size(); ++i) {
		auto *handleSize = new HandleSize(E_HANDLE_CONTROL + i, handleRect, polygonItem);
		handleSize->setPos(polygon.at(i));
	}
}

void selectRoiItem(QGraphicsItem *roiItem, QGraphicsScene *scene)
{
	if (!roiItem)
		return;

	if (PolygonItem *polygonItem = qgraphicsitem_cast<PolygonItem *>(roiItem))
		createPolygonHandles(polygonItem);

	for (QGraphicsItem *child : roiItem->childItems())
		child->setVisible(false);

	if (scene)
		scene->clearSelection();

	roiItem->setSelected(true);
}

} // namespace

void ImageItem::SetRoi(E_ItemType itemType, const QList<double> &roi)
{
	if (m_image.isNull())
		return;

	m_roiType = E_TYPE_NONE;
	m_bMove = false;
	m_bGrid = false;

	if (m_roiItem)
	{
		delete m_roiItem;
		m_roiItem = nullptr;
	}

	if (roi.isEmpty() || E_TYPE_NONE == itemType)
		return;

	switch (itemType)
	{
	case E_TYPE_RECT:
	{
		const QVector<QPointF> corners = roiToPoints(roi);
		if (corners.size() < 4)
			return;

		qreal minX = corners.at(0).x();
		qreal minY = corners.at(0).y();
		qreal maxX = minX;
		qreal maxY = minY;
		for (const QPointF &point : corners)
		{
			minX = qMin(minX, point.x());
			minY = qMin(minY, point.y());
			maxX = qMax(maxX, point.x());
			maxY = qMax(maxY, point.y());
		}

		const QRectF rect(0, 0, maxX - minX, maxY - minY);
		auto *rectItem = new RectItem(rect, this);
		rectItem->setPos(minX, minY);
		rectItem->SetBoundingRect(rect);
		m_roiItem = rectItem;
		break;
	}
	case E_TYPE_ROT_RECT:
	{
		const QVector<QPointF> corners = roiToPoints(roi);
		if (corners.size() < 4)
			return;

		const QPointF center = (corners.at(0) + corners.at(1) + corners.at(2) + corners.at(3)) / 4.0;
		const QLineF edge1(corners.at(0), corners.at(1));
		const QLineF edge2(corners.at(1), corners.at(2));
		const qreal width = edge1.length();
		const qreal height = edge2.length();
		if (width < 1e-6 || height < 1e-6)
			return;

		const QRectF rotRect(-width / 2, -height / 2, width, height);
		auto *rotRectItem = new RotRectItem(rotRect, this);
		rotRectItem->setPos(center);
		rotRectItem->setRotation(-edge1.angle());
		rotRectItem->SetBoundingRect(rotRect);
		m_roiItem = rotRectItem;
		break;
	}
	case E_TYPE_CIRCLE:
	{
		if (roi.size() < 3)
			return;

		const qreal radius = roi.at(2);
		if (radius < 1e-6)
			return;

		const QRectF circleRect(-radius, -radius, 2 * radius, 2 * radius);
		auto *circleItem = new CircleItem(circleRect, this);
		circleItem->setPos(roi.at(0), roi.at(1));
		circleItem->SetBoundingRect(circleRect);
		m_roiItem = circleItem;
		break;
	}
	case E_TYPE_RING:
	{
		if (roi.size() < 4)
			return;

		const qreal innerRadius = roi.at(2);
		const qreal outerRadius = roi.at(3);
		if (outerRadius < 1e-6)
			return;

		auto *ringItem = new RingItem(QRectF(), this);
		ringItem->SetOuterRect(QRectF(-outerRadius, -outerRadius, 2 * outerRadius, 2 * outerRadius));
		ringItem->SetInnerRect(QRectF(-innerRadius, -innerRadius, 2 * innerRadius, 2 * innerRadius));
		ringItem->setPos(roi.at(0), roi.at(1));
		m_roiItem = ringItem;
		break;
	}
	case E_TYPE_POLYGON:
	{
		const QVector<QPointF> points = roiToPoints(roi);
		if (points.size() < 3)
			return;

		m_roiItem = new PolygonItem(QPolygonF(points), this);
		break;
	}
	default:
		break;
	}

	if (m_roiItem)
		selectRoiItem(m_roiItem, scene());
}

void ImageItem::DrawPolygon(const QVector<QPointF> &points, E_DrawStep eStep)
{
	//正在绘制矩形/圆等 ROI 时，不处理多边形
	if (m_bMove)
	{
		return;
	}

	//新建ROI
	if (!m_roiItem)
	{
		m_roiItem = new PolygonItem(QPolygonF(), this);
	}

	//更新轮廓
	PolygonItem *polygonItem = qgraphicsitem_cast<PolygonItem*>(m_roiItem);
	if (!polygonItem)
	{
		return;
	}

	QPolygonF polygon(points);
	polygonItem->setPolygon(polygon);

	QRectF handleRect(0, 0, HANDLE_SHAPE_SIZE, HANDLE_SHAPE_SIZE);
	handleRect.moveCenter(QPointF(0, 0));
	const int handleTarget = (E_STEP_MID == eStep) ? points.size() - 1 : points.size();
	QList<QGraphicsItem *> children = polygonItem->childItems();
	for (int i = children.size(); i < handleTarget; ++i) {
		HandleSize *handleSize = new HandleSize(E_HANDLE_CONTROL + i, handleRect, polygonItem);
		handleSize->setPos(points[i]);
	}

	if (E_STEP_START == eStep) {
		polygonItem->SetDraw(true);
	} else if (E_STEP_END == eStep) {
		polygonItem->SetDraw(false);
	}
}

void ImageItem::SetGrid(bool bGrid)
{ 
	m_bGrid = bGrid; 
	this->update();
}

namespace {

void appendImagePoint(QList<double> &roi, const QGraphicsItem *roiItem, const QPointF &localPoint)
{
	const QPointF imagePoint = roiItem->mapToParent(localPoint);
	roi.append(imagePoint.x());
	roi.append(imagePoint.y());
}

void appendImagePoints(QList<double> &roi, const QGraphicsItem *roiItem, const QList<QPointF> &localPoints)
{
	for (const QPointF &localPoint : localPoints)
		appendImagePoint(roi, roiItem, localPoint);
}

QList<QPointF> rectCornerPoints(const QRectF &rect)
{
	return {rect.topLeft(), rect.topRight(), rect.bottomRight(), rect.bottomLeft()};
}

QList<QPointF> rotRectVertexPoints(const RotRectItem *rotRectItem)
{
	const QGraphicsItem *parentItem = rotRectItem->parentItem();
	const QTransform toParent = parentItem
		? parentItem->sceneTransform().inverted() * rotRectItem->sceneTransform()
		: rotRectItem->sceneTransform();

	QList<QPointF> vertices;
	for (const QPointF &localCorner : rectCornerPoints(rotRectItem->boundingRect()))
		vertices.append(toParent.map(localCorner));
	return vertices;
}

} // namespace

bool ImageItem::GetRoi(E_ItemType &itemType, QList<double> &roi) const
{
	roi.clear();
	itemType = E_TYPE_NONE;

	if (!m_roiItem)
		return false;

	const int type = m_roiItem->type() - QGraphicsItem::UserType;
	itemType = static_cast<E_ItemType>(type);

	switch (type)
	{
	case E_TYPE_RECT:
		if (const RectItem *rectItem = qgraphicsitem_cast<const RectItem *>(m_roiItem))
			appendImagePoints(roi, m_roiItem, rectCornerPoints(rectItem->boundingRect()));
		break;
	case E_TYPE_ROT_RECT:
		if (const RotRectItem *rotRectItem = qgraphicsitem_cast<const RotRectItem *>(m_roiItem))
		{
			for (const QPointF &imagePoint : rotRectVertexPoints(rotRectItem))
			{
				roi.append(imagePoint.x());
				roi.append(imagePoint.y());
			}
		}
		break;
	case E_TYPE_CIRCLE:
		if (const CircleItem *circleItem = qgraphicsitem_cast<const CircleItem *>(m_roiItem))
		{
			appendImagePoint(roi, m_roiItem, QPointF(0, 0));
			roi.append(circleItem->boundingRect().width() / 2.0);
		}
		break;
	case E_TYPE_RING:
		if (const RingItem *ringItem = qgraphicsitem_cast<const RingItem *>(m_roiItem))
		{
			appendImagePoint(roi, m_roiItem, QPointF(0, 0));
			roi.append(ringItem->innerRect().width() / 2.0);
			roi.append(ringItem->outerRect().width() / 2.0);
		}
		break;
	case E_TYPE_POLYGON:
		if (const PolygonItem *polygonItem = qgraphicsitem_cast<const PolygonItem *>(m_roiItem))
		{
			const QPolygonF polygon = polygonItem->polygon();
			for (const QPointF &point : polygon)
				appendImagePoint(roi, m_roiItem, point);
		}
		break;
	default:
		itemType = E_TYPE_NONE;
		break;
	}
	return !roi.isEmpty();
}

QRectF ImageItem::boundingRect() const
{
	if (m_image.isNull())
	{
		return QRectF();
	}

	return QRectF(0, 0, m_image.width(), m_image.height());
}

void ImageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	if (m_image.isNull())
	{
		return;
	}

	painter->save();

	// 直接 drawImage，避免每帧 QPixmap::fromImage 全图拷贝（大分辨率预览关键路径）
	painter->drawImage(0, 0, m_image);

	//绘制网格
	if (m_bGrid)
	{
		painter->setPen(QPen(Qt::yellow, 1.0 / this->scale(), Qt::SolidLine));
		QVector<QLineF> lines;
		qreal rOffset = 1.0 * m_image.height() / GRID_ROW_COUNT;
		qreal cOffset = 1.0 * m_image.width() / GRID_COL_COUNT;
		for (int row = 0; row <= GRID_ROW_COUNT; row++)
		{
			QLineF line(0, row * rOffset, m_image.width(), row * rOffset);
			lines.append(line);
		}
		for (int col = 0; col <= GRID_COL_COUNT; col++)
		{
			QLineF line(col * cOffset, 0, col * cOffset, m_image.height());
			lines.append(line);
		}
		painter->drawLines(lines);
	}

	painter->restore();
}

void ImageItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	//多边形由 GraphicsScene 处理
	if (E_TYPE_POLYGON == m_roiType)
	{
		event->ignore();
		return;
	}

	//无绘制
	if (E_TYPE_NONE == m_roiType)
	{
		QGraphicsItem::mousePressEvent(event);
		return;
	}

	//左键有效
	if (event->button() != Qt::LeftButton)
	{
		return;
	}

	//已销毁
	if (m_roiItem)
	{
		return;
	}

	//记录起始点
	m_scenePos = event->scenePos();
	QPointF pos = this->mapFromScene(m_scenePos);
	m_bMove = true;

	//添加ROI
	switch (m_roiType)
	{
	case E_TYPE_RECT:
		m_roiItem = new RectItem(QRectF(0, 0, 1, 1), this);
		break;
	case E_TYPE_ROT_RECT:
		m_roiItem = new RotRectItem(QRectF(-0.5, -0.5, 1, 1), this);
		break;
	case E_TYPE_CIRCLE:
		m_roiItem = new CircleItem(QRectF(-0.5, -0.5, 1, 1), this);
		break;
	case E_TYPE_RING:
		m_roiItem = new RingItem(QRectF(), this);
		break;
	default:
		break;
	}

	m_roiItem->setPos(pos);
}

void ImageItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	//无绘制
	if (!m_bMove)
	{
		QGraphicsItem::mouseMoveEvent(event);
		return;
	}

	//偏移
	QPointF scenePos = event->scenePos();
	QPointF prePos = this->mapFromScene(m_scenePos);
	QPointF curPos = this->mapFromScene(scenePos);

	//开始绘制
	switch (m_roiType)
	{
	case E_TYPE_RECT:
		drawRect(prePos, curPos);
		break;
	case E_TYPE_ROT_RECT:
		drawRotRect(prePos, curPos);
		break;
	case E_TYPE_CIRCLE:
		drawCircle(prePos, curPos);
		break;
	case E_TYPE_RING:
		drawRing(prePos, curPos);
		break;
	default:
		break;
	}
}

void ImageItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	//无绘制
	if (E_TYPE_NONE == m_roiType)
	{
		QGraphicsItem::mouseReleaseEvent(event);
		return;
	}

	m_bMove = false;
	m_roiType = E_TYPE_NONE;
}

void ImageItem::wheelEvent(QGraphicsSceneWheelEvent *event)
{
	qreal originScale = m_scale;
	if (event->delta() > 0)
	{
		m_scale *= 1.1;
	}
	else
	{
		m_scale *= 0.9;
	}
	setScale(m_scale);

	if (event->delta() > 0)
	{
		moveBy(-event->pos().x() * originScale * 0.1, -event->pos().y() * originScale * 0.1);
	}
	else
	{
		moveBy(event->pos().x() * originScale * 0.1, event->pos().y() * originScale * 0.1);
	}

	if (m_crosshairImageX >= 0.0)
		rebuildDefectCrosshair();

	QGraphicsItem::wheelEvent(event);
}

void ImageItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
	QPointF fPos = event->pos();
	QPoint iPos(fPos.x(), fPos.y());
	QSize size = boundingRect().size().toSize();
	QRgb rgb = m_image.pixel(iPos);
	QRectF roiBoundingRect;
	if (m_roiItem)
	{
		roiBoundingRect = m_roiItem->boundingRect();
	}
	MessageCenter::Center::Notify(EVENT_IMAGE_INFO, m_winId, size, iPos, rgb, roiBoundingRect);

	QGraphicsItem::hoverMoveEvent(event);
}

void ImageItem::drawRect(const QPointF &prePos, const QPointF &curPos)
{
	RectItem *rectItem = qgraphicsitem_cast<RectItem*>(m_roiItem);
	if (rectItem)
	{
		QRectF rect(0, 0, curPos.x() - prePos.x(), curPos.y() - prePos.y());
		rectItem->SetBoundingRect(rect);
	}
}

void ImageItem::drawRotRect(const QPointF &prePos, const QPointF &curPos)
{
	RotRectItem *rotRectItem = qgraphicsitem_cast<RotRectItem*>(m_roiItem);
	if (rotRectItem)
	{
		//标准化
		QRectF rect(0, 0, 2 * (curPos.x() - prePos.x()), 2 * (curPos.y() - prePos.y()));
		QRectF normalRect = rect.normalized();
		normalRect.moveCenter(QPointF(0, 0));

		//调整矩形
		rotRectItem->SetBoundingRect(normalRect);
	}
}

void ImageItem::drawCircle(const QPointF &prePos, const QPointF &curPos)
{
	CircleItem *circleItem = qgraphicsitem_cast<CircleItem*>(m_roiItem);
	if (circleItem)
	{
		//更新外接矩形
		QVector2D vector(curPos.x() - prePos.x(), curPos.y() - prePos.y());
		qreal radius = vector.length();
		QRectF rect(-radius, -radius, 2 * radius, 2 * radius);

		//调整矩形
		circleItem->SetBoundingRect(rect);
	}
}

void ImageItem::drawRing(const QPointF &prePos, const QPointF &curPos)
{
	RingItem *ringItem = qgraphicsitem_cast<RingItem*>(m_roiItem);
	if (ringItem)
	{
		//更新外接矩形
		QVector2D vector(curPos.x() - prePos.x(), curPos.y() - prePos.y());
		qreal radius = vector.length();
		QRectF outerRect(-radius, -radius, 2 * radius, 2 * radius);
		QRectF innerRect(-radius * RING_RATIO, -radius * RING_RATIO, 2 * radius * RING_RATIO, 2 * radius * RING_RATIO);

		//调整矩形
		ringItem->SetOuterRect(outerRect);
		ringItem->SetInnerRect(innerRect);
	}
}