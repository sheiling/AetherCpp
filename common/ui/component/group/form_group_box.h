#pragma once

/**
 * @file form_group_box.h
 * @brief 表单分组框：统一自定义标题栏 + 可注入内容区
 *
 * 替代已废弃的 `makeFormSection`（QGroupBox 壳）与 `CollapsibleSection`（配方折叠区）。
 * 标题栏与外框颜色经 `ThemeTokens` 自绘（标题条 paintEvent + FormGroupBox::paintEvent），
 * 随 `ThemeManager::themeChanged` 刷新；
 * 内容区：`ThemedLineEdit` / `ThemedPushButton` / `PillSwitch` / `ThemedSpinBox` 在各自类内刷新主题样式；
 * 纯展示标签仍用 QLabel，继承全局 ACSS。
 */

#include "controls/input/path_line_edit.h"
#include "controls/switch/pill_switch.h"
#include "controls/spinbox/themed_spinbox.h"

#include <QMargins>
#include <QWidget>
#include "component_global.h"

class QLabel;
class QLineEdit;
class QVBoxLayout;

namespace ui {

/** 标题栏模式：静态展示，或可点击折叠/展开内容区 */
enum class FormGroupBoxTitleStyle {
    Static,      ///< 标题条不可点击，内容始终可见
    Collapsible, ///< 点击整条标题栏切换内容区显隐，右侧显示 ▾/▸
};

/**
 * @brief 表单分组框
 *
 * 典型用法：
 * @code
 * auto *sec = new FormGroupBox(QStringLiteral("aoi.sys.section.switch_title"),
 *                              FormGroupBoxTitleStyle::Static, parent);
 * sec->setMinimumGroupWidth(400);
 * m_scanSwitch = sec->addSwitchRow(QStringLiteral("aoi.sys.switch.scan"), true);
 * @endcode
 *
 * 配方页注入整块面板：
 * @code
 * auto *group = new FormGroupBox(titleKey, FormGroupBoxTitleStyle::Collapsible, parent, true);
 * group->setContentWidget(buildParamBasicInfo(group));
 * @endcode
 */
class UI_COMPONENT_EXPORT FormGroupBox : public QWidget
{
    Q_OBJECT

public:
    /**
     * @param titleKey  区块标题翻译键（如 `aoi.sys.section.switch_title`）
     * @param style     静态或可折叠标题栏
     * @param parent    父控件
     * @param expanded  初始是否展开（仅 Collapsible 有效，Static 忽略）
     */
    explicit FormGroupBox(const QString &titleKey,
                          FormGroupBoxTitleStyle style = FormGroupBoxTitleStyle::Static,
                          QWidget *parent              = nullptr,
                          bool expanded                = true);

    /** 内容区根控件，可向 `contentLayout()` 继续 addWidget */
    QWidget *contentWidget() const { return m_content; }

    /** 内容区垂直布局，spacing 默认 6 */
    QVBoxLayout *contentLayout() const { return m_contentLayout; }

    /**
     * @brief 用外部构建好的面板替换内容区（会清空原有 content 子项）
     *
     * 用于配方页等「先 buildXxxPanel() 再包一层分组框」的场景；widget 会被 reparent 到内部 content。
     */
    void setContentWidget(QWidget *widget);

    void setTitleKey(const QString &key);
    QString titleKey() const { return m_titleKey; }

    /** 刷新标题栏文案及内容区内带 `trKey` 属性的子控件 */
    void retranslateGroup();

    void setExpanded(bool expanded);
    bool isExpanded() const { return m_expanded; }

    FormGroupBoxTitleStyle titleStyle() const { return m_style; }

    /** 限制整块最大宽度（系统配置页常用 520 / 560） */
    void setMaximumGroupWidth(int width);

    /** 限制整块最小宽度（系统配置子分组常用 400） */
    void setMinimumGroupWidth(int width);

    /** 内容区内边距，默认 (12, 8, 12, 12) */
    void setContentMargins(int left, int top, int right, int bottom);
    void setContentMargins(const QMargins &margins);
    QMargins contentMargins() const { return m_contentMargins; }

    // ── 便捷行 API（实现位于 .cpp，内部组装行布局）──

    /** 区块内小标题（如「通用开关」），`trKey` 用于 retranslateGroup */
    QLabel *addCaption(const QString &trKey);

    void addSpacing(int pixels);

    /** 标签 + PillSwitch 行；labelKey 为翻译键或缺陷默认显示名键 */
    PillSwitch *addSwitchRow(const QString &labelKey, bool checked,
                             QLabel **outLabel = nullptr);

    /** 路径选择行（标签 + 输入 + 浏览） */
    QLineEdit *addPathRow(const QString &labelKey, const QString &path = {});

    /** 阈值行：名称 + 下限/上限 ThemedSpinBox */
    QWidget *addThresholdBlock(const QString &name, int lo, int hi,
                              ThemedSpinBox **outLo = nullptr,
                              ThemedSpinBox **outHi  = nullptr,
                              QLabel **outNameLabel  = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void buildChrome();
    void applyThemeColors();
    void updateTitleBar();
    void setExpandedInternal(bool expanded, bool notifyTitleBar);

    QString               m_titleKey;
    FormGroupBoxTitleStyle m_style     = FormGroupBoxTitleStyle::Static;
    bool                  m_expanded   = true;
    QWidget              *m_titleBar   = nullptr;
    QWidget              *m_content    = nullptr;
    QVBoxLayout          *m_contentLayout = nullptr;
    QMargins              m_contentMargins{12, 8, 12, 12};
};

} // namespace ui
