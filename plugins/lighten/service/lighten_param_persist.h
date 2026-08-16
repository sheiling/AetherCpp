#pragma once

/**
 * @file lighten_param_persist.h
 * @brief 点亮插件参数持久化（设备配置读取、变更日志、配方 JSON 读写）
 */

#include <common/utility/json_tool/reflector.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <QString>

#include "recipe_model.h"
#include "system_config_model.h"
#include "virtual_pixel_model.h"

namespace common::device {
class DeviceManager;
}

namespace lighten {

/** device_config.json 中相机图像参数（DeviceManager.Camera[相机名]） */
struct CameraImageConfig
{
    int     xResolution = 0;
    int     yResolution = 0;
    QString pixelFormat;
};

/**
 * @brief 读取 exe 目录 config/device_config.json 中的设备配置（Model 层，无 UI 依赖）
 */
class DeviceConfigReader
{
public:
    /** 读取 DeviceManager.Camera 的 key 列表 */
    static std::vector<std::string> loadCameraKeys();
    /** 读取 DeviceManager.Camera 各条目的 path 字段（与 loadCameraKeys 顺序一致） */
    static std::vector<std::string> loadCameraPaths();
    /** 读取 DeviceManager.Camera：key → path */
    static std::unordered_map<std::string, std::string> loadCameraMap();
    /** 读取 DeviceManager.FlashDetect 的 key 列表 */
    static std::vector<std::string> loadFlashDetectKeys();
    /** 读取 DeviceManager.Scanner 的 key 列表 */
    static std::vector<std::string> loadScannerKeys();
    /** 读取 DeviceManager.MotionController.Card 的 key 列表 */
    static std::vector<std::string> loadMotionCardNames();
    /** 读取 DeviceManager.Camera[cameraName] 的 xResolution / yResolution / pixelFormat */
    static bool loadCameraImageConfig(const QString &cameraName, CameraImageConfig *out);
    /** 读取 DeviceManager.SoftwareScreen 的 key 列表 */
    static std::vector<std::string> loadSoftScreenKeys();
};

/** 从 IoC 容器安全解析 DeviceManager；未注册或解析失败时返回 nullptr */
std::shared_ptr<common::device::DeviceManager> tryGetDeviceManager();

/**
 * @brief 将界面保存时的参数变更追加写入 exe/parameter_change/yyyy-MM-dd.txt
 */
class ParameterChangeLogger
{
public:
    /** 记录系统配置保存前后的参数差异 */
    static void logSystemConfigSave(const SystemConfig &before, const SystemConfig &after);
    /** 记录配方文档保存前后的参数差异 */
    static void logRecipeDocumentSave(const RecipeDocument &before, const RecipeDocument &after);
    /** 记录虚拟像素文档保存前后的参数差异 */
    static void logVirtualPixelDocumentSave(const VirtualPixelDocument &before,
                                            const VirtualPixelDocument &after);

private:
    static void appendDocumentChange(const QString &pageTitle,
                                   const QString &typeName,
                                   const json &before,
                                   const json &after);
};

namespace config_dir {

/** 返回 exe 目录下 config 子目录路径 */
QString root();

} // namespace config_dir

namespace recipe_file {

/** 返回 exe 目录下 config/recipe 子目录路径 */
QString recipeDir();
/** 返回指定配方 ID 对应的 JSON 文件路径 */
QString filePath(const QString &recipeId);

/** 读取配方 JSON 根对象；文件不存在时 root 置为 {} 并返回 true */
bool readRoot(const QString &filePath, json &root);
/** 将配方 JSON 根对象写入文件 */
bool writeRoot(const QString &filePath, const json &root);

} // namespace recipe_file

} // namespace lighten
