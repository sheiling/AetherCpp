#pragma once

#include <QGraphicsScene>
#include "imageitem.h"

class GraphicsScene : public QGraphicsScene
{
	Q_OBJECT

public:
	GraphicsScene(QObject *parent = nullptr);
	~GraphicsScene();
	void SetImageItem(ImageItem *imageItem);
	void SetRoiType(int type);

protected:
	virtual void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent);
	virtual void keyPressEvent(QKeyEvent *keyEvent);

private:
	QList<QGraphicsItem *> selectedRoiItems() const;
	void moveSelectedRoiItems(qreal dx, qreal dy);

	int m_roiType;
	bool m_bMove;
	ImageItem *m_imageItem;
	QVector<QPointF> m_points;
};
