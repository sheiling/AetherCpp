#pragma once

/**
 * @file param_change_store.h
 * @brief 参数变更审计：存储模式、上下文与 IParamChangeStore 接口
 */

#include "param_meta.h"

#include <QString>
#include <QVector>

namespace aoi {

/**
 * @brief 参数修订落盘模式
 *
 * - None: 禁用审计
 * - Database: 仅写 ORM 数据库
 * - File: 仅写文本日志
 * - Dual: 数据库 + 文件双写
 */
enum class ParamStoreMode {
    None,
    Database,
    File,
    Dual,
};

/** 插件启动时由 AoiPlugin 注入的审计后端配置 */
struct ParamStoreConfig
{
    ParamStoreMode mode = ParamStoreMode::Database;
    QString dbPath;   ///< 数据库绝对路径；空则使用默认 {exe}/data/record.db
    QString fileDir;  ///< 文本日志目录；空则使用默认目录
};

/** 单次审计请求的上下文（与 ParamChangeRecord 一并落盘） */
struct ParamAuditContext
{
    QString moduleCode;    ///< 业务模块编码
    QString documentType;  ///< 文档类型；空则由模板类型自动推导
    QString documentId;    ///< 文档 ID（如配方 id）
    QString sourcePage;    ///< 来源页面，便于检索
    QString rootLabel;     ///< 根标签路径；空则由模板 rootLabel 推导
};

/** 参数变更存储后端抽象；DB/File 实现在 repository/param_change/ */
class IParamChangeStore
{
public:
    virtual ~IParamChangeStore() = default;

    /**
     * @brief 保存参数变更记录
     * @param ctx     上下文（模块、文档、来源页面等）
     * @param changes 变更明细列表（已由 parser 生成）
     */
    virtual void save(const ParamAuditContext &ctx,
                      const QVector<parammeta::ParamChangeRecord> &changes) = 0;
};

} // namespace aoi
