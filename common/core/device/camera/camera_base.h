#ifndef _CAMERA_BASE_HH
#define _CAMERA_BASE_HH

#include <stdint.h>
#include <atomic>
#include <string>
#include <functional>
#include <exception>
#include <sstream>
#include <type_traits>
#include <common/utility/json_tool/reflector.h>
#include <common/base/imgproc/image_type_map.h>
#include <Logger.h>
#include <common/utility/errors/include/error.h>
#include "camera_error_codes.h"
#include "camera_types.h"
#include "camera_feature_arg.h"

namespace common
{
namespace device    
{

using ImageGrabFuncCB = std::function<void(int height,
                                           int width,
                                           unsigned char *dataBuffer,
                                           int dataLen,
                                           common::imgproc::ImageFormat imageFormat)>;
using CameraOnlineFuncCB = std::function<void(std::string msg)>;
using CameraOfflineFuncCB = std::function<void(std::string msg)>;

using namespace common::errors;

// 相机采集设备型号定义
enum CameraDevType {
    CDN_GALAXY = 0,             // 大恒
    CDN_MVS = 1,                // 海康---MVS SDK
    CDN_DALSA = 2,              // 达尔萨---线扫相机
    CDN_COAXLINK = 3,           // Euresys的COAXLINK采集卡
    CDN_TOUP = 4,               // 图谱相机
    CDN_PCO = 5,                // PCO相机
    CDN_VC = 6,                 // 虚拟相机
    CDN_UNKNOWN                 // 未知型号 
};
NLOHMANN_JSON_SERIALIZE_ENUM(CameraDevType, {
{ CameraDevType::CDN_GALAXY,    "CDN_GALAXY" },
{ CameraDevType::CDN_MVS,       "CDN_MVS" },
{ CameraDevType::CDN_DALSA,     "CDN_DALSA" },
{ CameraDevType::CDN_COAXLINK,  "CDN_COAXLINK" },
{ CameraDevType::CDN_TOUP,      "CDN_TOUP" },
{ CameraDevType::CDN_PCO,       "CDN_PCO" },
{ CameraDevType::CDN_VC,        "CDN_VC" },
{ CameraDevType::CDN_UNKNOWN,   "CDN_UNKNOWN" }
    });

enum CameraConnType {
    CONN_BY_SN = 0,             
    CONN_BY_IP = 1,             
    CONN_BY_INDEX = 2,           
    CONN_BY_CONFIGFILE = 3,             
};
NLOHMANN_JSON_SERIALIZE_ENUM(CameraConnType, {
{ CameraConnType::CONN_BY_SN,           "CONN_BY_SN" },
{ CameraConnType::CONN_BY_IP,           "CONN_BY_IP" },
{ CameraConnType::CONN_BY_INDEX,        "CONN_BY_INDEX" },
{ CameraConnType::CONN_BY_CONFIGFILE,   "CONN_BY_CONFIGFILE" }
});

// 采集相机设备信息
struct CameraDevInfo {
    uint16_t devNO = 0;                             // 设备编号
    std::string ip = "";                            // IP地址
    uint16_t port = 0;                              // 端口
    std::string SN = "";                            // 相机SN
    uint16_t index = 0;                             // 相机索引
    std::string configFile = "";                    // 配置文件路径
    CameraConnType connType;                        // 连接类型
    void reflectFromJson(const json& j)
    {
        getFromJson(j, "devNO", devNO);
        getFromJson(j, "ip", ip);
        getFromJson(j, "port", port);
        getFromJson(j, "SN", SN);
        getFromJson(j, "index", index);
        getFromJson(j, "configFile", configFile);
        getFromJson(j, "connType", connType);
    }
    void reflectToJson(json& j)const
    {
        j["devNO"] = devNO;
        j["ip"] = ip;
        j["port"] = port;
        j["SN"] = SN;
        j["index"] = index;
        j["configFile"] = configFile;
        j["connType"] = connType;
    }
};

struct CameraInfo
{
    std::string cameraModelName = "";           // 相机型号（DeviceModelName）
    std::string cameraID = "";                  // 相机SN
    common::imgproc::ImageFormat imageFormat = common::imgproc::ImageFormat::UNKNOWN; // 图像格式
    std::string pixelFormat = "";               // 像素格式（如 BayerRG8、Mono8）
    PixelFormatType pixelFormatType = PixelFormatType::Unknown; // 像素排列类型
    int imageWidth = 0;                         // 图像宽度
    int imageHeight = 0;                        // 图像高度
    int bitDepth = 8;                           // 位深
    bool isSupportColor = false;                // 是否彩色（Open 后由 SDK 填充）
    std::string localMac;                       // 相机mac地址
};

enum class CaptureState : uint8_t {
    Idle      = 0,  // 空闲（未预览，可单次拍照）
    Preview   = 1,  // 实时预览中（AsyncGrab）
    Capturing = 2,  // 单次采图中（SyncGrab 阻塞期间，瞬态）
};

/**
 * @brief 相机抽象基类（Galaxy / Mv / Toup 等 SDK 驱动的统一入口）
 *
 * ## 四个采图接口（API 自管触发，不恢复进入前状态）
 *
 * | 接口 | 行为 | 内部触发 |
 * |------|------|----------|
 * | SyncGrab | 同步阻塞取单帧 | 自动 **关闭** m_triggerSwitch_ |
 * | AsyncGrab | 连续异步回调出图 | 自动 **关闭** m_triggerSwitch_ |
 * | TriggerCapture(source) | 触发单帧，经 GrabCB 出图 | 自动 **开启** 并设置 m_triggerSourceType_ |
 * | StopGrab | 停止 AsyncGrab / TriggerCapture 拉流 | 不改变触发状态 |
 *
 * 上层无需事先 SetCameraFeature(TRIGGER_ENABLE)；SetCameraFeature 仍可持久配置，但会被采图 API 覆盖。
 *
 * ## 线程与并发约定（三子类共同遵守）
 *
 * - **推荐**：同一 Camera 实例由单线程串行调用 Open/Close/采图/StopGrab；ImageCapture 层符合此假设。
 * - **mtx_**：保护设备生命周期、采图 API 入口、SDK 参数写入（set/getCameraFeatureImpl）。
 * - **condmtx_ / CaptureEventHandler::mtx_**：仅保护帧到达与 SyncGrab 条件变量，不与 mtx_ 嵌套持有。
 * - **syncGrabInProgress_**：SyncGrab 全程为 true；此期间 StopGrab 返回 kAlreadyStarted，Close 用 forceStop 打断。
 * - **asyncFlag_（atomic）**：SDK 回调线程与主线程分界——false 走 Sync 拷贝+notify，true 走 GrabCB（拷贝后回调）。
 * - **SyncGrab 返回值**：dataBuffer 为内部借用指针，须立即拷贝；不可跨线程/跨下次采图持有。
 * - **AsyncGrab/TriggerCapture 回调**：buffer 已为驱动内拷贝，回调返回后可用。
 *
 * ## 典型调用链
 * - Open → refreshCameraInfo → RegisterGrabCB
 * - 调参：SetExposureTime / SetCameraFeature
 * - 采图：SyncGrab | AsyncGrab | TriggerCapture(source) | StopGrab
 */
class CameraBase {
public:
    CameraBase() = default;
    virtual ~CameraBase() = default;
    virtual void reflectFromJson(const json& j)
    {
        fromJson(j, "connOpts", m_cameraDevInfo_);
        getFromJson(j, "type", m_typeName);
    }
    virtual void reflectToJson(json& j)const
    {
        toJson(j["connOpts"], m_cameraDevInfo_);
        j["type"] = m_typeName;
    }
public:
    // --- 设备生命周期与状态 ---
    virtual ErrorPtr Open() = 0;
    virtual ErrorPtr Close() = 0;
    virtual bool IsConnected() = 0;
    virtual ErrorPtr ReConnect() { return nullptr; }
    CaptureState GetCaptureState() const { return captureState_.load(); }

