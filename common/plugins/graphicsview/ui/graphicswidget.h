#pragma once

#include <QWidget>

class QShowEvent;
#include <QSize>
#include "imageitem.h"
#include "graphicsscene.h"

class QGraphicsView;
class QHBoxLayout;
class QLineEdit;
class QVBoxLayout;

class GraphicsWidget : public QWidget
{
	Q_OBJECT

public:
	GraphicsWidget(QWidget *parent = nullptr, int winId = 0);
	~GraphicsWidget();

	bool SetVisible(bool bStatus, bool bDraw);
	void SetImage(QImage image);
	bool GetImage(QImage &out) const;
	bool GetRoiRect(QRect &out) const;

	void DrawRect();
	void DrawRotRect();
	void DrawCircle();
	void DrawRing();
	void DrawPolygon();
	void DrawCross();
	void DrawClean();
	void SetRoi(E_ItemType itemType, const QList<double> &roi);
	bool GetRoi(E_ItemType &itemType, QList<double> &roi) const;
	void setPixelScaleUmPerPx(double umPerPx);
	void SetDefectCrosshair(double imageX, double imageY);
	void ClearDefectCrosshair();

public slots:
	void Reset();

protected:
	void showEvent(QShowEvent *event) override;

private slots:
	void setImageImpl(QImage image);
	void onImageInfo(const QVariantList &data);

private:
	void setDrawMode(int roiType);
	void updateRoiStatusText(const QRectF &roiRect);

	int m_winId = 0;
	ImageItem* m_imageItem;
	GraphicsScene* m_scene;
	QGraphicsView* m_view;
	QLineEdit* m_statusInfo[4];
	QHBoxLayout* m_statusLayout = nullptr;
	QWidget* m_rightPanel = nullptr;
	QVBoxLayout* m_rightLayout = nullptr;
	bool m_gridVisible = false;
	double m_umPerPx = 0.0;
	QRectF m_lastRoiRect;
};
