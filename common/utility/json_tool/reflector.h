#ifndef __COMMON_UTILITY_REFLECTOR_H__
#define __COMMON_UTILITY_REFLECTOR_H__
#include <map>
#include <string>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <list>
#include <set>
#include <unordered_set>
#include <array>
#include <memory>
#include <3rdparty/nlohmann/json.hpp>
#include <type_traits>

using json = nlohmann::json;

// ============================================================================
// 类型特征检测
// ============================================================================

// 检测类型是否有 reflectToJson 方法
template<typename T>
struct has_reflectToJson {
private:
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().reflectToJson(std::declval<json&>()), std::true_type{});
    template<typename>
    static std::false_type test(...);
public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

// 检测类型是否有 reflectFromJson 方法
template<typename T>
struct has_reflectFromJson {
private:
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().reflectFromJson(std::declval<const json&>()), std::true_type{});
    template<typename>
    static std::false_type test(...);
public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

// 判断是否为基本类型（可直接序列化）
template<typename T>
struct is_basic_type {
    static constexpr bool value = std::is_arithmetic<T>::value || 
                                  std::is_same<T, std::string>::value ||
                                  std::is_same<T, bool>::value;
};

// 检测枚举类型是否已经用 NLOHMANN_JSON_SERIALIZE_ENUM 注册（有 to_json 函数）
// 使用 ADL (Argument-Dependent Lookup) 来检测全局的 to_json 函数
template<typename T>
struct has_nlohmann_to_json {
private:
    template<typename U>
    static auto test(int) -> decltype(to_json(std::declval<json&>(), std::declval<const U&>()), std::true_type{});
    template<typename>
    static std::false_type test(...);
public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

// 检测枚举类型是否已经用 NLOHMANN_JSON_SERIALIZE_ENUM 注册（有 from_json 函数）
// 使用 ADL (Argument-Dependent Lookup) 来检测全局的 from_json 函数
template<typename T>
struct has_nlohmann_from_json {
private:
    template<typename U>
    static auto test(int) -> decltype(from_json(std::declval<const json&>(), std::declval<U&>()), std::true_type{});
    template<typename>
    static std::false_type test(...);
public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

// ============================================================================
// 基本类型和自定义类型的 toJson/fromJson
// ============================================================================

// toJson: 枚举类型（已用 NLOHMANN_JSON_SERIALIZE_ENUM 注册）
// 使用 ADL 调用全局的 to_json 函数
template<typename T>
typename std::enable_if<std::is_enum<T>::value && has_nlohmann_to_json<T>::value, void>::type
toJson(json& j, const T& value) {
    using nlohmann::to_json;
    to_json(j, value);
}

// toJson: 自定义类型（有 reflectToJson 方法）
template<typename T>
typename std::enable_if<!std::is_enum<T>::value && has_reflectToJson<T>::value, void>::type
toJson(json& j, const T& obj) {
    obj.reflectToJson(j);
}

// toJson: 基本类型直接赋值
template<typename T>
typename std::enable_if<!std::is_enum<T>::value && !has_reflectToJson<T>::value && is_basic_type<T>::value, void>::type
toJson(json& j, const T& value) {
    j = value;
}

// fromJson: 枚举类型（已用 NLOHMANN_JSON_SERIALIZE_ENUM 注册）
// 使用 ADL 调用全局的 from_json 函数
template<typename T>
typename std::enable_if<std::is_enum<T>::value && has_nlohmann_from_json<T>::value, void>::type
fromJson(const json& j, const std::string& key, T& value) {
    // 如果 key 为空，直接使用 j（j 本身就是枚举值）
    if (key.empty() && !j.is_null()) {
        using nlohmann::from_json;
        from_json(j, value);
    }
    // 否则从对象中获取指定 key 的值
    else if (j.contains(key) && !j[key].is_null()) {
        using nlohmann::from_json;
        from_json(j[key], value);
    }
}

// fromJson: 自定义类型（有 reflectFromJson 方法）
template<typename T>
typename std::enable_if<!std::is_enum<T>::value && has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, T& obj) {
    // 如果 key 为空，直接使用 j（j 本身就是对象）
    if (key.empty() && !j.is_null()) {
        obj.reflectFromJson(j);
    }
    // 否则从对象中获取指定 key 的值
    else if (j.contains(key) && !j[key].is_null()) {
        obj.reflectFromJson(j[key]);
    }
}

// fromJson: 基本类型直接读取
template<typename T>
typename std::enable_if<!std::is_enum<T>::value && !has_reflectFromJson<T>::value && is_basic_type<T>::value, void>::type
fromJson(const json& j, const std::string& key, T& value) {
    // 如果 key 为空，直接使用 j（j 本身就是基本类型值）
    if (key.empty() && !j.is_null()) {
        value = j.get<T>();
    }
    // 否则从对象中获取指定 key 的值
    else if (j.contains(key) && !j[key].is_null()) {
        value = j.at(key).get<T>();
    }
}

// ============================================================================
// 智能指针支持
// ============================================================================

// shared_ptr: toJson
template<typename T>
void toJson(json& j, const std::shared_ptr<T>& obj) {
    if (obj) {
        toJson(j, *obj);
    } else {
        j = nullptr;
    }
}

// shared_ptr: fromJson (T 有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::shared_ptr<T>& obj) {
    if (key.empty()) {
        // 如果 key 为空，直接使用 j
        if (!j.is_null()) {
            if (!obj)
                obj = std::make_shared<T>();
            obj->reflectFromJson(j);
        }
    } else if (j.contains(key) && !j[key].is_null()) {
        if (!obj)
            obj = std::make_shared<T>();
        obj->reflectFromJson(j[key]);
    }
}