    // --- 回调注册 ---
    virtual void RegisterGrabCB(ImageGrabFuncCB imageGrabFuncCB) = 0;
    virtual void RegisterOnlineCB(CameraOnlineFuncCB cameraOnlineFuncCB) {}
    virtual void RegisterOfflineCB(CameraOfflineFuncCB cameraOfflineFuncCB) = 0;

    // --- 采图 ---
    /**
     * 同步单帧；内部自动关闭触发（自由采集）。
     * @note dataBuffer 为驱动内部缓冲区借用指针，返回后须立即拷贝或同步消费；
     *       下次 SyncGrab / Close 可能失效，不可跨线程异步持有。
     *       SyncGrab 阻塞期间 StopGrab 将返回 kAlreadyStarted（Close 除外）。
     */
    virtual ErrorPtr SyncGrab(int &height, int &width, unsigned char* &dataBuffer, int &dataLen) { return nullptr; }
    /** 异步连续采图；内部自动关闭触发（自由采集） */
    virtual ErrorPtr AsyncGrab() { return nullptr; }
    /** 触发单帧；内部自动开启触发并设置 source，经 GrabCB 出图 */
    virtual ErrorPtr TriggerCapture(
        CameraTriggerSourceType source = CameraTriggerSourceType::SOFTWARE_TRIGGER)
    {
        (void)source;
        return nullptr;
    }
    /** 停止 AsyncGrab / TriggerCapture 拉流 */
    virtual ErrorPtr StopGrab() { return nullptr; }

