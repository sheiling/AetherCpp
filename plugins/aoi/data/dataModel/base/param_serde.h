#pragma once

#include <common/utility/json_tool/reflector.h>

#include <QtGlobal>

#include <QVector>
#include <QString>

#include <type_traits>

namespace aoi::serde {

template <typename T>
bool eqField(const T &a, const T &b)
{
    return a == b;
}

inline bool eqField(double a, double b)
{
    return qFuzzyCompare(a, b);
}

void fromJson(const json &j, const std::string &key, QString &out);
void toJson(json &j, const QString &val);

template <typename T>
void toJson(json &j, const QVector<T> &vec)
{
    j = json::array();
    for (const auto &elem : vec) {
        json item;
        ::toJson(item, elem);
        j.push_back(item);
    }
}

template <typename T>
typename std::enable_if<has_reflectFromJson<T>::value, void>::type
fromJson(const json &j, const std::string &key, QVector<T> &vec)
{
    vec.clear();
    const json *arr = nullptr;
    if (key.empty() && j.is_array())
        arr = &j;
    else if (j.is_object() && j.contains(key) && j.at(key).is_array())
        arr = &j.at(key);

    if (!arr)
        return;

    vec.reserve(static_cast<int>(arr->size()));
    for (const auto &elem : *arr) {
        T obj;
        obj.reflectFromJson(elem);
        vec.append(obj);
    }
}

template <typename T>
typename std::enable_if<!has_reflectFromJson<T>::value && !std::is_enum<T>::value, void>::type
fromJson(const json &j, const std::string &key, QVector<T> &vec)
{
    vec.clear();
    const json *arr = nullptr;
    if (key.empty() && j.is_array())
        arr = &j;
    else if (j.is_object() && j.contains(key) && j.at(key).is_array())
        arr = &j.at(key);

    if (!arr)
        return;

    vec.reserve(static_cast<int>(arr->size()));
    for (const auto &elem : *arr) {
        T obj{};
        ::fromJson(elem, "", obj);
        vec.append(obj);
    }
}

} // namespace aoi::serde

template <typename EnumT>
typename std::enable_if<std::is_enum<EnumT>::value && !has_nlohmann_from_json<EnumT>::value,
                        void>::type
fromJson(const json &j, const std::string &key, EnumT &out)
{
    int v = static_cast<int>(out);
    fromJson(j, key, v);
    out = static_cast<EnumT>(v);
}

template <typename EnumT>
typename std::enable_if<std::is_enum<EnumT>::value && !has_nlohmann_to_json<EnumT>::value,
                        void>::type
toJson(json &j, const EnumT &val)
{
    j = static_cast<int>(val);
}

inline void fromJson(const json &j, const std::string &key, QString &out)
{
    aoi::serde::fromJson(j, key, out);
}

inline void toJson(json &j, const QString &val)
{
    aoi::serde::toJson(j, val);
}

template <typename T>
void toJson(json &j, const QVector<T> &vec)
{
    aoi::serde::toJson(j, vec);
}

template <typename T>
void fromJson(const json &j, const std::string &key, QVector<T> &vec)
{
    aoi::serde::fromJson(j, key, vec);
}