// shared_ptr: fromJson (T 没有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<!has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::shared_ptr<T>& obj) {
    if (key.empty()) {
        // 如果 key 为空，直接使用 j
        if (!j.is_null()) {
            if (!obj)
                obj = std::make_shared<T>();
            fromJson(j, "", *obj);
        }
    } else if (j.contains(key) && !j[key].is_null()) {
        if (!obj)
            obj = std::make_shared<T>();
        fromJson(j[key], "", *obj);
    }
}

// unique_ptr: toJson
template<typename T>
void toJson(json& j, const std::unique_ptr<T>& obj) {
    if (obj) {
        toJson(j, *obj);
    } else {
        j = nullptr;
    }
}

// unique_ptr: fromJson (T 有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::unique_ptr<T>& obj) {
    if (key.empty()) {
        // 如果 key 为空，直接使用 j
        if (!j.is_null()) {
            obj = std::make_unique<T>();
            obj->reflectFromJson(j);
        }
    } else if (j.contains(key) && !j[key].is_null()) {
        obj = std::make_unique<T>();
        obj->reflectFromJson(j[key]);
    }
}

// unique_ptr: fromJson (T 没有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<!has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::unique_ptr<T>& obj) {
    if (key.empty()) {
        // 如果 key 为空，直接使用 j
        if (!j.is_null()) {
            obj = std::make_unique<T>();
            fromJson(j, "", *obj);
        }
    } else if (j.contains(key) && !j[key].is_null()) {
        obj = std::make_unique<T>();
        fromJson(j[key], "", *obj);
    }
}

// ============================================================================
// STL 容器支持：vector
// ============================================================================

// vector<T>: toJson
template<typename T>
void toJson(json& j, const std::vector<T>& v) {
    j = json::array();
    for (const auto& elem : v) {
        json obj;
        toJson(obj, elem);
        j.push_back(obj);
    }
}

// vector<T>: fromJson (T 有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::vector<T>& v) {
    v.clear();
    
    // 如果 key 为空或 JSON 本身就是数组，直接处理 j
    if (key.empty() && j.is_array()) {
        for (const auto& elem : j) {
            T obj;
            obj.reflectFromJson(elem);
            v.push_back(obj);
        }
    }
    // 从对象中获取指定 key 的值
    else if (j.is_object() && j.contains(key) && !j[key].is_null()) {
        // 如果 j[key] 是数组，遍历数组元素
        if (j[key].is_array()) {
            for (const auto& elem : j[key]) {
                T obj;
                obj.reflectFromJson(elem);
                v.push_back(obj);
            }
        }
        // 如果 j[key] 是对象，遍历对象的值（values）
        else if (j[key].is_object()) {
            for (auto it = j[key].begin(); it != j[key].end(); ++it) {
                T obj;
                obj.reflectFromJson(it.value());
                v.push_back(obj);
            }
        }
        // 如果 j[key] 既不是数组也不是对象，容器已清空
    }
    // 如果 key 不存在，容器已清空
}

// vector<T>: fromJson (T 没有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<!has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::vector<T>& v) {
    v.clear();
    
    // 如果 key 为空或 JSON 本身就是数组，直接处理 j
    if (key.empty() && j.is_array()) {
        for (const auto& elem : j) {
            T obj;
            fromJson(elem, "", obj);
            v.push_back(obj);
        }
    }
    // 从对象中获取指定 key 的值
    else if (j.is_object() && j.contains(key) && !j[key].is_null()) {
        // 如果 j[key] 是数组，遍历数组元素
        if (j[key].is_array()) {
            for (const auto& elem : j[key]) {
                T obj;
                fromJson(elem, "", obj);
                v.push_back(obj);
            }
        }
        // 如果 j[key] 是对象，遍历对象的值（values）
        else if (j[key].is_object()) {
            for (auto it = j[key].begin(); it != j[key].end(); ++it) {
                T obj;
                fromJson(it.value(), "", obj);
                v.push_back(obj);
            }
        }
        // 如果 j[key] 既不是数组也不是对象，容器已清空
    }
    // 如果 key 不存在，容器已清空
}

// vector<shared_ptr<T>>: toJson
template<typename T>
void toJson(json& j, const std::vector<std::shared_ptr<T>>& v) {
    j = json::array();
    for (const auto& elem : v) {
        json obj;
        toJson(obj, elem);
        j.push_back(obj);
    }
}

// vector<shared_ptr<T>>: fromJson (T 有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::vector<std::shared_ptr<T>>& v) {
    v.clear();
    
    // 如果 key 为空或 JSON 本身就是数组，直接处理 j
    if (key.empty() && j.is_array()) {
        for (const auto& elem : j) {
            std::shared_ptr<T> obj = std::make_shared<T>();
            obj->reflectFromJson(elem);
            v.push_back(std::move(obj));
        }
    }
    // 从对象中获取指定 key 的值
    else if (j.is_object() && j.contains(key) && !j[key].is_null()) {
        // 如果 j[key] 是数组，遍历数组元素
        if (j[key].is_array()) {
            for (const auto& elem : j[key]) {
                std::shared_ptr<T> obj = std::make_shared<T>();
                obj->reflectFromJson(elem);
                v.push_back(std::move(obj));
            }
        }
        // 如果 j[key] 是对象，遍历对象的值（values）
        else if (j[key].is_object()) {
            for (auto it = j[key].begin(); it != j[key].end(); ++it) {
                std::shared_ptr<T> obj = std::make_shared<T>();
                obj->reflectFromJson(it.value());
                v.push_back(std::move(obj));
            }
        }
        // 如果 j[key] 既不是数组也不是对象，容器已清空
    }
    // 如果 key 不存在，容器已清空
}

