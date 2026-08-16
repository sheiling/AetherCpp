#pragma once

#include "permission_ui_utils.h"

#include <QCheckBox>
#include <QPaintEvent>
#include <QPainter>
#include "theme/theme_tokens.h"

#include <QStyleOptionButton>

/// 与用户列表行内复选框一致的自定义 QCheckBox
class PermCheckBox : public QCheckBox
{
    Q_OBJECT

public:
    explicit PermCheckBox(QWidget *parent = nullptr);
    explicit PermCheckBox(const QString &text, QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

inline PermCheckBox::PermCheckBox(QWidget *parent)
    : QCheckBox(parent)
{
    permission_ui::initPermCheckBox(this);
}

inline PermCheckBox::PermCheckBox(const QString &text, QWidget *parent)
    : QCheckBox(text, parent)
{
    permission_ui::initPermCheckBox(this);
}

inline void PermCheckBox::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QStyleOptionButton option;
    initStyleOption(&option);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRect indicator =
        style()->subElementRect(QStyle::SE_CheckBoxIndicator, &option, this);
    if (!indicator.isValid() || indicator.width() < 8)
        indicator = QRect((width() - 18) / 2, (height() - 18) / 2, 18, 18);
    permission_ui::paintPermissionCheckbox(&painter, indicator, isChecked());

    const QRect textRect = style()->subElementRect(QStyle::SE_CheckBoxContents, &option, this);
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    painter.setPen(t.textPrimary);
    QFont f = font();
    f.setPixelSize(13);
    painter.setFont(f);
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, text());
}
