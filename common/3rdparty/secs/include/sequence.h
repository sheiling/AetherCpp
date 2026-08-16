#ifndef _SECS_SEQUENCE_HH
#define _SECS_SEQUENCE_HH

#include <iostream>
#include <atomic>

namespace SECS
{


class Sequence {
public:
    static Sequence& getInstance() {
        static Sequence sequence;
        return sequence;
    }
    uint32_t generateSeq() {
        if (startSeq_.load() >= maxSeq_) {
            startSeq_.store(1);
        }
        return startSeq_.fetch_add(1);
    }
private:
    std::atomic<uint32_t> startSeq_ = 1;
    uint32_t maxSeq_ = 4294967295;
};

}

#endif