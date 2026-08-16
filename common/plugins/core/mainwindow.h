#pragma once

#include "uibase/framelesswindow.h"
#include "uibase/navigationmanager.h"
#include "uibase/imainwindowservice.h"
#include "uibase/languagemanager.h"
#include "uibase/thememanager.h"
#include "uibase/permission/permservice.h"

#include <QStackedLayout>
#include <QPushButton>
#include <QLabel>
#include <QList>
#include <QRect>

/**
 * @brief 应用主窗口（纯代码实现，无 .ui 文件）
 *
 * 同时实现 IMainWindowService，插件通过此接口注册页面。
 * CorePlugin::extensionsInitialized() 调用 finalizeNavigation() 构建导航；
 * 主窗口在 PluginLoaderModule 中于 Splash finish 之前调用 showAfterSplash() 离屏预渲染。
 *
 * 一级/子页可见性由 PermService（PS）按资源全局唯一 key（通常为 titleKey）判定。
 *
 * 布局：
 *   ┌──────────────────────────────────────────────────────┐
 *   │ titlebar [navButtons...] [lang][theme] [─][□][✕]    │ ← 高度 32px，可拖动
 *   ├──────────────────────────────────────────────────────┤
 *   │                  mainContentContainer                 │
 *   └──────────────────────────────────────────────────────┘
 */
class MainWindow : public CFramelessWindow, public IMainWindowService
{
    Q_OBJECT
    Q_INTERFACES(IMainWindowService)

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

    // ── IMainWindowService interface ─────────────────────────────────────
    void addPage(const QString &id,
                 const QString &titleKey,
                 PageWidget    *page,
                 int            order = 100) override;

    PageWithSubNav *addContainerPage(const QString &id,
                                     const QString &titleKey,
                                     int            order = 100) override;

    void     finalizeNavigation() override;
    void     showAfterSplash() override;

    /**
     * @brief 动态显示/隐藏某一级导航项
     *
     * 由 PermService::permissionsChanged 驱动调用。
     */
    void     setPageVisible(const QString &id, bool visible) override;

    void     switchToPage(const QString &id) override;

    QWidget *mainWindowWidget() override { return this; }

public slots:
    /**
     * @brief 刷新标题栏“当前用户”显示
     *
     * 规则（最小耦合）：
     * - 若主窗口 dynamicProperty("currentUserDisplayName") 非空：显示该名称
     * - 否则：显示“未登录”
     *
     * 登录插件只需要设置 dynamicProperty 并调用此 slot，即可支持“切换账号再登录”刷新。
     */
    void updateCurrentUserLabel();

    /** 标题栏「退出登录」：清会话并恢复最小用户权限 */
    void onLogoutClicked();

private slots:
    void onMaximizeClicked();
    void onLanguageChanged();
    void onThemeChanged();
    void updateThemeButton();
    /** 权限变更时自动刷新所有导航项可见性 */
    void onPermissionsChanged();

protected:
    void changeEvent(QEvent *event) override;

private:
    void setupUi();
    void updateMaximizeButton();
    /** 在当前屏幕工作区内生成居中默认窗口矩形（还原/首次最大化前使用） */
    QRect defaultCenteredNormalGeometry() const;
    /** 记录非最大化状态下的 geometry，供 showNormal 还原 */
    void captureNormalGeometry();
    /** 将窗口置于 m_normalGeometry（无效时用 defaultCenteredNormalGeometry） */
    void applyNormalGeometry();
    /** 一级导航是否可见（key 通常为 addPage/addContainerPage 的 titleKey） */
    bool computeNavVisible(const QString &navPageId) const;
    /** 子页是否可见（key 通常为 addSubPage 的 titleKey） */
    bool computeSubPageVisible(const QString &subPageId) const;
    void activateSubNavForCurrentPage();
    /** 权限刷新后：当前页被隐藏时才重选第一个 visible 页/子页 */
    void syncNavigationSelectionAfterVisibilityChange();

    // ── Title bar widgets ─────────────────────────────────────────────────
    QWidget     *m_titleBar        = nullptr;
    QWidget     *m_titleBarContent = nullptr;
    QLabel      *m_logoLabel       = nullptr;
    QWidget     *m_primaryNavHost  = nullptr;
    QPushButton *m_btnMinimize     = nullptr;
    QPushButton *m_btnMaximize     = nullptr;
    QPushButton *m_btnClose        = nullptr;
    QPushButton *m_langButton      = nullptr;
    QPushButton *m_themeButton     = nullptr;
    QLabel      *m_currentUserLabel = nullptr;
    QPushButton *m_loginButton     = nullptr;
    QPushButton *m_logoutButton    = nullptr;

    // ── Content area ──────────────────────────────────────────────────────
    QWidget        *m_mainContentContainer = nullptr;
    QStackedLayout *m_mainContentLayout    = nullptr;

    // ── Navigation ───────────────────────────────────────────────────────
    PrimaryNavigationManager *m_primaryNavManager = nullptr;

    // ── Pending page registrations ────────────────────────────────────────
    struct PendingPage {
        int            order;
        QString        id;        ///< 导航 ID，如 "home"
        QString        titleKey;  ///< 翻译键，如 "nav.home"
        PageWidget    *page;      ///< 简单页（container 为 null 时有效）
        PageWithSubNav *container; ///< 容器页（非 null 时为容器页）
    };
    QList<PendingPage> m_pendingPages;

    /// 导航 ID → 菜单键（titleKey）映射，finalizeNavigation 后用于权限刷新
    QMap<QString, QString> m_idToMenuKey;

    /// 带子导航的容器页映射（导航 ID → 容器页），用于子菜单权限动态刷新
    QMap<QString, PageWithSubNav*> m_containerPages;

    /// 最大化前的 normal 几何；启动时预置居中矩形，避免还原落到左上角
    QRect m_normalGeometry;
};
