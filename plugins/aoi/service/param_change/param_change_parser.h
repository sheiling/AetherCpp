#pragma once

/**
 * @file param_change_parser.h
 * @brief 参数变更解析器：before/after 文档 → ParamChangeRecord 列表（纯解析，不落盘）
 */

#include "param_meta.h"

namespace aoi {

/**
 * @brief 基于类型元数据递归对比 JSON，生成标准化变更记录
 *
 * 责任边界：
 * - 输入 before/after 文档对象
 * - 基于类型元数据 + JSON 递归对比生成 ParamChangeRecord 列表
 * - 不关心结果写入数据库/文件/网络，便于后端自由替换
 */
class ParamChangeParser
{
public:
    /**
     * @brief 对比两个同类型文档对象
     *
     * 内部 reflectToJson 后走 parseJson；rootLabel 空则用 parammeta::rootLabel<T>()。
     */
    template <typename T>
    QVector<parammeta::ParamChangeRecord> parse(const T &before,
                                                const T &after,
                                                const QString &rootLabel = QString()) const
    {
        json beforeJ;
        json afterJ;
        before.reflectToJson(beforeJ);
        after.reflectToJson(afterJ);
        const auto *meta = parammeta::ParamTypeMetaProvider<T>::get();
        const QString root = rootLabel.isEmpty() ? parammeta::rootLabel<T>() : rootLabel;
        return parseJson(beforeJ, afterJ, meta, root);
    }

    /** 直接对比两份 JSON 与类型元数据（供非模板场景或单测） */
    QVector<parammeta::ParamChangeRecord> parseJson(const json &before,
                                                    const json &after,
                                                    const parammeta::ParamTypeMeta *meta,
                                                    const QString &rootLabel) const;
};

} // namespace aoi
