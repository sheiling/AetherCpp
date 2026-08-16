/**
 * @file cursor_toast.cpp
 * @brief CursorToast 实现：布局、主题绑定、自绘背景与父窗口居中定位
 */

#include "feedback/cursor_toast.h"

#include "theme/theme_font.h"
#include "theme/theme_refresh.h"
#include "theme/theme_tokens.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QResizeEvent>
#include <QTimer>

namespace ui {

CursorToast::CursorToast(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("statusToast"));
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setAttribute(Qt::WA_StyledBackground, false);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 8);

    m_label = new QLabel(this);
    m_label->setObjectName(QStringLiteral("statusToastLabel"));
    m_label->setWordWrap(true);
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    layout->addWidget(m_label);

    m_hideTimer = new QTimer(this);
    m_hideTimer->setSingleShot(true);
    connect(m_hideTimer, &QTimer::timeout, this, &CursorToast::hide);

    applyToastStyle();
    bindThemeRefresh(this, [this]() { applyToastStyle(); });

    hide();
}

void CursorToast::applyLabelColors()
{
    const ThemeTokens t = ThemeTokens::resolve();
    QPalette            pal = m_label->palette();
    pal.setColor(QPalette::WindowText, t.textPrimary);
    m_label->setPalette(pal);

    QFont f = themeUiFont(12);
    m_label->setFont(f);
}

void CursorToast::applyToastStyle()
{
    applyLabelColors();
    update();
}

void CursorToast::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    const ThemeTokens t = ThemeTokens::resolve();

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    p.setPen(QPen(t.surfaceBorder, 1.0));
    p.setBrush(t.surface);
    p.drawRoundedRect(r, 6.0, 6.0);
}

void CursorToast::repositionAtParentCenter()
{
    QWidget *host = parentWidget();
    if (!host)
        return;

    QWidget *win = host->window();
    if (!win)
        win = host;

    const int maxW = qMin(480, qMax(160, win->width() - 48));
    setMaximumWidth(maxW);
    adjustSize();

    QPoint centerInHost = host->rect().center();
    if (win != host) {
        const QPoint winCenterInHost = host->mapFrom(win, win->rect().center());
        if (host->rect().contains(winCenterInHost))
            centerInHost = winCenterInHost;
    }

    int x = centerInHost.x() - width() / 2;
    int y = centerInHost.y() - height() / 2;

    const int maxX = qMax(0, host->width() - width());
    const int maxY = qMax(0, host->height() - height());
    move(qBound(0, x, maxX), qBound(0, y, maxY));
}

void CursorToast::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (isVisible())
        repositionAtParentCenter();
}

void CursorToast::showMessage(const QString &text, const int hideMs)
{
    if (text.isEmpty())
        return;

    m_label->setText(text);
    if (!isVisible()) {
        show();
        raise();
    }
    repositionAtParentCenter();

    if (hideMs > 0)
        m_hideTimer->start(hideMs);
    else
        m_hideTimer->stop();
}

} // namespace ui
