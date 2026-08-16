#pragma once

/**
 * @file cursor_toast.h
 * @brief 临时状态提示（Toast）控件
 *
 * 半透明圆角浮层，显示在父控件所属顶层窗口的**几何中心**（映射至父控件坐标系）。
 * 鼠标穿透（WA_TransparentForMouseEvents），不拦截点击。
 * 主题色通过 ThemeTokens 自绘背景，文字色经 QPalette 设置。
 */

#include "component_global.h"

#include <QWidget>

class QLabel;
class QPaintEvent;
class QTimer;

namespace ui {

/**
 * 操作反馈 Toast。
 *
 * 典型用法：页面持有一个 CursorToast(parent=页面根控件)，调用
 * showMessage(text, hideMs)。hideMs > 0 时自动隐藏；<= 0 则保持至下次调用。
 */
class UI_COMPONENT_EXPORT CursorToast : public QWidget
{
    Q_OBJECT

public:
    explicit CursorToast(QWidget *parent = nullptr);

    /**
     * 显示文本并居中于父窗口。
     * @param text 提示内容；空串时忽略
     * @param hideMs 自动隐藏毫秒数；<= 0 表示不自动隐藏
     */
    void showMessage(const QString &text, int hideMs = 2500);

    /** 按父窗口中心重新计算位置（父控件坐标系内） */
    void repositionAtParentCenter();

    /** @deprecated 兼容旧名；等同 repositionAtParentCenter() */
    void repositionNearCursor() { repositionAtParentCenter(); }

    /** 从 ThemeTokens 刷新标签色与字体，并触发 repaint */
    void applyToastStyle();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void applyLabelColors();

    QLabel *m_label     = nullptr;
    QTimer *m_hideTimer = nullptr;
};

} // namespace ui
