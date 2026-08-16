#pragma once

/**
 * @file datastore.h
 * @brief 检测历史 DataStore 全局生命周期管理
 */

#include "repository/interface/idatastore.h"
#include "repository/storageconfig.h"

namespace lighten {

/**
 * @brief 检测历史数据存储全局入口
 *
 * 应用启动时 configure → initialize；业务层通过 instance() 访问仓储。
 */
class DataStore
{
public:
    /** 注入存储配置（须在 initialize 之前调用） */
    static void configure(const StorageConfig &config);
    /** 返回当前生效的配置副本 */
    static StorageConfig currentConfig();

    /** 按配置创建并打开 IDataStore，成功后可 instance() */
    static bool initialize(QString *errorString = nullptr);
    /** 当前已打开的 IDataStore，未初始化时返回 nullptr */
    static IDataStore *instance();
    /** 关闭并释放全局实例 */
    static void shutdown();

private:
    DataStore() = delete;
};

} // namespace lighten
