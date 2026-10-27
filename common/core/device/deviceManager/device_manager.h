#ifndef __COMMON_CORE_DEVICE_MANAGER_H__
#define __COMMON_CORE_DEVICE_MANAGER_H__

#include <string>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <unordered_map>
#include <memory>

#include <common/utility/json_tool/reflector.h>

#include "device_manager_global.h"

// 基类头文件始终包含；具体子类（驱动实现）仅在 device_manager.cpp 中按 BUILD_* 条件包含，
// 创建实例时在 create*Instance 内屏蔽未开启的子类。宏由 deviceManager/CMakeLists.txt 注入。

#include <common/core/device/lamphouse/lamphouse_base.h>
#include <common/core/device/scanner/scanner_base.h>
#include <common/core/device/rangefinder/rangefinder_base.h>
#include <common/core/device/pressurePickUp/pressurepickup_base.h>
#include <common/core/device/displayControl/software_screen/software_screen_base.h>
#include <common/core/device/camera/camera_base.h>
#include <common/core/device/motionControl/motion_controller.h>
#include <common/core/device/colorimeter/colorimeter_base.h>
#include <common/core/device/flashDetect/flash_detect_base.h>
// #include <common/core/device/imageCapture/image_capture.h>

namespace common
{
namespace device
{

class DEVICE_MANAGER_EXPORT DeviceManager {
public:
    DeviceManager() = default;
    ~DeviceManager() = default;
public:
    int Init();
    void Release();
public:
    void reflectFromJson(const json& j);
    void reflectToJson(json& j) const;
    /// 从设备配置文件加载（根 JSON 须含 DeviceManager 对象）；相对路径基于 exe 目录。
    bool loadDeviceConfig(const std::string& configPath);

    /// 将当前设备数据写回 loadDeviceConfig 时使用的路径（ordered_json 保序）。
    bool saveDeviceConfig() const;
    /// loadDeviceConfig 成功后的绝对路径；未加载过则为空。
    const std::string& deviceConfigPath() const noexcept { return m_deviceConfigPath_; }

    const std::shared_ptr<common::device::MotionController> MotionHandler() const;									// 获取运动控制句柄
    const std::shared_ptr<common::device::CameraBase> CameraHandler(const std::string& key) const;					// 获取采集相机句柄
    const std::shared_ptr<common::device::LamphouseBase> LamphouseHandler(const std::string& key) const;			// 获取光源控制句柄
    const std::shared_ptr<common::device::ScannerBase> ScannerHandler(const std::string& key) const;				// 获取SN扫码器句柄
    const std::shared_ptr<common::device::RangefinderBase> RangeFinderHandler(const std::string& key) const;		// 获取测距仪句柄
    const std::shared_ptr<common::device::PressurePickupBase> PressurePickupHandler(const std::string& key) const;	// 获取压力传感器句柄
    const std::shared_ptr<common::device::SoftwareScreenBase> SoftScreenHandler(const std::string& key) const;		// 获取软件打屏设备句柄
    const std::shared_ptr<common::device::ColorimeterBase> ColorimeterHandler(const std::string& key) const;		// 获取色度计控制句柄
    const std::shared_ptr<common::device::FlashDetectBase> FlashDetectHandler(const std::string& key) const;		// 获取Flash检测句柄
    // const std::shared_ptr<common::device::ImageCapture> ImageCaptureHandler(const std::string& key) const;			// 获取图像采集器句柄

    //配置平台增删管理器硬件
    void addCamera(const std::string& name, common::device::CameraDevType type);
    void removeCamera(const std::string& name);
    void addLamphouse(const std::string& name, common::device::LamphouseDevType type);
    void removeLamphouse(const std::string& name);
    void addScanner(const std::string& name, common::device::ScannerDevType type);
    void removeScanner(const std::string& name);
    void addRangefinder(const std::string& name, common::device::RangefinderDevType type);
    void removeRangefinder(const std::string& name);
    void addPressurePickup(const std::string& name, common::device::PressurePickupDevType type);
    void removePressurePickup(const std::string& name);
    void addSoftwareScreen(const std::string& name, common::device::SoftwareScreenDevType type);
    void removeSoftwareScreen(const std::string& name);
    void addMotionCard(const std::string& name, common::device::motion::MotionCardDevType type);
    void removeMotionCard(const std::string& name);
    void addAxis(const std::string& name, const std::string& cardName);
    void removeAxis(const std::string& name);
    void addDeviceIO(const std::string& name, const std::string& cardName);
    void removeDeviceIO(const std::string& name);
    void addDeviceAD(const std::string& name, const std::string& cardName);
    void removeDeviceAD(const std::string& name);
    void addColorimeter(const std::string& name, common::device::ColorimeterDevType type);
    void removeColorimeter(const std::string& name);
    void addFlashDetect(const std::string& name, common::device::FlashDetectDevType type);
    void removeFlashDetect(const std::string& name);

