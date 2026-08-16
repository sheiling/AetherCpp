#include "error.h"

#include <mutex>
#include <iomanip>
#include <sstream>
#include <utility>

namespace common::errors {

namespace {
std::unordered_map<int, CoderPtr>& RegistryRef() {
    static std::unordered_map<int, CoderPtr> registry;
    return registry;
}

std::mutex& RegistryMutex() {
    static std::mutex registry_mu;
    return registry_mu;
}

std::string BuildFrameText(const StackFrame& frame) {
    std::ostringstream oss;
    oss << frame.function << " (" << frame.file << ":" << frame.line << ")";
    return oss.str();
}

std::string FormatCode6(int code) {
    std::ostringstream oss;
    oss << std::setw(6) << std::setfill('0') << code;
    return oss.str();
}
} // namespace

SimpleCoder::SimpleCoder(int code, std::string external, std::string reference)
    : code_(code), external_(std::move(external)), reference_(std::move(reference)) {}

std::string SimpleCoder::String() const { return external_; }
std::string SimpleCoder::Reference() const { return reference_; }
int SimpleCoder::Code() const { return code_; }

int ModuleOfCode(int code) { return code / 1000; }
int BusinessOfCode(int code) { return code % 1000; }

Severity SeverityOfCode(int code) {
    const int business = BusinessOfCode(code);
    const int bucket = business / 100;
    switch (bucket) {
    case 0:
        return Severity::kInfo;
    case 1:
        return Severity::kWarning;
    case 2:
        return Severity::kError;
    case 3:
        return Severity::kCritical;
    default:
        return Severity::kUnknown;
    }
}

std::string SeverityToString(Severity s) {
    switch (s) {
    case Severity::kInfo:
        return "info";
    case Severity::kWarning:
        return "warning";
    case Severity::kError:
        return "error";
    case Severity::kCritical:
        return "critical";
    default:
        return "unknown";
    }
}

bool RegisterCoder(const CoderPtr& coder) {
    if (coder == nullptr) {
        return false;
    }
    std::lock_guard<std::mutex> guard(RegistryMutex());
    RegistryRef()[coder->Code()] = coder;
    return true;
}

bool RegisterCoder(int code, std::string external, std::string reference) {
    return RegisterCoder(std::make_shared<SimpleCoder>(code, std::move(external), std::move(reference)));
}

CoderPtr FindCoder(int code) {
    std::lock_guard<std::mutex> guard(RegistryMutex());
    const auto it = RegistryRef().find(code);
    if (it == RegistryRef().end()) {
        return nullptr;
    }
    return it->second;
}

const std::unordered_map<int, CoderPtr>& AllRegisteredCoders() { return RegistryRef(); }

Error::Error(int code, std::string message, std::shared_ptr<Error> cause, SourceLoc loc)
    : code_(code), message_(std::move(message)), cause_(std::move(cause)) {
    if (loc.file != nullptr && loc.function != nullptr && loc.line > 0) {
        AddFrame(loc);
    }
}

const char* Error::what() const noexcept {
    what_cache_ = Format(FormatStyle::kCompact);
    return what_cache_.c_str();
}

int Error::Code() const { return code_; }
std::string Error::Message() const { return message_; }
std::shared_ptr<Error> Error::Unwrap() const { return cause_; }
const std::vector<StackFrame>& Error::Stack() const { return stack_; }

void Error::AddFrame(SourceLoc loc) {
    StackFrame frame;
    frame.file = loc.file == nullptr ? "" : loc.file;
    frame.line = loc.line;
    frame.function = loc.function == nullptr ? "" : loc.function;
    stack_.push_back(std::move(frame));
}

std::shared_ptr<Error> Error::Cause() const {
    auto cursor = const_cast<Error*>(this)->shared_from_this();
    while (cursor != nullptr && cursor->Unwrap() != nullptr) {
        cursor = cursor->Unwrap();
    }
    return cursor;
}

std::string Error::Format(FormatStyle style) const {
    std::ostringstream oss;
    const auto coder = FindCoder(code_);
    const auto severity = SeverityToString(SeverityOfCode(code_));
    const std::string ext_msg = coder == nullptr ? "" : coder->String();

    if (style == FormatStyle::kCompact) {
        oss << "[" << FormatCode6(code_) << "/" << severity << "] " << message_;
        if (!ext_msg.empty()) {
            oss << " | " << ext_msg;
        }
        return oss.str();
    }

    if (style == FormatStyle::kChain) {
        const Error* cursor = this;
        bool first = true;
        while (cursor != nullptr) {
            if (!first) {
                oss << " -> ";
            }
            oss << "[" << FormatCode6(cursor->Code()) << "] " << cursor->Message();
            cursor = cursor->Unwrap().get();
            first = false;
        }
        return oss.str();
    }

    oss << "code: " << FormatCode6(code_) << "\n";
    oss << "severity: " << severity << "\n";
    oss << "message: " << message_ << "\n";
    if (coder != nullptr) {
        oss << "public: " << coder->String() << "\n";
        oss << "reference: " << coder->Reference() << "\n";
    }
    if (!stack_.empty()) {
        oss << "stack:\n";
        for (std::size_t i = 0; i < stack_.size(); ++i) {
            oss << "  #" << i << " " << BuildFrameText(stack_[i]) << "\n";
        }
    }
    if (cause_ != nullptr) {
        oss << "cause:\n" << cause_->Format(FormatStyle::kVerbose);
    }
    return oss.str();
}

ErrorPtr New(int code, std::string message, SourceLoc loc) {
    return std::make_shared<Error>(code, std::move(message), nullptr, loc);
}

ErrorPtr Wrap(const ErrorPtr& err, int code, std::string message, SourceLoc loc) {
    if (err == nullptr) {
        return New(code, std::move(message), loc);
    }
    auto wrapped = std::make_shared<Error>(code, std::move(message), err, loc);
    return wrapped;
}

ErrorPtr Wrapf(const ErrorPtr& err, std::string message, SourceLoc loc) {
    if (err == nullptr) {
        return nullptr;
    }
    return Wrap(err, err->Code(), std::move(message), loc);
}

ErrorPtr Unwrap(const ErrorPtr& err) {
    return err == nullptr ? nullptr : err->Unwrap();
}

ErrorPtr Cause(const ErrorPtr& err) {
    if (err == nullptr) {
        return nullptr;
    }
    auto cursor = err;
    while (cursor != nullptr && cursor->Unwrap() != nullptr) {
        cursor = cursor->Unwrap();
    }
    return cursor;
}

bool Is(const ErrorPtr& err, int code) {
    auto cursor = err;
    while (cursor != nullptr) {
        if (cursor->Code() == code) {
            return true;
        }
        cursor = cursor->Unwrap();
    }
    return false;
}

bool Is(const ErrorPtr& err, const ErrorPtr& target) {
    if (target == nullptr) {
        return err == nullptr;
    }
    return Is(err, target->Code());
}

std::string FormatError(const ErrorPtr& err, FormatStyle style) {
    if (err == nullptr) {
        return "<nil>";
    }
    return err->Format(style);
}

std::ostream& operator<<(std::ostream& os, const Error& err) {
    os << err.Format(FormatStyle::kCompact);
    return os;
}

namespace {
const bool kBuiltinRegistered = []() {
    RegisterCoder(common_module::kUnknown, "internal unknown error", "https://docs.example/errors/unknown");
    RegisterCoder(common_module::kInvalidArgument, "invalid argument", "https://docs.example/errors/invalid-argument");
    RegisterCoder(common_module::kTimeout, "request timeout", "https://docs.example/errors/timeout");
    return true;
}();
} // namespace

} // namespace common::errors
