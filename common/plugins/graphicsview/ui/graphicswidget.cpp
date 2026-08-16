#include "graphicswidget.h"
#include "messagecenter/message_center.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QGraphicsView>
#include <QOpenGLWidget>
#include <QLineEdit>
#include <QFileDialog>
#include <QImage>
#include <QMetaObject>
#include <QPixmap>
#include <QThread>
#include <QTimer>
#include <QSharedPointer>
#include <QShowEvent>

namespace {

QRect roiFloatToPixelRect(const QRectF &roiF, const QRect &imageBounds)
{
    if (roiF.isNull() || roiF.width() < 0.5 || roiF.height() < 0.5)
        return {};

    const int w = qRound(roiF.width());
    const int h = qRound(roiF.height());
    if (w < 1 || h < 1)
        return {};

    return QRect(qRound(roiF.x()), qRound(roiF.y()), w, h).intersected(imageBounds);
}

} // namespace

GraphicsWidget::GraphicsWidget(QWidget *parent, int winId)
	: QWidget(parent),
	m_winId(winId),
	m_imageItem(new ImageItem(winId)),
	m_scene(new GraphicsScene(this)),
	m_view(nullptr)
{
	setObjectName(QStringLiteral("graphicsWidget_%1").arg(m_winId));

	m_scene->SetImageItem(m_imageItem);
	m_scene->addItem(m_imageItem);

	m_scene->setItemIndexMethod(QGraphicsScene::NoIndex);
	m_view = new QGraphicsView(m_scene, this);
	m_view->setViewport(new QOpenGLWidget());
	m_view->setScene(m_scene);
	m_view->setRenderHint(QPainter::Antialiasing, false);
	m_view->setCacheMode(QGraphicsView::CacheBackground);
	m_view->setDragMode(QGraphicsView::NoDrag);
	m_view->setOptimizationFlags(QGraphicsView::DontSavePainterState);
	m_view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	m_view->setBackgroundBrush(Qt::black);
	m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	static const QString statusEditStyle = QStringLiteral(
		"background-color: #1a222c; color: #e8eef6;"
		" border: 1px solid #3a4a5d; border-radius: 4px; padding: 4px;");
	for (int i = 0; i < 4; ++i)
	{
		m_statusInfo[i] = new QLineEdit(this);
		m_statusInfo[i]->setObjectName(QStringLiteral("graphicsStatusEdit"));
		m_statusInfo[i]->setReadOnly(true);
		m_statusInfo[i]->setFixedHeight(24);
		m_statusInfo[i]->setAlignment(Qt::AlignCenter);
		m_statusInfo[i]->setStyleSheet(statusEditStyle);
	}

	MessageCenter::Center::Attach(EVENT_IMAGE_INFO, this, SLOT(onImageInfo(const QVariantList &)));

	auto *btnOpen = new QPushButton(QStringLiteral("打开..."), this);
	connect(btnOpen, &QPushButton::clicked, this, [this]() {
		const QString path = QFileDialog::getOpenFileName(
			this,
			QStringLiteral("打开图片"),
			QString(),
			QStringLiteral("Images (*.png *.jpg *.jpeg *.bmp *.tif *.tiff);;All Files (*)"));
		if (path.isEmpty())
			return;

		SetImage(QImage(path));
	});
	auto *btnRect = new QPushButton(QStringLiteral("矩形"), this);
	connect(btnRect, &QPushButton::clicked, this, &GraphicsWidget::DrawRect);
	auto *btnRotRect = new QPushButton(QStringLiteral("旋转矩形"), this);
	connect(btnRotRect, &QPushButton::clicked, this, &GraphicsWidget::DrawRotRect);
	auto *btnCircle = new QPushButton(QStringLiteral("圆"), this);
	connect(btnCircle, &QPushButton::clicked, this, &GraphicsWidget::DrawCircle);
	auto *btnRing = new QPushButton(QStringLiteral("圆环"), this);
	connect(btnRing, &QPushButton::clicked, this, &GraphicsWidget::DrawRing);
	auto *btnPolygon = new QPushButton(QStringLiteral("多边形"), this);
	connect(btnPolygon, &QPushButton::clicked, this, &GraphicsWidget::DrawPolygon);
	auto *btnGrid = new QPushButton(QStringLiteral("十字线"), this);
	connect(btnGrid, &QPushButton::clicked, this, [this]() {
		m_gridVisible = !m_gridVisible;
		m_imageItem->SetGrid(m_gridVisible);
	});
	auto *btnClear = new QPushButton(QStringLiteral("清屏"), this);
	connect(btnClear, &QPushButton::clicked, this, &GraphicsWidget::DrawClean);

	m_statusLayout = new QHBoxLayout();
	m_statusLayout->setSpacing(4);
	for (int i = 0; i < 4; ++i)
	{
		m_statusLayout->addWidget(m_statusInfo[i], 1);
	}

	auto *leftLayout = new QVBoxLayout();
	leftLayout->setContentsMargins(0, 0, 0, 0);
	leftLayout->setSpacing(4);
	leftLayout->addWidget(m_view, 1);
	leftLayout->addLayout(m_statusLayout);

	m_rightLayout = new QVBoxLayout();
	m_rightLayout->setSpacing(6);
	m_rightLayout->addWidget(btnOpen);
	m_rightLayout->addWidget(btnRect);
	m_rightLayout->addWidget(btnRotRect);
	m_rightLayout->addWidget(btnCircle);
	m_rightLayout->addWidget(btnRing);
	m_rightLayout->addWidget(btnPolygon);
	m_rightLayout->addStretch();
	m_rightLayout->addWidget(btnGrid);
	m_rightLayout->addWidget(btnClear);

	m_rightPanel = new QWidget(this);
	m_rightPanel->setLayout(m_rightLayout);

	auto *mainLayout = new QHBoxLayout(this);
	mainLayout->setContentsMargins(8, 8, 8, 8);
	mainLayout->setSpacing(8);
	mainLayout->addLayout(leftLayout, 1);
	mainLayout->addWidget(m_rightPanel);
}

