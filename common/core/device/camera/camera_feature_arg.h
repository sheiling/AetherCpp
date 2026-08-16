#ifndef COMMON_CORE_DEVICE_CAMERA_CAMERA_FEATURE_ARG_H
#define COMMON_CORE_DEVICE_CAMERA_CAMERA_FEATURE_ARG_H

#include <common/utility/errors/include/error.h>
#include "camera_error_codes.h"
#include "camera_types.h"

#include <string>

namespace common {
namespace device {

// 相机 feature 参数（带标签值）；模式同 common/utility/typed_value/typed_value.h，
// 此处保留领域枚举字段（TriggerSource / FilterWheel）供驱动层直接使用。
struct CameraFeatureArg {
    enum class Kind {
        Invalid,
        Number,
        Int,
        Bool,
        String,
        TriggerSource,
        FilterWheel,
    };

    Kind kind = Kind::Invalid;
    double number = 0;
    int integer = 0;
    bool flag = false;
    std::string text;
    CameraTriggerSourceType triggerSource = CameraTriggerSourceType::SOFTWARE_TRIGGER;
    CameraFilterWheelType filterWheel = CameraFilterWheelType::FILTER_UNDEFINED;

    bool valid() const { return kind != Kind::Invalid; }
};

namespace detail {

inline bool isNumericFeature(CameraFeatureType featureType)
{
    return featureType == CameraFeatureType::CAMERA_EXPOSURE_TIME
        || featureType == CameraFeatureType::CAMERA_BALANCE_RATIO;
}

inline bool isIntFeature(CameraFeatureType featureType)
{
    switch (featureType) {
    case CameraFeatureType::CAMERA_OFFSETX:
    case CameraFeatureType::CAMERA_OFFSETY:
    case CameraFeatureType::CAMERA_HEIGHT:
    case CameraFeatureType::CAMERA_WIDTH:
    case CameraFeatureType::CAMERA_ND_FILTER_WHEEL:
    case CameraFeatureType::CAMERA_FILTER_WHEEL:
        return true;
    default:
        return false;
    }
}

inline CameraFeatureArg toFeatureArg(CameraFeatureType featureType, int value)
{
    if (isNumericFeature(featureType)) {
        CameraFeatureArg arg;
        arg.kind = CameraFeatureArg::Kind::Number;
        arg.number = static_cast<double>(value);
        return arg;
    }
    if (isIntFeature(featureType)) {
        CameraFeatureArg arg;
        arg.kind = CameraFeatureArg::Kind::Int;
        arg.integer = value;
        return arg;
    }
    if (featureType == CameraFeatureType::CAMERA_TRIGGER_SOURCE) {
        CameraFeatureArg arg;
        arg.kind = CameraFeatureArg::Kind::TriggerSource;
        arg.triggerSource = static_cast<CameraTriggerSourceType>(value);
        return arg;
    }
    return CameraFeatureArg{};
}

inline CameraFeatureArg toFeatureArg(CameraFeatureType featureType, float value)
{
    if (isNumericFeature(featureType)) {
        CameraFeatureArg arg;
        arg.kind = CameraFeatureArg::Kind::Number;
        arg.number = static_cast<double>(value);
        return arg;
    }
    return CameraFeatureArg{};
}

inline CameraFeatureArg toFeatureArg(CameraFeatureType featureType, double value)
{
    if (isNumericFeature(featureType)) {
        CameraFeatureArg arg;
        arg.kind = CameraFeatureArg::Kind::Number;
        arg.number = value;
        return arg;
    }
    return CameraFeatureArg{};
}

inline CameraFeatureArg toFeatureArg(CameraFeatureType featureType, bool value)
{
    if (featureType == CameraFeatureType::CAMERA_TRIGGER_ENABLE) {
        CameraFeatureArg arg;
        arg.kind = CameraFeatureArg::Kind::Bool;
        arg.flag = value;
        return arg;
    }
    return CameraFeatureArg{};
}

inline CameraFeatureArg toFeatureArg(CameraFeatureType featureType, const std::string &value)
{
    switch (featureType) {
    case CameraFeatureType::CAMERA_BALANCE_RATIO_SELECTOR:
    case CameraFeatureType::CAMERA_LIGHT_SOURCE_PRESET:
    case CameraFeatureType::CAMERA_BALANCE_WHITE_AUTO: {
        CameraFeatureArg arg;
        arg.kind = CameraFeatureArg::Kind::String;
        arg.text = value;
        return arg;
    }
    default:
        return CameraFeatureArg{};
    }
}

inline CameraFeatureArg toFeatureArg(CameraFeatureType featureType, CameraTriggerSourceType value)
{
    if (featureType == CameraFeatureType::CAMERA_TRIGGER_SOURCE) {
        CameraFeatureArg arg;
        arg.kind = CameraFeatureArg::Kind::TriggerSource;
        arg.triggerSource = value;
        return arg;
    }
    return CameraFeatureArg{};
}

inline CameraFeatureArg toFeatureArg(CameraFeatureType featureType, CameraFilterWheelType value)
{
    if (featureType == CameraFeatureType::CAMERA_FILTER_WHEEL) {
        CameraFeatureArg arg;
        arg.kind = CameraFeatureArg::Kind::FilterWheel;
        arg.filterWheel = value;
        return arg;
    }
    return CameraFeatureArg{};
}

inline common::errors::ErrorPtr fromFeatureArg(CameraFeatureType featureType,
                                               const CameraFeatureArg &arg,
                                               int &value)
{
    if (isNumericFeature(featureType) && arg.kind == CameraFeatureArg::Kind::Number) {
        value = static_cast<int>(arg.number);
        return nullptr;
    }
    if (isIntFeature(featureType) && arg.kind == CameraFeatureArg::Kind::Int) {
        value = arg.integer;
        return nullptr;
    }
    if (featureType == CameraFeatureType::CAMERA_TRIGGER_SOURCE
        && arg.kind == CameraFeatureArg::Kind::TriggerSource) {
        value = static_cast<int>(arg.triggerSource);
        return nullptr;
    }
    return common::errors::New(common::errors::camera_module::kInvalidConfig,
                               "unsupported GetCameraFeature int conversion",
                               ERR_SOURCE_LOC);
}

inline common::errors::ErrorPtr fromFeatureArg(CameraFeatureType featureType,
                                               const CameraFeatureArg &arg,
                                               float &value)
{
    if (isNumericFeature(featureType) && arg.kind == CameraFeatureArg::Kind::Number) {
        value = static_cast<float>(arg.number);
        return nullptr;
    }
    return common::errors::New(common::errors::camera_module::kInvalidConfig,
                               "unsupported GetCameraFeature float conversion",
                               ERR_SOURCE_LOC);
}

inline common::errors::ErrorPtr fromFeatureArg(CameraFeatureType featureType,
                                               const CameraFeatureArg &arg,
                                               double &value)
{
    if (isNumericFeature(featureType) && arg.kind == CameraFeatureArg::Kind::Number) {
        value = arg.number;
        return nullptr;
    }
    return common::errors::New(common::errors::camera_module::kInvalidConfig,
                               "unsupported GetCameraFeature double conversion",
                               ERR_SOURCE_LOC);
}

inline common::errors::ErrorPtr fromFeatureArg(CameraFeatureType featureType,
                                               const CameraFeatureArg &arg,
                                               bool &value)
{
    if (featureType == CameraFeatureType::CAMERA_TRIGGER_ENABLE
        && arg.kind == CameraFeatureArg::Kind::Bool) {
        value = arg.flag;
        return nullptr;
    }
    return common::errors::New(common::errors::camera_module::kInvalidConfig,
                               "unsupported GetCameraFeature bool conversion",
                               ERR_SOURCE_LOC);
}

inline common::errors::ErrorPtr fromFeatureArg(CameraFeatureType featureType,
                                               const CameraFeatureArg &arg,
                                               std::string &value)
{
    switch (featureType) {
    case CameraFeatureType::CAMERA_BALANCE_RATIO_SELECTOR:
    case CameraFeatureType::CAMERA_LIGHT_SOURCE_PRESET:
    case CameraFeatureType::CAMERA_BALANCE_WHITE_AUTO:
        if (arg.kind == CameraFeatureArg::Kind::String) {
            value = arg.text;
            return nullptr;
        }
        break;
    default:
        break;
    }
    return common::errors::New(common::errors::camera_module::kInvalidConfig,
                               "unsupported GetCameraFeature string conversion",
                               ERR_SOURCE_LOC);
}

inline common::errors::ErrorPtr fromFeatureArg(CameraFeatureType featureType,
                                               const CameraFeatureArg &arg,
                                               CameraTriggerSourceType &value)
{
    if (featureType == CameraFeatureType::CAMERA_TRIGGER_SOURCE
        && arg.kind == CameraFeatureArg::Kind::TriggerSource) {
        value = arg.triggerSource;
        return nullptr;
    }
    return common::errors::New(common::errors::camera_module::kInvalidConfig,
                               "unsupported GetCameraFeature trigger source conversion",
                               ERR_SOURCE_LOC);
}

inline common::errors::ErrorPtr fromFeatureArg(CameraFeatureType featureType,
                                               const CameraFeatureArg &arg,
                                               CameraFilterWheelType &value)
{
    if (featureType == CameraFeatureType::CAMERA_FILTER_WHEEL
        && arg.kind == CameraFeatureArg::Kind::FilterWheel) {
        value = arg.filterWheel;
        return nullptr;
    }
    return common::errors::New(common::errors::camera_module::kInvalidConfig,
                               "unsupported GetCameraFeature filter wheel conversion",
                               ERR_SOURCE_LOC);
}

} // namespace detail

} // namespace device
} // namespace common

#endif
