#ifndef COMMON_UTILITY_ERRORS_ERROR_H
#define COMMON_UTILITY_ERRORS_ERROR_H

#include <exception>
#include <cstdio>
#include <memory>
#include <ostream>
#include <string>
#include <type_traits>
#include <vector>

#include "error_codes.h"

namespace common::errors {

struct StackFrame {
    std::string file;
    int line = 0;
    std::string function;
};

enum class FormatStyle {
    kCompact,
    kChain,
    kVerbose
};

struct SourceLoc {
    const char* file = "";
    int line = 0;
    const char* function = "";
};

#define ERR_SOURCE_LOC ::common::errors::SourceLoc{__FILE__, __LINE__, __func__}

class ERRORS_API Error : public std::exception, public std::enable_shared_from_this<Error> {
public:
    Error(int code, std::string message, std::shared_ptr<Error> cause = nullptr, SourceLoc loc = {});

    const char* what() const noexcept override;

    int Code() const;
    std::string Message() const;
    std::shared_ptr<Error> Unwrap() const;
    const std::vector<StackFrame>& Stack() const;

    void AddFrame(SourceLoc loc);
    std::string Format(FormatStyle style = FormatStyle::kCompact) const;
    std::shared_ptr<Error> Cause() const;

    CoderPtr Coder() const {
        return FindCoder(this->Code());
    }
    std::string CoderString() const {
        auto coder = this->Coder();
        return coder ? coder->String() : "";
    }
    std::string CoderReference() const {
        auto coder = this->Coder();
        return coder ? coder->Reference() : "";
    }
private:
    int code_ = 0;
    std::string message_;
    std::shared_ptr<Error> cause_;
    std::vector<StackFrame> stack_;
    mutable std::string what_cache_;
};

using ErrorPtr = std::shared_ptr<Error>;

ERRORS_API ErrorPtr New(int code, std::string message, SourceLoc loc = {});

/**
 * 使用printf风格格式化字符串创建ErrorPtr。
 * 例: auto err = common::errors::Newf(1200, "detail: %d", 42);
 */
template<typename... Args>
inline ErrorPtr Newf(int code, const char* fmt, Args&&... args) {
    constexpr size_t kBufferSize = 1024;
    char buffer[kBufferSize];
#if defined(_MSC_VER)
    int n = _snprintf_s(buffer, kBufferSize, _TRUNCATE, fmt, std::forward<Args>(args)...);
#else
    int n = std::snprintf(buffer, kBufferSize, fmt, std::forward<Args>(args)...);
#endif
    std::string message = (n >= 0) ? std::string(buffer) : std::string(fmt);
    return New(code, message);
}
ERRORS_API ErrorPtr Wrap(const ErrorPtr& err, int code, std::string message, SourceLoc loc = {});
ERRORS_API ErrorPtr Wrapf(const ErrorPtr& err, std::string message, SourceLoc loc = {});
ERRORS_API ErrorPtr Unwrap(const ErrorPtr& err);
ERRORS_API ErrorPtr Cause(const ErrorPtr& err);

ERRORS_API bool Is(const ErrorPtr& err, int code);
ERRORS_API bool Is(const ErrorPtr& err, const ErrorPtr& target);

template <typename T>
bool As(const ErrorPtr& err, std::shared_ptr<T>& out) {
    static_assert(std::is_base_of<Error, T>::value, "T must derive from common::errors::Error");
    ErrorPtr cursor = err;
    while (cursor != nullptr) {
        auto casted = std::dynamic_pointer_cast<T>(cursor);
        if (casted != nullptr) {
            out = casted;
            return true;
        }
        cursor = cursor->Unwrap();
    }
    out.reset();
    return false;
}

ERRORS_API std::string FormatError(const ErrorPtr& err, FormatStyle style = FormatStyle::kCompact);
ERRORS_API std::ostream& operator<<(std::ostream& os, const Error& err);

} // namespace common::errors

#endif // COMMON_UTILITY_ERRORS_ERROR_H
