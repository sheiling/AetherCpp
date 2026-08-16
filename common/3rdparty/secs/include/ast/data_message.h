#ifndef _SECS_DATA_MESSAGE_HH
#define _SECS_DATA_MESSAGE_HH

#include "item.h"
#include "buffer.h"
#include "hsms.h"

namespace SECS
{

class DataMessage : public HSMSMessage
{
public:
    DataMessage(){}
    DataMessage(uint16_t deviceID, uint8_t stream, uint8_t func, uint32_t systemBytes, bool replyExpected) 
        : m_deviceID(deviceID)
        , m_stream(stream)
        , m_func(func)
        , m_pType(0)
        , m_sType(0)
        , m_systemBytes(systemBytes)
        , m_replyExpected(replyExpected){}
    ~DataMessage(){}

    SType Type() const { return SType::DataMessage; }
    uint32_t getSystemBytes() const { return m_systemBytes; }
    std::vector<uint8_t> pack();
    void expand(const std::vector<uint8_t> &data);
    ItemPtr getItem(size_t index);
    void appendItem(ItemPtr item);
    size_t getItemCount();

    uint8_t getStream() { return m_stream; }
    uint8_t getFunc() { return m_func; }

private:
    void inputMsgHeader(Buffer &in);
    void inputItemHeader(Buffer &in, const ItemPtr &item);
    void inputItem(Buffer &in, const ItemPtr &item);

    void outMsgHeader(Buffer &out);
    void outItemHeader(Buffer &out, Format &format, int &size);
    ItemPtr outItem(Buffer &out);
    Value outItemValue(Buffer &out, const Format &format);
    
private:
    uint16_t m_deviceID = 1;
    uint8_t m_stream = 0;
    uint8_t m_func = 0;
    uint8_t m_pType = 0;
    uint8_t m_sType = 0;
    uint32_t m_systemBytes = 0;
    bool m_replyExpected = false;
    std::vector<ItemPtr> m_items;
};

using DataMessagePtr = std::shared_ptr<DataMessage>;

}

#endif