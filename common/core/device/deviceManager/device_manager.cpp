#include <common/core/config/include/Config.h>
#include "device_manager.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <common/3rdparty/nlohmann/json.hpp>
#include <common/utility/file_tool/file_manager.hpp>
#include <Logger.h>

// 子类（驱动实现）头文件：按 BUILD_* 条件包含，与 deviceManager/CMakeLists.txt 硬件开关一致
#ifdef BUILD_CAMERA_MVS
#include <common/core/device/camera/mv_camera/mv_camera.h>
#endif
#ifdef BUILD_CAMERA_GALAXY
#include <common/core/device/camera/galaxy_camera/galaxy_camera.h>
#endif
#ifdef BUILD_CAMERA_TOUP
#include <common/core/device/camera/toup_camera/toup_camera.h>
#endif
#ifdef BUILD_CAMERA_VC
#include <common/core/device/camera/virtual_camera/virtual_camera.h>
#endif
#ifdef BUILD_DEVICE_LAMPHOUSE
#include <common/core/device/lamphouse/drivers/hdps/hdps_2c_lamphouse.h>
#include <common/core/device/lamphouse/drivers/jl_dvc/jl_dvc_lamphouse.h>
#include <common/core/device/lamphouse/drivers/lts/lts_lamphouse.h>
#include <common/core/device/lamphouse/drivers/cst/cst_dps_lamphouse.h>
#endif
#ifdef BUILD_DEVICE_SCANNER
#include <common/core/device/scanner/haikang/hk_scanner.h>
#include <common/core/device/scanner/cino/cino_scanner.h>
#endif
#ifdef BUILD_DEVICE_SOFTWARE_SCREEN
#include <common/core/device/displayControl/software_screen/winscreen/winscreen.h>
#endif
#ifdef BUILD_COLORIMETER_KONICA
#include <common/core/device/colorimeter/konicaminolta/konica_colorimeter.h>
#endif
#ifdef BUILD_DEVICE_FLASH_DETECT
#include <common/core/device/flashDetect/flashDetect_by_serial/flash_detect_by_serial.h>
#endif

