#pragma once

/**
 * @file system_config_view.h
 * @brief 系统配置容器页底栏：统一保存按钮与绑定 flush
 *
 * 由插件在 PageWithSubNav 容器上调用 mountSystemConfigSaveBar，
 * 保存前依次 flush 开关/通用/阈值三子页的 BindingSession。
 */

class PageWithSubNav;

namespace aoi {

class SystemConfigSwitchView;
class SystemConfigGeneralView;
class SystemConfigThresholdView;
class SystemConfigViewModel;

/** 在 `PageWithSubNav` 底部安装「保存配置」栏并连接 VM saveCommand */
void mountSystemConfigSaveBar(PageWithSubNav *container, SystemConfigViewModel *viewModel,
                              SystemConfigSwitchView *switchView,
                              SystemConfigGeneralView *generalView,
                              SystemConfigThresholdView *thresholdView);

} // namespace aoi