// vector<shared_ptr<T>>: fromJson (T 没有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<!has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::vector<std::shared_ptr<T>>& v) {
    v.clear();
    
    // 如果 key 为空或 JSON 本身就是数组，直接处理 j
    if (key.empty() && j.is_array()) {
        for (const auto& elem : j) {
            std::shared_ptr<T> obj = std::make_shared<T>();
            fromJson(elem, "", *obj);
            v.push_back(std::move(obj));
        }
    }
    // 从对象中获取指定 key 的值
    else if (j.is_object() && j.contains(key) && !j[key].is_null()) {
        // 如果 j[key] 是数组，遍历数组元素
        if (j[key].is_array()) {
            for (const auto& elem : j[key]) {
                std::shared_ptr<T> obj = std::make_shared<T>();
                fromJson(elem, "", *obj);
                v.push_back(std::move(obj));
            }
        }
        // 如果 j[key] 是对象，遍历对象的值（values）
        else if (j[key].is_object()) {
            for (auto it = j[key].begin(); it != j[key].end(); ++it) {
                std::shared_ptr<T> obj = std::make_shared<T>();
                fromJson(it.value(), "", *obj);
                v.push_back(std::move(obj));
            }
        }
        // 如果 j[key] 既不是数组也不是对象，容器已清空
    }
    // 如果 key 不存在，容器已清空
}

// vector<unique_ptr<T>>: toJson
template<typename T>
void toJson(json& j, const std::vector<std::unique_ptr<T>>& v) {
    j = json::array();
    for (const auto& elem : v) {
        json obj;
        toJson(obj, elem);
        j.push_back(obj);
    }
}

// vector<unique_ptr<T>>: fromJson (T 有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::vector<std::unique_ptr<T>>& v) {
    v.clear();
    
    // 如果 key 为空或 JSON 本身就是数组，直接处理 j
    if (key.empty() && j.is_array()) {
        for (const auto& elem : j) {
            std::unique_ptr<T> obj = std::make_unique<T>();
            obj->reflectFromJson(elem);
            v.push_back(std::move(obj));
        }
    }
    // 从对象中获取指定 key 的值
    else if (j.is_object() && j.contains(key) && !j[key].is_null()) {
        // 如果 j[key] 是数组，遍历数组元素
        if (j[key].is_array()) {
            for (const auto& elem : j[key]) {
                std::unique_ptr<T> obj = std::make_unique<T>();
                obj->reflectFromJson(elem);
                v.push_back(std::move(obj));
            }
        }
        // 如果 j[key] 是对象，遍历对象的值（values）
        else if (j[key].is_object()) {
            for (auto it = j[key].begin(); it != j[key].end(); ++it) {
                std::unique_ptr<T> obj = std::make_unique<T>();
                obj->reflectFromJson(it.value());
                v.push_back(std::move(obj));
            }
        }
        // 如果 j[key] 既不是数组也不是对象，容器已清空
    }
    // 如果 key 不存在，容器已清空
}

// vector<unique_ptr<T>>: fromJson (T 没有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<!has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::vector<std::unique_ptr<T>>& v) {
    v.clear();
    
    // 如果 key 为空或 JSON 本身就是数组，直接处理 j
    if (key.empty() && j.is_array()) {
        for (const auto& elem : j) {
            std::unique_ptr<T> obj = std::make_unique<T>();
            fromJson(elem, "", *obj);
            v.push_back(std::move(obj));
        }
    }
    // 从对象中获取指定 key 的值
    else if (j.is_object() && j.contains(key) && !j[key].is_null()) {
        // 如果 j[key] 是数组，遍历数组元素
        if (j[key].is_array()) {
            for (const auto& elem : j[key]) {
                std::unique_ptr<T> obj = std::make_unique<T>();
                fromJson(elem, "", *obj);
                v.push_back(std::move(obj));
            }
        }
        // 如果 j[key] 是对象，遍历对象的值（values）
        else if (j[key].is_object()) {
            for (auto it = j[key].begin(); it != j[key].end(); ++it) {
                std::unique_ptr<T> obj = std::make_unique<T>();
                fromJson(it.value(), "", *obj);
                v.push_back(std::move(obj));
            }
        }
        // 如果 j[key] 既不是数组也不是对象，容器已清空
    }
    // 如果 key 不存在，容器已清空
}

// ============================================================================
// STL 容器支持：list
// ============================================================================

// list<T>: toJson
template<typename T>
void toJson(json& j, const std::list<T>& v) {
    j = json::array();
    for (const auto& elem : v) {
        json obj;
        toJson(obj, elem);
        j.push_back(obj);
    }
}

// list<T>: fromJson (T 有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::list<T>& v) {
    v.clear();
    
    // 如果 key 为空或 JSON 本身就是数组，直接处理 j
    if (key.empty() && j.is_array()) {
        for (const auto& elem : j) {
            T obj;
            obj.reflectFromJson(elem);
            v.push_back(obj);
        }
    }
    // 从对象中获取指定 key 的值
    else if (j.is_object() && j.contains(key) && !j[key].is_null()) {
        // 如果 j[key] 是数组，遍历数组元素
        if (j[key].is_array()) {
            v.clear();
            for (const auto& elem : j[key]) {
                T obj;
                obj.reflectFromJson(elem);
                v.push_back(obj);
            }
        }
        // 如果 j[key] 是对象，遍历对象的值（values）
        else if (j[key].is_object()) {
            v.clear();
            for (auto it = j[key].begin(); it != j[key].end(); ++it) {
                T obj;
                obj.reflectFromJson(it.value());
                v.push_back(obj);
            }
        }
        // 如果 j[key] 既不是数组也不是对象，容器已清空
    }
    // 如果 key 不存在，容器已清空
}

