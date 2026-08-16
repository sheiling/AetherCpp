/**
 * @file qt_json_adapters.h
 * @brief Qt 类型与 nlohmann::json 互转适配（QString / QDateTime / QMap）
 */
#pragma once

#include <QDateTime>
#include <QMap>
#include <QPointF>
#include <QString>
#include <QVector>

#include <common/utility/json_tool/reflector.h>

namespace nlohmann {

template <>
struct adl_serializer<QString>
{
    static void to_json(json &j, const QString &value)
    {
        j = value.toStdString();
    }

    static void from_json(const json &j, QString &value)
    {
        if (j.is_string())
            value = QString::fromStdString(j.get<std::string>());
        else
            value = QString();
    }
};

template <>
struct adl_serializer<QDateTime>
{
    static void to_json(json &j, const QDateTime &value)
    {
        j = value.isValid() ? value.toString(Qt::ISODateWithMs).toStdString() : std::string();
    }

    static void from_json(const json &j, QDateTime &value)
    {
        if (!j.is_string()) {
            value = QDateTime();
            return;
        }
        const QString text = QString::fromStdString(j.get<std::string>());
        value              = QDateTime::fromString(text, Qt::ISODateWithMs);
        if (!value.isValid())
            value = QDateTime::fromString(text, Qt::ISODate);
    }
};

template <>
struct adl_serializer<QPointF>
{
    static void to_json(json &j, const QPointF &value)
    {
        j = json{{"x", value.x()}, {"y", value.y()}};
    }

    static void from_json(const json &j, QPointF &value)
    {
        if (j.is_object()) {
            if (j.contains("x"))
                value.setX(j.at("x").get<qreal>());
            if (j.contains("y"))
                value.setY(j.at("y").get<qreal>());
            return;
        }
        if (j.is_array() && j.size() >= 2) {
            value.setX(j.at(0).get<qreal>());
            value.setY(j.at(1).get<qreal>());
        }
    }
};

} // namespace nlohmann

namespace json_reflect {

inline void toJson(json &j, const QPointF &obj) noexcept
{
    j = json{{"x", obj.x()}, {"y", obj.y()}};
}

inline void fromJson(const json &j, const std::string &key, QPointF &obj,
                     bool /*clearContainer*/ = true) noexcept
{
    getFromJson(j, key, obj);
}

inline void toJson(json &j, const QString &obj) noexcept
{
    j = obj.toStdString();
}

inline void fromJson(const json &j, const std::string &key, QString &obj, bool /*clearContainer*/ = true) noexcept
{
    getFromJson(j, key, obj);
}

inline void toJson(json &j, const QDateTime &obj) noexcept
{
    j = obj.isValid() ? obj.toString(Qt::ISODateWithMs).toStdString() : std::string();
}

inline void fromJson(const json &j, const std::string &key, QDateTime &obj, bool /*clearContainer*/ = true) noexcept
{
    if (!key.empty() && !j.contains(key))
        return;

    const json &targetJson = key.empty() ? j : j.at(key);
    if (!targetJson.is_string()) {
        obj = QDateTime();
        return;
    }

    const QString text = QString::fromStdString(targetJson.get<std::string>());
    obj              = QDateTime::fromString(text, Qt::ISODateWithMs);
    if (!obj.isValid())
        obj = QDateTime::fromString(text, Qt::ISODate);
}

template <typename V>
inline void toJson(json &j, const QMap<QString, V> &obj) noexcept
{
    j = json::object();
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
        json subJson;
        toJson(subJson, it.value());
        j[it.key().toStdString()] = subJson;
    }
}

template <typename V>
inline void fromJson(const json &j, const std::string &key, QMap<QString, V> &obj,
                     bool clearContainer = true) noexcept
{
    if (!key.empty() && !j.contains(key))
        return;

    if (clearContainer)
        obj.clear();

    const json &targetJson = key.empty() ? j : j.at(key);
    if (!targetJson.is_object())
        return;

    for (const auto &[k, v] : targetJson.items()) {
        typename QMap<QString, V>::mapped_type subObj;
        fromJson(v, "", subObj, clearContainer);
        obj[QString::fromStdString(k)] = subObj;
    }
}

} // namespace json_reflect

// ============================================================================
// 全局 toJson/fromJson（供 reflector.h 宏使用）
// ============================================================================

inline void toJson(json& j, const QString& value)
{
    j = value.toStdString();
}

