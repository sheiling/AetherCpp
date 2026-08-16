/**
 * @file themed_panel_dialog.cpp
 * @brief ThemedPanelDialog 实现：壳布局、主题/语言刷新、标题栏拖拽与居中
 */

#include "dialog/themed_panel_dialog.h"

#include "controls/label/themed_form_label.h"
#include "dialog/content_dialog_style.h"
#include "theme/theme_font.h"
#include "theme/theme_refresh.h"

#include "uibase/languagemanager.h"

#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
#include <QScreen>
#include <QShowEvent>
#include <QSizePolicy>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>

namespace ui {

namespace {

constexpr int kTitleBarHeight = 36; /**< 标题栏固定高度，与 content_dialog_style QSS 一致 */

/** 对子树执行 unpolish/polish，使 setStyleSheet 后控件立即应用新 QSS */
void repolishWidgetTree(QWidget *root)
{
    if (!root || !root->style())
        return;
    for (auto *w : root->findChildren<QWidget *>()) {
        w->style()->unpolish(w);
        w->style()->polish(w);
    }
    root->style()->unpolish(root);
    root->style()->polish(root);
}

} // namespace

ThemedPanelDialog::ThemedPanelDialog(QWidget *parent)
    : QDialog(parent)
{
    setObjectName(QStringLiteral("themedPanelDialog"));
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setModal(false);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    bindThemeRefresh(this, [this]() { applyTheme(); });

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(0);

    m_panel = new QFrame(this);
    m_panel->setObjectName(QStringLiteral("themedPanelFrame"));
    m_panel->setAttribute(Qt::WA_StyledBackground, true);
    m_panel->setMinimumWidth(m_panelMinWidth);

    auto *shadow = new QGraphicsDropShadowEffect(m_panel);
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 120));
    m_panel->setGraphicsEffect(shadow);

    auto *panelLayout = new QVBoxLayout(m_panel);
    panelLayout->setContentsMargins(0, 0, 0, 12);
    panelLayout->setSpacing(8);

    m_titleBar = new QWidget(m_panel);
    m_titleBar->setObjectName(QStringLiteral("themedPanelTitleBar"));
    m_titleBar->setFixedHeight(kTitleBarHeight);
    auto *titleHl = new QHBoxLayout(m_titleBar);
    titleHl->setContentsMargins(12, 0, 6, 0);
    titleHl->setSpacing(8);
    titleHl->setAlignment(Qt::AlignVCenter);
    m_titleLabel = new QLabel(m_titleBar);
    m_titleLabel->setObjectName(QStringLiteral("themedPanelTitle"));
    m_titleCloseBtn = new QPushButton(QStringLiteral("\u00D7"), m_titleBar);
    m_titleCloseBtn->setObjectName(QStringLiteral("themedPanelTitleCloseBtn"));
    m_titleCloseBtn->setCursor(Qt::PointingHandCursor);
    m_titleCloseBtn->setFocusPolicy(Qt::NoFocus);
    m_titleCloseBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    titleHl->addWidget(m_titleLabel, 1, Qt::AlignVCenter);
    titleHl->addWidget(m_titleCloseBtn, 0, Qt::AlignVCenter);
    panelLayout->addWidget(m_titleBar);

    m_bodyHost = new QWidget(m_panel);
    m_bodyLayout = new QVBoxLayout(m_bodyHost);
    m_bodyLayout->setContentsMargins(12, 8, 12, 12);
    m_bodyLayout->setSpacing(0);
    panelLayout->addWidget(m_bodyHost, 1);

    root->addWidget(m_panel, 1);

    connect(m_titleCloseBtn, &QPushButton::clicked, this, &QDialog::close);

    if (LM) {
        connect(LM, &LanguageManager::languageChanged, this,
                [this](LanguageManager::Language) { retranslateDialog(); });
    }

    applyTheme();
    retranslateDialog();
}

void ThemedPanelDialog::setTitleKey(const QString &key)
{
    if (!key.isEmpty())
        m_titleKey = key;
    retranslateShell();
}

void ThemedPanelDialog::setTitleText(const QString &text)
{
    if (m_titleLabel)
        m_titleLabel->setText(text);
    if (m_titleBar)
        m_titleBar->updateGeometry();
}

void ThemedPanelDialog::setBodyWidget(QWidget *widget)
{
    if (m_bodyWidget && m_bodyLayout) {
        m_bodyLayout->removeWidget(m_bodyWidget);
        m_bodyWidget->setParent(nullptr);
    }
    m_bodyWidget = widget;
    if (!widget || !m_bodyLayout)
        return;
    widget->setParent(m_bodyHost);
    m_bodyLayout->addWidget(widget, 1);
    applyTheme();
}