    // --- Feature / 参数 ---
    template<typename T>
    ErrorPtr SetCameraFeature(CameraFeatureType featureType, T value);

    template<typename T>
    ErrorPtr GetCameraFeature(CameraFeatureType featureType, T &value);

    ErrorPtr SetExposureTime(int exposureTime) {
        return SetCameraFeature(CameraFeatureType::CAMERA_EXPOSURE_TIME, exposureTime);
    }
    ErrorPtr SetExposureTime(double exposureTime) {
        return SetCameraFeature(CameraFeatureType::CAMERA_EXPOSURE_TIME, exposureTime);
    }

    CameraInfo &GetCameraInfo() { return m_cameraInfo_; }
    const CameraInfo &GetCameraInfo() const { return m_cameraInfo_; }

    /** 当前 SDK 触发开关（由采图 API 或 SetCameraFeature 写入） */
    bool IsTriggerEnabled() const { return m_triggerSwitch_; }
    CameraTriggerSourceType GetTriggerSource() const { return m_triggerSourceType_; }

    virtual int errCode() { return errCode_; }
    virtual std::string errMsg() { return errMsg_; }
protected:
    void SetCaptureState(CaptureState state);
    void SetConnectedState(bool connected);
    virtual void refreshCameraInfo() {}

    /**
     * 处理 CAMERA_TRIGGER_ENABLE / CAMERA_TRIGGER_SOURCE 写入。
     * @return true 表示 feature 已被基类消费，err 为 SDK 应用结果
     */
    bool trySetTriggerFeature(CameraFeatureType featureType, const CameraFeatureArg &arg, ErrorPtr &err);
    /** @return true 表示 feature 已被基类填充 */
    bool tryGetTriggerFeature(CameraFeatureType featureType, CameraFeatureArg &arg) const;

    /** 子类在开关/触发源变更后同步到 SDK（设备未 Open 时可 no-op） */
    virtual ErrorPtr onTriggerSwitchChanged() { return Ok(); }
    virtual ErrorPtr onTriggerSourceChanged() { return Ok(); }

    /** SyncGrab/AsyncGrab 入口：关闭触发并同步 SDK */
    ErrorPtr ensureFreeRunMode();
    /** TriggerCapture 入口：开启触发、设置 source 并同步 SDK */
    ErrorPtr ensureTriggerMode(CameraTriggerSourceType source);

    virtual ErrorPtr setCameraFeatureImpl(CameraFeatureType featureType, const CameraFeatureArg &arg)
    {
        (void)featureType;
        (void)arg;
        return Err("feature not supported", common::errors::camera_module::kInvalidConfig);
    }
    virtual ErrorPtr getCameraFeatureImpl(CameraFeatureType featureType, CameraFeatureArg &arg)
    {
        (void)featureType;
        (void)arg;
        return Err("feature get not supported", common::errors::camera_module::kInvalidConfig);
    }

    virtual int CameraFilterWheelType2Int(CameraFilterWheelType type) const { return (int)type; };
    
    // SFINAE 辅助：检测类型是否有 GetErrorCode() 方法
    template<typename T>
    class has_GetErrorCode {
    private:
        template<typename U>
        static auto test(int) -> decltype(std::declval<U>().GetErrorCode(), std::true_type{});
        template<typename>
        static std::false_type test(...);
    public:
        static constexpr bool value = decltype(test<T>(0))::value;
    };
    // 错误处理 - 有 GetErrorCode() 的异常（如 CGalaxyException）；由 HandleException 宏注入调用点位置
    template<typename ExceptionType>
    inline typename std::enable_if<has_GetErrorCode<ExceptionType>::value, void>::type
    HandleExceptionAt(spdlog::source_loc loc, const std::string& customMsg, const ExceptionType& e)
    {
        int errorCode = const_cast<ExceptionType&>(e).GetErrorCode();
        const char* errorMsgPtr = e.what();
        std::string errorMsg = errorMsgPtr ? errorMsgPtr : "";
        errCode_ = errorCode;
        errMsg_ = errorMsg;
        logErrorAt(loc, customMsg, errorCode, errorMsg);
    }
    template<typename ExceptionType>
    inline typename std::enable_if<has_GetErrorCode<ExceptionType>::value, void>::type
    HandleExceptionAt(spdlog::source_loc loc, const std::string& customMsg, int errorCode, const ExceptionType& e)
    {
        const char* errorMsgPtr = e.what();
        std::string errorMsg = errorMsgPtr ? errorMsgPtr : "";
        errCode_ = errorCode;
        errMsg_ = errorMsg;
        logErrorAt(loc, customMsg, errorCode, errorMsg);
    }
    // 错误处理 - 标准异常（仅 what()）；由 HandleException 宏注入调用点位置
    template<typename ExceptionType>
    inline typename std::enable_if<!has_GetErrorCode<ExceptionType>::value &&
                                   std::is_base_of<std::exception, ExceptionType>::value, void>::type
    HandleExceptionAt(spdlog::source_loc loc, const std::string& customMsg, const ExceptionType& e)
    {
        const char* errorMsgPtr = e.what();
        std::string errorMsg = errorMsgPtr ? errorMsgPtr : "";
        errCode_ = common::errors::common_module::kUnknown;
        errMsg_ = errorMsg;
        logErrorAt(loc, customMsg, common::errors::common_module::kUnknown, errorMsg);
    }
    template<typename ExceptionType>
    inline typename std::enable_if<!has_GetErrorCode<ExceptionType>::value &&
                                   std::is_base_of<std::exception, ExceptionType>::value, void>::type
    HandleExceptionAt(spdlog::source_loc loc, const std::string& customMsg, int errorCode, const ExceptionType& e)
    {
        const char* errorMsgPtr = e.what();
        std::string errorMsg = errorMsgPtr ? errorMsgPtr : "";
        errCode_ = errorCode;
        errMsg_ = errorMsg;
        logErrorAt(loc, customMsg, errorCode, errorMsg);
    }
    // 内部辅助：组装错误日志正文（不含源码位置）
    std::string formatErrorLog(const std::string& customMsg, int errorCode, const std::string& errorMsg) const;
    void logErrorAt(spdlog::source_loc loc, const std::string& customMsg, int errorCode, const std::string& errorMsg);
    void LogError(const std::string& customMsg, int errorCode, const std::string& errorMsg);
    void HandleErrorAt(spdlog::source_loc loc, const std::string& customMsg, int errorCode = -1, const std::string& errorMsg = "");
    