// list<T>: fromJson (T 没有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<!has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::list<T>& v) {
    v.clear();
    
    // 如果 key 为空或 JSON 本身就是数组，直接处理 j
    if (key.empty() && j.is_array()) {
        for (const auto& elem : j) {
            T obj;
            fromJson(elem, "", obj);
            v.push_back(obj);
        }
    }
    // 从对象中获取指定 key 的值
    else if (j.is_object() && j.contains(key) && !j[key].is_null()) {
        // 如果 j[key] 是数组，遍历数组元素
        if (j[key].is_array()) {
            v.clear();
            for (const auto& elem : j[key]) {
                T obj;
                fromJson(elem, "", obj);
                v.push_back(obj);
            }
        }
        // 如果 j[key] 是对象，遍历对象的值（values）
        else if (j[key].is_object()) {
            v.clear();
            for (auto it = j[key].begin(); it != j[key].end(); ++it) {
                T obj;
                fromJson(it.value(), "", obj);
                v.push_back(obj);
            }
        }
        // 如果 j[key] 既不是数组也不是对象，容器已清空
    }
    // 如果 key 不存在，容器已清空
}

// list<shared_ptr<T>>: toJson
template<typename T>
void toJson(json& j, const std::list<std::shared_ptr<T>>& v) {
    j = json::array();
    for (const auto& elem : v) {
        json obj;
        toJson(obj, elem);
        j.push_back(obj);
    }
}

// list<shared_ptr<T>>: fromJson (T 有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::list<std::shared_ptr<T>>& v) {
    v.clear();
    
    // 如果 key 为空或 JSON 本身就是数组，直接处理 j
    if (key.empty() && j.is_array()) {
        for (const auto& elem : j) {
            std::shared_ptr<T> obj = std::make_shared<T>();
            obj->reflectFromJson(elem);
            v.push_back(std::move(obj));
        }
    }
    // 从对象中获取指定 key 的值
    else if (j.is_object() && j.contains(key) && !j[key].is_null()) {
        // 如果 j[key] 是数组，遍历数组元素
        if (j[key].is_array()) {
            v.clear();
            for (const auto& elem : j[key]) {
                std::shared_ptr<T> obj = std::make_shared<T>();
                obj->reflectFromJson(elem);
                v.push_back(std::move(obj));
            }
        }
        // 如果 j[key] 是对象，遍历对象的值（values）
        else if (j[key].is_object()) {
            v.clear();
            for (auto it = j[key].begin(); it != j[key].end(); ++it) {
                std::shared_ptr<T> obj = std::make_shared<T>();
                obj->reflectFromJson(it.value());
                v.push_back(std::move(obj));
            }
        }
        // 如果 j[key] 既不是数组也不是对象，容器已清空
    }
    // 如果 key 不存在，容器已清空
}

// list<shared_ptr<T>>: fromJson (T 没有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<!has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::list<std::shared_ptr<T>>& v) {
    v.clear();
    
    // 如果 key 为空或 JSON 本身就是数组，直接处理 j
    if (key.empty() && j.is_array()) {
        for (const auto& elem : j) {
            std::shared_ptr<T> obj = std::make_shared<T>();
            fromJson(elem, "", *obj);
            v.push_back(std::move(obj));
        }
    }
    // 从对象中获取指定 key 的值
    else if (j.is_object() && j.contains(key) && !j[key].is_null()) {
        // 如果 j[key] 是数组，遍历数组元素
        if (j[key].is_array()) {
            v.clear();
            for (const auto& elem : j[key]) {
                std::shared_ptr<T> obj = std::make_shared<T>();
                fromJson(elem, "", *obj);
                v.push_back(std::move(obj));
            }
        }
        // 如果 j[key] 是对象，遍历对象的值（values）
        else if (j[key].is_object()) {
            v.clear();
            for (auto it = j[key].begin(); it != j[key].end(); ++it) {
                std::shared_ptr<T> obj = std::make_shared<T>();
                fromJson(it.value(), "", *obj);
                v.push_back(std::move(obj));
            }
        }
        // 如果 j[key] 既不是数组也不是对象，容器已清空
    }
    // 如果 key 不存在，容器已清空
}

// ============================================================================
// STL 容器支持：set
// ============================================================================

// set<T>: toJson
template<typename T>
void toJson(json& j, const std::set<T>& v) {
    j = json::array();
    for (const auto& elem : v) {
        json obj;
        toJson(obj, elem);
        j.push_back(obj);
    }
}

// set<T>: fromJson (T 有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::set<T>& v) {
    v.clear();
    
    // 如果 key 为空或 JSON 本身就是数组，直接处理 j
    if (key.empty() && j.is_array()) {
        for (const auto& elem : j) {
            T obj;
            obj.reflectFromJson(elem);
            v.insert(obj);
        }
    }
    // 从对象中获取指定 key 的值
    else if (j.is_object() && j.contains(key) && !j[key].is_null()) {
        // 如果 j[key] 是数组，遍历数组元素
        if (j[key].is_array()) {
            v.clear();
            for (const auto& elem : j[key]) {
                T obj;
                obj.reflectFromJson(elem);
                v.insert(obj);
            }
        }
        // 如果 j[key] 是对象，遍历对象的值（values）
        else if (j[key].is_object()) {
            v.clear();
            for (auto it = j[key].begin(); it != j[key].end(); ++it) {
                T obj;
                obj.reflectFromJson(it.value());
                v.insert(obj);
            }
        }
        // 如果 j[key] 既不是数组也不是对象，容器已清空
    }
    // 如果 key 不存在，容器已清空
}

