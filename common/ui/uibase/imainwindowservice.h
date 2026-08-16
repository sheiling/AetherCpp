#pragma once

#include "uibase_global.h"
#include "pagewidget.h"
#include "examplepages.h"
#include <QtPlugin>
#include <QString>

/**
 * @brief 主窗口服务接口（纯接口，无 QObject 继承）
 *
 * CorePlugin 创建 MainWindow 并将其注册到 PluginManager 对象池。
 * MainWindow 通过 Q_INTERFACES(IMainWindowService) 声明实现了此接口，
 * 使得 PluginManager::getObject<IMainWindowService>() 能够找到它。
 *
 * 其他插件在 initialize() 时通过此接口注册页面；
 * CorePlugin 在 extensionsInitialized() 时调用 finalizeNavigation() 完成导航构建；
 * PluginLoaderModule 在 Splash finish 前调用 showAfterSplash() 离屏预渲染，finish 后显示主窗口。
 *
 * 新增：
 *   - setPageVisible()  — 运行时动态显示/隐藏某导航项（供权限管理器回调）
 */
// 纯接口不需要 DLL 导出宏：没有数据成员，实现类在各插件 DLL 中
class IMainWindowService
{
public:
    virtual ~IMainWindowService() = default;

    /**
     * @brief 注册顶层简单页面
     * @param id        页面唯一标识（如 "home"）
     * @param titleKey  翻译键（如 "nav.home"）
     * @param page      页面 widget（所有权转移给服务）
     * @param order     排列顺序（数值小的排在最左/最上）
     */
    virtual void addPage(const QString &id,
                         const QString &titleKey,
                         PageWidget    *page,
                         int            order = 100) = 0;

    /**
     * @brief 注册带二级导航的容器页面
     * @return 返回 PageWithSubNav*，调用者继续调用其 addSubPage() 添加子页
     */
    virtual PageWithSubNav *addContainerPage(const QString &id,
                                             const QString &titleKey,
                                             int            order = 100) = 0;

    /**
     * @brief 所有插件注册完毕后，由 CorePlugin::extensionsInitialized() 调用，
     *        按 order 排序、结合菜单配置和初始权限，添加所有导航项。
     */
    virtual void finalizeNavigation() = 0;

    /**
     * @brief Splash finish 之前由 bootstrap 调用：离屏完成布局与首帧绘制。
     *        实际显示由 QSplashScreen::finish() 触发。
     */
    virtual void showAfterSplash() = 0;

    /**
     * @brief 动态显示/隐藏某一级导航项（权限变更时调用）
     *
     * @param id      注册时使用的页面 ID（如 "home"、"middleware"）
     * @param visible true = 显示，false = 隐藏
     */
    virtual void setPageVisible(const QString &id, bool visible) = 0;

    /** 切换一级导航页（如 aoi.nav.review） */
    virtual void switchToPage(const QString &id) = 0;

    /** @brief 获取主窗口 QWidget 指针 */
    virtual QWidget *mainWindowWidget() = 0;
};

Q_DECLARE_INTERFACE(IMainWindowService,
                    "com.example.pluginsystem.IMainWindowService/1.0")
