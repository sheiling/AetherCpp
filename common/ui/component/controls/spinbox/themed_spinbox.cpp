/**
 * @file themed_spinbox.cpp
 * @brief ThemedSpinBox / ThemedDoubleSpinBox 实现：QSS、自绘箭头、minimumSizeHint
 */

#include "controls/spinbox/themed_spinbox.h"

#include "controls/spinbox/spinbox_style.h"

#include "theme/theme_refresh.h"

#include <QAbstractSpinBox>
#include <QEvent>
#include <QFontMetrics>
#include <QLineEdit>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QStyleOptionSpinBox>

namespace ui {

namespace {

void refreshSpinBox(QAbstractSpinBox *spin)
{
    if (!spin)
        return;
    applyThemedSpinBoxStyle(spin);
    spin->update();
}

void bindSpinBoxTheme(QAbstractSpinBox *spin)
{
    if (!spin)
        return;
    bindThemeRefresh(spin, [spin]() { refreshSpinBox(spin); });
}

} // namespace

void ThemedSpinBox::syncLineEditGeometry()
{
    QLineEdit *le = lineEdit();
    if (!le)
        return;

    const SpinBoxStyleMetrics m;
    const int btnW =
        buttonSymbols() != QAbstractSpinBox::NoButtons ? m.buttonWidth : 0;
    const int left  = m.paddingH;
    const int right = btnW + m.paddingH;
    const int w     = width() - left - right;
    if (w > 0)
        le->setGeometry(left, 0, w, height());
}

ThemedSpinBox::ThemedSpinBox(QWidget *parent)
    : QSpinBox(parent)
{
    setMinimumWidth(0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    bindSpinBoxTheme(this);
    refreshSpinBox(this);
}

QSize ThemedSpinBox::minimumSizeHint() const
{
    const QSize        fallback = QSpinBox::sizeHint();
    const SpinBoxStyleMetrics m;
    QFontMetrics       fm(font());
    QString            text = textFromValue(value());
    if (text.isEmpty())
        text = QStringLiteral("0");

    int w = m.paddingH + fm.horizontalAdvance(text) + m.paddingH;
    if (buttonSymbols() != QAbstractSpinBox::NoButtons)
        w += m.buttonWidth;

    w = qMax(minimumWidth(), w);
    const int maxW = maximumWidth();
    if (maxW > 0 && maxW < QWIDGETSIZE_MAX)
        w = qMin(w, maxW);
    return QSize(w, fallback.height());
}

void ThemedSpinBox::changeEvent(QEvent *event)
{
    QSpinBox::changeEvent(event);
    if (event->type() == QEvent::EnabledChange)
        refreshSpinBox(this);
}

void ThemedSpinBox::resizeEvent(QResizeEvent *event)
{
    QSpinBox::resizeEvent(event);
    syncLineEditGeometry();
}

void ThemedSpinBox::paintEvent(QPaintEvent *event)
{
    QSpinBox::paintEvent(event);
    QStyleOptionSpinBox opt;
    initStyleOption(&opt);
    paintThemedSpinArrows(this, opt);
}

ThemedDoubleSpinBox::ThemedDoubleSpinBox(QWidget *parent)
    : QDoubleSpinBox(parent)
{
    setKeyboardTracking(false);
    setMinimumWidth(0);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    bindSpinBoxTheme(this);
    refreshSpinBox(this);
}

QSize ThemedDoubleSpinBox::minimumSizeHint() const
{
    const QSize        fallback = QDoubleSpinBox::sizeHint();
    const SpinBoxStyleMetrics m;
    QFontMetrics       fm(font());
    QString            text = textFromValue(value());
    if (text.isEmpty())
        text = QStringLiteral("0");

    int w = m.paddingH + fm.horizontalAdvance(text) + m.paddingH;
    if (buttonSymbols() != QAbstractSpinBox::NoButtons)
        w += m.buttonWidth;

    w = qMax(minimumWidth(), w);
    const int maxW = maximumWidth();
    if (maxW > 0 && maxW < QWIDGETSIZE_MAX)
        w = qMin(w, maxW);
    return QSize(w, fallback.height());
}

void ThemedDoubleSpinBox::syncLineEditGeometry()
{
    QLineEdit *le = lineEdit();
    if (!le)
        return;

    const SpinBoxStyleMetrics m;
    const int btnW =
        buttonSymbols() != QAbstractSpinBox::NoButtons ? m.buttonWidth : 0;
    const int left  = m.paddingH;
    const int right = btnW + m.paddingH;
    const int w     = width() - left - right;
    if (w > 0)
        le->setGeometry(left, 0, w, height());
}

void ThemedDoubleSpinBox::changeEvent(QEvent *event)
{
    QDoubleSpinBox::changeEvent(event);
    if (event->type() == QEvent::EnabledChange)
        refreshSpinBox(this);
}

void ThemedDoubleSpinBox::resizeEvent(QResizeEvent *event)
{
    QDoubleSpinBox::resizeEvent(event);
    syncLineEditGeometry();
}

void ThemedDoubleSpinBox::paintEvent(QPaintEvent *event)
{
    QDoubleSpinBox::paintEvent(event);
    QStyleOptionSpinBox opt;
    initStyleOption(&opt);
    paintThemedSpinArrows(this, opt);
}

ThemedSpinBox *makeSpinBox(QWidget *parent, const QString &objectName)
{
    auto *sp = new ThemedSpinBox(parent);
    if (!objectName.isEmpty())
        sp->setObjectName(objectName);
    return sp;
}

ThemedDoubleSpinBox *makeDoubleSpinBox(QWidget *parent, const QString &objectName)
{
    auto *sp = new ThemedDoubleSpinBox(parent);
    if (!objectName.isEmpty())
        sp->setObjectName(objectName);
    return sp;
}

void ensureThemedSpinStyles(QWidget *root)
{
    if (!root)
        return;
    for (auto *sp : root->findChildren<ThemedSpinBox *>())
        refreshSpinBox(sp);
    for (auto *sp : root->findChildren<ThemedDoubleSpinBox *>())
        refreshSpinBox(sp);
}

} // namespace ui
