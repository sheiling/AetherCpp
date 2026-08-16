#pragma once

/**
 * @file no_wheel_input_guard.h
 * @brief 禁止 SpinBox/ComboBox 滚轮改值，滚轮事件转发给祖先 QScrollArea
 *
 * 在嵌套滚动面板中，鼠标悬停在 SpinBox/ComboBox 上时滚轮会意外改值。
 * NoWheelInputGuard 作为 eventFilter 拦截 Wheel 事件并转发给最近的
 * QScrollArea::viewport()，使滚轮继续驱动外层滚动。
 */

#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QEvent>
#include <QObject>
#include <QScrollArea>
#include <QWidget>

namespace ui {

/**
 * 自 widget 向上遍历父链，查找第一个 QScrollArea 祖先。
 * @return 找到的 QScrollArea，无则 nullptr
 */
inline QScrollArea *findAncestorScrollArea(QWidget *widget)
{
    for (QWidget *p = widget->parentWidget(); p; p = p->parentWidget()) {
        if (auto *scroll = qobject_cast<QScrollArea *>(p))
            return scroll;
    }
    return nullptr;
}

/**
 * 事件过滤器：拦截 QAbstractSpinBox / QComboBox 的滚轮事件。
 *
 * 生命周期：由 installOn 创建并以 root 为 parent，随 root 销毁。
 */
class NoWheelInputGuard : public QObject
{
public:
    explicit NoWheelInputGuard(QObject *parent = nullptr) : QObject(parent) {}

    /**
     * 在 root 子树中为所有 SpinBox 与 ComboBox 安装滚轮守卫。
     * @param root 搜索起点；为 nullptr 时无操作
     */
    static void installOn(QWidget *root)
    {
        if (!root)
            return;
        auto *guard = new NoWheelInputGuard(root);
        for (auto *spin : root->findChildren<QAbstractSpinBox *>())
            spin->installEventFilter(guard);
        for (auto *combo : root->findChildren<QComboBox *>())
            combo->installEventFilter(guard);
    }

protected:
    /**
     * Wheel 事件：转发至祖先 QScrollArea 并吞掉（return true）；
     * 其他事件原样传递。
     */
    bool eventFilter(QObject *obj, QEvent *event) override
    {
        if (event->type() != QEvent::Wheel)
            return QObject::eventFilter(obj, event);

        auto *widget = qobject_cast<QWidget *>(obj);
        if (!widget)
            return QObject::eventFilter(obj, event);

        if (auto *scroll = findAncestorScrollArea(widget))
            QApplication::sendEvent(scroll->viewport(), event);
        return true;
    }
};

} // namespace ui
