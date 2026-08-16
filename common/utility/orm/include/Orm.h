/**
 * @file    Orm.h
 * @brief   ORM 模块统一入口头文件
 *
 * 业务代码只需包含此一个头文件：
 * @code
 *   #include <Orm.h>
 * @endcode
 *
 * 提供：
 *   - orm::DbConfig       — 数据库连接配置
 *   - orm::DbManager      — 连接管理器（单例）
 *   - orm::Repository<T>  — 泛型 CRUD Repository
 *   - ORM_ENTITY_HPP / ORM_ENTITY_CPP / ORM_REGISTER — 模型注册宏
 *   - orm::NoBase         — 无基类占位符
 */
#pragma once

#include "orm_global.h"
#include "DbConfig.h"
#include "DbManager.h"
#include "Repository.h"
