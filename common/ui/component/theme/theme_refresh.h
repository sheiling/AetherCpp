#pragma once

/**
 * @file theme_refresh.h
 * @brief 自绘控件订阅 ThemeManager 亮/暗切换与样式表刷新
 *
 * 封装对 ThemeManager::themeChanged 与 stylesheetUpdated 的双信号绑定，
 * 使用 Qt::QueuedConnection 确保回调执行时 ACSS 色板与 QSS 已更新完毕。
 */

#include "uibase/thememanager.h"

#include <functional>

namespace ui {

/**
 * 将 refresh 回调绑定到 ThemeManager 的主题变更信号。
 * @param receiver 信号接收方（通常传 this）；其生命周期应覆盖 guard 对象
 * @param refresh 主题变化时执行的刷新逻辑（如 update()、applyStyle()）
 *
 * 若 receiver 或 TM 为空则静默跳过。连接为 Queued，避免在 ThemeManager
 * 内部同步更新色板期间读取到半成品状态。
 */
inline void bindThemeRefresh(QObject *receiver, const std::function<void()> &refresh)
{
    if (!receiver || !TM) {
        return;
    }
    const auto queued = [refresh]() {
        if (refresh) {
            refresh();
        }
    };
    QObject::connect(TM, &ThemeManager::themeChanged, receiver,
                     [queued](ThemeManager::Theme) { queued(); }, Qt::QueuedConnection);
    QObject::connect(TM, &ThemeManager::stylesheetUpdated, receiver, queued,
                     Qt::QueuedConnection);
}

} // namespace ui