namespace {

void setLayoutVisible(QLayout *layout, bool visible)
{
	if (!layout)
		return;
	for (int i = 0; i < layout->count(); ++i)
	{
		QLayoutItem *item = layout->itemAt(i);
		if (!item)
			continue;
		if (QWidget *widget = item->widget())
			widget->setVisible(visible);
		else if (QLayout *childLayout = item->layout())
			setLayoutVisible(childLayout, visible);
	}
}

} // namespace

bool GraphicsWidget::SetVisible(bool bStatus, bool bDraw)
{
	setLayoutVisible(m_statusLayout, bStatus);
	if (m_rightPanel)
		m_rightPanel->setVisible(bDraw);
	return true;
}

void GraphicsWidget::SetImage(QImage image)
{
	if (image.isNull())
		return;

	if (QThread::currentThread() != thread()) {
		const auto holder = QSharedPointer<QImage>::create(std::move(image));
		QMetaObject::invokeMethod(this, [this, holder]() mutable {
			setImageImpl(std::move(*holder));
		}, Qt::QueuedConnection);
		return;
	}

	setImageImpl(std::move(image));
}

bool GraphicsWidget::GetImage(QImage &out) const
{
	if (!m_imageItem)
		return false;

	out = m_imageItem->currentImage();
	return !out.isNull();
}

bool GraphicsWidget::GetRoiRect(QRect &out) const
{
	if (!m_imageItem)
		return false;

	const QRectF roiF = m_imageItem->currentRoiRect();
	if (roiF.isNull())
		return false;

	const QImage image = m_imageItem->currentImage();
	if (image.isNull())
		return false;

	const QRect roi = roiFloatToPixelRect(roiF, image.rect());
	if (roi.isEmpty())
		return false;

	out = roi;
	return true;
}

void GraphicsWidget::setImageImpl(QImage image)
{
	const bool sizeChanged = m_imageItem->SetItem(std::move(image));
	if (sizeChanged && isVisible()) {
		QTimer::singleShot(0, this, &GraphicsWidget::Reset);
	}
}

void GraphicsWidget::Reset()
{
	m_imageItem->ResetItem();
}

