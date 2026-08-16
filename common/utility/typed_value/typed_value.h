#ifndef COMMON_UTILITY_TYPED_VALUE_TYPED_VALUE_H
#define COMMON_UTILITY_TYPED_VALUE_TYPED_VALUE_H

#include <string>
#include <type_traits>

namespace common {
namespace utility {

// 跨虚函数边界的「带标签值」容器：公开 API 可用模板，多态子类 hook 用统一非模板签名。
enum class TypedValueKind {
    Empty,
    Number,
    Int,
    Bool,
    String,
    Enum,
};

struct TypedValue {
    TypedValueKind kind = TypedValueKind::Empty;
    double number = 0.0;
    int integer = 0;
    bool flag = false;
    std::string text;
    int enumValue = 0;

    bool valid() const { return kind != TypedValueKind::Empty; }

    static TypedValue makeNumber(double value)
    {
        TypedValue v;
        v.kind = TypedValueKind::Number;
        v.number = value;
        return v;
    }

    static TypedValue makeInt(int value)
    {
        TypedValue v;
        v.kind = TypedValueKind::Int;
        v.integer = value;
        return v;
    }

    static TypedValue makeBool(bool value)
    {
        TypedValue v;
        v.kind = TypedValueKind::Bool;
        v.flag = value;
        return v;
    }

    static TypedValue makeString(std::string value)
    {
        TypedValue v;
        v.kind = TypedValueKind::String;
        v.text = std::move(value);
        return v;
    }

    template<typename Enum,
             typename = typename std::enable_if<std::is_enum<Enum>::value>::type>
    static TypedValue makeEnum(Enum value)
    {
        TypedValue v;
        v.kind = TypedValueKind::Enum;
        v.enumValue = static_cast<int>(value);
        return v;
    }
};

template<typename T>
inline bool tryUnbox(const TypedValue &value, T &out)
{
    if constexpr (std::is_same_v<std::decay_t<T>, int>) {
        if (value.kind == TypedValueKind::Number) {
            out = static_cast<int>(value.number);
            return true;
        }
        if (value.kind == TypedValueKind::Int) {
            out = value.integer;
            return true;
        }
        if (value.kind == TypedValueKind::Enum) {
            out = value.enumValue;
            return true;
        }
        return false;
    } else if constexpr (std::is_same_v<std::decay_t<T>, float>) {
        if (value.kind == TypedValueKind::Number) {
            out = static_cast<float>(value.number);
            return true;
        }
        return false;
    } else if constexpr (std::is_same_v<std::decay_t<T>, double>) {
        if (value.kind == TypedValueKind::Number) {
            out = value.number;
            return true;
        }
        return false;
    } else if constexpr (std::is_same_v<std::decay_t<T>, bool>) {
        if (value.kind == TypedValueKind::Bool) {
            out = value.flag;
            return true;
        }
        return false;
    } else if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
        if (value.kind == TypedValueKind::String) {
            out = value.text;
            return true;
        }
        return false;
    } else if constexpr (std::is_enum_v<std::decay_t<T>>) {
        if (value.kind == TypedValueKind::Enum) {
            out = static_cast<std::decay_t<T>>(value.enumValue);
            return true;
        }
        return false;
    } else {
        return false;
    }
}

} // namespace utility
} // namespace common

#endif