// set<T>: fromJson (T 没有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<!has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::set<T>& v) {
    v.clear();
    
    // 如果 key 为空或 JSON 本身就是数组，直接处理 j
    if (key.empty() && j.is_array()) {
        for (const auto& elem : j) {
            T obj;
            fromJson(elem, "", obj);
            v.insert(obj);
        }
    }
    // 从对象中获取指定 key 的值
    else if (j.is_object() && j.contains(key) && !j[key].is_null()) {
        // 如果 j[key] 是数组，遍历数组元素
        if (j[key].is_array()) {
            v.clear();
            for (const auto& elem : j[key]) {
                T obj;
                fromJson(elem, "", obj);
                v.insert(obj);
            }
        }
        // 如果 j[key] 是对象，遍历对象的值（values）
        else if (j[key].is_object()) {
            v.clear();
            for (auto it = j[key].begin(); it != j[key].end(); ++it) {
                T obj;
                fromJson(it.value(), "", obj);
                v.insert(obj);
            }
        }
        // 如果 j[key] 既不是数组也不是对象，容器已清空
    }
    // 如果 key 不存在，容器已清空
}

// ============================================================================
// STL 容器支持：unordered_set
// ============================================================================

// unordered_set<T>: toJson
template<typename T>
void toJson(json& j, const std::unordered_set<T>& v) {
    j = json::array();
    for (const auto& elem : v) {
        json obj;
        toJson(obj, elem);
        j.push_back(obj);
    }
}

// unordered_set<T>: fromJson (T 有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::unordered_set<T>& v) {
    v.clear();
    
    // 如果 key 为空或 JSON 本身就是数组，直接处理 j
    if (key.empty() && j.is_array()) {
        for (const auto& elem : j) {
            T obj;
            obj.reflectFromJson(elem);
            v.insert(obj);
        }
    }
    // 从对象中获取指定 key 的值
    else if (j.is_object() && j.contains(key) && !j[key].is_null()) {
        // 如果 j[key] 是数组，遍历数组元素
        if (j[key].is_array()) {
            v.clear();
            for (const auto& elem : j[key]) {
                T obj;
                obj.reflectFromJson(elem);
                v.insert(obj);
            }
        }
        // 如果 j[key] 是对象，遍历对象的值（values）
        else if (j[key].is_object()) {
            v.clear();
            for (auto it = j[key].begin(); it != j[key].end(); ++it) {
                T obj;
                obj.reflectFromJson(it.value());
                v.insert(obj);
            }
        }
        // 如果 j[key] 既不是数组也不是对象，容器已清空
    }
    // 如果 key 不存在，容器已清空
}

// unordered_set<T>: fromJson (T 没有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<!has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::unordered_set<T>& v) {
    v.clear();
    
    // 如果 key 为空或 JSON 本身就是数组，直接处理 j
    if (key.empty() && j.is_array()) {
        for (const auto& elem : j) {
            T obj;
            fromJson(elem, "", obj);
            v.insert(obj);
        }
    }
    // 从对象中获取指定 key 的值
    else if (j.is_object() && j.contains(key) && !j[key].is_null()) {
        // 如果 j[key] 是数组，遍历数组元素
        if (j[key].is_array()) {
            v.clear();
            for (const auto& elem : j[key]) {
                T obj;
                fromJson(elem, "", obj);
                v.insert(obj);
            }
        }
        // 如果 j[key] 是对象，遍历对象的值（values）
        else if (j[key].is_object()) {
            v.clear();
            for (auto it = j[key].begin(); it != j[key].end(); ++it) {
                T obj;
                fromJson(it.value(), "", obj);
                v.insert(obj);
            }
        }
        // 如果 j[key] 既不是数组也不是对象，容器已清空
    }
    // 如果 key 不存在，容器已清空
}

// ============================================================================
// STL 容器支持：map
// ============================================================================

// map<string, T>: toJson
template<typename T>
void toJson(json& j, const std::map<std::string, T>& m) {
    for (const auto& pair : m) {
        json obj;
        toJson(obj, pair.second);
        j[pair.first] = obj;
    }
}

// map<string, T>: fromJson (T 有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::map<std::string, T>& m) {
    m.clear();
    // 如果 key 为空且 JSON 本身就是对象，直接处理 j
    if (key.empty() && j.is_object()) {
        for (auto it = j.begin(); it != j.end(); ++it) {
            T obj;
            obj.reflectFromJson(it.value());
            m[it.key()] = obj;
        }
    }
    // 否则从对象中获取指定 key 的值
    else if (j.contains(key) && !j[key].is_null() && j[key].is_object()) {
        for (auto it = j[key].begin(); it != j[key].end(); ++it) {
            T obj;
            obj.reflectFromJson(it.value());
            m[it.key()] = obj;
        }
    }
}

// map<string, T>: fromJson (T 没有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<!has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::map<std::string, T>& m) {
    m.clear();
    // 如果 key 为空且 JSON 本身就是对象，直接处理 j
    if (key.empty() && j.is_object()) {
        for (auto it = j.begin(); it != j.end(); ++it) {
            T obj;
            fromJson(it.value(), "", obj);
            m[it.key()] = obj;
        }
    }
    // 否则从对象中获取指定 key 的值
    else if (j.contains(key) && !j[key].is_null() && j[key].is_object()) {
        for (auto it = j[key].begin(); it != j[key].end(); ++it) {
            T obj;
            fromJson(it.value(), "", obj);
            m[it.key()] = obj;
        }
    }
}

// map<string, shared_ptr<T>>: toJson
template<typename T>
void toJson(json& j, const std::map<std::string, std::shared_ptr<T>>& m) {
    for (const auto& pair : m) {
        json obj;
        toJson(obj, pair.second);
        j[pair.first] = obj;
    }
}

