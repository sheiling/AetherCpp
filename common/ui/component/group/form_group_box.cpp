/**
 * @file form_group_box.cpp
 * @brief FormGroupBox 自绘标题栏、内容区行 API 与主题/语言刷新
 */

#include "group/form_group_box.h"

#include "theme/theme_font.h"
#include "theme/theme_refresh.h"
#include "theme/theme_tokens.h"

#include "uibase/languagemanager.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include <QFontMetrics>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPaintEvent>
#include <QStyle>
#include <QVBoxLayout>

namespace ui {

namespace {

constexpr qreal kFormGroupRadius = 6.0;

QPainterPath topRoundedRect(const QRectF &r, qreal radius)
{
    const qreal rad  = qMin(radius, qMin(r.width(), r.height()) / 2.0);
    const qreal diam = rad * 2.0;

    QPainterPath path;
    path.moveTo(r.left(), r.bottom());
    path.lineTo(r.left(), r.top() + rad);
    path.arcTo(r.left(), r.top(), diam, diam, 180.0, -90.0);
    path.lineTo(r.right() - rad, r.top());
    path.arcTo(r.right() - diam, r.top(), diam, diam, 90.0, -90.0);
    path.lineTo(r.right(), r.bottom());
    path.closeSubpath();
    return path;
}

QPainterPath bottomRoundedRect(const QRectF &r, qreal radius)
{
    const qreal rad  = qMin(radius, qMin(r.width(), r.height()) / 2.0);
    const qreal diam = rad * 2.0;

    QPainterPath path;
    path.moveTo(r.left(), r.top());
    path.lineTo(r.right(), r.top());
    path.lineTo(r.right(), r.bottom() - rad);
    path.arcTo(r.right() - diam, r.bottom() - diam, diam, diam, 0.0, -90.0);
    path.lineTo(r.left() + rad, r.bottom());
    path.arcTo(r.left(), r.bottom() - diam, diam, diam, 270.0, -90.0);
    path.closeSubpath();
    return path;
}

QPainterPath fullRoundedRect(const QRectF &r, qreal radius)
{
    const qreal rad = qMin(radius, qMin(r.width(), r.height()) / 2.0);
    QPainterPath  path;
    path.addRoundedRect(r, rad, rad);
    return path;
}

constexpr int kThresholdNameLabelW = 140;

/**
 * @brief 分组框标题栏：自绘标题底色与文字；外扩 1px 与父级 border 圆角对齐。
 */
class FormGroupBoxTitleBar : public QWidget
{
public:
    explicit FormGroupBoxTitleBar(FormGroupBox *owner)
        : QWidget(owner)
        , m_owner(owner)
    {
        setAttribute(Qt::WA_StyledBackground, false);
        setAutoFillBackground(false);
        setStyleSheet(QStringLiteral("background: transparent;"));
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setFixedHeight(36);
        setCursor(isCollapsible() ? Qt::PointingHandCursor : Qt::ArrowCursor);
    }

    void setTitleText(const QString &text)
    {
        m_titleText = text;
        update();
    }

    void setExpandedVisual(bool expanded)
    {
        m_expanded = expanded;
        update();
        if (m_owner)
            m_owner->update();
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event);
        const ThemeTokens t = ThemeTokens::resolve();

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        const bool bodyVisible = m_owner && m_owner->isExpanded();
        // 与 layout 1px margin 配合：向父级外缘外扩，使圆角弧与 FormGroupBox 外框一致
        const QRectF r = bodyVisible ? QRectF(rect()).adjusted(-1, -1, 1, 0)
                                     : QRectF(rect()).adjusted(-1, -1, 1, 1);
        const QPainterPath bgPath =
            bodyVisible ? topRoundedRect(r, kFormGroupRadius) : fullRoundedRect(r, kFormGroupRadius);

        QColor bg = t.headerBg;
        if (isCollapsible() && m_pressed)
            bg = t.headerBgPressed;
        else if (isCollapsible() && m_hovered)
            bg = t.headerBgHover;
        p.fillPath(bgPath, bg);

        QFont f = themeUiFont(12);
        f.setBold(true);
        p.setFont(f);
        p.setPen(t.textPrimary);

        QString text = m_titleText;
        if (isCollapsible()) {
            const QString icon = m_expanded ? QStringLiteral("\u25BE  ") : QStringLiteral("\u25B8  ");
            text = icon + text;
        }

        const QRect textRect = rect().adjusted(12, 0, -12, 0);
        p.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        if (isCollapsible() && event->button() == Qt::LeftButton) {
            m_pressed = true;
            update();
            event->accept();
            return;
        }
        QWidget::mousePressEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if (isCollapsible() && event->button() == Qt::LeftButton && m_pressed) {
            m_pressed = false;
            update();
            if (m_owner)
                m_owner->setExpanded(!m_owner->isExpanded());
            event->accept();
            return;
        }
        QWidget::mouseReleaseEvent(event);
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override
#else
    void enterEvent(QEvent *event) override
#endif
    {
        if (isCollapsible()) {
            m_hovered = true;
            update();
        }
        QWidget::enterEvent(event);
    }

