#include "lamphouse_base.h"

namespace common
{
namespace device
{
void LamphouseBase::reflectFromJson(const json& j)
{
    fromJson(j,"connOpts", m_lamphouseDevInfo_);
    getFromJson(j, "type", m_typeName);
}
void LamphouseBase::reflectToJson(json& j) const
{
    toJson(j["connOpts"], m_lamphouseDevInfo_);
    j["type"] = m_typeName;
}

bool LamphouseBase::Init() {
    common::serial::SerialInfo mserialOpts;
    mserialOpts.rspTimeout_ = m_lamphouseDevInfo_.serialInfo.rspTimeout_;
    mserialOpts.com_ = m_lamphouseDevInfo_.serialInfo.com_;
    mserialOpts.baud_ = m_lamphouseDevInfo_.serialInfo.baud_;
    mserialOpts.parity_ = (common::serial::ParityMode)m_lamphouseDevInfo_.serialInfo.parity_;
    mserialOpts.dataBit_ = m_lamphouseDevInfo_.serialInfo.dataBit_;
    mserialOpts.stopBit_ = m_lamphouseDevInfo_.serialInfo.stopBit_;

    int ret = serial_.init(mserialOpts);
    if (0 != ret) {
        return false;
    }
    
    return true;
}

bool LamphouseBase::OpenChc(const uint8_t& _ind) {
    std::string reqFrame = open_chc(_ind);
    int ret = serial_.write(reqFrame);
    if (ret != 0) {
        return false;
    }

    std::string rspFrame;
    ret = serial_.read(rspFrame, 1, 2000);
    if (ret != 0) {
        return false;
    }

    return true;
}

bool LamphouseBase::CloseChc(const uint8_t& _ind) {
    std::string reqFrame = close_chc(_ind);
    int ret = serial_.write(reqFrame);
    if (ret != 0) {
        return false;
    }
    
    std::string rspFrame;
    ret = serial_.read(rspFrame, 1, 2000);
    if (ret != 0) {
        return false;
    }

    return true;
}

bool LamphouseBase::SetBrightness(const uint8_t& _ind, const uint8_t& _val) {
    std::string reqFrame = set_brightness(_ind, _val);
    int ret = serial_.write(reqFrame);
    if (ret != 0) {
        return false;
    }
    
    std::string rspFrame;
    ret = serial_.read(rspFrame, 1, 2000);
    if (ret != 0) {
        return false;
    }

    return true;
}

bool LamphouseBase::OpenAllChc() {
    std::string reqFrame = open_all_chc();
    int ret = serial_.write(reqFrame);
    if (ret != 0) {
        return false;
    }
    
    std::string rspFrame;
    ret = serial_.read(rspFrame, 1, 2000);
    if (ret != 0) {
        return false;
    }

    return  true;
}

bool LamphouseBase::CloseAllChc() {
    std::string reqFrame = close_all_chc();
    int ret = serial_.write(reqFrame);
    if (ret != 0) {
        return false;
    }
    
    std::string rspFrame;
    ret = serial_.read(rspFrame, 1, 2000);
    if (ret != 0) {
        return false;
    }

    return true;
}

}

}


