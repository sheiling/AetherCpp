#pragma once

/**
 * @file progress_overlay.h
 * @brief 通用进度浮层：阶段说明 + 进度条 + 可选取消
 *
 * 半透明圆角面板浮于父控件之上，不依赖具体业务；文案由调用方传入（含 i18n）。
 * 主题色通过 ThemeTokens 绑定，随 ThemeManager 切换自动刷新。
 */

#include "component_global.h"

#include <QWidget>

class QFrame;
class QLabel;
class QProgressBar;
class QResizeEvent;
class QShowEvent;

namespace ui {

class ThemedPushButton;

/**
 * 居中进度遮罩。
 *
 * 典型用法：作为预览区子控件铺满父级，长任务开始时 show + setProgress，
 * 结束时 hide；取消按钮 connect cancelClicked。
 */
class UI_COMPONENT_EXPORT ProgressOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit ProgressOverlay(QWidget *parent = nullptr);

    /** 更新进度（0–100）与阶段说明（已翻译明文） */
    void setProgress(int percent, const QString &message);

    /** 取消按钮文案；空串时保留按钮但无文字 */
    void setCancelText(const QString &text);

    /** 是否显示取消按钮，默认 true */
    void setCancelVisible(bool visible);

    /**
     * 面板居中参考区域（父坐标系内）。
     * 未设置或与父控件相同时，使用 overlay 自身 rect。
     */
    void setAnchorWidget(QWidget *anchor);

    /** 从 ThemeTokens 刷新面板、标签与进度条样式 */
    void applyThemeStyle();

signals:
    void cancelClicked();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void layoutPanel();
    QRect anchorRect() const;

    QWidget           *m_anchorWidget   = nullptr;
    QFrame            *m_panel          = nullptr;
    QLabel            *m_messageLabel   = nullptr;
    QProgressBar      *m_progressBar  = nullptr;
    ThemedPushButton  *m_cancelBtn    = nullptr;
};

} // namespace ui
