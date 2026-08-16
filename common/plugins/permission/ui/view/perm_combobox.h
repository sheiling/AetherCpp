#pragma once

/**
 * @file perm_combobox.h
 * @brief 工具栏下拉：继承 ThemedComboBox，弹出层与控件等宽并紧贴下方
 */

#include "controls/combo/themed_combo_box.h"

#include <QAbstractItemView>
#include <QLayout>
#include <QTimer>

namespace permission_ui {

class PermComboBox : public ui::ThemedComboBox
{
public:
    using ui::ThemedComboBox::ThemedComboBox;

protected:
    void showPopup() override
    {
        ui::ThemedComboBox::showPopup();
        QAbstractItemView *v = view();
        if (!v)
            return;

        const int w = width();
        v->setMinimumWidth(w);
        v->setMaximumWidth(w);

        QWidget *popup = v->parentWidget();
        while (popup && !popup->isWindow())
            popup = popup->parentWidget();
        if (!popup)
            return;

        QTimer::singleShot(0, this, [this, popup] {
            const QPoint below = mapToGlobal(QPoint(0, height()));
            popup->move(below.x(), below.y() + 2);
            if (QLayout *ly = popup->layout())
                ly->setContentsMargins(0, 0, 0, 0);
            popup->setFixedWidth(width());
        });
    }
};

} // namespace permission_ui
