#pragma once

#include <common/utility/json_tool/reflector.h>

#include <QList>
#include <QMap>
#include <QString>
#include <QVector>

#include <map>
#include <typeinfo>
#include <vector>

namespace aoi::parammeta {

enum class ParamValueKind {
    Scalar,
    Object,
    Sequence,
    Map,
};

struct ParamFieldMeta;
struct ParamTypeMeta;

struct ParamFieldMeta
{
    const char          *jsonKey = "";
    const char          *labelTag = "";
    ParamValueKind       valueKind = ParamValueKind::Scalar;
    const ParamTypeMeta *childType = nullptr;
};

struct ParamTypeMeta
{
    const char            *typeName = "";
    const char            *labelTag = "";
    QVector<ParamFieldMeta> fields;
};

struct ParamChangeRecord
{
    QString fullLabel;
    QString labelPattern;
    QString beforeValue;
    QString afterValue;
    QString valueType;
};

template <typename T>
struct ParamTypeMetaProvider
{
    static const ParamTypeMeta *get() { return nullptr; }
};

template <typename T, typename = void>
struct ValueMetaResolver
{
    static ParamValueKind kind()
    {
        return ParamTypeMetaProvider<T>::get() ? ParamValueKind::Object : ParamValueKind::Scalar;
    }

    static const ParamTypeMeta *childType() { return ParamTypeMetaProvider<T>::get(); }
};

// In Qt6, QVector<T> is a type alias for QList<T> — the two specializations would clash.
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
template <typename T>
struct ValueMetaResolver<QVector<T>, void>
{
    static ParamValueKind kind() { return ParamValueKind::Sequence; }
    static const ParamTypeMeta *childType() { return ParamTypeMetaProvider<T>::get(); }
};
#endif

template <typename T>
struct ValueMetaResolver<QList<T>, void>
{
    static ParamValueKind kind() { return ParamValueKind::Sequence; }
    static const ParamTypeMeta *childType() { return ParamTypeMetaProvider<T>::get(); }
};

template <typename T>
struct ValueMetaResolver<std::vector<T>, void>
{
    static ParamValueKind kind() { return ParamValueKind::Sequence; }
    static const ParamTypeMeta *childType() { return ParamTypeMetaProvider<T>::get(); }
};

template <typename K, typename V>
struct ValueMetaResolver<QMap<K, V>, void>
{
    static ParamValueKind kind() { return ParamValueKind::Map; }
    static const ParamTypeMeta *childType() { return ParamTypeMetaProvider<V>::get(); }
};

template <typename K, typename V>
struct ValueMetaResolver<std::map<K, V>, void>
{
    static ParamValueKind kind() { return ParamValueKind::Map; }
    static const ParamTypeMeta *childType() { return ParamTypeMetaProvider<V>::get(); }
};

template <typename T>
QString documentTypeName()
{
    const ParamTypeMeta *meta = ParamTypeMetaProvider<T>::get();
    return (meta && meta->typeName && *meta->typeName) ? QString::fromUtf8(meta->typeName)
                                                        : QString::fromUtf8(typeid(T).name());
}

template <typename T>
QString rootLabel()
{
    const ParamTypeMeta *meta = ParamTypeMetaProvider<T>::get();
    return (meta && meta->labelTag && *meta->labelTag) ? QString::fromUtf8(meta->labelTag)
                                                        : documentTypeName<T>();
}

QString normalizeArrayPattern(const QString &label);
QString normalizeMapPattern(const QString &label);

} // namespace aoi::parammeta

#define PARAM_META_FIELD_ITEM(type, name, default_value, label_tag) \
    {#name, label_tag, ::aoi::parammeta::ValueMetaResolver<type>::kind(), \
     ::aoi::parammeta::ValueMetaResolver<type>::childType()},

#define DEFINE_PARAM_TYPE_META(type, root_label, fields_macro) \
    namespace aoi::parammeta { \
    template <> \
    struct ParamTypeMetaProvider<type> \
    { \
        static const ParamTypeMeta *get() \
        { \
            static const ParamTypeMeta meta{#type, root_label, {fields_macro(PARAM_META_FIELD_ITEM)}}; \
            return &meta; \
        } \
    }; \
    }
