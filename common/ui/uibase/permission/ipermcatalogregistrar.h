#pragma once

#include "uibase_global.h"

#include <QString>

/**
 * @brief 权限资源目录注册后端（由 Permission 插件实现）
 *
 * 供业务插件在运行时向 perm_permission 注册 nav / sub_page / action，
 * 替代手写 SQL 种子。注册幂等：key 已存在则跳过。
 *
 * i18n 约定（详见 plugins/aoi/docs/knowledge-base/i18n/权限资源与翻译约定.md）：
 * - 参数 displayName：存 i18n source key 字符串，禁止传 TR() 译文。
 * - Nav/SubPage：key 与 displayName 通常相同（如 aoi.nav.home）。
 * - Action：key 为业务短名（如 home.start），displayName 为对应插件文案 key（如 aoi.sidebar.btn.start）。
 * - 展示层 permissionResourceDisplayText：TR(key) → TR(display_name) → 回退原文。
 */
class UIBASE_EXPORT IPermCatalogRegistrar
{
public:
    virtual ~IPermCatalogRegistrar() = default;

    virtual bool isReady() const = 0;

    /// 顶栏导航；displayName 为 i18n source key（通常与 key 相同）
    virtual bool registerNav(const QString &key, const QString &displayName, int sortOrder = 0) = 0;

    /// 二级子页；parentNavKey 为所属 nav 的全局 key；displayName 为 i18n source key
    virtual bool registerSubPage(const QString &key, const QString &parentNavKey,
                                 const QString &displayName, int sortOrder = 0) = 0;

    /// 操作；pageKey 为所属 nav 或 sub_page 的全局 key；displayName 为 i18n source key（非 TR 结果）
    virtual bool registerAction(const QString &key, const QString &pageKey,
                                const QString &displayName) = 0;
};