// map<string, shared_ptr<T>>: fromJson (T 有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::map<std::string, std::shared_ptr<T>>& m) {
    m.clear();
    // 如果 key 为空且 JSON 本身就是对象，直接处理 j
    if (key.empty() && j.is_object()) {
        for (auto it = j.begin(); it != j.end(); ++it) {
            std::shared_ptr<T> obj = std::make_shared<T>();
            obj->reflectFromJson(it.value());
            m[it.key()] = std::move(obj);
        }
    }
    // 否则从对象中获取指定 key 的值
    else if (j.contains(key) && !j[key].is_null() && j[key].is_object()) {
        for (auto it = j[key].begin(); it != j[key].end(); ++it) {
            std::shared_ptr<T> obj = std::make_shared<T>();
            obj->reflectFromJson(it.value());
            m[it.key()] = std::move(obj);
        }
    }
}

// map<string, shared_ptr<T>>: fromJson (T 没有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<!has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::map<std::string, std::shared_ptr<T>>& m) {
    m.clear();
    // 如果 key 为空且 JSON 本身就是对象，直接处理 j
    if (key.empty() && j.is_object()) {
        for (auto it = j.begin(); it != j.end(); ++it) {
            std::shared_ptr<T> obj = std::make_shared<T>();
            fromJson(it.value(), "", *obj);
            m[it.key()] = std::move(obj);
        }
    }
    // 否则从对象中获取指定 key 的值
    else if (j.contains(key) && !j[key].is_null() && j[key].is_object()) {
        for (auto it = j[key].begin(); it != j[key].end(); ++it) {
            std::shared_ptr<T> obj = std::make_shared<T>();
            fromJson(it.value(), "", *obj);
            m[it.key()] = std::move(obj);
        }
    }
}

// map<string, unique_ptr<T>>: toJson
template<typename T>
void toJson(json& j, const std::map<std::string, std::unique_ptr<T>>& m) {
    for (const auto& pair : m) {
        json obj;
        toJson(obj, pair.second);
        j[pair.first] = obj;
    }
}

// map<string, unique_ptr<T>>: fromJson (T 有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::map<std::string, std::unique_ptr<T>>& m) {
    m.clear();
    // 如果 key 为空且 JSON 本身就是对象，直接处理 j
    if (key.empty() && j.is_object()) {
        for (auto it = j.begin(); it != j.end(); ++it) {
            std::unique_ptr<T> obj = std::make_unique<T>();
            obj->reflectFromJson(it.value());
            m[it.key()] = std::move(obj);
        }
    }
    // 否则从对象中获取指定 key 的值
    else if (j.contains(key) && !j[key].is_null() && j[key].is_object()) {
        for (auto it = j[key].begin(); it != j[key].end(); ++it) {
            std::unique_ptr<T> obj = std::make_unique<T>();
            obj->reflectFromJson(it.value());
            m[it.key()] = std::move(obj);
        }
    }
}

// map<string, unique_ptr<T>>: fromJson (T 没有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<!has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::map<std::string, std::unique_ptr<T>>& m) {
    m.clear();
    // 如果 key 为空且 JSON 本身就是对象，直接处理 j
    if (key.empty() && j.is_object()) {
        for (auto it = j.begin(); it != j.end(); ++it) {
            std::unique_ptr<T> obj = std::make_unique<T>();
            fromJson(it.value(), "", *obj);
            m[it.key()] = std::move(obj);
        }
    }
    // 否则从对象中获取指定 key 的值
    else if (j.contains(key) && !j[key].is_null() && j[key].is_object()) {
        for (auto it = j[key].begin(); it != j[key].end(); ++it) {
            std::unique_ptr<T> obj = std::make_unique<T>();
            fromJson(it.value(), "", *obj);
            m[it.key()] = std::move(obj);
        }
    }
}

// ============================================================================
// STL 容器支持：unordered_map
// ============================================================================

// unordered_map<string, T>: toJson
template<typename T>
void toJson(json& j, const std::unordered_map<std::string, T>& m) {
    for (const auto& pair : m) {
        json obj;
        toJson(obj, pair.second);
        j[pair.first] = obj;
    }
}

// unordered_map<string, T>: fromJson (T 有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::unordered_map<std::string, T>& m) {
    m.clear();
    // 如果 key 为空且 JSON 本身就是对象，直接处理 j
    if (key.empty() && j.is_object()) {
        for (auto it = j.begin(); it != j.end(); ++it) {
            T obj;
            obj.reflectFromJson(it.value());
            m[it.key()] = obj;
        }
    }
    // 否则从对象中获取指定 key 的值
    else if (j.contains(key) && !j[key].is_null() && j[key].is_object()) {
        for (auto it = j[key].begin(); it != j[key].end(); ++it) {
            T obj;
            obj.reflectFromJson(it.value());
            m[it.key()] = obj;
        }
    }
}

// unordered_map<string, T>: fromJson (T 没有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<!has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::unordered_map<std::string, T>& m) {
    m.clear();
    // 如果 key 为空且 JSON 本身就是对象，直接处理 j
    if (key.empty() && j.is_object()) {
        for (auto it = j.begin(); it != j.end(); ++it) {
            T obj;
            fromJson(it.value(), "", obj);
            m[it.key()] = obj;
        }
    }
    // 否则从对象中获取指定 key 的值
    else if (j.contains(key) && !j[key].is_null() && j[key].is_object()) {
        for (auto it = j[key].begin(); it != j[key].end(); ++it) {
            T obj;
            fromJson(it.value(), "", obj);
            m[it.key()] = obj;
        }
    }
}

// unordered_map<string, shared_ptr<T>>: toJson
template<typename T>
void toJson(json& j, const std::unordered_map<std::string, std::shared_ptr<T>>& m) {
    for (const auto& pair : m) {
        json obj;
        toJson(obj, pair.second);
        j[pair.first] = obj;
    }
}