inline void fromJson(const json& j, const std::string& key, QString& value)
{
    if (key.empty() && !j.is_null() && j.is_string()) {
        value = QString::fromStdString(j.get<std::string>());
    } else if (j.contains(key) && !j[key].is_null() && j[key].is_string()) {
        value = QString::fromStdString(j[key].get<std::string>());
    }
}

inline void toJson(json& j, const QDateTime& value)
{
    j = value.isValid() ? value.toString(Qt::ISODateWithMs).toStdString() : std::string();
}

inline void fromJson(const json& j, const std::string& key, QDateTime& value)
{
    const json* target = nullptr;
    if (key.empty() && j.is_string())
        target = &j;
    else if (j.contains(key) && j[key].is_string())
        target = &j[key];

    if (!target)
        return;

    const QString text = QString::fromStdString(target->get<std::string>());
    value              = QDateTime::fromString(text, Qt::ISODateWithMs);
    if (!value.isValid())
        value = QDateTime::fromString(text, Qt::ISODate);
}

inline void toJson(json& j, const QPointF& value)
{
    j = json{{"x", value.x()}, {"y", value.y()}};
}

inline void fromJson(const json& j, const std::string& key, QPointF& value)
{
    const json* target = nullptr;
    if (key.empty())
        target = &j;
    else if (j.contains(key))
        target = &j[key];

    if (!target || target->is_null())
        return;

    if (target->is_object()) {
        if (target->contains("x"))
            value.setX(target->at("x").get<qreal>());
        if (target->contains("y"))
            value.setY(target->at("y").get<qreal>());
        return;
    }
    if (target->is_array() && target->size() >= 2) {
        value.setX(target->at(0).get<qreal>());
        value.setY(target->at(1).get<qreal>());
    }
}

template<typename T>
void toJson(json& j, const QVector<T>& v)
{
    j = json::array();
    for (const auto& elem : v) {
        json obj;
        toJson(obj, elem);
        j.push_back(obj);
    }
}

template<typename T>
typename std::enable_if<has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, QVector<T>& v)
{
    v.clear();
    if (key.empty() && j.is_array()) {
        for (const auto& elem : j) {
            T obj;
            obj.reflectFromJson(elem);
            v.push_back(obj);
        }
    } else if (j.is_object() && j.contains(key) && !j[key].is_null()) {
        if (j[key].is_array()) {
            for (const auto& elem : j[key]) {
                T obj;
                obj.reflectFromJson(elem);
                v.push_back(obj);
            }
        } else if (j[key].is_object()) {
            for (auto it = j[key].begin(); it != j[key].end(); ++it) {
                T obj;
                obj.reflectFromJson(it.value());
                v.push_back(obj);
            }
        }
    }
}

template<typename T>
typename std::enable_if<!has_reflectFromJson<T>::value, void>::type
fromJson(const json& j, const std::string& key, QVector<T>& v)
{
    v.clear();
    if (key.empty() && j.is_array()) {
        for (const auto& elem : j) {
            T obj;
            fromJson(elem, "", obj);
            v.push_back(obj);
        }
    } else if (j.is_object() && j.contains(key) && !j[key].is_null()) {
        if (j[key].is_array()) {
            for (const auto& elem : j[key]) {
                T obj;
                fromJson(elem, "", obj);
                v.push_back(obj);
            }
        } else if (j[key].is_object()) {
            for (auto it = j[key].begin(); it != j[key].end(); ++it) {
                T obj;
                fromJson(it.value(), "", obj);
                v.push_back(obj);
            }
        }
    }
}

template<typename V>
void toJson(json& j, const QMap<QString, V>& m)
{
    for (auto it = m.constBegin(); it != m.constEnd(); ++it) {
        json obj;
        toJson(obj, it.value());
        j[it.key().toStdString()] = obj;
    }
}

template<typename V>
void fromJson(const json& j, const std::string& key, QMap<QString, V>& m)
{
    m.clear();
    if (key.empty() && j.is_object()) {
        for (auto it = j.begin(); it != j.end(); ++it) {
            V obj;
            fromJson(it.value(), "", obj);
            m[QString::fromStdString(it.key())] = obj;
        }
    } else if (j.contains(key) && !j[key].is_null() && j[key].is_object()) {
        for (auto it = j[key].begin(); it != j[key].end(); ++it) {
            V obj;
            fromJson(it.value(), "", obj);
            m[QString::fromStdString(it.key())] = obj;
        }
    }
}
