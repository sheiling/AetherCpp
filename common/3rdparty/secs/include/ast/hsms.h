#ifndef _SECS_HSMS_HH
#define _SECS_HSMS_HH

#include <string>
#include <vector>
#include "buffer.h"
#include <memory>

namespace SECS
{

class HSMSMessage;
using HSMSMessagePtr = std::shared_ptr<HSMSMessage>;

enum class SType
{
    DataMessage = 0,		       
    SelectReq   = 1,
    SelectRsp   = 2,
    DeselectReq = 3,
    DeselectRsp = 4,
    LinktestReq = 5,
    LinktestRsp = 6,
    RejectReq   = 7,
    SeparateReq = 9,
    UNDEFINED   = 0xFF
};

class HSMSMessage
{
public:
    virtual ~HSMSMessage(){}

    virtual SType Type() const = 0;
    virtual uint32_t getSystemBytes() const = 0;
    virtual std::vector<uint8_t> pack() = 0;
    virtual void expand(const std::vector<uint8_t> &data) = 0;
};

class SelectReqMessage : public HSMSMessage
{
public:
    SelectReqMessage(){}
    SelectReqMessage(uint16_t sessionID, uint32_t systemBytes) 
        : m_sessionID(sessionID), m_systemBytes(systemBytes) {}
    ~SelectReqMessage(){}
    
    SType Type() const { return SType::SelectReq; }
    uint32_t getSystemBytes() const { return m_systemBytes; }
    std::vector<uint8_t> pack();
    void expand(const std::vector<uint8_t> &data);
    
public:
    uint16_t m_sessionID = 65535;
    uint32_t m_systemBytes = 0; 
};

class SelectRspMessage : public HSMSMessage
{
public:
    SelectRspMessage(){}
    SelectRspMessage(const SelectReqMessage &msg, uint8_t selectStatus) 
        : m_msg(msg), m_selectStatus(selectStatus) {}
    ~SelectRspMessage(){}
    
    SType Type() const { return SType::SelectRsp; }
    uint32_t getSystemBytes() const { return m_msg.m_systemBytes; }
    std::vector<uint8_t> pack();
    void expand(const std::vector<uint8_t> &data);
    
public:
    SelectReqMessage m_msg;
    uint8_t m_selectStatus = 0; 
};

class DeselectReqMessage : public HSMSMessage
{
public:
    DeselectReqMessage(){}
    DeselectReqMessage(uint16_t sessionID, uint32_t systemBytes) 
        : m_sessionID(sessionID), m_systemBytes(systemBytes) {}
    ~DeselectReqMessage(){}
    
    SType Type() const { return SType::DeselectReq; }
    uint32_t getSystemBytes() const { return m_systemBytes; }
    std::vector<uint8_t> pack();
    void expand(const std::vector<uint8_t> &data);
    
public:
    uint16_t m_sessionID = 65535;
    uint32_t m_systemBytes = 0; 
};

class DeselectRspMessage : public HSMSMessage
{
public:
    DeselectRspMessage(){}
    DeselectRspMessage(const DeselectReqMessage &msg, uint8_t deselectStatus) 
        : m_msg(msg), m_deselectStatus(deselectStatus) {}
    ~DeselectRspMessage(){}
    
    SType Type() const { return SType::DeselectRsp; }
    uint32_t getSystemBytes() const { return m_msg.m_systemBytes; }
    std::vector<uint8_t> pack();
    void expand(const std::vector<uint8_t> &data);
    
public:
    DeselectReqMessage m_msg;
    uint8_t m_deselectStatus = 0; 
};

class LinktestReqMessage : public HSMSMessage
{
public:
    LinktestReqMessage(){}
    explicit LinktestReqMessage(uint32_t systemBytes) : m_systemBytes(systemBytes) {}
    ~LinktestReqMessage(){}
    
    SType Type() const { return SType::LinktestReq; }
    uint32_t getSystemBytes() const { return m_systemBytes; }
    std::vector<uint8_t> pack();
    void expand(const std::vector<uint8_t> &data);
    
public:
    uint32_t m_systemBytes = 0; 
};

class LinktestRspMessage : public HSMSMessage
{
public:
    LinktestRspMessage(){}
    explicit LinktestRspMessage(const LinktestReqMessage &msg) : m_msg(msg) {}
    ~LinktestRspMessage(){}
    
    SType Type() const { return SType::LinktestRsp; }
    uint32_t getSystemBytes() const { return m_msg.m_systemBytes; }
    std::vector<uint8_t> pack();
    void expand(const std::vector<uint8_t> &data);
    
public:
    LinktestReqMessage m_msg;
};

class RejectReqMessage : public HSMSMessage
{
public:
    RejectReqMessage(){}
    RejectReqMessage(uint16_t sessionID, uint32_t systemBytes, uint8_t pType, uint8_t sType, uint8_t reasonCode) 
        : m_sessionID(sessionID)
        , m_systemBytes(systemBytes)
        , m_pType(pType)
        , m_sType(sType)
        , m_reasonCode(reasonCode)
        {}
    ~RejectReqMessage(){}
    
    SType Type() const { return SType::RejectReq; }
    uint32_t getSystemBytes() const { return m_systemBytes; }
    std::vector<uint8_t> pack();
    void expand(const std::vector<uint8_t> &data);
    
public:
    uint16_t m_sessionID = 65535;
    uint32_t m_systemBytes = 0;
    uint8_t m_pType = 0;
    uint8_t m_sType = 0;
    uint8_t m_reasonCode = 0; 
};

class SeparateReqMessage : public HSMSMessage
{
public:
    SeparateReqMessage(){}
    SeparateReqMessage(uint16_t sessionID, uint32_t systemBytes) 
        : m_sessionID(sessionID), m_systemBytes(systemBytes) {}
    ~SeparateReqMessage(){}
    
    SType Type() const { return SType::SeparateReq; }
    uint32_t getSystemBytes() const { return m_systemBytes; }
    std::vector<uint8_t> pack();
    void expand(const std::vector<uint8_t> &data);
    
public:
    uint16_t m_sessionID = 65535;
    uint32_t m_systemBytes = 0; 
};

}

#endif

