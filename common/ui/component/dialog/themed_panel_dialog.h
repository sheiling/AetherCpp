#pragma once

/**
 * @file themed_panel_dialog.h
 * @brief 通用主题弹窗壳：标题栏 + 可注入内容区 + 拖拽/居中
 *
 * 无边框 QDialog（FramelessWindowHint + 半透明根），内嵌圆角面板与阴影。
 * 业务弹窗将自定义 QWidget 传入 setBodyWidget()，按钮与表单放在内容控件内。
 *
 * 生命周期要点：
 * - 构造后已绑定主题刷新与语言切换
 * - 注入 body 或改标题后须 applyTheme()（非子类静态调用场景必调）
 * - 布局完成后调用 finalizeLayoutSize() 锁定尺寸，避免 show 时抖动
 */

#include <QDialog>
#include <QPoint>
#include <QSize>
#include <QString>
#include "component_global.h"

class QFrame;
class QLabel;
class QPushButton;
class QVBoxLayout;
class QWidget;

namespace ui {

/**
 * 可复用主题弹窗框架。
 *
 * 结构：根布局(8px 边距) → themedPanelFrame → 标题栏(36px) + bodyHost。
 * 标题栏支持左键拖拽移动；关闭钮发射 close → dialogClosed。
 * 子类可覆盖 retranslateBody() 以响应语言切换。
 */
class UI_COMPONENT_EXPORT ThemedPanelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ThemedPanelDialog(QWidget *parent = nullptr);

    /** 设置 i18n 标题键；非空时 retranslateShell 用 TR(key) 刷新标题 */
    void setTitleKey(const QString &key);
    /** 直接设置标题栏可见文案（不经过 TR） */
    void setTitleText(const QString &text);
    QString titleKey() const { return m_titleKey; }

    /**
     * 注入主体内容（会 reparent 到 m_bodyHost 并加入 m_bodyLayout）。
     * 替换旧 widget 时先从布局移除。注入后自动 applyTheme()。
     */
    void setBodyWidget(QWidget *widget);
    QWidget *bodyWidget() const { return m_bodyWidget; }
    /** 内容区宿主控件（themedPanelFrame 内、标题栏下方） */
    QWidget *bodyHost() const { return m_bodyHost; }

    /** 设置面板最小宽度，下限 320px */
    void setMinimumPanelWidth(int width);
    /** 设置面板最小高度，下限 240px；finalizeLayoutSize 时与内容 sizeHint 取较大值 */
    void setMinimumPanelHeight(int height);
    /**
     * 按当前内容计算并锁定对话框固定尺寸。
     * 须在 setBodyWidget / 内容布局完成后、exec() 或 show() 之前调用。
     * 若已 enableResizableWindow()，则仅记录建议尺寸，不调用 setFixedSize。
     */
    void finalizeLayoutSize();

    /**
     * 允许拖拽改变大小、showMaximized() 及标题栏双击切换最大化。
     * 大内容弹窗（如检测详情）须在 finalizeLayoutSize 之后或不调用 finalize 时使用。
     */
    void enableResizableWindow(const QSize &minSize = QSize(640, 480));

    bool isResizableWindow() const { return m_resizable; }

    /**
     * 刷新 QSS（buildContentDialogStylesheet）、根/标题字体及 body 内 ThemedFormLabel。
     * 注入 body 或改标题后调用；ThemedMessageBox 等非子类场景必调。
     */
    void applyTheme();

signals:
    /** closeEvent 时发射，供外部监听弹窗已关闭 */
    void dialogClosed();

protected:
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    /** 根据 m_titleKey 刷新 windowTitle 与 m_titleLabel（键非空时） */
    void retranslateShell();
    /** 子类覆盖以刷新 body 区文案（语言切换时由 retranslateDialog 调用） */
    virtual void retranslateBody() {}
    /** retranslateShell + retranslateBody */
    void retranslateDialog();
    /** 相对父窗口居中，并钳制在可用屏幕区域内 */
    void centerOnParent();
    /** 判断坐标是否落在标题栏区域内（用于拖拽起始判定） */
    bool isInTitleBar(const QPoint &pos) const;

    QFrame      *m_panel          = nullptr; /**< 圆角面板外框 themedPanelFrame */
    QWidget     *m_titleBar       = nullptr; /**< 标题栏行 themedPanelTitleBar */
    QLabel      *m_titleLabel     = nullptr; /**< 标题文本 themedPanelTitle */
    QPushButton *m_titleCloseBtn  = nullptr; /**< 关闭钮 themedPanelTitleCloseBtn */
    QWidget     *m_bodyHost       = nullptr; /**< 内容区宿主 */
    QVBoxLayout *m_bodyLayout     = nullptr; /**< 内容区垂直布局 */
    QWidget     *m_bodyWidget       = nullptr; /**< 业务注入的主体控件 */

private:
    QString m_titleKey;          /**< i18n 标题键，空则仅 setTitleText 生效 */
    int     m_panelMinWidth  = 480; /**< 面板最小宽度（像素） */
    int     m_panelMinHeight = 0;   /**< 面板最小高度（像素），0 表示仅随内容 */
    bool    m_dragging      = false; /**< 标题栏拖拽进行中 */
    bool    m_resizable     = false; /**< 可缩放/最大化模式 */
    QPoint  m_dragOffset;        /**< 拖拽时鼠标相对窗口左上角的偏移 */
    QSize   m_stableSize;        /**< finalizeLayoutSize 锁定的尺寸，show 时恢复 */
};

} // namespace ui
