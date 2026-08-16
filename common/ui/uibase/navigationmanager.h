#pragma once

#include "uibase_global.h"
#include "pagewidget.h"
#include <QObject>
#include <QMap>
#include <QList>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedLayout>
#include <QPushButton>

struct UIBASE_EXPORT NavigationItem
{
    QString     id;
    QString     titleKey;
    PageWidget *page   = nullptr;
    QPushButton *button = nullptr;

    NavigationItem() = default;
    NavigationItem(const QString &id, const QString &titleKey, PageWidget *page)
        : id(id), titleKey(titleKey), page(page) {}
};

/**
 * @brief 一级导航管理器（水平标签式）
 *
 * 新增：
 *   - setNavItemVisible(id, bool)  — 运行时动态显示/隐藏某导航按钮
 *   - isNavItemVisible(id)         — 查询当前可见状态
 */
class UIBASE_EXPORT PrimaryNavigationManager : public QObject
{
    Q_OBJECT

public:
    explicit PrimaryNavigationManager(QWidget *navBar,
                                      QStackedLayout *contentLayout,
                                      QObject *parent = nullptr);

    void addNavigation(const QString &id, const QString &titleKey, PageWidget *page);
    void addRightWidget(QWidget *widget);
    void switchToPage(const QString &id);

    /**
     * @brief 动态显示或隐藏某导航按钮
     *
     * - 隐藏时：按钮不可见，页面仍保留在堆栈中；
     *   若当前正显示该页，自动跳转到第一个可见页。
     * - 显示时：按钮重新可见，不改变当前页面。
     */
    void setNavItemVisible(const QString &id, bool visible);

    /**
     * @brief 查询某导航项当前的可见状态
     */
    bool isNavItemVisible(const QString &id) const;

    /** 按注册顺序切换到第一个未隐藏（意图可见）的导航项 */
    void activateFirstVisible();

    QString     currentPageId() const { return m_currentPageId; }
    PageWidget *currentPage()   const;

    /** @brief 返回已注册的所有导航 ID（按注册顺序） */
    QStringList registeredIds() const;

signals:
    void pageChanged(const QString &pageId);

private slots:
    void onButtonClicked();
    void onLanguageChanged();

private:
    void updateButtonStates();
    void switchToFirstVisible();

    QWidget        *m_navBar;
    QStackedLayout *m_contentLayout;
    QHBoxLayout    *m_navLayout;
    QMap<QString, NavigationItem> m_items;
    QList<QString>                m_navOrder;
    QString         m_currentPageId;
};

/**
 * @brief 二级导航管理器（垂直侧栏式）
 *
 * 新增：
 *   - setNavItemVisible(id, bool)
 *   - isNavItemVisible(id)
 */
class UIBASE_EXPORT SecondaryNavigationManager : public QObject
{
    Q_OBJECT

public:
    explicit SecondaryNavigationManager(QWidget *navBar,
                                        QStackedLayout *contentLayout,
                                        QObject *parent = nullptr);

    void addNavigation(const QString &id, const QString &titleKey, PageWidget *page);
    void switchToPage(const QString &id);
    void refreshCurrentPage();
    void clear();

    /** 动态显示/隐藏二级导航按钮 */
    void setNavItemVisible(const QString &id, bool visible);
    bool isNavItemVisible(const QString &id) const;

    /** 按注册顺序切换到第一个未隐藏（意图可见）的子导航项 */
    void activateFirstVisible();

    QString     currentPageId() const { return m_currentPageId; }
    PageWidget *currentPage()   const;

    /** @brief 返回已注册的所有子导航 ID（按注册顺序） */
    QStringList registeredIds() const;

signals:
    void pageChanged(const QString &pageId);

private slots:
    void onButtonClicked();
    void onLanguageChanged();

private:
    void updateButtonStates();
    void switchToFirstVisible();

    QWidget        *m_navBar;
    QStackedLayout *m_contentLayout;
    QVBoxLayout    *m_navLayout;
    QMap<QString, NavigationItem> m_items;
    QList<QString>                m_navOrder;
    QString         m_currentPageId;
};
