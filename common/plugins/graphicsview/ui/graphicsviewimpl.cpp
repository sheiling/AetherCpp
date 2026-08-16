#include "graphicsviewimpl.h"

#include "graphicswidget.h"

namespace {

template<typename Fn>
void withGraphicsWidget(const QMap<int, QWidget *> &widgets, int winId, Fn &&fn)
{
    QWidget *widget = widgets.value(winId, nullptr);
    if (auto *graphicsWidget = qobject_cast<GraphicsWidget *>(widget)) {
        fn(graphicsWidget);
    }
}

} // namespace

GraphicsViewImpl::GraphicsViewImpl()
    : QObject()
{
}

QWidget *GraphicsViewImpl::CreateWidget(int winId)
{
    if (QWidget *existing = m_widgets.value(winId, nullptr)) {
        return existing;
    }

    QWidget *widget = new GraphicsWidget(nullptr, winId);
    m_widgets.insert(winId, widget);
    return widget;
}

bool GraphicsViewImpl::SetVisible(int winId, bool bStatus, bool bDraw)
{
    GraphicsWidget *widget = graphicsWidget(winId);
    if (!widget) {
        return false;
    }
    return widget->SetVisible(bStatus, bDraw);
}

GraphicsWidget *GraphicsViewImpl::graphicsWidget(int winId) const
{
    return qobject_cast<GraphicsWidget *>(m_widgets.value(winId, nullptr));
}

void GraphicsViewImpl::SetImage(int winId, QImage image)
{
    withGraphicsWidget(m_widgets, winId,
                       [&](GraphicsWidget *w) { w->SetImage(std::move(image)); });
}

bool GraphicsViewImpl::GetImage(int winId, QImage &out) const
{
    GraphicsWidget *widget = graphicsWidget(winId);
    if (!widget)
        return false;
    return widget->GetImage(out);
}

void GraphicsViewImpl::Reset(int winId)
{
    withGraphicsWidget(m_widgets, winId, [](GraphicsWidget *w) { w->Reset(); });
}

void GraphicsViewImpl::DrawRect(int winId)
{
    withGraphicsWidget(m_widgets, winId, [](GraphicsWidget *w) { w->DrawRect(); });
}

void GraphicsViewImpl::DrawRotRect(int winId)
{
    withGraphicsWidget(m_widgets, winId, [](GraphicsWidget *w) { w->DrawRotRect(); });
}

void GraphicsViewImpl::DrawCircle(int winId)
{
    withGraphicsWidget(m_widgets, winId, [](GraphicsWidget *w) { w->DrawCircle(); });
}

void GraphicsViewImpl::DrawRing(int winId)
{
    withGraphicsWidget(m_widgets, winId, [](GraphicsWidget *w) { w->DrawRing(); });
}

void GraphicsViewImpl::DrawPolygon(int winId)
{
    withGraphicsWidget(m_widgets, winId, [](GraphicsWidget *w) { w->DrawPolygon(); });
}

void GraphicsViewImpl::DrawCross(int winId)
{
    withGraphicsWidget(m_widgets, winId, [](GraphicsWidget *w) { w->DrawCross(); });
}

void GraphicsViewImpl::DrawClean(int winId)
{
    withGraphicsWidget(m_widgets, winId, [](GraphicsWidget *w) { w->DrawClean(); });
}

void GraphicsViewImpl::SetRoi(int winId, E_ItemType itemType, const QList<double> &roi)
{
    withGraphicsWidget(m_widgets, winId, [&](GraphicsWidget *w) { w->SetRoi(itemType, roi); });
}

bool GraphicsViewImpl::GetRoi(int winId, E_ItemType &itemType, QList<double> &roi)
{
    GraphicsWidget *widget = graphicsWidget(winId);
    if (!widget) {
        return false;
    }
    return widget->GetRoi(itemType, roi);
}

bool GraphicsViewImpl::GetRoiRect(int winId, QRect &out) const
{
    GraphicsWidget *widget = graphicsWidget(winId);
    if (!widget)
        return false;
    return widget->GetRoiRect(out);
}

void GraphicsViewImpl::SetPixelScaleUmPerPx(int winId, double umPerPx)
{
    withGraphicsWidget(m_widgets, winId,
                       [&](GraphicsWidget *w) { w->setPixelScaleUmPerPx(umPerPx); });
}

void GraphicsViewImpl::SetDefectCrosshair(int winId, double imageX, double imageY)
{
    withGraphicsWidget(m_widgets, winId, [&](GraphicsWidget *w) {
        w->SetDefectCrosshair(imageX, imageY);
    });
}

void GraphicsViewImpl::ClearDefectCrosshair(int winId)
{
    withGraphicsWidget(m_widgets, winId, [](GraphicsWidget *w) { w->ClearDefectCrosshair(); });
}