    void leaveEvent(QEvent *event) override
    {
        m_hovered = false;
        m_pressed = false;
        update();
        QWidget::leaveEvent(event);
    }

private:
    bool isCollapsible() const
    {
        return m_owner && m_owner->titleStyle() == FormGroupBoxTitleStyle::Collapsible;
    }

    FormGroupBox *m_owner     = nullptr;
    QString       m_titleText;
    bool          m_expanded  = true;
    bool          m_hovered   = false;
    bool          m_pressed   = false;
};

} // namespace

FormGroupBox::FormGroupBox(const QString &titleKey, FormGroupBoxTitleStyle style, QWidget *parent,
                           bool expanded)
    : QWidget(parent)
    , m_titleKey(titleKey)
    , m_style(style)
    , m_expanded(style == FormGroupBoxTitleStyle::Static ? true : expanded)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    buildChrome();
    applyThemeColors();
    updateTitleBar();

    bindThemeRefresh(this, [this]() { applyThemeColors(); });
    if (LM) {
        connect(LM, &LanguageManager::languageChanged, this, &FormGroupBox::retranslateGroup);
    }

    retranslateGroup();
    setExpandedInternal(m_expanded, true);
}

void FormGroupBox::buildChrome()
{
    auto *root = new QVBoxLayout(this);
    // 1px 内边距：子控件不盖住 paintEvent 外边框；标题/内容 fillPath 外扩 1px 与之对齐
    root->setContentsMargins(1, 1, 1, 1);
    root->setSpacing(0);

    m_titleBar = new FormGroupBoxTitleBar(this);
    root->addWidget(m_titleBar);

    m_content = new QWidget(this);
    m_content->setAutoFillBackground(false);
    m_content->setAttribute(Qt::WA_StyledBackground, false);
    m_content->setStyleSheet(QStringLiteral("background: transparent;"));
    m_content->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_contentLayout = new QVBoxLayout(m_content);
    m_contentLayout->setContentsMargins(m_contentMargins);
    m_contentLayout->setSpacing(6);
    root->addWidget(m_content);
}

void FormGroupBox::applyThemeColors()
{
    if (m_content) {
        const auto repolish = [](QWidget *w) {
            if (!w || !w->style())
                return;
            w->style()->unpolish(w);
            w->style()->polish(w);
            w->update();
        };
        for (auto *w : m_content->findChildren<QWidget *>())
            repolish(w);
    }
    for (auto *sw : findChildren<PillSwitch *>())
        sw->update();
    update();
    if (m_titleBar)
        m_titleBar->update();
    if (m_content)
        m_content->update();
}

void FormGroupBox::updateTitleBar()
{
    if (!m_titleBar)
        return;
    auto *bar = static_cast<FormGroupBoxTitleBar *>(m_titleBar);
    bar->setTitleText(TR(m_titleKey.toUtf8().constData()));
    bar->setExpandedVisual(m_expanded);
}

void FormGroupBox::setContentWidget(QWidget *widget)
{
    if (!m_contentLayout || !widget)
        return;

    while (QLayoutItem *item = m_contentLayout->takeAt(0)) {
        if (QWidget *w = item->widget())
            w->setParent(nullptr);
        delete item;
    }

    widget->setParent(m_content);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_contentLayout->addWidget(widget);
}

void FormGroupBox::setTitleKey(const QString &key)
{
    if (!key.isEmpty())
        m_titleKey = key;
    updateTitleBar();
}

void FormGroupBox::retranslateGroup()
{
    updateTitleBar();

    for (auto *row : findChildren<PathLineEdit *>())
        row->retranslateRow();

    for (auto *lbl : findChildren<QLabel *>()) {
        if (lbl->property("trKey").isValid()) {
            const QString key = lbl->property("trKey").toString();
            if (lbl->property("suffixColon").toBool())
                lbl->setText(TR(key.toUtf8().constData()) + QStringLiteral(":"));
            else
                lbl->setText(TR(key.toUtf8().constData()));
        }
    }
    for (auto *btn : findChildren<QPushButton *>()) {
        if (btn->property("trKey").isValid())
            btn->setText(TR(btn->property("trKey").toString().toUtf8().constData()));
    }
}

void FormGroupBox::setExpanded(bool expanded)
{
    setExpandedInternal(expanded, false);
}

void FormGroupBox::setExpandedInternal(bool expanded, bool notifyTitleBar)
{
    if (m_style == FormGroupBoxTitleStyle::Static) {
        m_expanded = true;
        if (m_content)
            m_content->setVisible(true);
        return;
    }

    m_expanded = expanded;
    if (m_content)
        m_content->setVisible(m_expanded);
    if (notifyTitleBar)
        updateTitleBar();
    else if (m_titleBar)
        static_cast<FormGroupBoxTitleBar *>(m_titleBar)->setExpandedVisual(m_expanded);
    update();
}

void FormGroupBox::setMaximumGroupWidth(int width)
{
    setMaximumWidth(width);
}

void FormGroupBox::setMinimumGroupWidth(int width)
{
    setMinimumWidth(width);
}

void FormGroupBox::setContentMargins(int left, int top, int right, int bottom)
{
    setContentMargins(QMargins(left, top, right, bottom));
}

