/**
 * @file    Department.h
 * @brief   部门实体模型（SQLite 示例）
 */
#pragma once

#include <QxOrm.h>

#include <QtCore/QString>
#include <memory>
#include <vector>

// 前向声明（避免循环引用）
struct User;

// ─────────────────────────────────────────────────────────────────────────────
// 实体定义
// ─────────────────────────────────────────────────────────────────────────────
struct Department
{
    long    id   = 0;
    QString name;
    QString code;           // 部门编号

    // OneToMany 关联：一个部门有多名用户
    std::vector<std::shared_ptr<User>> users;

    Department() = default;
    Department(const QString &name_, const QString &code_)
        : name(name_), code(code_) {}
};

// ─────────────────────────────────────────────────────────────────────────────
// QxOrm 注册声明（头文件中）
// ─────────────────────────────────────────────────────────────────────────────
QX_REGISTER_HPP(Department, qx::trait::no_base_class_defined, 0)
QX_REGISTER_SERIALIZE_QDATASTREAM_HPP(, Department)

namespace qx {
template <> void register_class(QxClass<Department> &t);
}