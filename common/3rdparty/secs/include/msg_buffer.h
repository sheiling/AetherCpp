#ifndef SECS_MSG_BUFFER_HH
#define SECS_MSG_BUFFER_HH

#include <vector>
#include <string>

namespace SECS
{

class MsgBuffer
{
public:
    MsgBuffer() = default;
    ~MsgBuffer() = default;

    void append(const std::string &payload);
    size_t size() const { return payloadBuffer_.size(); }
    uint32_t getMessageLength();
    std::vector<uint8_t> recvData();

private:
    std::vector<uint8_t> payloadBuffer_;
};

}

#endif