#ifndef MELSEC_RESPONSE_HH
#define MELSEC_RESPONSE_HH

#include <cstdint>
#include <vector>

namespace common::regaccess {
namespace mc {
class MelsecResponse {
public:
    MelsecResponse() = default;
    bool expand(const std::vector<uint8_t>& data);
    uint16_t getErrorCode() const noexcept { return errorCode_; }
    uint8_t getDeputyHead() const noexcept { return deputyHead_; }
    const std::vector<uint8_t>& getData() const noexcept { return data_; }
private:
    uint8_t deputyHead_{0};
    uint16_t dataLen_{0};
    uint16_t errorCode_{0};
    std::vector<uint8_t> data_;
};
} // end of namespace mc
} // end of namespace common::regaccess

#endif