void FormGroupBox::setContentMargins(const QMargins &margins)
{
    m_contentMargins = margins;
    if (m_contentLayout)
        m_contentLayout->setContentsMargins(m_contentMargins);
}

QLabel *FormGroupBox::addCaption(const QString &trKey)
{
    if (!m_content)
        return nullptr;
    auto *cap = new QLabel(TR(trKey.toUtf8().constData()), m_content);
    cap->setProperty("trKey", trKey);
    QFont f = cap->font();
    f.setBold(true);
    cap->setFont(f);
    m_contentLayout->addWidget(cap);
    return cap;
}

void FormGroupBox::addSpacing(int pixels)
{
    if (m_contentLayout)
        m_contentLayout->addSpacing(pixels);
}

PillSwitch *FormGroupBox::addSwitchRow(const QString &labelKey, bool checked, QLabel **outLabel)
{
    if (!m_content)
        return nullptr;

    auto *row = new QWidget(m_content);
    row->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *rl = new QHBoxLayout(row);
    rl->setContentsMargins(0, 4, 0, 4);
    rl->setSpacing(10);

    auto *lbl = new QLabel(TR(labelKey.toUtf8().constData()), row);
    lbl->setProperty("trKey", labelKey);
    lbl->setWordWrap(false);

    auto *sw = makePillSwitch(checked, row);
    rl->addWidget(lbl, 1);
    rl->addWidget(sw, 0, Qt::AlignRight);
    m_contentLayout->addWidget(row);

    if (outLabel)
        *outLabel = lbl;
    return sw;
}

QLineEdit *FormGroupBox::addPathRow(const QString &labelKey, const QString &path)
{
    if (!m_content)
        return nullptr;

    auto *row = makePathLineEdit(labelKey, m_content, path);
    m_contentLayout->addWidget(row);
    return row->lineEdit();
}

QWidget *FormGroupBox::addThresholdBlock(const QString &name, int lo, int hi,
                                         ThemedSpinBox **outLo, ThemedSpinBox **outHi,
                                         QLabel **outNameLabel)
{
    if (!m_content)
        return nullptr;

    auto *block = new QWidget(m_content);
    block->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *hb = new QHBoxLayout(block);
    hb->setContentsMargins(0, 6, 0, 6);
    hb->setSpacing(10);

    auto *nl = new QLabel(name, block);
    nl->setFixedWidth(kThresholdNameLabelW);
    nl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    auto *lbLo = new QLabel(TR("aoi.sys.threshold.lower"), block);
    lbLo->setProperty("trKey", QStringLiteral("aoi.sys.threshold.lower"));
    lbLo->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto *lbHi = new QLabel(TR("aoi.sys.threshold.upper"), block);
    lbHi->setProperty("trKey", QStringLiteral("aoi.sys.threshold.upper"));
    lbHi->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    const QFontMetrics boundFm(lbLo->font());
    const int          boundLabelW =
        qMax(boundFm.horizontalAdvance(TR("aoi.sys.threshold.lower")),
             boundFm.horizontalAdvance(TR("aoi.sys.threshold.upper")))
        + 4;
    lbLo->setFixedWidth(boundLabelW);
    lbHi->setFixedWidth(boundLabelW);
    lbLo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    lbHi->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    auto *spLo = makeSpinBox(block);
    spLo->setRange(0, 9999);
    spLo->setValue(lo);

    auto *spHi = makeSpinBox(block);
    spHi->setRange(0, 9999);
    spHi->setValue(hi);

    hb->addWidget(nl);
    hb->addWidget(lbLo);
    hb->addWidget(spLo, 1);
    hb->addSpacing(4);
    hb->addWidget(lbHi);
    hb->addWidget(spHi, 1);
    m_contentLayout->addWidget(block);

    if (outLo)
        *outLo = spLo;
    if (outHi)
        *outHi = spHi;
    if (outNameLabel)
        *outNameLabel = nl;
    return block;
}

void FormGroupBox::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if (!m_titleBar)
        return;

    const ThemeTokens t = ThemeTokens::resolve();
    QPainter          p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QRectF outer    = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    const QRectF titleR   = QRectF(m_titleBar->geometry());
    const bool   showBody = m_content && m_content->isVisible();

    if (showBody) {
        const QRectF contentR = QRectF(m_content->geometry()).adjusted(-1, 0, 1, 1);
        if (contentR.isValid())
            p.fillPath(bottomRoundedRect(contentR, kFormGroupRadius), t.inputBackground);

        p.setPen(QPen(t.surfaceBorder, 1.0));
        p.drawLine(QPointF(titleR.left(), titleR.bottom()),
                   QPointF(titleR.right(), titleR.bottom()));
    } else {
        p.fillPath(fullRoundedRect(outer, kFormGroupRadius), t.headerBg);
    }

    QPainterPath border;
    border.addRoundedRect(outer, kFormGroupRadius, kFormGroupRadius);
    p.setPen(QPen(t.surfaceBorder, 1.0));
    p.setBrush(Qt::NoBrush);
    p.drawPath(border);
}

} // namespace ui
