#ifndef MELSEC_REQUEST_HH
#define MELSEC_REQUEST_HH

#include <cstdint>
#include <vector>

namespace common::regaccess {
namespace mc {

class MelsecRequest {
public:
    MelsecRequest(uint32_t address, uint16_t reqCnt) 
        : address_(address)
        , reqCnt_(reqCnt) {
    }
    void setData(const std::vector<uint8_t> &data) { data_ = data; }
    void setAddressType(uint8_t addressType) { addressType_ = addressType;}
    std::vector<uint8_t> pack() const;
private:
    uint32_t address_;
    std::vector<uint8_t> data_;
    uint16_t reqCnt_ = 0;
    uint8_t addressType_ = 0xA8;
};

} // end of namespace mc
} // end of namespace common::regaccess

#endif