    // ErrorPtr 辅助函数
    static common::errors::ErrorPtr Ok() { return nullptr; }
    static common::errors::ErrorPtr Err(std::string msg, int code = common::errors::common_module::kUnknown) {
        return common::errors::New(code, std::move(msg), ERR_SOURCE_LOC);
    }
    template<typename... Args>
    static common::errors::ErrorPtr Errf(int code, const std::string& fmt, Args&&... args) {
        try {
            return common::errors::Newf(code, fmt.c_str(), std::forward<Args>(args)...);
        } catch (const std::exception& e) {
            // 如果格式化失败，返回原始错误消息和格式化异常
            std::string fallbackMsg = fmt + " [format error: " + e.what() + "]";
            return common::errors::New(code, std::move(fallbackMsg), ERR_SOURCE_LOC);
        }
    }
protected:
    CameraDevInfo m_cameraDevInfo_;             // 设备信息
    CameraDevType m_typeName = CDN_UNKNOWN;     // 设备型号
    CameraInfo m_cameraInfo_;                   // 相机信息
    bool isConnected_ = false;
    int errCode_ = 0;
    std::string errMsg_ = "";

    /** 运行时触发开关，由采图 API 或 SetCameraFeature 维护 */
    bool m_triggerSwitch_ = false;
    CameraTriggerSourceType m_triggerSourceType_ = CameraTriggerSourceType::SOFTWARE_TRIGGER;

    std::atomic<CaptureState> captureState_{CaptureState::Idle};
};

template<typename T>
inline ErrorPtr CameraBase::SetCameraFeature(CameraFeatureType featureType, T value)
{
    const CameraFeatureArg arg = detail::toFeatureArg(featureType, std::forward<T>(value));
    if (arg.kind == CameraFeatureArg::Kind::Invalid) {
        return Err("unsupported SetCameraFeature argument for feature",
                   common::errors::camera_module::kInvalidConfig);
    }
    return setCameraFeatureImpl(featureType, arg);
}

template<typename T>
inline ErrorPtr CameraBase::GetCameraFeature(CameraFeatureType featureType, T &value)
{
    CameraFeatureArg arg;
    if (const ErrorPtr err = getCameraFeatureImpl(featureType, arg)) {
        return err;
    }
    if (arg.kind == CameraFeatureArg::Kind::Invalid) {
        return Err("unsupported GetCameraFeature argument for feature",
                   common::errors::camera_module::kInvalidConfig);
    }
    return detail::fromFeatureArg(featureType, arg, value);
}

/** 在调用点记录真实源码位置；用法与 HandleError(msg, code?, detail?) 相同 */
#define HandleError(customMsg, ...) \
    do { \
        HandleErrorAt(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, customMsg, ##__VA_ARGS__); \
    } while (0)

/** 在调用点记录真实源码位置；用法：HandleException(msg, e) 或 HandleException(msg, code, e) */
#define HandleException(customMsg, ...) \
    do { \
        HandleExceptionAt(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, customMsg, ##__VA_ARGS__); \
    } while (0)

}

}
#endif
