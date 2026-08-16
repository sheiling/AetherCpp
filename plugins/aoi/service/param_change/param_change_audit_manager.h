#pragma once

/**
 * @file param_change_audit_manager.h
 * @brief 参数审计单例：解析器 + 存储后端编排
 */

#include "param_change_parser.h"
#include "param_change_store.h"

#include <memory>

namespace aoi {

/**
 * @brief 参数审计管理器（解析器 + 存储后端编排）
 *
 * 启动时 configure() 注入存储策略；业务侧调用 audit(before, after, ctx) 即可。
 */
class ParamChangeAuditManager
{
public:
    static ParamChangeAuditManager &instance();

    /** 按 ParamStoreMode 重建 DB/File 后端列表 */
    void configure(const ParamStoreConfig &config);

    /** 单测或特殊场景：直接注入自定义 store 列表 */
    void setStores(const QVector<std::shared_ptr<IParamChangeStore>> &stores);

    /**
     * @brief 解析并落盘一次参数变更
     *
     * ctx.documentType / rootLabel 为空时由模板类型 T 自动推导。
     */
    template <typename T>
    void audit(const T &before, const T &after, ParamAuditContext ctx = {})
    {
        if (ctx.documentType.isEmpty())
            ctx.documentType = parammeta::documentTypeName<T>();
        if (ctx.rootLabel.isEmpty())
            ctx.rootLabel = parammeta::rootLabel<T>();

        const QVector<parammeta::ParamChangeRecord> changes = m_parser.parse(before, after, ctx.rootLabel);
        persist(ctx, changes);
    }

private:
    ParamChangeAuditManager();
    void persist(const ParamAuditContext &ctx,
                 const QVector<parammeta::ParamChangeRecord> &changes);

    QVector<std::shared_ptr<IParamChangeStore>> m_stores;
    ParamChangeParser                             m_parser;
    ParamStoreConfig                              m_config;
};

} // namespace aoi
