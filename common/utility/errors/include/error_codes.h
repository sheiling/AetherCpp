#ifndef COMMON_UTILITY_ERRORS_ERROR_CODES_H
#define COMMON_UTILITY_ERRORS_ERROR_CODES_H

#include <memory>
#include <string>
#include <unordered_map>

#include "errors_global.h"

namespace common::errors {

enum class Severity {
    kInfo = 0,
    kWarning = 1,
    kError = 2,
    kCritical = 3,
    kUnknown = 9
};

class ERRORS_API Coder {
public:
    virtual ~Coder() = default;
    virtual std::string String() const = 0;
    virtual std::string Reference() const = 0;
    virtual int Code() const = 0;
};

class ERRORS_API SimpleCoder final : public Coder {
public:
    SimpleCoder(int code, std::string external, std::string reference);

    std::string String() const override;
    std::string Reference() const override;
    int Code() const override;

private:
    int code_ = 0;
    std::string external_;
    std::string reference_;
};

using CoderPtr = std::shared_ptr<const Coder>;

ERRORS_API int ModuleOfCode(int code);
ERRORS_API int BusinessOfCode(int code);
ERRORS_API Severity SeverityOfCode(int code);
ERRORS_API std::string SeverityToString(Severity s);

ERRORS_API bool RegisterCoder(const CoderPtr& coder);
ERRORS_API bool RegisterCoder(int code, std::string external, std::string reference);
ERRORS_API CoderPtr FindCoder(int code);
ERRORS_API const std::unordered_map<int, CoderPtr>& AllRegisteredCoders();

// 通用模块错误码（对外暴露，供业务侧获取/定义）
// 6 位错误码规则：
// - 前 3 位：模块（通用模块固定为 001）
// - 后 3 位：业务错误码（按百位映射错误级别：0xx=info, 1xx=warning, 2xx=error, 3xx=critical）
//
// 注意：在 C++ 的 `int` 表达中前导 0 会被省略，因此 `001200` 在代码中为 `1200`。
namespace common_module {
inline constexpr int kModuleCommon = 1; // 001
inline constexpr int kUnknown = 1900; // 001900
inline constexpr int kInvalidArgument = 1200; // 001200
inline constexpr int kTimeout = 1201; // 001201
} // namespace common_module

} // namespace common::errors

#endif // COMMON_UTILITY_ERRORS_ERROR_CODES_H