    //获取对应类型硬件的个数
    const std::vector<std::string>  getCameraNameList();				//获取所有相机名称列表
    const std::vector<std::string>  getLightSourceNameList();			//获取所有光源名称列表
    const std::vector<std::string>  getImageCaptureNameList();			//获取所有采集器名称列表
    const std::vector<std::string>  getScannerNameList();				//获取所有扫码器名称列表
    const std::vector<std::string>  getFlashDetectNameList();			//获取所有 Flash 检测设备名称列表
    const std::vector<std::string>  getMotionCardNameList();			//获取所有运控卡名称列表

public:
    void DeviceInfoToLog();
private:
    /// 按文件中已有键序写回 DeviceManager 段（nlohmann::ordered_json）。
    void reflectToJsonPreserveOrder(nlohmann::ordered_json& deviceManager) const;

    std::unique_ptr<common::device::CameraBase> createCameraInstance(common::device::CameraDevType type);
    std::unique_ptr<common::device::LamphouseBase> createLamphouseInstance(common::device::LamphouseDevType type);
    std::unique_ptr<common::device::ScannerBase> createScannerInstance(common::device::ScannerDevType type);
    std::unique_ptr<common::device::RangefinderBase> createRangefinderInstance(common::device::RangefinderDevType type);
    std::unique_ptr<common::device::PressurePickupBase> createPressurePickupInstance(common::device::PressurePickupDevType type);
    std::unique_ptr<common::device::SoftwareScreenBase> createSoftwareScreenInstance(common::device::SoftwareScreenDevType type);
    std::unique_ptr<common::device::ColorimeterBase> createColorimeterInstance(common::device::ColorimeterDevType type);
    std::unique_ptr<common::device::FlashDetectBase> createFlashDetectInstance(common::device::FlashDetectDevType type);

private:
    int connAllDev();
    int connMotionDev();
    int connCameraDev();
    int connLamphouseDev();
    int connScannerDev();
    int connRangefinderDev();
    int connPressurePickupDev();
    int connSoftwareScreenDev();
    int connColorimeterDev();
    int connFlashDetectDev();

private:
    std::mutex mtx_;

    std::shared_ptr<common::device::MotionController> m_motionController_;											// 运动控制
    std::unordered_map<std::string, std::shared_ptr<common::device::CameraBase>> m_cameraPool_;						// 相机设备连接池
    std::unordered_map<std::string, std::shared_ptr<common::device::LamphouseBase>> m_lamphousePool_;				// 光源控制设备连接池
    std::unordered_map<std::string, std::shared_ptr<common::device::ScannerBase>> m_scannerPool_;					// 扫码器设备连接池
    std::unordered_map<std::string, std::shared_ptr<common::device::RangefinderBase>> m_rangefinderPool_;			// 测距仪设备连接池
    std::unordered_map<std::string, std::shared_ptr<common::device::PressurePickupBase>> m_pressurePickupPool_;		// 压力传感器设备连接池
    std::unordered_map<std::string, std::shared_ptr<common::device::SoftwareScreenBase>> m_softwareScreenPool_;		// 软件打屏设备连接池
    std::unordered_map<std::string, std::shared_ptr<common::device::ColorimeterBase>> m_colorimeterPool_;			// 色度计设备
    std::unordered_map<std::string, std::shared_ptr<common::device::FlashDetectBase>> m_flashDetectPool_;		// Flash检测设备
    // std::unordered_map<std::string, std::shared_ptr<common::device::ImageCapture>> m_imageCapturePool_;				// 图像采集器

    mutable std::shared_mutex m_cameraMutex_;
    mutable std::shared_mutex m_lamphouseMutex_;
    mutable std::shared_mutex m_scannerMutex_;
    mutable std::shared_mutex m_rangefinderMutex_;
    mutable std::shared_mutex m_pressurePickupMutex_;
    mutable std::shared_mutex m_softwareScreenMutex_;
    mutable std::shared_mutex m_colorimeterMutex_;
    mutable std::shared_mutex m_flashDetectMutex_;
    mutable std::shared_mutex m_imageCaptureMutex_;

    std::string m_deviceConfigPath_;
};

} // end of namespace device

}

#endif