// unordered_map<string, shared_ptr<T>>: fromJson (T 有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::unordered_map<std::string, std::shared_ptr<T>>& m) {
    m.clear();
    // 如果 key 为空且 JSON 本身就是对象，直接处理 j
    if (key.empty() && j.is_object()) {
        for (auto it = j.begin(); it != j.end(); ++it) {
            std::shared_ptr<T> obj = std::make_shared<T>();
            obj->reflectFromJson(it.value());
            m[it.key()] = obj;
        }
    }
    // 否则从对象中获取指定 key 的值
    else if (j.contains(key) && !j[key].is_null() && j[key].is_object()) {
        for (auto it = j[key].begin(); it != j[key].end(); ++it) {
            std::shared_ptr<T> obj = std::make_shared<T>();
            obj->reflectFromJson(it.value());
            m[it.key()] = obj;
        }
    }
}

// unordered_map<string, shared_ptr<T>>: fromJson (T 没有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<!has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::unordered_map<std::string, std::shared_ptr<T>>& m) {
    m.clear();
    // 如果 key 为空且 JSON 本身就是对象，直接处理 j
    if (key.empty() && j.is_object()) {
        for (auto it = j.begin(); it != j.end(); ++it) {
            std::shared_ptr<T> obj = std::make_shared<T>();
            fromJson(it.value(), "", *obj);
            m[it.key()] = obj;
        }
    }
    // 否则从对象中获取指定 key 的值
    else if (j.contains(key) && !j[key].is_null() && j[key].is_object()) {
        for (auto it = j[key].begin(); it != j[key].end(); ++it) {
            std::shared_ptr<T> obj = std::make_shared<T>();
            fromJson(it.value(), "", *obj);
            m[it.key()] = obj;
        }
    }
}

// unordered_map<string, unique_ptr<T>>: toJson
template<typename T>
void toJson(json& j, const std::unordered_map<std::string, std::unique_ptr<T>>& m) {
    for (const auto& pair : m) {
        json obj;
        toJson(obj, pair.second);
        j[pair.first] = obj;
    }
}

// unordered_map<string, unique_ptr<T>>: fromJson (T 有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::unordered_map<std::string, std::unique_ptr<T>>& m) {
    m.clear();
    // 如果 key 为空且 JSON 本身就是对象，直接处理 j
    if (key.empty() && j.is_object()) {
        for (auto it = j.begin(); it != j.end(); ++it) {
            std::unique_ptr<T> obj = std::make_unique<T>();
            obj->reflectFromJson(it.value());
            m[it.key()] = std::move(obj);
        }
    }
    // 否则从对象中获取指定 key 的值
    else if (j.contains(key) && !j[key].is_null() && j[key].is_object()) {
        for (auto it = j[key].begin(); it != j[key].end(); ++it) {
            std::unique_ptr<T> obj = std::make_unique<T>();
            obj->reflectFromJson(it.value());
            m[it.key()] = std::move(obj);
        }
    }
}

// unordered_map<string, unique_ptr<T>>: fromJson (T 没有 reflectFromJson 方法)
template<typename T>
typename std::enable_if<!has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::unordered_map<std::string, std::unique_ptr<T>>& m) {
    m.clear();
    // 如果 key 为空且 JSON 本身就是对象，直接处理 j
    if (key.empty() && j.is_object()) {
        for (auto it = j.begin(); it != j.end(); ++it) {
            std::unique_ptr<T> obj = std::make_unique<T>();
            fromJson(it.value(), "", *obj);
            m[it.key()] = std::move(obj);
        }
    }
    // 否则从对象中获取指定 key 的值
    else if (j.contains(key) && !j[key].is_null() && j[key].is_object()) {
        for (auto it = j[key].begin(); it != j[key].end(); ++it) {
            std::unique_ptr<T> obj = std::make_unique<T>();
            fromJson(it.value(), "", *obj);
            m[it.key()] = std::move(obj);
        }
    }
}

// ============================================================================
// STL 容器支持：array
// ============================================================================

// array<T, N>: toJson
template<typename T, size_t N>
void toJson(json& j, const std::array<T, N>& a) {
    j = json::array();
    for (const auto& elem : a) {
        json obj;
        toJson(obj, elem);
        j.push_back(obj);
    }
}

// array<T, N>: fromJson (T 有 reflectFromJson 方法)
template<typename T, size_t N>
typename std::enable_if<has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::array<T, N>& a) {
    const json* arrayJson = nullptr;
    // 如果 key 为空或 JSON 本身就是数组，直接处理 j
    if (key.empty() && j.is_array()) {
        arrayJson = &j;
    }
    // 否则从对象中获取指定 key 的数组
    else if (j.is_object() && j.contains(key) && !j[key].is_null() && j[key].is_array()) {
        arrayJson = &j[key];
    }
    
    if (arrayJson) {
        size_t count = 0;
        for (auto it = arrayJson->begin(); it != arrayJson->end() && count < N; ++it, ++count) {
            a[count].reflectFromJson(it.value());
        }
    }
}

// array<T, N>: fromJson (T 没有 reflectFromJson 方法)
template<typename T, size_t N>
typename std::enable_if<!has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::array<T, N>& a) {
    const json* arrayJson = nullptr;
    // 如果 key 为空或 JSON 本身就是数组，直接处理 j
    if (key.empty() && j.is_array()) {
        arrayJson = &j;
    }
    // 否则从对象中获取指定 key 的数组
    else if (j.is_object() && j.contains(key) && !j[key].is_null() && j[key].is_array()) {
        arrayJson = &j[key];
    }
    
    if (arrayJson) {
        size_t count = 0;
        for (auto it = arrayJson->begin(); it != arrayJson->end() && count < N; ++it, ++count) {
            fromJson(it.value(), "", a[count]);
        }
    }
}