void GraphicsWidget::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	QTimer::singleShot(0, this, &GraphicsWidget::Reset);
}

void GraphicsWidget::DrawRect()
{
	setDrawMode(E_TYPE_RECT);
}

void GraphicsWidget::DrawRotRect()
{
	setDrawMode(E_TYPE_ROT_RECT);
}

void GraphicsWidget::DrawCircle()
{
	setDrawMode(E_TYPE_CIRCLE);
}

void GraphicsWidget::DrawRing()
{
	setDrawMode(E_TYPE_RING);
}

void GraphicsWidget::DrawPolygon()
{
	setDrawMode(E_TYPE_POLYGON);
}

void GraphicsWidget::DrawCross()
{
	m_gridVisible = !m_gridVisible;
	m_imageItem->SetGrid(m_gridVisible);
}

void GraphicsWidget::DrawClean()
{
	setDrawMode(E_TYPE_NONE);
	m_gridVisible = false;
	m_imageItem->SetGrid(false);
}

void GraphicsWidget::setDrawMode(int roiType)
{
	m_imageItem->SetRoiType(roiType);
	m_scene->SetRoiType(roiType);
}

void GraphicsWidget::SetRoi(E_ItemType itemType, const QList<double> &roi)
{
	DrawClean();
	Reset();
	m_imageItem->SetRoi(itemType, roi);
}

void GraphicsWidget::SetDefectCrosshair(double imageX, double imageY)
{
	if (m_imageItem)
		m_imageItem->SetDefectCrosshair(imageX, imageY);
}

void GraphicsWidget::ClearDefectCrosshair()
{
	if (m_imageItem)
		m_imageItem->ClearDefectCrosshair();
}

bool GraphicsWidget::GetRoi(E_ItemType &itemType, QList<double> &roi) const
{
	return m_imageItem->GetRoi(itemType, roi);
}

void GraphicsWidget::setPixelScaleUmPerPx(const double umPerPx)
{
	m_umPerPx = umPerPx;
	updateRoiStatusText(m_lastRoiRect);
}

void GraphicsWidget::updateRoiStatusText(const QRectF &roiRect)
{
	if (!m_statusInfo[3])
		return;

	if (!roiRect.isValid()) {
		m_statusInfo[3]->clear();
		return;
	}

	const int wPx = qRound(roiRect.width());
	const int hPx = qRound(roiRect.height());
	if (m_umPerPx > 0.0) {
		const double wMm = wPx * m_umPerPx / 1000.0;
		const double hMm = hPx * m_umPerPx / 1000.0;
		m_statusInfo[3]->setText(
			QStringLiteral("W:%1 H:%2 | W:%3mm H:%4mm")
				.arg(wPx)
				.arg(hPx)
				.arg(wMm, 0, 'f', 3)
				.arg(hMm, 0, 'f', 3));
	} else {
		m_statusInfo[3]->setText(QStringLiteral("W:%1 H:%2").arg(wPx).arg(hPx));
	}
}

GraphicsWidget::~GraphicsWidget()
{
	MessageCenter::Center::Detach(EVENT_IMAGE_INFO, this, SLOT(onImageInfo(const QVariantList &)));
}

void GraphicsWidget::onImageInfo(const QVariantList &data)
{
	if (data.size() < 5)
		return;

	if (data.at(0).toInt() != m_winId)
		return;

	const QSize size = data.at(1).toSize();
	const QPoint pos = data.at(2).toPoint();
	const QRgb rgb = data.at(3).toUInt();
	const QRectF roiRect = data.at(4).toRectF();

	m_statusInfo[0]->setText(QStringLiteral("%1x%2").arg(size.width()).arg(size.height()));
	m_statusInfo[1]->setText(QStringLiteral("X:%1 Y:%2").arg(pos.x()).arg(pos.y()));
	m_statusInfo[2]->setText(QStringLiteral("R:%1 G:%2 B:%3")
		.arg(qRed(rgb)).arg(qGreen(rgb)).arg(qBlue(rgb)));
	m_lastRoiRect = roiRect;
	updateRoiStatusText(roiRect);
}
