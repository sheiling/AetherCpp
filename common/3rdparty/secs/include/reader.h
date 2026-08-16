#ifndef SECS_READER_HH
#define SECS_READER_HH

#include "msg_buffer.h"
#include <queue>
#include <mutex>
#include <vector>
#include <cstdint>

namespace SECS
{

class Message
{
public:
    Message() = default;
    Message(const Message &msg) : msgLen(msg.msgLen), data(msg.data) {}
    ~Message() = default;
    uint8_t getFunction() const;
    uint8_t getStype() const;
    uint32_t getSystemBytes() const;
public:
    uint32_t msgLen = 0;  
    std::vector<uint8_t> data;
};

class Reader
{
public:
    Reader() = default;
    ~Reader() = default;

    void read(const std::string &payload);
    void enqueue(const Message &msg);
    Message dequeue();
    bool empty() { return msgQ_.empty(); }
private:
    MsgBuffer msgBuffer_;
    std::queue<Message> msgQ_;
    std::mutex mutexQ_;
};

}

#endif