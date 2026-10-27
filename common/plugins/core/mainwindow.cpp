#include "mainwindow.h"

#include <QEventLoop>
#include <QApplication>
#include <QChar>
#include <QCoreApplication>
#include <QDir>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QPalette>
#include <QPixmap>
#include <QScreen>
#include <QTimer>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <algorithm>
#include <QDebug>

#include "command_audit_registry.h"

namespace {
QString rgba(const QColor &c, int alpha)
{
    return QStringLiteral("rgba(%1,%2,%3,%4)")
        .arg(c.red()).arg(c.green()).arg(c.blue()).arg(alpha);
}

QColor themeColorOrDefault(const QString &name, const QColor &fallback)
{
    const QColor c = TM ? TM->themeColor(name) : QColor{};
    return c.isValid() ? c : fallback;
}

void applyTitleBarThemeStyle(QWidget *titleBar)
{
    if (!titleBar) return;

    // Match the reference screenshot with fixed, deterministic colors.
    // const QColor barBg(QStringLiteral("#041937"));
    // const QColor bottomLine(QStringLiteral("#133b76"));
    // const QColor textNormal(QStringLiteral("#d7e2f1"));
    // const QColor textActive(QStringLiteral("#ffffff"));
    // const QColor logoBorder(QStringLiteral("#3c6ca8"));
    // const QColor navBorder(QStringLiteral("#4f8fd5"));
    // const QColor navHoverBg(QStringLiteral("rgba(35,72,122,0.45)"));
    // const QColor navCheckedBg(QStringLiteral("#2f5f9c"));
    const QColor barBg = themeColorOrDefault(QStringLiteral("secondaryDarkColor"), QColor(QStringLiteral("#041937")));
    const QColor bottomLine = themeColorOrDefault(QStringLiteral("secondaryLightColor"), QColor(QStringLiteral("#133b76")));
    const QColor textNormal = themeColorOrDefault(QStringLiteral("secondaryTextColor"), QColor(QStringLiteral("#d7e2f1")));
    const QColor textActive = themeColorOrDefault(QStringLiteral("primaryTextColor"), QColor(QStringLiteral("#ffffff")));
    const QColor logoBorder(QStringLiteral("#2f4763"));
    const QColor navBorder(QStringLiteral("#4f8fd5"));
    const QColor navHoverBg = themeColorOrDefault(QStringLiteral("primaryLightColor"), QColor(QStringLiteral("#2f5f9c")));
    const QColor navCheckedBg = themeColorOrDefault(QStringLiteral("primaryColor"), QColor(QStringLiteral("#2f5f9c")));

    const QString style = QStringLiteral(
        "QWidget#titlebar {"
        "  background-color: %1;"
        "  border-bottom: 1px solid %2;"
        "}"
        "QWidget#titleBarContent {"
        "  background-color: %1;"
        "}"
        "QWidget#titleRightArea {"
        "  background-color: %1;"
        "  border: none;"
        "  margin: 0px;"
        "  padding: 0px;"
        "}"
        "QLabel#titleLogo {"
        "  border: 1px solid %3;"
        "  border-radius: 3px;"
        "  padding-left: 6px;"
        "  background-color: %1;"
        "}"
        "QPushButton#primaryNavBtn,"
        "QPushButton#secondaryNavBtn {"
        "  color: %4;"
        "  background-color: transparent;"
        "  border: 1px solid transparent;"
        "  border-radius: 3px;"
        "  padding: 0 16px;"
        "  font-size: 14px;"
        "  min-height: 32px;"
        "}"
        "QPushButton#primaryNavBtn:hover,"
        "QPushButton#secondaryNavBtn:hover {"
        "  color: %5;"
        "  border-color: transparent;"
        "  background-color: %6;"
        "}"
        "QPushButton#primaryNavBtn:checked,"
        "QPushButton#secondaryNavBtn:checked {"
        "  color: %5;"
        "  border: 1px solid %9;"
        "  background-color: %7;"
        "}"
        "QLabel#currentUserLabel {"
        "  color: %4;"
        "  padding: 0 8px;"
        "}"
        "QPushButton#loginButton,"
        "QPushButton#logoutButton {"
        "  background-color: transparent;"
        "  color: %4;"
        "  border: none;"
        "  padding: 0 8px;"
        "}"
        "QPushButton#loginButton:hover,"
        "QPushButton#logoutButton:hover {"
        "  color: %5;"
        "  background-color: %8;"
        "}"
        "QPushButton#langButton,"
        "QPushButton#themeButton,"
        "QPushButton#btnMinimize,"
        "QPushButton#btnMaximize,"
        "QPushButton#btnClose {"
        "  background-color: %1;"
        "  color: %4;"
        "  border: none;"
        "  margin: 0px;"
        "  padding: 0px 8px;"
        "}"
        "QPushButton#langButton:hover,"
        "QPushButton#themeButton:hover,"
        "QPushButton#btnMinimize:hover,"
        "QPushButton#btnMaximize:hover,"
        "QPushButton#btnClose:hover {"
        "  color: %5;"
        "  background-color: %8;"
        "}"
        "QPushButton#langButton:pressed,"
        "QPushButton#themeButton:pressed,"
        "QPushButton#btnMinimize:pressed,"
        "QPushButton#btnMaximize:pressed,"
        "QPushButton#btnClose:pressed,"
        "QPushButton#langButton:checked,"
        "QPushButton#themeButton:checked,"
        "QPushButton#btnMinimize:checked,"
        "QPushButton#btnMaximize:checked,"
        "QPushButton#btnClose:checked {"
        "  background-color: %1;"
        "  color: %5;"
        "  border: none;"
        "}")
        .arg(barBg.name(),
             bottomLine.name(),
             logoBorder.name(),
             textNormal.name(),
             textActive.name(),
             navHoverBg.name(),
             navCheckedBg.name(),
             navHoverBg.name(),
             navBorder.name());

    titleBar->setStyleSheet(style);
}

QString titleBarLogoResourcePath()
{
    // 暗色标题栏用 white_logo；亮色用 black_logo（资源见 ui_resources.qrc）
    if (TM && TM->isDark())
        return QStringLiteral(":/resources/image/white_logo.png");
    return QStringLiteral(":/resources/image/black_logo.png");
}

void applyTitleBarLogo(QLabel *logoLabel)
{
    if (!logoLabel)
        return;

    const QPixmap logo(titleBarLogoResourcePath());
    if (logo.isNull())
        return;

    logoLabel->setPixmap(logo.scaled(160, 28, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void applyWidgetWindowBackground(QWidget *widget)
{
    if (!widget)
        return;

    const QColor windowBg = themeColorOrDefault(
        QStringLiteral("windowColor"),
        themeColorOrDefault(QStringLiteral("secondaryDarkColor"), QColor(QStringLiteral("#0f1923"))));
    widget->setAutoFillBackground(true);
    QPalette pal = widget->palette();
    pal.setColor(QPalette::Window, windowBg);
    widget->setPalette(pal);
}
} // namespace

MainWindow::MainWindow(QWidget *parent)
    : CFramelessWindow(parent)
{
    setCornerRadius(0);
    setupUi();

    // 显示与最大化在 showAfterSplash()（Splash finish 之后）；此处仅无屏幕时的兜底尺寸
    if (!QApplication::primaryScreen())
        resize(1920, 1000);

    const QString windowTitle = QCoreApplication::applicationName();
    if (!windowTitle.isEmpty())
        setWindowTitle(windowTitle);
}

void MainWindow::setupUi()
{
    // ── 1. Central widget ──────────────────────────────────────────────────
    auto *central = new QWidget(this);
    central->setObjectName("centralwidget");
    setCentralWidget(central);
    applyWidgetWindowBackground(central);
    applyWidgetWindowBackground(this);

    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ── 2. Title bar row ──────────────────────────────────────────────────
    m_titleBar = new QWidget(central);
    m_titleBar->setObjectName("titlebar");
    m_titleBar->setFixedHeight(44);

    auto *titleLayout = new QHBoxLayout(m_titleBar);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(0);

    m_titleBarContent = new QWidget(m_titleBar);
    m_titleBarContent->setObjectName("titleBarContent");
    auto *titleContentLayout = new QHBoxLayout(m_titleBarContent);
    titleContentLayout->setContentsMargins(10, 0, 12, 0);
    titleContentLayout->setSpacing(12);

    m_logoLabel = new QLabel(m_titleBarContent);
    m_logoLabel->setObjectName("titleLogo");
    m_logoLabel->setFixedSize(170, 32);
    m_logoLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_logoLabel->setText(QString());

    applyTitleBarLogo(m_logoLabel);

    m_primaryNavHost = new QWidget(m_titleBarContent);
    m_primaryNavHost->setObjectName("primaryNavHost");
    m_primaryNavHost->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    titleContentLayout->addWidget(m_logoLabel, 0, Qt::AlignVCenter);
    titleContentLayout->addWidget(m_primaryNavHost, 1);

    auto makeBtn = [this](const QString &objName, QWidget *parent) -> QPushButton * {
        auto *btn = new QPushButton(parent);
        btn->setObjectName(objName);
        btn->setFixedSize(46, 32);
        btn->setFocusPolicy(Qt::NoFocus);
        btn->setFlat(true);
        btn->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        return btn;
    };

    m_btnMinimize = makeBtn("btnMinimize", m_titleBar);
    m_btnMinimize->setText(QChar(0x2212));
    m_btnMaximize = makeBtn("btnMaximize", m_titleBar);
    updateMaximizeButton();
    m_btnClose = makeBtn("btnClose", m_titleBar);
    m_btnClose->setText(QChar(0x00D7));

    m_langButton = new QPushButton(TR("lang.switch"), m_titleBar);
    m_langButton->setObjectName("langButton");
    m_langButton->setFixedSize(72, 32);
    m_langButton->setCursor(Qt::PointingHandCursor);

    m_themeButton = new QPushButton(TR("theme.toDark"), m_titleBar);
    m_themeButton->setObjectName("themeButton");
    m_themeButton->setFixedSize(56, 32);
    m_themeButton->setCursor(Qt::PointingHandCursor);

    m_currentUserLabel = new QLabel(m_titleBar);
    m_currentUserLabel->setObjectName(QStringLiteral("currentUserLabel"));
    m_currentUserLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

    m_loginButton = new QPushButton(QStringLiteral("登录"), m_titleBar);
    m_loginButton->setObjectName(QStringLiteral("loginButton"));
    m_loginButton->setFixedHeight(32);
    m_loginButton->setCursor(Qt::PointingHandCursor);

    m_logoutButton = new QPushButton(QStringLiteral("退出登录"), m_titleBar);
    m_logoutButton->setObjectName(QStringLiteral("logoutButton"));
    m_logoutButton->setFixedHeight(32);
    m_logoutButton->setCursor(Qt::PointingHandCursor);

    auto *titleRightArea = new QWidget(m_titleBar);
    titleRightArea->setObjectName("titleRightArea");
    auto *rightLayout = new QHBoxLayout(titleRightArea);
    rightLayout->setContentsMargins(12, 0, 0, 0);
    rightLayout->setSpacing(0);
    rightLayout->addWidget(m_currentUserLabel);
    rightLayout->addWidget(m_loginButton);
    rightLayout->addWidget(m_logoutButton);
    rightLayout->addWidget(m_langButton);
    rightLayout->addWidget(m_themeButton);
    rightLayout->addWidget(m_btnMinimize);
    rightLayout->addWidget(m_btnMaximize);
    rightLayout->addWidget(m_btnClose);

    titleLayout->addWidget(m_titleBarContent, 1);
    titleLayout->addWidget(titleRightArea, 0);
    connect(m_logoutButton, &QPushButton::clicked, this, &MainWindow::onLogoutClicked);

    updateCurrentUserLabel();

    // ── 3. Main content area ──────────────────────────────────────────────
    auto *titleBottomLine = new QWidget(central);
    titleBottomLine->setObjectName("titleBottomLine");
    titleBottomLine->setFixedHeight(1);
    titleBottomLine->setStyleSheet(QStringLiteral("background-color: %1;").arg(themeColorOrDefault(QStringLiteral("secondaryLightColor"), QColor(QStringLiteral("#133b76"))).name()));

    m_mainContentContainer = new QWidget(central);
    m_mainContentLayout    = new QStackedLayout(m_mainContentContainer);
    m_mainContentLayout->setContentsMargins(0, 0, 0, 0);
    applyWidgetWindowBackground(m_mainContentContainer);

    mainLayout->addWidget(m_titleBar);
    mainLayout->addWidget(titleBottomLine);
    mainLayout->addWidget(m_mainContentContainer, 1);

    // ── 4. Frameless window configuration ────────────────────────────────
    setTitleBar(m_titleBar);
    addIgnoreWidget(m_titleBarContent);
    addIgnoreWidget(m_logoLabel);
    addIgnoreWidget(m_primaryNavHost);

    // ── 5. Connect signals ────────────────────────────────────────────────
    connect(m_btnMinimize, &QPushButton::clicked, this, [this] { showMinimized(); });
    connect(m_btnMaximize, &QPushButton::clicked, this, &MainWindow::onMaximizeClicked);
    connect(m_btnClose,    &QPushButton::clicked, this, [this] { close(); });

    connect(m_langButton,  &QPushButton::clicked, this, [] { LM->toggleLanguage(); });
    connect(m_themeButton, &QPushButton::clicked, this, [] { TM->toggleTheme(); });

    connect(LM, &LanguageManager::languageChanged, this, &MainWindow::onLanguageChanged);
    connect(TM, &ThemeManager::themeChanged,       this, &MainWindow::onThemeChanged);

    // ── 6. 连接权限运行时服务 ─────────────────────────────────────────────
    connect(PS, &PermService::permissionsChanged, this, &MainWindow::onPermissionsChanged);

    applyTitleBarThemeStyle(m_titleBar);
}

// ── IMainWindowService implementation ────────────────────────────────────────

void MainWindow::addPage(const QString &id,
                         const QString &titleKey,
                         PageWidget    *page,
                         int            order)
{
    m_pendingPages.append({order, id, titleKey, page, nullptr});
}

PageWithSubNav *MainWindow::addContainerPage(const QString &id,
                                             const QString &titleKey,
                                             int            order)
{
    auto *container = new PageWithSubNav(m_mainContentContainer);
    m_pendingPages.append({order, id, titleKey, nullptr, container});
    return container;
}

bool MainWindow::computeNavVisible(const QString &navPageId) const
{
    return PS->canNav(navPageId);
}

void MainWindow::finalizeNavigation()
{
    // 按 order 排序
    std::sort(m_pendingPages.begin(), m_pendingPages.end(),
              [](const PendingPage &a, const PendingPage &b) {
                  return a.order < b.order;
              });

    // 创建导航管理器
    m_primaryNavManager = new PrimaryNavigationManager(
        m_primaryNavHost, m_mainContentLayout, this);

    for (const PendingPage &entry : m_pendingPages) {
        PageWidget *page = (entry.page != nullptr) ? entry.page : entry.container;
        if (!page) continue;

        // 保存 id → titleKey 映射，供权限刷新使用
        m_idToMenuKey[entry.id] = entry.titleKey;

        // 若是容器页，记录到 m_containerPages 供子菜单刷新使用
        if (entry.container) {
            m_containerPages[entry.id] = entry.container;
        }

        m_primaryNavManager->addNavigation(entry.id, entry.titleKey, page);

        const bool navVisible = computeNavVisible(entry.titleKey);
        m_primaryNavManager->setNavItemVisible(entry.id, navVisible);
        if (!navVisible) {
            qDebug() << "[MainWindow] Nav item hidden (PermService):" << entry.titleKey;
        }

        if (entry.container) {
            for (const QString &subTitleKey : entry.container->subNavTitleKeys()) {
                const bool subVisible = computeSubPageVisible(subTitleKey);
                entry.container->setSubItemVisible(subTitleKey, subVisible);
                if (!subVisible) {
                    qDebug() << "[MainWindow] Sub-nav hidden (PermService):" << subTitleKey
                             << "under" << entry.titleKey;
                }
            }
        }
    }
    m_pendingPages.clear();

    m_primaryNavManager->activateFirstVisible();
    activateSubNavForCurrentPage();

    qDebug() << "[MainWindow] Navigation finalized. windowSize=" << size()
             << "PermService ready=" << PS->isReady()
             << "effectiveUser=" << PS->effectiveUserId()
             << "grantsNav(permission)=" << PS->canNav(QStringLiteral("nav.permission"));
}

void MainWindow::showAfterSplash()
{
    m_normalGeometry = defaultCenteredNormalGeometry();

    // Splash 仍可见时离屏完成布局、懒加载与首帧绘制，避免切换后出现白框
    setAttribute(Qt::WA_DontShowOnScreen, true);
    showMaximized();
    ensurePolished();
    if (auto *central = centralWidget()) {
        central->ensurePolished();
        central->update();
    }
    if (m_mainContentContainer)
        m_mainContentContainer->update();

    if (QApplication *app = qobject_cast<QApplication *>(QCoreApplication::instance()))
        app->processEvents(QEventLoop::ExcludeUserInputEvents);

    setAttribute(Qt::WA_DontShowOnScreen, false);
    // 离屏 showMaximized 会使 isVisible()==true，但窗口并未真正显示；
    // 须 hide 复位，否则 QSplashScreen::finish() 会跳过 show()。
    hide();
}

void MainWindow::setPageVisible(const QString &id, bool visible)
{
    if (m_primaryNavManager)
        m_primaryNavManager->setNavItemVisible(id, visible);
}

void MainWindow::switchToPage(const QString &id)
{
    if (m_primaryNavManager)
        m_primaryNavManager->switchToPage(id);
}

// ── Slots ──────────────────────────────────────────────────────────────────

void MainWindow::onPermissionsChanged()
{
    if (!m_primaryNavManager) return;

    // ── 1. 刷新一级导航（顶部标签）────────────────────────────────────────
    for (const QString &id : m_primaryNavManager->registeredIds()) {
        const QString titleKey = m_idToMenuKey.value(id);
        m_primaryNavManager->setNavItemVisible(id, computeNavVisible(titleKey));
    }

    // ── 2. 刷新二级导航（容器页侧边栏）────────────────────────────────────
    for (auto it = m_containerPages.cbegin(); it != m_containerPages.cend(); ++it) {
        PageWithSubNav *container = it.value();
        if (!container) continue;

        for (const QString &subTitleKey : container->subNavTitleKeys()) {
            container->setSubItemVisible(subTitleKey, computeSubPageVisible(subTitleKey));
        }
    }
    syncNavigationSelectionAfterVisibilityChange();
    qDebug() << "[MainWindow] Navigation refreshed. PermService user=" << PS->effectiveUserId()
             << "role=" << PS->effectiveRoleId();
}

void MainWindow::updateCurrentUserLabel()
{
    if (!m_currentUserLabel)
        return;
    const QString name = property("currentUserDisplayName").toString().trimmed();
    m_currentUserLabel->setText(QStringLiteral("当前用户:%1").arg(name.isEmpty() ? QStringLiteral("未登录") : name));
}

void MainWindow::onLogoutClicked()
{
    ManualAuditEvent audit;
    audit.actionType = QStringLiteral("auth");
    audit.actionName = QStringLiteral("logout");
    audit.moduleCode = QStringLiteral("auth");
    AuditLogRegistry::emitManualAudit(audit);

    if (PS->isReady()) {
        PS->logout();
    } else {
        PS->reloadAsMinimalUser();
    }

    setProperty("currentUserDisplayName", QVariant());
    updateCurrentUserLabel();
    qDebug() << "[MainWindow] Logout: session cleared, permissions reset to minimal user.";
}

bool MainWindow::computeSubPageVisible(const QString &subPageId) const
{
    return PS->canSubPage(subPageId);
}

void MainWindow::activateSubNavForCurrentPage()
{
    if (!m_primaryNavManager)
        return;
    if (auto *container = qobject_cast<PageWithSubNav *>(m_primaryNavManager->currentPage()))
        container->activateFirstVisibleSubPage();
}

void MainWindow::syncNavigationSelectionAfterVisibilityChange()
{
    if (!m_primaryNavManager)
        return;

    const QString currentId = m_primaryNavManager->currentPageId();
    if (!currentId.isEmpty() && !m_primaryNavManager->isNavItemVisible(currentId)) {
        m_primaryNavManager->activateFirstVisible();
        activateSubNavForCurrentPage();
        return;
    }

    if (auto *container = qobject_cast<PageWithSubNav *>(m_primaryNavManager->currentPage())) {
        if (!container->isCurrentSubVisible())
            container->activateFirstVisibleSubPage();
    }
}

void MainWindow::onMaximizeClicked()
{
    if (isMaximized()) {
        showNormal();
        // 无边框窗口还原后 Win32 可能仍用无效 normalGeometry，下一帧强制居中
        QTimer::singleShot(0, this, [this]() { applyNormalGeometry(); });
    } else {
        captureNormalGeometry();
        if (!m_normalGeometry.isValid() || m_normalGeometry.isEmpty())
            m_normalGeometry = defaultCenteredNormalGeometry();
        showMaximized();
    }
}

void MainWindow::onLanguageChanged()
{
    if (m_langButton)  m_langButton->setText(TR("lang.switch"));
    updateThemeButton();
}

void MainWindow::onThemeChanged()
{
    applyTitleBarThemeStyle(m_titleBar);
    applyTitleBarLogo(m_logoLabel);
    updateThemeButton();
}

void MainWindow::updateThemeButton()
{
    if (m_themeButton)
        m_themeButton->setText(TM->isDark() ? TR("theme.toLight") : TR("theme.toDark"));
}

void MainWindow::updateMaximizeButton()
{
    if (m_btnMaximize)
        m_btnMaximize->setText(isMaximized() ? QChar(0x25A1) : QChar(0x25A0));
}

QRect MainWindow::defaultCenteredNormalGeometry() const
{
    const QScreen *scr = screen() ? screen() : QApplication::primaryScreen();
    if (!scr)
        return QRect(0, 0, 1920, 1000);

    const QRect avail = scr->availableGeometry();
    int w = qMin(1920, avail.width() * 80 / 100);
    int h = qMin(1000, avail.height() * 85 / 100);
    w = qMax(w, minimumWidth());
    h = qMax(h, minimumHeight());

    const int x = avail.x() + (avail.width() - w) / 2;
    const int y = avail.y() + (avail.height() - h) / 2;
    return QRect(x, y, w, h);
}

void MainWindow::captureNormalGeometry()
{
    if (!isMaximized() && !isFullScreen() && !isMinimized())
        m_normalGeometry = geometry();
}

void MainWindow::applyNormalGeometry()
{
    if (isMaximized() || isFullScreen())
        return;

    const QRect target =
        (m_normalGeometry.isValid() && !m_normalGeometry.isEmpty())
            ? m_normalGeometry
            : defaultCenteredNormalGeometry();
    setGeometry(target);
}

// ── Protected events ───────────────────────────────────────────────────────

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange)
        updateMaximizeButton();
    CFramelessWindow::changeEvent(event);
}