#define DEVICE_FROM_JSON(deviceJSON,deviceName,memberName) \
do{\
    if (deviceJSON.contains(#deviceName) && !deviceJSON[#deviceName].is_null()){\
        const json& deviceObj = j[#deviceName];\
        for (auto it = deviceObj.begin(); it != deviceObj.end(); ++it){\
            std::string deviceKey = it.key();\
            const json& obj = it.value();\
            if (obj.contains("type") && !obj["type"].is_null()){\
                auto type_name = obj.at("type").get<common::device::##deviceName##DevType>();\
                auto ptr = create##deviceName##Instance(type_name);\
                if (ptr){\
                    ptr->reflectFromJson(obj);\
                    addDeviceModule(#deviceName, deviceKey, std::move(ptr),m_##memberName##Mutex_, m_##memberName##Pool_);\
                }\
            }\
        }\
    }\
}while(false);

template <typename T, typename MutexType>
bool addDeviceModule(const std::string & moduleName, const std::string & key, std::unique_ptr<T> device, MutexType & mutex, std::unordered_map<std::string, std::shared_ptr<T>>&map) {
    std::unique_lock<MutexType> lock(mutex);
    if (map.find(key) == map.end()) {
        map[key] = std::move(device);
        return true;
    }
    else {
        LOG_ERROR("{} failed, name={} is already existed", moduleName, key);
    }
    return false;
}

template <typename T, typename MutexType>
bool removeDeviceModule(const std::string & moduleName, const std::string & key, MutexType & mutex, std::unordered_map<std::string, std::shared_ptr<T>>&map) {
    std::unique_lock<MutexType> lock(mutex);
    auto it = map.find(key);
    if (it != map.end()) {
        map.erase(it);
        return true;
    }
    else {
        LOG_ERROR("{} failed, name={} is inexistence", moduleName, key);
    }
    return false;
}

namespace common
{
namespace device
{

namespace {

using ordered_json = nlohmann::ordered_json;

std::filesystem::path resolveDeviceConfigAbsolutePath(const std::string &configPath)
{
    std::filesystem::path path(configPath);
    if (path.is_relative()) {
        path = std::filesystem::path(common::utility::FileManager::getCurrentDir()) / path;
    }
    return path.lexically_normal();
}

/// 以 target 的键序为准，用 source 的值覆盖/追加（递归处理 object）。
void mergeOrderedPreservingKeyOrder(ordered_json &target, const json &source)
{
    if (!source.is_object()) {
        target = source;
        return;
    }
    if (!target.is_object()) {
        target = ordered_json::object();
    }
    for (auto it = target.begin(); it != target.end(); ++it) {
        const auto &key = it.key();
        if (!source.contains(key)) {
            continue;
        }
        if (it->is_object() && source[key].is_object()) {
            mergeOrderedPreservingKeyOrder(target[key], source[key]);
        } else {
            target[key] = source[key];
        }
    }
    for (auto it = source.begin(); it != source.end(); ++it) {
        if (!target.contains(it.key())) {
            target[it.key()] = it.value();
        }
    }
}

template <typename T, typename MutexType>
void writeDevicePoolSection(ordered_json &section,
                            const std::unordered_map<std::string, std::shared_ptr<T>> &pool,
                            MutexType &mutex)
{
    ordered_json merged = ordered_json::object();
    if (section.is_object()) {
        for (auto it = section.begin(); it != section.end(); ++it) {
            const std::string &key = it.key();
            std::shared_ptr<T> device;
            {
                std::shared_lock<MutexType> lock(mutex);
                const auto found = pool.find(key);
                if (found != pool.end()) {
                    device = found->second;
                }
            }
            if (!device) {
                continue;
            }
            ordered_json deviceJson = it.value();
            json fresh;
            device->reflectToJson(fresh);
            mergeOrderedPreservingKeyOrder(deviceJson, fresh);
            merged[key] = std::move(deviceJson);
        }
    }
    {
        std::shared_lock<MutexType> lock(mutex);
        for (const auto &pair : pool) {
            if (!pair.second || merged.contains(pair.first)) {
                continue;
            }
            json fresh;
            pair.second->reflectToJson(fresh);
            merged[pair.first] = std::move(fresh);
        }
    }
    section = std::move(merged);
}

} // namespace

bool DeviceManager::loadDeviceConfig(const std::string &configPath)
{
    if (configPath.empty()) {
        LOG_ERROR("[common][device_manager] device config path is empty");
        return false;
    }

    const auto absolutePath = resolveDeviceConfigAbsolutePath(configPath);
    std::ifstream ifs(absolutePath.string());
    if (!ifs) {
        LOG_ERROR("[common][device_manager] device config not found: {}", absolutePath.string());
        return false;
    }

    try {
        ordered_json root;
        ifs >> root;
        if (!root.contains("DeviceManager") || !root["DeviceManager"].is_object()) {
            LOG_ERROR("[common][device_manager] missing DeviceManager object in {}",
                      absolutePath.string());
            return false;
        }
        const json deviceManagerJson = root["DeviceManager"];
        reflectFromJson(deviceManagerJson);
        m_deviceConfigPath_ = absolutePath.string();
        LOG_INFO("[common][device_manager] config loaded: {}", m_deviceConfigPath_);
    } catch (const std::exception &e) {
        LOG_ERROR("[common][device_manager] config parse failed: {} ({})",
                  absolutePath.string(), e.what());
        return false;
    }

    return true;
}

bool DeviceManager::saveDeviceConfig() const
{
    if (m_deviceConfigPath_.empty()) {
        LOG_ERROR("[common][device_manager] device config path not set, loadDeviceConfig first");
        return false;
    }

    try {
        ordered_json root = ordered_json::object();
        if (std::ifstream ifs(m_deviceConfigPath_); ifs) {
            ifs >> root;
            if (!root.is_object()) {
                root = ordered_json::object();
            }
        }

        reflectToJsonPreserveOrder(root["DeviceManager"]);

        std::ofstream ofs(m_deviceConfigPath_);
        if (!ofs) {
            LOG_ERROR("[common][device_manager] device config save failed: {}", m_deviceConfigPath_);
            return false;
        }
        ofs << root.dump(4) << '\n';
        LOG_INFO("[common][device_manager] config saved: {}", m_deviceConfigPath_);
        return true;
    } catch (const std::exception &e) {
        LOG_ERROR("[common][device_manager] config save failed: {} ({})",
                  m_deviceConfigPath_, e.what());
        return false;
    }
}

int DeviceManager::Init() {
    this->DeviceInfoToLog();
    //连接所有设备
    return (this->connAllDev());
}

void DeviceManager::Release() 
{
#ifdef BUILD_DEVICE_FLASH_DETECT
    std::shared_lock<std::shared_mutex> lock(m_flashDetectMutex_);
    for (auto& pair : m_flashDetectPool_) {
        if (pair.second) {
            pair.second->CloseComm();
        }
    }
#endif
}

void DeviceManager::DeviceInfoToLog()
{
#if defined(BUILD_CAMERA_MVS) || defined(BUILD_CAMERA_GALAXY) || defined(BUILD_CAMERA_TOUP) || defined(BUILD_CAMERA_VC)
    {
        std::shared_lock<std::shared_mutex> lock(m_cameraMutex_);
        for (const auto &pair : m_cameraPool_) {
            if (!pair.second) {
                continue;
            }
            json cfg;
            pair.second->reflectToJson(cfg);
            LOG_INFO("[device_manager] 相机名称={} config={}", pair.first, cfg.dump());
        }
    }
#endif
#ifdef BUILD_DEVICE_LAMPHOUSE
    {
        std::shared_lock<std::shared_mutex> lock(m_lamphouseMutex_);
        for (const auto &pair : m_lamphousePool_) {
            if (!pair.second) {
                continue;
            }
            json cfg;
            pair.second->reflectToJson(cfg);
            LOG_INFO("[device_manager] Lamphouse name={} config={}", pair.first, cfg.dump());
        }
    }
#endif
#ifdef BUILD_DEVICE_SCANNER
    {
        std::shared_lock<std::shared_mutex> lock(m_scannerMutex_);
        for (const auto &pair : m_scannerPool_) {
            if (!pair.second) {
                continue;
            }
            json cfg;
            pair.second->reflectToJson(cfg);
            LOG_INFO("[device_manager] 扫码器名称={} config={}", pair.first, cfg.dump());
        }
    }
#endif
#ifdef BUILD_DEVICE_RANGEFINDER
    {
        std::shared_lock<std::shared_mutex> lock(m_rangefinderMutex_);
        for (const auto &pair : m_rangefinderPool_) {
            if (!pair.second) {
                continue;
            }
            json cfg;
            pair.second->reflectToJson(cfg);
            LOG_INFO("[device_manager] Rangefinder name={} config={}", pair.first, cfg.dump());
        }
    }
#endif
#ifdef BUILD_DEVICE_PRESSUREPICKUP
    {
        std::shared_lock<std::shared_mutex> lock(m_pressurePickupMutex_);
        for (const auto &pair : m_pressurePickupPool_) {
            if (!pair.second) {
                continue;
            }
            json cfg;
            pair.second->reflectToJson(cfg);
            LOG_INFO("[device_manager] PressurePickup name={} config={}", pair.first, cfg.dump());
        }
    }
#endif
#ifdef BUILD_DEVICE_SOFTWARE_SCREEN
    {
        std::shared_lock<std::shared_mutex> lock(m_softwareScreenMutex_);
        for (const auto &pair : m_softwareScreenPool_) {
            if (!pair.second) {
                continue;
            }
            json cfg;
            pair.second->reflectToJson(cfg);
            LOG_INFO("[device_manager] SoftwareScreen name={} config={}", pair.first, cfg.dump());
        }
    }
#endif
#ifdef BUILD_DEVICE_MOTION
    if (m_motionController_) {
        json cfg;
        m_motionController_->reflectToJson(cfg);
        LOG_INFO("[device_manager] MotionController config={}", cfg.dump());
    }
#endif
#ifdef BUILD_DEVICE_COLORIMETER
    {
        std::shared_lock<std::shared_mutex> lock(m_colorimeterMutex_);
        for (const auto &pair : m_colorimeterPool_) {
            if (!pair.second) {
                continue;
            }
            json cfg;
            pair.second->reflectToJson(cfg);
            LOG_INFO("[device_manager] Colorimeter name={} config={}", pair.first, cfg.dump());
        }
    }
#endif
#ifdef BUILD_DEVICE_FLASH_DETECT
    {
        std::shared_lock<std::shared_mutex> lock(m_flashDetectMutex_);
        for (const auto &pair : m_flashDetectPool_) {
            if (!pair.second) {
                continue;
            }
            json cfg;
            pair.second->reflectToJson(cfg);
            LOG_INFO("[device_manager] FlashDetect name={} config={}", pair.first, cfg.dump());
        }
    }
#endif
#ifdef BUILD_DEVICE_IMAGE_CAPTURE
    {
        std::shared_lock<std::shared_mutex> lock(m_imageCaptureMutex_);
        for (const auto &pair : m_imageCapturePool_) {
            if (!pair.second) {
                continue;
            }
            LOG_INFO("[device_manager] ImageCapture name={} camera={} light={}",
                     pair.first, pair.second->getCameraName(), pair.second->getLightName());
        }
    }
#endif
}

void DeviceManager::reflectFromJson(const json& j)
{
    // 未链接的设备库不调用 reflectFromJson，避免 LNK2019（头文件保留基类 API）
#ifdef BUILD_DEVICE_MOTION
    fromJson(j, "MotionController", m_motionController_);
#endif
#if defined(BUILD_CAMERA_MVS) || defined(BUILD_CAMERA_GALAXY) || defined(BUILD_CAMERA_TOUP) || defined(BUILD_CAMERA_VC)
    DEVICE_FROM_JSON(j, Camera, camera)
#endif
#ifdef BUILD_DEVICE_LAMPHOUSE
    DEVICE_FROM_JSON(j, Lamphouse, lamphouse);
#endif
#ifdef BUILD_DEVICE_SCANNER
    DEVICE_FROM_JSON(j, Scanner, scanner);
#endif
#ifdef BUILD_DEVICE_RANGEFINDER
    DEVICE_FROM_JSON(j, Rangefinder, rangefinder);
#endif
#ifdef BUILD_DEVICE_PRESSUREPICKUP
    DEVICE_FROM_JSON(j, PressurePickup, pressurePickup);
#endif
#ifdef BUILD_DEVICE_SOFTWARE_SCREEN
    DEVICE_FROM_JSON(j, SoftwareScreen, softwareScreen);
#endif
#ifdef BUILD_DEVICE_COLORIMETER
    DEVICE_FROM_JSON(j, Colorimeter, colorimeter);
#endif
#ifdef BUILD_DEVICE_FLASH_DETECT
    DEVICE_FROM_JSON(j, FlashDetect, flashDetect);
#endif

#ifdef BUILD_DEVICE_IMAGE_CAPTURE
    fromJson(j, "ImageCaptureList", m_imageCapturePool_);
    for (auto &pair : m_imageCapturePool_) {
        if (!pair.second) {
            continue;
        }
        pair.second->configure(pair.first, pair.second->getCameraName(), pair.second->getLightName());
        pair.second->setDeviceResolvers(
            [this](const std::string &name) { return CameraHandler(name); },
            [this](const std::string &name) { return LamphouseHandler(name); });
    }
#endif
}

void DeviceManager::reflectToJson(json& j) const
{
    // toJson(map) 只 upsert 键；每段先置空对象，保证与内存池一致（含已删项）。
    j["MotionController"] = json::object();
#ifdef BUILD_DEVICE_MOTION
    toJson(j["MotionController"], m_motionController_);
#endif

    j["Camera"] = json::object();
#if defined(BUILD_CAMERA_MVS) || defined(BUILD_CAMERA_GALAXY) || defined(BUILD_CAMERA_TOUP) || defined(BUILD_CAMERA_VC)
    toJson(j["Camera"], m_cameraPool_);
#endif

    j["Lamphouse"] = json::object();
#ifdef BUILD_DEVICE_LAMPHOUSE
    toJson(j["Lamphouse"], m_lamphousePool_);
#endif

    j["Scanner"] = json::object();
#ifdef BUILD_DEVICE_SCANNER
    toJson(j["Scanner"], m_scannerPool_);
#endif

    j["Rangefinder"] = json::object();
#ifdef BUILD_DEVICE_RANGEFINDER
    toJson(j["Rangefinder"], m_rangefinderPool_);
#endif

    j["PressurePickup"] = json::object();
#ifdef BUILD_DEVICE_PRESSUREPICKUP
    toJson(j["PressurePickup"], m_pressurePickupPool_);
#endif

    j["SoftwareScreen"] = json::object();
#ifdef BUILD_DEVICE_SOFTWARE_SCREEN
    toJson(j["SoftwareScreen"], m_softwareScreenPool_);
#endif

    j["Colorimeter"] = json::object();
#ifdef BUILD_DEVICE_COLORIMETER
    toJson(j["Colorimeter"], m_colorimeterPool_);
#endif

    j["FlashDetect"] = json::object();
#ifdef BUILD_DEVICE_FLASH_DETECT
    toJson(j["FlashDetect"], m_flashDetectPool_);
#endif

    j["ImageCaptureList"] = json::object();
#ifdef BUILD_DEVICE_IMAGE_CAPTURE
    toJson(j["ImageCaptureList"], m_imageCapturePool_);
#endif
}

void DeviceManager::reflectToJsonPreserveOrder(ordered_json &deviceManager) const
{
    const ordered_json existing =
        deviceManager.is_object() ? deviceManager : ordered_json::object();

    auto writeMotionController = [&](ordered_json &section) {
        if (!m_motionController_) {
            section = nullptr;
            return;
        }
        json fresh;
        m_motionController_->reflectToJson(fresh);
        if (section.is_object()) {
            mergeOrderedPreservingKeyOrder(section, fresh);
        } else {
            section = fresh;
        }
    };

    auto writeSectionByKey = [&](const std::string &key, ordered_json &section) {
        if (key == "MotionController") {
            writeMotionController(section);
        } else if (key == "Camera") {
            writeDevicePoolSection(section, m_cameraPool_, m_cameraMutex_);
        } else if (key == "Lamphouse") {
            writeDevicePoolSection(section, m_lamphousePool_, m_lamphouseMutex_);
        } else if (key == "Scanner") {
            writeDevicePoolSection(section, m_scannerPool_, m_scannerMutex_);
        } else if (key == "Rangefinder") {
            writeDevicePoolSection(section, m_rangefinderPool_, m_rangefinderMutex_);
        } else if (key == "PressurePickup") {
            writeDevicePoolSection(section, m_pressurePickupPool_, m_pressurePickupMutex_);
        } else if (key == "SoftwareScreen") {
            writeDevicePoolSection(section, m_softwareScreenPool_, m_softwareScreenMutex_);
        } else if (key == "Colorimeter") {
            writeDevicePoolSection(section, m_colorimeterPool_, m_colorimeterMutex_);
        } else if (key == "FlashDetect") {
            writeDevicePoolSection(section, m_flashDetectPool_, m_flashDetectMutex_);
        } else if (key == "ImageCaptureList") {
            writeDevicePoolSection(section, m_imageCapturePool_, m_imageCaptureMutex_);
        }
    };

    ordered_json updated = ordered_json::object();
    if (existing.is_object()) {
        for (auto it = existing.begin(); it != existing.end(); ++it) {
            ordered_json section = it.value();
            writeSectionByKey(it.key(), section);
            if (section.is_null()) {
                continue;
            }
            updated[it.key()] = std::move(section);
        }
    }

    static constexpr const char *kDefaultSectionOrder[] = {
        "MotionController", "Camera",       "Lamphouse",    "Scanner",      "Rangefinder",
        "PressurePickup",   "SoftwareScreen", "Colorimeter", "FlashDetect",  "ImageCaptureList",
    };
    for (const char *key : kDefaultSectionOrder) {
        if (updated.contains(key)) {
            continue;
        }
        ordered_json section = ordered_json::object();
        writeSectionByKey(key, section);
        if (section.is_null()) {
            continue;
        }
        if (section.is_object() && section.empty()) {
            continue;
        }
        updated[key] = std::move(section);
    }

    deviceManager = std::move(updated);
}

#pragma region 获取操作句柄
const std::shared_ptr<common::device::MotionController> DeviceManager::MotionHandler() const
{
    return m_motionController_;
}
const std::shared_ptr<common::device::CameraBase> DeviceManager::CameraHandler(const std::string& key) const {
    std::shared_lock<std::shared_mutex> lock(m_cameraMutex_);
    std::shared_ptr<common::device::CameraBase> camera = nullptr;
    auto iter = m_cameraPool_.find(key);
    if (iter != m_cameraPool_.end()) {
        camera = iter->second;
    }
    return camera;
}
const std::shared_ptr<common::device::LamphouseBase> DeviceManager::LamphouseHandler(const std::string& key) const {
    std::shared_lock<std::shared_mutex> lock(m_lamphouseMutex_);
    std::shared_ptr<common::device::LamphouseBase> lamphouse = nullptr;
    auto iter = m_lamphousePool_.find(key);
    if (iter != m_lamphousePool_.end()) {
        lamphouse = iter->second;
    }
    return lamphouse;
}
const std::shared_ptr<common::device::ScannerBase> DeviceManager::ScannerHandler(const std::string& key) const {
    std::shared_lock<std::shared_mutex> lock(m_scannerMutex_);
    std::shared_ptr<common::device::ScannerBase> scanner = nullptr;
    auto iter = m_scannerPool_.find(key);
    if (iter != m_scannerPool_.end()) {
        scanner = iter->second;
    }
    return scanner;
}
const std::shared_ptr<common::device::RangefinderBase> DeviceManager::RangeFinderHandler(const std::string& key) const {
    std::shared_lock<std::shared_mutex> lock(m_rangefinderMutex_);
    std::shared_ptr<common::device::RangefinderBase> rangefinder = nullptr;
    auto iter = m_rangefinderPool_.find(key);
    if (iter != m_rangefinderPool_.end()) {
        rangefinder = iter->second;
    }
    return rangefinder;
}
const std::shared_ptr<common::device::PressurePickupBase> DeviceManager::PressurePickupHandler(const std::string& key) const {
    std::shared_lock<std::shared_mutex> lock(m_pressurePickupMutex_);
    std::shared_ptr<common::device::PressurePickupBase> pressurePickup = nullptr;
    auto iter = m_pressurePickupPool_.find(key);
    if (iter != m_pressurePickupPool_.end()) {
        pressurePickup = iter->second;
    }
    return pressurePickup;
}
const std::shared_ptr<common::device::SoftwareScreenBase> DeviceManager::SoftScreenHandler(const std::string& key) const
{
    std::shared_lock<std::shared_mutex> lock(m_softwareScreenMutex_);
    std::shared_ptr<common::device::SoftwareScreenBase> softwareScreen = nullptr;
    auto iter = m_softwareScreenPool_.find(key);
    if (iter != m_softwareScreenPool_.end()) {
        softwareScreen = iter->second;
    }
    return softwareScreen;
}
const std::shared_ptr<common::device::ColorimeterBase> DeviceManager::ColorimeterHandler(const std::string& key) const
{
    std::shared_lock<std::shared_mutex> lock(m_colorimeterMutex_);
    std::shared_ptr<common::device::ColorimeterBase> colorimeter = nullptr;
    auto iter = m_colorimeterPool_.find(key);
    if (iter != m_colorimeterPool_.end()) {
        colorimeter = iter->second;
    }
    return colorimeter;
}
const std::shared_ptr<common::device::FlashDetectBase> DeviceManager::FlashDetectHandler(const std::string& key) const
{
    std::shared_lock<std::shared_mutex> lock(m_flashDetectMutex_);
    std::shared_ptr<common::device::FlashDetectBase> flashDetect = nullptr;
    auto iter = m_flashDetectPool_.find(key);
    if (iter != m_flashDetectPool_.end()) {
        flashDetect = iter->second;
    }
    return flashDetect;
}
const std::shared_ptr<common::device::ImageCapture> DeviceManager::ImageCaptureHandler(const std::string &key) const
{
    std::shared_lock<std::shared_mutex> lock(m_imageCaptureMutex_);
    std::shared_ptr<common::device::ImageCapture> imageCapture = nullptr;
    auto                                          iter         = m_imageCapturePool_.find(key);
    if (iter != m_imageCapturePool_.end()) {
        imageCapture = iter->second;
    }
    return imageCapture;
}
#pragma endregion 获取操作句柄
 

#pragma region 设备添加与删除
void DeviceManager::addCamera(const std::string& name, common::device::CameraDevType type)
{
    auto ptr = createCameraInstance(type); 
    if (ptr) 
        addDeviceModule("Camera", name, std::move(ptr), m_cameraMutex_, m_cameraPool_); 
}
void DeviceManager::removeCamera(const std::string& name)
{
    removeDeviceModule("Camera", name, m_cameraMutex_, m_cameraPool_);
}
void DeviceManager::addLamphouse(const std::string& name, common::device::LamphouseDevType type)
{
    auto ptr = createLamphouseInstance(type);
    if (ptr)
        addDeviceModule("Lamphouse", name, std::move(ptr), m_lamphouseMutex_, m_lamphousePool_);
}
void DeviceManager::removeLamphouse(const std::string& name)
{
    removeDeviceModule("Lamphouse", name, m_lamphouseMutex_, m_lamphousePool_);
}
void DeviceManager::addScanner(const std::string& name, common::device::ScannerDevType type)
{
    auto ptr = createScannerInstance(type);
    if (ptr)
        addDeviceModule("Scanner", name, std::move(ptr), m_scannerMutex_, m_scannerPool_);
}
void DeviceManager::removeScanner(const std::string& name)
{
    removeDeviceModule("Scanner", name, m_scannerMutex_, m_scannerPool_);
}
void DeviceManager::addRangefinder(const std::string& name, common::device::RangefinderDevType type)
{
    auto ptr = createRangefinderInstance(type);
    if (ptr)
        addDeviceModule("Rangefinder", name, std::move(ptr), m_rangefinderMutex_, m_rangefinderPool_);
}
void DeviceManager::removeRangefinder(const std::string& name)
{
    removeDeviceModule("Rangefinder", name, m_rangefinderMutex_, m_rangefinderPool_);
}
void DeviceManager::addPressurePickup(const std::string& name, common::device::PressurePickupDevType type)
{
    auto ptr = createPressurePickupInstance(type);
    if (ptr)
        addDeviceModule("PressurePickup", name, std::move(ptr), m_pressurePickupMutex_, m_pressurePickupPool_);
}
void DeviceManager::removePressurePickup(const std::string& name)
{
    removeDeviceModule("PressurePickup", name, m_pressurePickupMutex_, m_pressurePickupPool_);
}
void DeviceManager::addSoftwareScreen(const std::string& name, common::device::SoftwareScreenDevType type)
{
    auto ptr = createSoftwareScreenInstance(type);
    if (ptr)
        addDeviceModule("SoftwareScreen", name, std::move(ptr), m_softwareScreenMutex_, m_softwareScreenPool_);
}
void DeviceManager::removeSoftwareScreen(const std::string& name)
{
    removeDeviceModule("SoftwareScreen", name, m_softwareScreenMutex_, m_softwareScreenPool_);
}

void DeviceManager::addMotionCard(const std::string& name, common::device::motion::MotionCardDevType type)
{
    if (m_motionController_)
        m_motionController_->addDevice("Card", name, type);
}
void DeviceManager::removeMotionCard(const std::string& name)
{
    if (m_motionController_)
        m_motionController_->removeDevice("Card", name);
}
void DeviceManager::addAxis(const std::string& name, const std::string& cardName)
{
    if (m_motionController_)
        m_motionController_->addDevice("Axis", name, common::device::motion::MotionCardDevType::LeiShineDmc, cardName);
}
void DeviceManager::removeAxis(const std::string& name)
{
    if (m_motionController_)
        m_motionController_->removeDevice("Axis", name);
}
void DeviceManager::addDeviceIO(const std::string& name, const std::string& cardName)
{
    if (m_motionController_)
        m_motionController_->addDevice("DeviceIO", name, common::device::motion::MotionCardDevType::LeiShineDmc, cardName);
}
void DeviceManager::removeDeviceIO(const std::string& name)
{
    if (m_motionController_)
        m_motionController_->removeDevice("DeviceIO", name);
}
void DeviceManager::addDeviceAD(const std::string& name, const std::string& cardName)
{
    if (m_motionController_)
        m_motionController_->addDevice("DeviceAD", name, common::device::motion::MotionCardDevType::LeiShineDmc, cardName);
}
void DeviceManager::removeDeviceAD(const std::string& name)
{
    if (m_motionController_)
        m_motionController_->removeDevice("DeviceAD", name);
}
void DeviceManager::addColorimeter(const std::string& name, common::device::ColorimeterDevType type)
{
    auto ptr = createColorimeterInstance(type);
    if (ptr)
        addDeviceModule("Colorimeter", name, std::move(ptr), m_colorimeterMutex_, m_colorimeterPool_);
}
void DeviceManager::removeColorimeter(const std::string& name)
{
    removeDeviceModule("Colorimeter", name, m_colorimeterMutex_, m_colorimeterPool_);
}
void DeviceManager::addFlashDetect(const std::string& name, common::device::FlashDetectDevType type)
{
    auto ptr = createFlashDetectInstance(type);
    if (ptr)
        addDeviceModule("FlashDetect", name, std::move(ptr), m_flashDetectMutex_, m_flashDetectPool_);
}
void DeviceManager::removeFlashDetect(const std::string& name)
{
    removeDeviceModule("FlashDetect", name, m_flashDetectMutex_, m_flashDetectPool_);
}

#pragma endregion 设备添加与删除


const std::vector<std::string>  DeviceManager::getCameraNameList()
{
    std::vector<std::string> vec;
    std::shared_lock<std::shared_mutex> lock(m_cameraMutex_);
    for (const auto &[key, val] : m_cameraPool_)
    {
        vec.push_back(key);

    }
    return vec;
}

const std::vector<std::string>  DeviceManager::getLightSourceNameList()
{
    std::vector<std::string> vec;
    std::shared_lock<std::shared_mutex> lock(m_lamphouseMutex_);
    for (const auto &[key, val] : m_lamphousePool_)
    {
        vec.push_back(key);

    }
    return vec;
}

const std::vector<std::string> DeviceManager::getImageCaptureNameList()
{
    std::vector<std::string>            vec;
    std::shared_lock<std::shared_mutex> lock(m_imageCaptureMutex_);
    for (const auto &[key, val] : m_imageCapturePool_)
    {
        vec.push_back(key);
    }
    return vec;
}

const std::vector<std::string> DeviceManager::getScannerNameList()
{
    std::vector<std::string>            vec;
    std::shared_lock<std::shared_mutex> lock(m_scannerMutex_);
    for (const auto &[key, val] : m_scannerPool_)
    {
        vec.push_back(key);
    }
    return vec;
}

const std::vector<std::string> DeviceManager::getFlashDetectNameList()
{
    std::vector<std::string>            vec;
    std::shared_lock<std::shared_mutex> lock(m_flashDetectMutex_);
    for (const auto &[key, val] : m_flashDetectPool_)
    {
        vec.push_back(key);
    }
    return vec;
}

const std::vector<std::string> DeviceManager::getMotionCardNameList()
{
    const auto motion = MotionHandler();
    if (!motion)
        return {};
    return motion->getAllCardName();
}


#pragma region 设备创建

std::unique_ptr<common::device::CameraBase> DeviceManager::createCameraInstance(common::device::CameraDevType type)
{
    if (CFG_GET("virtual_debug.virtual_camera_enable", bool, false)) {
        type = common::device::CameraDevType::CDN_VC;
    }

    std::unique_ptr<common::device::CameraBase> camera = nullptr;
    switch (type)
    {
#ifdef BUILD_CAMERA_VC
    case common::device::CDN_VC: {
        camera = std::make_unique<common::device::VirtualCamera>();
        break;
    }
#endif
#ifdef BUILD_CAMERA_GALAXY
        case common::device::CDN_GALAXY: {
            camera = std::make_unique<common::device::GalaxyCamera>();
            break;
        }
#endif
#ifdef BUILD_CAMERA_MVS
        case common::device::CDN_MVS: {
            camera = std::make_unique<common::device::MvCamera>();
            break;
        }
#endif
#ifdef BUILD_CAMERA_TOUP
        case common::device::CDN_TOUP: {
            camera = std::make_unique<common::device::ToupCamera>();
            break;
        }
#endif
        case common::device::CDN_UNKNOWN:
        default:
            break;
    }
    return camera;
}
std::unique_ptr<common::device::LamphouseBase> DeviceManager::createLamphouseInstance(common::device::LamphouseDevType type)
{
    std::unique_ptr<common::device::LamphouseBase> lamphouse = nullptr;
    switch (type)
    {
#ifdef BUILD_DEVICE_LAMPHOUSE
    case common::device::LamphouseDevType::LDN_CST_DPS:     lamphouse = std::make_unique<common::device::CstDpsLamphouse>(); break;
    case common::device::LamphouseDevType::LDN_HDPS_2C:     lamphouse = std::make_unique<common::device::Hdps2cLamphouse>(); break;
    case common::device::LamphouseDevType::LDN_JL_DVC:      lamphouse = std::make_unique<common::device::JlDvcLamphouse>(); break;
    case common::device::LamphouseDevType::LDN_LTS:         lamphouse = std::make_unique<common::device::LtsLamphouse>(); break;
#endif
    case common::device::LamphouseDevType::LDN_UNKNOWN:break;
    default: break;
    }
    return lamphouse;
}
std::unique_ptr<common::device::ScannerBase> DeviceManager::createScannerInstance(common::device::ScannerDevType type)
{
    std::unique_ptr<common::device::ScannerBase> scanner = nullptr;
    switch (type)
    {
#ifdef BUILD_DEVICE_SCANNER
    case common::device::ScannerDevType::SSDN_HAIKANG: scanner = std::make_unique<common::device::HKScanner>(); break;
    case common::device::ScannerDevType::SSDN_CINO: scanner = std::make_unique<common::device::CINOScanner>(); break;
#endif
    case common::device::ScannerDevType::SSDN_UNKNOWN:break;
    default: break;
    }
    return scanner;
}
std::unique_ptr<common::device::RangefinderBase> DeviceManager::createRangefinderInstance(common::device::RangefinderDevType type)
{
    std::unique_ptr<common::device::RangefinderBase> rangeFinder = nullptr;
    switch (type)
    {
#ifdef BUILD_DEVICE_RANGEFINDER
    case common::device::RangefinderDevType::RDN_UNKNOWN: rangeFinder = std::make_unique<common::device::RangefinderBase>(); break;
#endif
    default: break;
    }
    return rangeFinder;
}
std::unique_ptr<common::device::PressurePickupBase> DeviceManager::createPressurePickupInstance(common::device::PressurePickupDevType type)
{
    std::unique_ptr<common::device::PressurePickupBase> pressurePickup = nullptr;
    switch (type)
    {
#ifdef BUILD_DEVICE_PRESSUREPICKUP
    case common::device::PressurePickupDevType::PPDN_UNKNOWN:
        pressurePickup = std::make_unique<common::device::PressurePickupBase>();
        break;
#endif
    default: break;
    }
    return pressurePickup;
}
std::unique_ptr<common::device::SoftwareScreenBase> DeviceManager::createSoftwareScreenInstance(common::device::SoftwareScreenDevType type)
{
    std::unique_ptr<common::device::SoftwareScreenBase> softwareScreen = nullptr;
    switch (type)
    {
#ifdef BUILD_DEVICE_SOFTWARE_SCREEN
    case common::device::SoftwareScreenDevType::SWSDN_WIN32: softwareScreen = std::make_unique<common::device::WinScreen>(); break;
#endif
    default: break;
    }
    return softwareScreen;
}
std::unique_ptr<common::device::ColorimeterBase> DeviceManager::createColorimeterInstance(common::device::ColorimeterDevType type)
{
     std::unique_ptr<common::device::ColorimeterBase> colorimeter = nullptr;
    switch (type) 
    {
#ifdef BUILD_COLORIMETER_KONICA
        case common::device::ColorimeterDevType::KONICA_MINOLTA:    colorimeter = std::make_unique<common::device::KonicaColorimeter>(); break;
#endif
    default:
        break;
    }
    return colorimeter;
}

std::unique_ptr<common::device::FlashDetectBase> DeviceManager::createFlashDetectInstance(common::device::FlashDetectDevType type)
{
    std::unique_ptr<common::device::FlashDetectBase> flashDetect = nullptr;
    switch (type)
    {
#ifdef BUILD_DEVICE_FLASH_DETECT
    case common::device::FlashDetectDevType::FD_BY_SERIAL:
        flashDetect = std::make_unique<common::device::FlashDetectBySerial>();
        break;
#endif
    case common::device::FlashDetectDevType::FD_UNKNOWN:
    default:
        break;
    }
    return flashDetect;
}

#pragma endregion 设备创建


#pragma region 设备连接
int DeviceManager::connAllDev() {
    int ret = this->connMotionDev();
    if (ret != 0) {return ret;}
    ret = this->connCameraDev();
    if (ret != 0) {return ret;}
    ret = this->connLamphouseDev();
    if (ret != 0) {return ret;}
    ret = this->connScannerDev();
    if (ret != 0) {return ret;}
    ret = this->connRangefinderDev();
    if (ret != 0) {return ret;}
    ret = this->connPressurePickupDev();
    if (ret != 0) {return ret;}
    ret = this->connSoftwareScreenDev();
    if (ret != 0) {return ret;}
    ret = this->connColorimeterDev();
    if (ret != 0) { return ret; }
    ret = this->connFlashDetectDev();
    if (ret != 0) { return ret; }
    return 0;
}
int DeviceManager::connMotionDev() {
#ifdef BUILD_DEVICE_MOTION
    if (m_motionController_)
        m_motionController_->InitDevice();
#endif
    return 0;
}
int DeviceManager::connCameraDev() {
#if defined(BUILD_CAMERA_MVS) || defined(BUILD_CAMERA_GALAXY) || defined(BUILD_CAMERA_TOUP) || defined(BUILD_CAMERA_VC)
    for (auto iter = m_cameraPool_.begin(); iter != m_cameraPool_.end(); ++iter) {
        auto err = iter->second->Open();
        if (err) {
            LOG_ERROR("[common][device_manager]--相机初始化失败，key={}", iter->first);
            continue;
        }
        const auto &info = iter->second->GetCameraInfo();
        LOG_INFO("[common][device_manager]--相机连接成功，key={} model={} sn={} size={}x{} "
                 "pixelFormat={} imageFormat={} pixelFormatType={} bitDepth={} isColor={} mac={}",
                 iter->first, info.cameraModelName, info.cameraID, info.imageWidth,
                 info.imageHeight, info.pixelFormat,
                 json(info.imageFormat).get<std::string>(),
                 json(info.pixelFormatType).get<std::string>(),
                 info.bitDepth, info.isSupportColor, info.localMac);
    }
#endif
    return 0;
}
int DeviceManager::connLamphouseDev() {
#ifdef BUILD_DEVICE_LAMPHOUSE
    for (auto iter = m_lamphousePool_.begin(); iter != m_lamphousePool_.end(); ++iter) {
        bool bret = iter->second->Init();
        if (!bret) {
            LOG_ERROR("[common][device_manager]--光源初始化失败，key={}", iter->first);
            continue;
        }
        else {
            continue;
        }
    }
#endif
    return 0;
}
int DeviceManager::connScannerDev() {
#ifdef BUILD_DEVICE_SCANNER
    for (auto iter = m_scannerPool_.begin(); iter != m_scannerPool_.end(); ++iter) {
        bool bret = iter->second->Init();
        if (!bret) {
            LOG_ERROR("[common][device_manager]--扫码器初始化失败，key={}", iter->first);
            continue;
        }
        else {
            continue;
        }
    }
#endif
    return 0;
}
int DeviceManager::connRangefinderDev() {
#ifdef BUILD_DEVICE_RANGEFINDER
    for (auto iter = m_rangefinderPool_.begin(); iter != m_rangefinderPool_.end(); ++iter) {
        bool bret = iter->second->Init();
        if (!bret) {
            LOG_ERROR("[common][device_manager]--Rangefinder Init failed, key={}", iter->first);
            continue;
        }
        else {
            continue;
        }
    }
#endif
    return 0;
}
int DeviceManager::connPressurePickupDev() {
#ifdef BUILD_DEVICE_PRESSUREPICKUP
    for (auto iter = m_pressurePickupPool_.begin(); iter != m_pressurePickupPool_.end(); ++iter) {
        bool bret = iter->second->Init();
        if (!bret) {
            LOG_ERROR("[common][device_manager]--PressurePickup Init failed, key={}", iter->first);
            continue;
        }
        else {
            continue;
        }
    }
#endif
    return 0;
}
int DeviceManager::connSoftwareScreenDev()
{
#ifdef BUILD_DEVICE_SOFTWARE_SCREEN
    for (auto iter = m_softwareScreenPool_.begin(); iter != m_softwareScreenPool_.end(); ++iter) {
        bool bret = iter->second->Init();
        if (!bret) {
            LOG_ERROR("[common][device_manager]--SoftwareScreen Init failed, key={}", iter->first);
            continue;
        }
        else {
            continue;
        }
    }
#endif
    return 0;
}
int DeviceManager::connColorimeterDev()
{
#ifdef BUILD_DEVICE_COLORIMETER
    for (auto iter = m_colorimeterPool_.begin(); iter != m_colorimeterPool_.end(); ++iter) {
        bool bret = iter->second->init();
        if (!bret) {
            LOG_ERROR("[common][device_manager]-Colorimeter Init failed, key={}", iter->first);
            continue;
        }
        else {
            continue;
        }
    }
#endif
    return 0;
}
int DeviceManager::connFlashDetectDev()
{
#ifdef BUILD_DEVICE_FLASH_DETECT
    for (auto iter = m_flashDetectPool_.begin(); iter != m_flashDetectPool_.end(); ++iter) {
        int ret = iter->second->OpenComm();
        if (ret != 0) {
            LOG_ERROR("[common][device_manager]-FlashDetect OpenComm failed, key={}", iter->first);
            continue;
        }
        iter->second->CloseComm();
    }
#endif
    return 0;
}

#pragma endregion 设备连接

} // end of namespace device

}
