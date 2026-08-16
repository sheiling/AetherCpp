/**
 * @file themed_line_edit.cpp
 * @brief ThemedLineEdit 实现：line_edit_style QSS 与主题/只读态刷新
 */

#include "controls/input/themed_line_edit.h"

#include "controls/input/line_edit_style.h"

#include "theme/theme_refresh.h"

#include <QEvent>

namespace ui {

ThemedLineEdit::ThemedLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    bindTheme();
    refreshStyle();
}

void ThemedLineEdit::bindTheme()
{
    bindThemeRefresh(this, [this]() { refreshStyle(); });
}

void ThemedLineEdit::refreshStyle()
{
    setStyleSheet(buildLineEditStylesheet());
}

void ThemedLineEdit::changeEvent(QEvent *event)
{
    QLineEdit::changeEvent(event);
    switch (event->type()) {
    case QEvent::EnabledChange:
    case QEvent::ReadOnlyChange:
        refreshStyle();
        break;
    default:
        break;
    }
}

ThemedLineEdit *makeLineEdit(QWidget *parent)
{
    return new ThemedLineEdit(parent);
}

} // namespace ui