// array<shared_ptr<T>, N>: toJson
template<typename T, size_t N>
void toJson(json& j, const std::array<std::shared_ptr<T>, N>& a) {
    j = json::array();
    for (const auto& elem : a) {
        json obj;
        toJson(obj, elem);
        j.push_back(obj);
    }
}

// array<shared_ptr<T>, N>: fromJson (T 有 reflectFromJson 方法)
template<typename T, size_t N>
typename std::enable_if<has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::array<std::shared_ptr<T>, N>& a) {
    const json* arrayJson = nullptr;
    // 如果 key 为空或 JSON 本身就是数组，直接处理 j
    if (key.empty() && j.is_array()) {
        arrayJson = &j;
    }
    // 否则从对象中获取指定 key 的数组
    else if (j.is_object() && j.contains(key) && !j[key].is_null() && j[key].is_array()) {
        arrayJson = &j[key];
    }
    
    if (arrayJson) {
        size_t count = 0;
        for (auto it = arrayJson->begin(); it != arrayJson->end() && count < N; ++it, ++count) {
            if (!a[count])
                a[count] = std::make_shared<T>();
            a[count]->reflectFromJson(it.value());
        }
    }
}

// array<shared_ptr<T>, N>: fromJson (T 没有 reflectFromJson 方法)
template<typename T, size_t N>
typename std::enable_if<!has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::array<std::shared_ptr<T>, N>& a) {
    const json* arrayJson = nullptr;
    // 如果 key 为空或 JSON 本身就是数组，直接处理 j
    if (key.empty() && j.is_array()) {
        arrayJson = &j;
    }
    // 否则从对象中获取指定 key 的数组
    else if (j.is_object() && j.contains(key) && !j[key].is_null() && j[key].is_array()) {
        arrayJson = &j[key];
    }
    
    if (arrayJson) {
        size_t count = 0;
        for (auto it = arrayJson->begin(); it != arrayJson->end() && count < N; ++it, ++count) {
            if (!a[count])
                a[count] = std::make_shared<T>();
            fromJson(it.value(), "", *a[count]);
        }
    }
}

// array<unique_ptr<T>, N>: toJson
template<typename T, size_t N>
void toJson(json& j, const std::array<std::unique_ptr<T>, N>& a) {
    j = json::array();
    for (const auto& elem : a) {
        json obj;
        toJson(obj, elem);
        j.push_back(obj);
    }
}

// array<unique_ptr<T>, N>: fromJson (T 有 reflectFromJson 方法)
template<typename T, size_t N>
typename std::enable_if<has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::array<std::unique_ptr<T>, N>& a) {
    const json* arrayJson = nullptr;
    // 如果 key 为空或 JSON 本身就是数组，直接处理 j
    if (key.empty() && j.is_array()) {
        arrayJson = &j;
    }
    // 否则从对象中获取指定 key 的数组
    else if (j.is_object() && j.contains(key) && !j[key].is_null() && j[key].is_array()) {
        arrayJson = &j[key];
    }
    
    if (arrayJson) {
        size_t count = 0;
        for (auto it = arrayJson->begin(); it != arrayJson->end() && count < N; ++it, ++count) {
            a[count] = std::make_unique<T>();
            a[count]->reflectFromJson(it.value());
        }
    }
}

// array<unique_ptr<T>, N>: fromJson (T 没有 reflectFromJson 方法)
template<typename T, size_t N>
typename std::enable_if<!has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, std::array<std::unique_ptr<T>, N>& a) {
    const json* arrayJson = nullptr;
    // 如果 key 为空或 JSON 本身就是数组，直接处理 j
    if (key.empty() && j.is_array()) {
        arrayJson = &j;
    }
    // 否则从对象中获取指定 key 的数组
    else if (j.is_object() && j.contains(key) && !j[key].is_null() && j[key].is_array()) {
        arrayJson = &j[key];
    }
    
    if (arrayJson) {
        size_t count = 0;
        for (auto it = arrayJson->begin(); it != arrayJson->end() && count < N; ++it, ++count) {
            a[count] = std::make_unique<T>();
            fromJson(it.value(), "", *a[count]);
        }
    }
}

// ============================================================================
// 辅助函数（向后兼容）
// ============================================================================

template<typename T>
inline void getFromJson(const json& j, const std::string& key, T& value) {
    fromJson(j, key, value);
}

// ============================================================================
// 宏定义
// ============================================================================

inline const char* extractJsonKey(const char* name) {
    if (name && name[0] == 'm' && name[1] == '_' && name[2] != '\0') {
        return name + 2;
    }
    return name;
}

#define FROM_JSON(jsonValue, key) \
    fromJson(jsonValue, extractJsonKey(#key), key)

#define TO_JSON(jsonValue, key) \
    toJson(jsonValue[extractJsonKey(#key)], key)

#define FROM_JSON_KEY(jsonValue, key, value) \
    fromJson(jsonValue, key, value)

#define TO_JSON_KEY(jsonValue, key, value) \
    toJson(jsonValue[key], value)

#define FROM_JSON_AUTO(jsonValue, key) \
    fromJson(jsonValue, #key, m_##key)

#define FROM_JSON_AUTO_SAME_NAME(jsonValue, key) \
    fromJson(jsonValue, #key, key)

#define TO_JSON_AUTO(jsonValue, key) \
    toJson(jsonValue[#key], m_##key)

#define TO_JSON_AUTO_SAME_NAME(jsonValue, key) \
    toJson(jsonValue[#key], key)

#endif