void ThemedPanelDialog::enableResizableWindow(const QSize &minSize)
{
    m_resizable = true;
    const int minW = qMax(minSize.width() > 0 ? minSize.width() : 640, m_panelMinWidth + 16);
    const int minH = qMax(minSize.height() > 0 ? minSize.height() : 480, 320);
    setMinimumSize(minW, minH);
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    if (m_panel) {
        m_panel->setMinimumWidth(m_panelMinWidth);
        m_panel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
    if (m_bodyHost)
        m_bodyHost->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    if (m_bodyWidget)
        m_bodyWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void ThemedPanelDialog::setMinimumPanelWidth(int width)
{
    m_panelMinWidth = qMax(320, width);
    if (m_panel)
        m_panel->setMinimumWidth(m_panelMinWidth);
}

void ThemedPanelDialog::setMinimumPanelHeight(int height)
{
    m_panelMinHeight = qMax(240, height);
    if (m_panel)
        m_panel->setMinimumHeight(m_panelMinHeight);
}

void ThemedPanelDialog::finalizeLayoutSize()
{
    if (!m_panel)
        return;
    setMinimumSize(0, 0);
    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    if (layout())
        layout()->activate();
    m_panel->adjustSize();
    const QSize panelHint = m_panel->sizeHint();
    const int   rootMargin = 8;
    QSize       target     = panelHint + QSize(rootMargin * 2, rootMargin * 2);
    target.setWidth(qMax(target.width(), m_panelMinWidth + rootMargin * 2));
    if (m_panelMinHeight > 0)
        target.setHeight(qMax(target.height(), m_panelMinHeight + rootMargin * 2));
    resize(target);
    m_stableSize = size();
    if (!m_resizable)
        setFixedSize(m_stableSize);
}

void ThemedPanelDialog::applyTheme()
{
    const QFont uiFont = themeUiFont();
    setFont(uiFont);
    if (m_titleLabel) {
        QFont titleFont = themeUiFont(13);
        titleFont.setBold(true);
        m_titleLabel->setFont(titleFont);
    }
    setStyleSheet(buildContentDialogStylesheet(objectName()));

    if (m_bodyWidget)
        ThemedFormLabel::applyFontsIn(m_bodyWidget);
    if (m_panel)
        repolishWidgetTree(m_panel);
}

void ThemedPanelDialog::retranslateShell()
{
    if (!m_titleKey.isEmpty())
        setWindowTitle(TR(m_titleKey.toUtf8().constData()));
    if (m_titleLabel && !m_titleKey.isEmpty())
        m_titleLabel->setText(TR(m_titleKey.toUtf8().constData()));
}

void ThemedPanelDialog::retranslateDialog()
{
    retranslateShell();
    retranslateBody();
}

void ThemedPanelDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    applyTheme();
    if (!m_resizable && !isMaximized() && !m_stableSize.isEmpty())
        resize(m_stableSize);
    if (!m_resizable && !isMaximized())
        centerOnParent();
    // 首次 show 前控件在隐藏态构建，QSS / 字体需等可见后再刷一次
    QTimer::singleShot(0, this, [this]() {
        if (!isVisible())
            return;
        applyTheme();
    });
}

void ThemedPanelDialog::closeEvent(QCloseEvent *event)
{
    QDialog::closeEvent(event);
    emit dialogClosed();
}

void ThemedPanelDialog::centerOnParent()
{
    QWidget *host = parentWidget();
    if (!host)
        host = window();
    if (!host)
        return;

    const QPoint center = host->mapToGlobal(host->rect().center());
    move(center - rect().center());

    if (QScreen *scr = screen()) {
        const QRect avail = scr->availableGeometry();
        QRect       geo   = frameGeometry();
        if (!avail.contains(geo)) {
            if (geo.right() > avail.right())
                move(avail.right() - geo.width(), geo.y());
            if (geo.left() < avail.left())
                move(avail.left(), geo.y());
            if (geo.bottom() > avail.bottom())
                move(geo.x(), avail.bottom() - geo.height());
            if (geo.top() < avail.top())
                move(geo.x(), avail.top());
        }
    }
}

bool ThemedPanelDialog::isInTitleBar(const QPoint &pos) const
{
    return m_titleBar && m_titleBar->geometry().contains(pos);
}

void ThemedPanelDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isInTitleBar(event->pos())) {
        if (isMaximized()) {
            const qreal ratio =
                width() > 0 ? static_cast<qreal>(event->pos().x()) / static_cast<qreal>(width()) : 0.5;
            showNormal();
            const int newW = width();
            move(event->globalPos().x() - static_cast<int>(newW * ratio),
                 event->globalPos().y() - event->pos().y());
        }
        m_dragging   = true;
        m_dragOffset = event->globalPos() - frameGeometry().topLeft();
        event->accept();
        return;
    }
    QDialog::mousePressEvent(event);
}

void ThemedPanelDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPos() - m_dragOffset);
        event->accept();
        return;
    }
    QDialog::mouseMoveEvent(event);
}

void ThemedPanelDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        m_dragging = false;
    QDialog::mouseReleaseEvent(event);
}

void ThemedPanelDialog::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (m_resizable && event->button() == Qt::LeftButton && isInTitleBar(event->pos())) {
        if (isMaximized())
            showNormal();
        else
            showMaximized();
        event->accept();
        return;
    }
    QDialog::mouseDoubleClickEvent(event);
}

} // namespace ui
