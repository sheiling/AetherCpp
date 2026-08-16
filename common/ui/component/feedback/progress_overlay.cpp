/**
 * @file progress_overlay.cpp
 * @brief ProgressOverlay 实现：主题绑定、锚区居中布局
 */

#include "feedback/progress_overlay.h"

#include "controls/button/themed_push_button.h"
#include "theme/theme_font.h"
#include "theme/theme_refresh.h"
#include "theme/theme_tokens.h"

#include <QFrame>
#include <QLabel>
#include <QProgressBar>
#include <QResizeEvent>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QtMath>

namespace ui {

ProgressOverlay::ProgressOverlay(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("progressOverlay"));
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAutoFillBackground(false);

    m_panel = new QFrame(this);
    m_panel->setObjectName(QStringLiteral("progressOverlayPanel"));
    m_panel->setAttribute(Qt::WA_StyledBackground, true);
    m_panel->setMinimumWidth(300);
    m_panel->setMaximumWidth(420);

    auto *layout = new QVBoxLayout(m_panel);
    layout->setContentsMargins(16, 10, 16, 10);
    layout->setSpacing(8);

    m_messageLabel = new QLabel(m_panel);
    m_messageLabel->setObjectName(QStringLiteral("progressOverlayMessage"));
    m_messageLabel->setAlignment(Qt::AlignCenter);
    m_messageLabel->setWordWrap(true);
    layout->addWidget(m_messageLabel);

    m_progressBar = new QProgressBar(m_panel);
    m_progressBar->setObjectName(QStringLiteral("progressOverlayBar"));
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    layout->addWidget(m_progressBar);

    m_cancelBtn = makePushButton(QString(), PushButtonVariant::Secondary, m_panel);
    m_cancelBtn->setObjectName(QStringLiteral("progressOverlayCancelBtn"));
    layout->addWidget(m_cancelBtn, 0, Qt::AlignHCenter);

    connect(m_cancelBtn, &ThemedPushButton::clicked, this, &ProgressOverlay::cancelClicked);

    applyThemeStyle();
    bindThemeRefresh(this, [this]() { applyThemeStyle(); });

    hide();
}

void ProgressOverlay::setProgress(int percent, const QString &message)
{
    m_progressBar->setValue(qBound(0, percent, 100));
    m_messageLabel->setText(message);
    layoutPanel();
}

void ProgressOverlay::setCancelText(const QString &text)
{
    if (m_cancelBtn)
        m_cancelBtn->setText(text);
}

void ProgressOverlay::setCancelVisible(const bool visible)
{
    if (m_cancelBtn)
        m_cancelBtn->setVisible(visible);
}

void ProgressOverlay::setAnchorWidget(QWidget *anchor)
{
    m_anchorWidget = anchor;
    layoutPanel();
}

void ProgressOverlay::applyThemeStyle()
{
    const ThemeTokens t = ThemeTokens::resolve();

    if (m_panel) {
        const QString panelStyle = QStringLiteral(
            "QFrame#progressOverlayPanel {"
            "  background-color: rgba(%1, %2, %3, 210);"
            "  border: 1px solid rgba(%4, %5, %6, 180);"
            "  border-radius: 8px;"
            "}")
                                       .arg(t.surface.red())
                                       .arg(t.surface.green())
                                       .arg(t.surface.blue())
                                       .arg(t.surfaceBorder.red())
                                       .arg(t.surfaceBorder.green())
                                       .arg(t.surfaceBorder.blue());
        m_panel->setStyleSheet(panelStyle);
    }

    if (m_messageLabel) {
        QPalette pal = m_messageLabel->palette();
        pal.setColor(QPalette::WindowText, t.textPrimary);
        m_messageLabel->setPalette(pal);
        m_messageLabel->setFont(themeUiFont(12));
    }

    if (m_progressBar) {
        const QString barStyle = QStringLiteral(
            "QProgressBar#progressOverlayBar {"
            "  border: 1px solid rgba(%1, %2, %3, 200);"
            "  border-radius: 4px;"
            "  background-color: rgba(%4, %5, %6, 160);"
            "  text-align: center;"
            "  color: rgb(%7, %8, %9);"
            "  min-height: 18px;"
            "}"
            "QProgressBar#progressOverlayBar::chunk {"
            "  background-color: rgb(%10, %11, %12);"
            "  border-radius: 3px;"
            "}")
                                     .arg(t.surfaceBorder.red())
                                     .arg(t.surfaceBorder.green())
                                     .arg(t.surfaceBorder.blue())
                                     .arg(t.surfaceElevated.red())
                                     .arg(t.surfaceElevated.green())
                                     .arg(t.surfaceElevated.blue())
                                     .arg(t.textPrimary.red())
                                     .arg(t.textPrimary.green())
                                     .arg(t.textPrimary.blue())
                                     .arg(t.accent.red())
                                     .arg(t.accent.green())
                                     .arg(t.accent.blue());
        m_progressBar->setStyleSheet(barStyle);
    }
}

QRect ProgressOverlay::anchorRect() const
{
    QWidget *host = parentWidget();
    if (!m_anchorWidget || !host)
        return rect();

    if (m_anchorWidget == host)
        return QRect(QPoint(0, 0), host->size());

    const QPoint topLeft = host->mapFrom(m_anchorWidget, QPoint(0, 0));
    return QRect(topLeft, m_anchorWidget->size());
}

void ProgressOverlay::layoutPanel()
{
    if (!m_panel || width() <= 0 || height() <= 0)
        return;

    const int panelW =
        qBound(m_panel->minimumWidth(), width() * 3 / 5, m_panel->maximumWidth());
    m_panel->setFixedWidth(panelW);
    m_panel->adjustSize();

    const int panelH = m_panel->height();
    const QRect anchor = anchorRect();

    const int cx = anchor.x() + anchor.width() / 2;
    const int cy = anchor.y() + anchor.height() / 2;
    const int xOffset = qMax(4, anchor.width() / 64);
    const int yOffset = qMax(4, anchor.height() / 64);
    const int x       = qBound(anchor.left() + 8, cx - panelW / 2 + xOffset,
                               anchor.right() - panelW - 8);
    const int y       = qBound(anchor.top() + 8, cy - panelH / 2 - yOffset,
                               anchor.bottom() - panelH - 8);
    m_panel->setGeometry(x, y, panelW, panelH);
}

void ProgressOverlay::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (parentWidget())
        setGeometry(parentWidget()->rect());
    layoutPanel();
}

void ProgressOverlay::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    layoutPanel();
}

} // namespace ui
