#pragma once

#include "uibase_global.h"
#include "pagewidget.h"
#include "navigationmanager.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QMap>
#include <QStackedLayout>

/**
 * @brief 简单内容展示页（首页、帮助页等）
 *
 * 居中显示一段翻译文本（通过 contentKey 查 LanguageManager）。
 */
class UIBASE_EXPORT SimplePage : public PageWidget
{
    Q_OBJECT

public:
    explicit SimplePage(const QString &contentKey, QWidget *parent = nullptr);

protected:
    void onLoad()        override;
    void retranslateUi() override;

private:
    QString  m_contentKey;
    QLabel  *m_label = nullptr;
};

// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief 带标题 + 说明文字的子页面
 */
class UIBASE_EXPORT SubPage : public PageWidget
{
    Q_OBJECT

public:
    explicit SubPage(const QString &titleKey, QWidget *parent = nullptr);

protected:
    void onLoad()        override;
    void retranslateUi() override;

private:
    QString  m_titleKey;
    QLabel  *m_titleLabel   = nullptr;
    QLabel  *m_contentLabel = nullptr;
};

// ─────────────────────────────────────────────────────────────────────────────

/**
 * @brief 带二级侧边导航的容器页
 *
 * 布局：
 *   ┌───────────┬─────────────────────────┐
 *   │ 侧边导航  │   子页面内容区（堆栈）  │
 *   │ (150px)   │                         │
 *   └───────────┴─────────────────────────┘
 *
 * 使用：在插件 initialize() 中调用 addSubPage() 注册子页。
 * SecondaryNavigationManager 在构造时立即创建，因此 addSubPage()
 * 可在 widget 显示前安全调用。
 */
class UIBASE_EXPORT PageWithSubNav : public PageWidget
{
    Q_OBJECT

public:
    explicit PageWithSubNav(QWidget *parent = nullptr);

    /**
     * @brief 添加子页面（可在 widget 显示前调用）
     *
     * 内部维护 titleKey → navId 映射，以便通过翻译键动态控制可见性。
     */
    void addSubPage(const QString &id,
                    const QString &titleKey,
                    PageWidget    *page);

    /**
     * @brief 根据 titleKey 显示 / 隐藏子导航项（与 addSubPage 第二个参数、权限资源 key 对齐）
     */
    void setSubItemVisible(const QString &titleKey, bool visible);

    /// 已注册的子页 titleKey 列表（MainWindow 刷新子页权限时用）
    QStringList subNavTitleKeys() const;

    QString currentSubPageId() const;

    /** 当前子页是否意图可见（!isHidden，与权限隐藏一致） */
    bool isCurrentSubVisible() const;

    /** 按注册顺序切换到第一个意图可见的子页 */
    void activateFirstVisibleSubPage();

private:
    void setSubItemVisibleById(const QString &subPageId, bool visible);

    QWidget                    *m_navBar        = nullptr;
    QWidget                    *m_contentArea   = nullptr;
    QStackedLayout             *m_contentLayout = nullptr;
    SecondaryNavigationManager *m_navManager    = nullptr;

    /// titleKey（如 "sub.general"）→ navId（如 "general"）映射
    QMap<QString, QString>      m_keyToId;
};
