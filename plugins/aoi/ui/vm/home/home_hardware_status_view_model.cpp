/**
 * @file home_hardware_status_view_model.cpp
 * @brief DeviceManager 硬件枚举与 MessageCenter 连接态事件 → runtime.hardware
 *
 * 约定（暂）：相机 JSON 内 `deviceName` 与 DeviceManager 注册键（对象 key）一致，
 * MessageCenter 首参与 `deviceKey` 才能对齐；未配置时 CameraBase 会 fallback 到 SN。
 */

#include "home_hardware_status_view_model.h"

#include "home_view_model.h"

#include <common/core/container/application.h>
#include <common/core/device/camera/camera_base.h>
#include <common/core/device/deviceManager/device_manager.h>
#include <messagecenter/message_center.h>
#include <messagecenter/message_define.h>

#include <Logger.h>

#include <functional>

namespace aoi {

namespace {

using common::device::CaptureState;
using ui::LinkIndicatorState;

std::shared_ptr<common::device::DeviceManager> tryGetDeviceManager()
{
    try {
        auto *app = Container::Application::instance();
        if (!app || !app->bound<common::device::DeviceManager>())
            return nullptr;
        return app->make<common::device::DeviceManager>();
    } catch (const std::exception &e) {
        LOG_WARN("[HomeHardwareStatusViewModel] DeviceManager unavailable: {}", e.what());
        return nullptr;
    }
}

LinkIndicatorState linkFromConnected(bool connected)
{
    return connected ? LinkIndicatorState::Connected : LinkIndicatorState::Disconnected;
}

void setConnectionRow(HardwareStatusRow &row, bool active)
{
    row.state     = linkFromConnected(active);
    row.statusKey = active ? QStringLiteral("aoi.sidebar.status.connected")
                             : QStringLiteral("aoi.sidebar.status.disconnected");
    if (!active)
        row.subStatusKey.clear();
}

void setCameraCaptureSubStatus(HardwareStatusRow &row, CaptureState captureState, bool connected)
{
    if (!connected) {
        setConnectionRow(row, false);
        return;
    }

    row.state     = LinkIndicatorState::Connected;
    row.statusKey = QStringLiteral("aoi.sidebar.status.connected");
    switch (captureState) {
    case CaptureState::Preview:
    case CaptureState::Capturing:
        row.subStatusKey = QStringLiteral("aoi.sidebar.substatus.capturing");
        break;
    case CaptureState::Idle:
    default:
        row.subStatusKey = QStringLiteral("aoi.sidebar.substatus.idle");
        break;
    }
}

bool queryRowActive(const HardwareStatusRow &row, common::device::DeviceManager *dm)
{
    if (!dm || row.deviceKey.isEmpty())
        return false;

    switch (row.kind) {
    case HardwareDeviceKind::Camera: {
        const auto camera = dm->CameraHandler(row.deviceKey.toStdString());
        return camera && camera->IsConnected();
    }
    case HardwareDeviceKind::Flash: {
        const auto flash = dm->FlashDetectHandler(row.deviceKey.toStdString());
        return flash && flash->IsOpened();
    }
    case HardwareDeviceKind::Motion: {
        const auto motion = dm->MotionHandler();
        return motion && motion->GetCardConnectedState(row.deviceKey.toStdString());
    }
    case HardwareDeviceKind::Scanner: {
        const auto scanner = dm->ScannerHandler(row.deviceKey.toStdString());
        return scanner && scanner->IsConnected();
    }
    }
    return false;
}

HardwareStatusRow makeHardwareRow(HardwareDeviceKind kind, const QString &key)
{
    HardwareStatusRow row;
    row.kind         = kind;
    row.deviceKey    = key;
    row.displayName  = key;
    row.state        = LinkIndicatorState::Unknown;
    row.statusKey    = QStringLiteral("aoi.sidebar.status.disconnected");
    if (kind == HardwareDeviceKind::Camera)
        row.subStatusKey = QStringLiteral("aoi.sidebar.substatus.idle");
    return row;
}

void applyMotionSubStatus(HardwareStatusRow &row, bool active)
{
    if (active)
        row.subStatusKey = QStringLiteral("aoi.sidebar.substatus.running");
    else
        row.subStatusKey.clear();
}

void applyRowLiveState(HardwareStatusRow &row, common::device::DeviceManager *dm)
{
    const bool active = queryRowActive(row, dm);
    setConnectionRow(row, active);

    if (row.kind == HardwareDeviceKind::Camera && active && dm) {
        const auto camera = dm->CameraHandler(row.deviceKey.toStdString());
        if (camera)
            setCameraCaptureSubStatus(row, camera->GetCaptureState(), true);
    } else if (row.kind == HardwareDeviceKind::Motion) {
        applyMotionSubStatus(row, active);
    }
}

QVector<HardwareStatusRow> buildHardwareRowsFromDeviceManager(common::device::DeviceManager *dm)
{
    QVector<HardwareStatusRow> rows;
    if (!dm)
        return rows;

    for (const std::string &key : dm->getCameraNameList())
        rows.append(makeHardwareRow(HardwareDeviceKind::Camera, QString::fromStdString(key)));

    for (const std::string &key : dm->getFlashDetectNameList())
        rows.append(makeHardwareRow(HardwareDeviceKind::Flash, QString::fromStdString(key)));

    for (const std::string &key : dm->getMotionCardNameList())
        rows.append(makeHardwareRow(HardwareDeviceKind::Motion, QString::fromStdString(key)));

    for (const std::string &key : dm->getScannerNameList())
        rows.append(makeHardwareRow(HardwareDeviceKind::Scanner, QString::fromStdString(key)));

    return rows;
}

int findHardwareRowIndex(const QVector<HardwareStatusRow> &rows, HardwareDeviceKind kind,
                         const QString &deviceKey)
{
    for (int i = 0; i < rows.size(); ++i) {
        if (rows[i].kind == kind && rows[i].deviceKey == deviceKey)
            return i;
    }
    return -1;
}

bool applyHardwareConnectionEvent(QVector<HardwareStatusRow> &hardware, HardwareDeviceKind kind,
                                  const QString &deviceKey, bool active)
{
    const int idx = findHardwareRowIndex(hardware, kind, deviceKey);
    if (idx < 0)
        return false;

    HardwareStatusRow &row = hardware[idx];
    setConnectionRow(row, active);

    if (kind == HardwareDeviceKind::Camera && active) {
        if (const auto dm = tryGetDeviceManager()) {
            const auto camera = dm->CameraHandler(deviceKey.toStdString());
            if (camera)
                setCameraCaptureSubStatus(row, camera->GetCaptureState(), true);
        }
    } else if (kind == HardwareDeviceKind::Motion) {
        applyMotionSubStatus(row, active);
    }

    return true;
}

bool applyCameraCaptureEvent(QVector<HardwareStatusRow> &hardware, const QString &cameraKey,
                             int captureState)
{
    const int idx = findHardwareRowIndex(hardware, HardwareDeviceKind::Camera, cameraKey);
    if (idx < 0)
        return false;

    const bool connected = hardware[idx].state != LinkIndicatorState::Disconnected;
    setCameraCaptureSubStatus(hardware[idx], static_cast<CaptureState>(captureState), connected);
    return true;
}

} // namespace

HomeHardwareStatusViewModel::HomeHardwareStatusViewModel(HomeViewModel *homeViewModel,
                                                           QObject *parent)
    : QObject(parent)
    , m_homeViewModel(homeViewModel)
{
    attachMessageCenter();
    refresh();
}

HomeHardwareStatusViewModel::~HomeHardwareStatusViewModel()
{
    detachMessageCenter();
}

void HomeHardwareStatusViewModel::refresh()
{
    if (!m_homeViewModel)
        return;

    const auto dm = tryGetDeviceManager();
    m_homeViewModel->modifyRuntime([&](HomeRuntimeState &state) {
        if (!dm) {
            state.hardware.clear();
            return;
        }

        state.hardware = buildHardwareRowsFromDeviceManager(dm.get());
        for (HardwareStatusRow &row : state.hardware)
            applyRowLiveState(row, dm.get());
    });
}

void HomeHardwareStatusViewModel::attachMessageCenter()
{
    MessageCenter::Center::Attach(
        EVENT_CAMERA_CONN_STATE, this, SLOT(onCameraConnState(const QVariantList &)));
    MessageCenter::Center::Attach(
        EVENT_CAMERA_STATE, this, SLOT(onCameraState(const QVariantList &)));
    MessageCenter::Center::Attach(
        EVENT_FLASH_DETECT_STATE, this, SLOT(onFlashDetectState(const QVariantList &)));
    MessageCenter::Center::Attach(
        EVENT_MOTION_CONN_STATE, this, SLOT(onMotionConnState(const QVariantList &)));
    MessageCenter::Center::Attach(
        EVENT_SCANNER_CONN_STATE, this, SLOT(onScannerConnState(const QVariantList &)));
}

void HomeHardwareStatusViewModel::detachMessageCenter()
{
    MessageCenter::Center::Detach(
        EVENT_CAMERA_CONN_STATE, this, SLOT(onCameraConnState(const QVariantList &)));
    MessageCenter::Center::Detach(
        EVENT_CAMERA_STATE, this, SLOT(onCameraState(const QVariantList &)));
    MessageCenter::Center::Detach(
        EVENT_FLASH_DETECT_STATE, this, SLOT(onFlashDetectState(const QVariantList &)));
    MessageCenter::Center::Detach(
        EVENT_MOTION_CONN_STATE, this, SLOT(onMotionConnState(const QVariantList &)));
    MessageCenter::Center::Detach(
        EVENT_SCANNER_CONN_STATE, this, SLOT(onScannerConnState(const QVariantList &)));
}

void HomeHardwareStatusViewModel::onCameraConnState(const QVariantList &args)
{
    if (!m_homeViewModel || args.size() < 2)
        return;

    const QString deviceKey = args.at(0).toString(); // 须与 JSON deviceName / 注册键一致
    const bool    connected = args.at(1).toBool();
    m_homeViewModel->modifyRuntime([&](HomeRuntimeState &state) {
        applyHardwareConnectionEvent(state.hardware, HardwareDeviceKind::Camera, deviceKey,
                                     connected);
    });
}

void HomeHardwareStatusViewModel::onCameraState(const QVariantList &args)
{
    if (!m_homeViewModel || args.size() < 2)
        return;

    m_homeViewModel->modifyRuntime([&](HomeRuntimeState &state) {
        applyCameraCaptureEvent(state.hardware, args.at(0).toString(), args.at(1).toInt());
    });
}

void HomeHardwareStatusViewModel::onFlashDetectState(const QVariantList &args)
{
    if (!m_homeViewModel || args.size() < 2)
        return;

    m_homeViewModel->modifyRuntime([&](HomeRuntimeState &state) {
        applyHardwareConnectionEvent(state.hardware, HardwareDeviceKind::Flash, args.at(0).toString(),
                                     args.at(1).toBool());
    });
}

void HomeHardwareStatusViewModel::onMotionConnState(const QVariantList &args)
{
    if (!m_homeViewModel || args.size() < 2)
        return;

    m_homeViewModel->modifyRuntime([&](HomeRuntimeState &state) {
        applyHardwareConnectionEvent(state.hardware, HardwareDeviceKind::Motion, args.at(0).toString(),
                                     args.at(1).toBool());
    });
}

void HomeHardwareStatusViewModel::onScannerConnState(const QVariantList &args)
{
    if (!m_homeViewModel || args.size() < 2)
        return;

    m_homeViewModel->modifyRuntime([&](HomeRuntimeState &state) {
        applyHardwareConnectionEvent(state.hardware, HardwareDeviceKind::Scanner, args.at(0).toString(),
                                     args.at(1).toBool());
    });
}

} // namespace aoi
