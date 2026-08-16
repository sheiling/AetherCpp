/**
 * @file param_change_parser.cpp
 * @brief ParamChangeParser：JSON 递归 diff 与 ParamChangeRecord 生成
 */

#include "param_change_parser.h"

#include <QSet>

#include <algorithm>

namespace aoi {
namespace {

using namespace parammeta;

// 将 nlohmann::json 值转换为统一可落盘字符串。
QString jsonValueToString(const json &v)
{
    if (v.is_null())
        return QStringLiteral("null");
    if (v.is_string())
        return QString::fromStdString(v.get<std::string>());
    if (v.is_boolean())
        return v.get<bool>() ? QStringLiteral("true") : QStringLiteral("false");
    if (v.is_number_integer())
        return QString::number(v.get<long long>());
    if (v.is_number_unsigned())
        return QString::number(v.get<unsigned long long>());
    if (v.is_number_float())
        return QString::number(v.get<double>());
    return QString::fromStdString(v.dump());
}

// 将 nlohmann::json 值映射为类型描述字符串。
QString jsonTypeName(const json &v)
{
    if (v.is_null())
        return QStringLiteral("null");
    if (v.is_boolean())
        return QStringLiteral("bool");
    if (v.is_number())
        return QStringLiteral("number");
    if (v.is_string())
        return QStringLiteral("string");
    if (v.is_array())
        return QStringLiteral("array");
    if (v.is_object())
        return QStringLiteral("object");
    return QStringLiteral("unknown");
}

// 将 labelTag 拼接到父级路径上，形成层级标签。
QString joinLabel(const QString &prefix, const char *labelTag)
{
    if (!labelTag || !*labelTag)
        return prefix;
    if (prefix.isEmpty())
        return QString::fromUtf8(labelTag);
    return prefix + QLatin1Char('.') + QString::fromUtf8(labelTag);
}

void collectDiffByMeta(const json                 &before,
                       const json                 &after,
                       const ParamTypeMeta        *meta,
                       const QString              &labelPrefix,
                       const QString              &patternPrefix,
                       QVector<ParamChangeRecord> &out);

void collectScalarDiff(const json                 &before,
                       const json                 &after,
                       const QString              &fullLabel,
                       const QString              &labelPattern,
                       QVector<ParamChangeRecord> &out)
{
    // 标量节点：值变化即生成一条记录。
    if (before == after)
        return;
    ParamChangeRecord rec;
    rec.fullLabel    = fullLabel.isEmpty() ? QStringLiteral("(root)") : fullLabel;
    rec.labelPattern = labelPattern.isEmpty() ? rec.fullLabel : labelPattern;
    rec.beforeValue  = jsonValueToString(before);
    rec.afterValue   = jsonValueToString(after);
    rec.valueType    = jsonTypeName(after.is_null() ? before : after);
    out.append(rec);
}

void collectFallbackDiff(const json                 &before,
                         const json                 &after,
                         const QString              &labelPrefix,
                         const QString              &patternPrefix,
                         QVector<ParamChangeRecord> &out)
{
    // 兜底路径：当没有类型元数据时，按 JSON 结构递归比对。
    if (before == after)
        return;

    if (before.is_object() && after.is_object()) {
        QSet<QString> keys;
        for (auto it = before.begin(); it != before.end(); ++it)
            keys.insert(QString::fromStdString(it.key()));
        for (auto it = after.begin(); it != after.end(); ++it)
            keys.insert(QString::fromStdString(it.key()));
        for (const QString &key : keys) {
            const std::string stdKey    = key.toStdString();
            const json        beforeVal = before.contains(stdKey) ? before.at(stdKey) : json();
            const json        afterVal  = after.contains(stdKey) ? after.at(stdKey) : json();
            const QString childLabel =
                labelPrefix.isEmpty() ? key : labelPrefix + QLatin1Char('.') + key;
            collectFallbackDiff(beforeVal, afterVal, childLabel, childLabel, out);
        }
        return;
    }

    if (before.is_array() && after.is_array()) {
        const size_t count = std::max(before.size(), after.size());
        for (size_t i = 0; i < count; ++i) {
            const json    beforeVal = i < before.size() ? before.at(i) : json();
            const json    afterVal  = i < after.size() ? after.at(i) : json();
            const QString indexSeg  = QStringLiteral("[%1]").arg(static_cast<int>(i));
            const QString childLabel = labelPrefix + indexSeg;
            const QString childPattern =
                (patternPrefix.isEmpty() ? labelPrefix : patternPrefix) + QStringLiteral("[*]");
            collectFallbackDiff(beforeVal, afterVal, childLabel, childPattern, out);
        }
        return;
    }

    collectScalarDiff(before, after, labelPrefix, patternPrefix, out);
}

void collectMapDiff(const json                 &before,
                    const json                 &after,
                    const ParamFieldMeta       &field,
                    const QString              &fieldLabel,
                    const QString              &fieldPattern,
                    QVector<ParamChangeRecord> &out)
{
    // map 节点：按 key 递归；labelPattern 使用 {*} 聚合。
    if (!before.is_object() || !after.is_object()) {
        collectScalarDiff(before, after, fieldLabel, fieldPattern, out);
        return;
    }

    QSet<QString> keys;
    for (auto it = before.begin(); it != before.end(); ++it)
        keys.insert(QString::fromStdString(it.key()));
    for (auto it = after.begin(); it != after.end(); ++it)
        keys.insert(QString::fromStdString(it.key()));

    for (const QString &key : keys) {
        const std::string stdKey    = key.toStdString();
        const json        beforeVal = before.contains(stdKey) ? before.at(stdKey) : json();
        const json        afterVal  = after.contains(stdKey) ? after.at(stdKey) : json();
        const QString childLabel    = fieldLabel + QStringLiteral("{%1}").arg(key);
        const QString childPattern  = fieldPattern + QStringLiteral("{*}");
        if (field.childType) {
            collectDiffByMeta(beforeVal, afterVal, field.childType, childLabel, childPattern, out);
        } else {
            collectFallbackDiff(beforeVal, afterVal, childLabel, childPattern, out);
        }
    }
}

void collectSequenceDiff(const json                 &before,
                         const json                 &after,
                         const ParamFieldMeta       &field,
                         const QString              &fieldLabel,
                         const QString              &fieldPattern,
                         QVector<ParamChangeRecord> &out)
{
    // 序列节点：按索引递归；labelPattern 使用 [*] 聚合。
    if (!before.is_array() || !after.is_array()) {
        collectScalarDiff(before, after, fieldLabel, fieldPattern, out);
        return;
    }

    const size_t count = std::max(before.size(), after.size());
    for (size_t i = 0; i < count; ++i) {
        const json    beforeVal = i < before.size() ? before.at(i) : json();
        const json    afterVal  = i < after.size() ? after.at(i) : json();
        const QString childLabel = fieldLabel + QStringLiteral("[%1]").arg(static_cast<int>(i));
        const QString childPattern = fieldPattern + QStringLiteral("[*]");
        if (field.childType) {
            collectDiffByMeta(beforeVal, afterVal, field.childType, childLabel, childPattern, out);
        } else {
            collectFallbackDiff(beforeVal, afterVal, childLabel, childPattern, out);
        }
    }
}

void collectDiffByMeta(const json                 &before,
                       const json                 &after,
                       const ParamTypeMeta        *meta,
                       const QString              &labelPrefix,
                       const QString              &patternPrefix,
                       QVector<ParamChangeRecord> &out)
{
    // 主路径：优先依据元数据识别字段类别，保证标签语义稳定。
    if (before == after)
        return;

    if (!meta || !before.is_object() || !after.is_object()) {
        collectFallbackDiff(before, after, labelPrefix, patternPrefix, out);
        return;
    }

    for (const ParamFieldMeta &field : meta->fields) {
        const std::string key = field.jsonKey ? std::string(field.jsonKey) : std::string();
        if (key.empty())
            continue;

        const json beforeVal = before.contains(key) ? before.at(key) : json();
        const json afterVal  = after.contains(key) ? after.at(key) : json();
        const QString fieldLabel = joinLabel(labelPrefix, field.labelTag);
        const QString fieldPattern = joinLabel(patternPrefix, field.labelTag);

        switch (field.valueKind) {
        case ParamValueKind::Scalar:
            collectScalarDiff(beforeVal, afterVal, fieldLabel, fieldPattern, out);
            break;
        case ParamValueKind::Object:
            collectDiffByMeta(beforeVal, afterVal, field.childType, fieldLabel, fieldPattern, out);
            break;
        case ParamValueKind::Sequence:
            collectSequenceDiff(beforeVal, afterVal, field, fieldLabel, fieldPattern, out);
            break;
        case ParamValueKind::Map:
            collectMapDiff(beforeVal, afterVal, field, fieldLabel, fieldPattern, out);
            break;
        }
    }
}

} // namespace

QVector<parammeta::ParamChangeRecord> ParamChangeParser::parseJson(
    const json                     &before,
    const json                     &after,
    const parammeta::ParamTypeMeta *meta,
    const QString                  &rootLabel) const
{
    // 解析器的唯一输出：标准化变更记录集合。
    QVector<parammeta::ParamChangeRecord> changes;
    if (before == after)
        return changes;
    collectDiffByMeta(before, after, meta, rootLabel, rootLabel, changes);
    return changes;
}

} // namespace aoi
