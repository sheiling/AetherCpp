#pragma once

/**
 * @file home_hardware_status_view_model.h
 * @brief 首页硬件连接状态：DeviceManager 枚举 + MessageCenter 事件 → `HomeRuntimeState::hardware`
 */

#include "home_runtime_state.h"

#include <QObject>
#include <QVariantList>

namespace aoi {

class HomeViewModel;

/**
 * @brief 硬件连接状态子 ViewModel
 *
 * 订阅 EVENT_CAMERA_CONN_STATE / EVENT_CAMERA_STATE / EVENT_FLASH_DETECT_STATE /
 * EVENT_MOTION_CONN_STATE / EVENT_SCANNER_CONN_STATE，增量更新父 VM 的 runtime.hardware。
 */
class HomeHardwareStatusViewModel : public QObject
{
    Q_OBJECT

public:
    explicit HomeHardwareStatusViewModel(HomeViewModel *homeViewModel, QObject *parent = nullptr);
    ~HomeHardwareStatusViewModel() override;

    /** 从 DeviceManager 重建 hardware 列表并刷新连接状态 */
    void refresh();

private slots:
    void onCameraConnState(const QVariantList &args);
    void onCameraState(const QVariantList &args);
    void onFlashDetectState(const QVariantList &args);
    void onMotionConnState(const QVariantList &args);
    void onScannerConnState(const QVariantList &args);

private:
    void attachMessageCenter();
    void detachMessageCenter();

    HomeViewModel *m_homeViewModel = nullptr;
};

} // namespace aoi
