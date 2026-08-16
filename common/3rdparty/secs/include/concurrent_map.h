#ifndef SECS_CONCURRENT_MAP_HH
#define SECS_CONCURRENT_MAP_HH

#include <mutex>
#include <map>
#include <vector>
#include <memory>
#include "channel.h"
#include "reader.h"

namespace SECS
{

using ChannelPtr = std::shared_ptr<Channel<Message>>;

struct ConcurrentMapShared
{
    std::map<uint32_t, ChannelPtr> items;
    std::mutex mtx_;
};

using ConcurrentMapSharedPtr = std::shared_ptr<ConcurrentMapShared>;

class ConcurrentMap
{
public:
    ConcurrentMap();
    ~ConcurrentMap();

    ConcurrentMapSharedPtr getShared(uint32_t key);
    void set(uint32_t key, ChannelPtr item);
    ChannelPtr get(uint32_t key);
    void del(uint32_t key);
private:
    int shareCount_ = 32;
    std::vector<ConcurrentMapSharedPtr> item_;
};

}

#endif