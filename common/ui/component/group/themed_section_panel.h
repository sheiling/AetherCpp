#pragma once

/**
 * @file themed_section_panel.h
 * @brief 带标题栏的卡片面板：外框 + 固定高度标题 + 可注入内容区
 *
 * 用于首页区块、侧栏分组等；标题栏默认 30px，内容通过 `contentLayout()` 填充。
 * 与 `FormGroupBox` 区别：无折叠/表单行 API，结构更轻。
 */

#include <QFrame>
#include <QString>
#include "component_global.h"

class QLabel;
class QVBoxLayout;

namespace ui {

/** 区块标题相对内容区的排布方式 */
enum class SectionTitlePlacement {
    TopLeft,      ///< 标题横排在内容区上方左上角（默认）
    LeftVertical  ///< 标题竖排在内容区左侧，释放顶部空间
};

class UI_COMPONENT_EXPORT ThemedSectionPanel : public QFrame
{
    Q_OBJECT

public:
    /**
     * @param titleKey       标题翻译键
     * @param parent         父控件
     * @param titleBarHeight TopLeft 模式下标题行高度（px）；LeftVertical 模式下为左侧竖排标题区宽度
     * @param titlePlacement 标题排布方式
     */
    explicit ThemedSectionPanel(const QString &titleKey,
                              QWidget *parent         = nullptr,
                              int      titleBarHeight = 30,
                              SectionTitlePlacement titlePlacement = SectionTitlePlacement::TopLeft);

    SectionTitlePlacement titlePlacement() const { return m_titlePlacement; }
    void setTitlePlacement(SectionTitlePlacement placement);

    QString titleKey() const { return m_titleKey; }
    void    setTitleKey(const QString &key);

    int  titleBarHeight() const { return m_titleBarHeight; }
    void setTitleBarHeight(int height);

    /** 内容区垂直布局（标题栏下方，默认 stretch=1） */
    QVBoxLayout *contentLayout() const { return m_contentLayout; }

    /** 内容区宿主控件 */
    QWidget *contentHost() const { return m_contentHost; }

    /**
     * 用单个控件填充内容区（会清空原有 content 子项）
     * @param stretch 传给 contentLayout->addWidget 的 stretch，默认 1
     */
    void setContentWidget(QWidget *widget, int stretch = 1);

    void setContentMargins(int left, int top, int right, int bottom);
    void setContentSpacing(int spacing);

    /** 语言切换时更新标题文案 */
    void retranslatePanel();

protected:
    void changeEvent(QEvent *event) override;

private:
    void buildLayout();
    void bindTheme();
    /** 根据 ThemeTokens 生成面板/标题 QSS */
    void refreshStyle();
    void updateTitleGeometry();

    QString               m_titleKey;
    int                   m_titleBarHeight = 30;
    SectionTitlePlacement m_titlePlacement = SectionTitlePlacement::TopLeft;
    QLabel               *m_titleLabel     = nullptr;
    QWidget              *m_contentHost    = nullptr;
    QLayout              *m_rootLayout     = nullptr;
    QVBoxLayout          *m_contentLayout  = nullptr;
};

} // namespace ui
