#include "camera_base.h"
#include <sstream>

#include "camera_error_codes.h"
#include <common/core/messagecenter/message_center.h>

#include <QString>

namespace {
// 相机库加载时注册错误码（重复注册会覆盖同 code 的描述，属于幂等操作）
const bool kCameraCodersRegistered = ::common::errors::camera_module::RegisterCameraCoders();
} // namespace

namespace common
{
namespace device
{

void CameraBase::SetConnectedState(bool connected)
{
    if (isConnected_ == connected) {
        return;
    }
    isConnected_ = connected;

    MessageCenter::Center::Notify(
        EVENT_CAMERA_CONN_STATE,
        QString::fromStdString(m_cameraDevInfo_.SN),
        connected);
}

void CameraBase::SetCaptureState(CaptureState state)
{
    const CaptureState oldState = captureState_.exchange(state);
    if (oldState == state)
        return;

    MessageCenter::Center::Notify(
        EVENT_CAMERA_STATE,
        QString::fromStdString(m_cameraDevInfo_.SN),
        static_cast<int>(state));
}

std::string CameraBase::formatErrorLog(const std::string& customMsg, int errorCode, const std::string& errorMsg) const
{
    std::ostringstream oss;
    oss << customMsg;

    if (!m_cameraDevInfo_.SN.empty()) {
        oss << " [SN: " << m_cameraDevInfo_.SN << "]";
    }

    if (errorCode != -1) {
        oss << " errCode=" << errorCode;
    }

    if (!errorMsg.empty()) {
        if (errorCode != -1) {
            oss << ", errMsg=" << errorMsg;
        } else {
            oss << " errMsg=" << errorMsg;
        }
    }

    return oss.str();
}

void CameraBase::logErrorAt(spdlog::source_loc loc, const std::string& customMsg, int errorCode, const std::string& errorMsg)
{
    if (auto logger = ::logger::Logger::get())
        logger->log(loc, spdlog::level::err, formatErrorLog(customMsg, errorCode, errorMsg));
}

void CameraBase::LogError(const std::string& customMsg, int errorCode, const std::string& errorMsg)
{
    LOG_ERROR("{}", formatErrorLog(customMsg, errorCode, errorMsg));
}

void CameraBase::HandleErrorAt(spdlog::source_loc loc, const std::string& customMsg, int errorCode, const std::string& errorMsg)
{
    errCode_ = errorCode;
    errMsg_ = errorMsg.empty() ? customMsg : errorMsg;

    logErrorAt(loc, customMsg, errorCode, errMsg_);
}

bool CameraBase::trySetTriggerFeature(CameraFeatureType featureType,
                                      const CameraFeatureArg &arg,
                                      ErrorPtr &err)
{
    if (featureType == CameraFeatureType::CAMERA_TRIGGER_ENABLE) {
        if (arg.kind != CameraFeatureArg::Kind::Bool) {
            err = Err("invalid trigger enable arg", common::errors::camera_module::kInvalidConfig);
            return true;
        }
        m_triggerSwitch_ = arg.flag;
        err = onTriggerSwitchChanged();
        return true;
    }

    if (featureType == CameraFeatureType::CAMERA_TRIGGER_SOURCE) {
        if (arg.kind == CameraFeatureArg::Kind::TriggerSource) {
            m_triggerSourceType_ = arg.triggerSource;
        } else if (arg.kind == CameraFeatureArg::Kind::Int) {
            m_triggerSourceType_ = static_cast<CameraTriggerSourceType>(arg.integer);
        } else {
            err = Err("invalid trigger source arg", common::errors::camera_module::kInvalidConfig);
            return true;
        }
        err = onTriggerSourceChanged();
        return true;
    }

    return false;
}

bool CameraBase::tryGetTriggerFeature(CameraFeatureType featureType, CameraFeatureArg &arg) const
{
    if (featureType == CameraFeatureType::CAMERA_TRIGGER_ENABLE) {
        arg.kind = CameraFeatureArg::Kind::Bool;
        arg.flag = m_triggerSwitch_;
        return true;
    }
    if (featureType == CameraFeatureType::CAMERA_TRIGGER_SOURCE) {
        arg.kind = CameraFeatureArg::Kind::TriggerSource;
        arg.triggerSource = m_triggerSourceType_;
        return true;
    }
    return false;
}

ErrorPtr CameraBase::ensureFreeRunMode()
{
    if (!m_triggerSwitch_) {
        return Ok();
    }
    m_triggerSwitch_ = false;
    return onTriggerSwitchChanged();
}

ErrorPtr CameraBase::ensureTriggerMode(CameraTriggerSourceType source)
{
    m_triggerSourceType_ = source;
    const bool needEnable = !m_triggerSwitch_;
    m_triggerSwitch_ = true;
    if (needEnable) {
        if (auto err = onTriggerSwitchChanged()) {
            return err;
        }
    }
    return onTriggerSourceChanged();
}

} // namespace device
} // namespace common
