#include "usermodel.h"

// 模拟数据库（静态数据）
QList<User> UserRepository::m_users = {
    User("张三", 25, "zhangsan@example.com", true),
    User("李四", 30, "lisi@example.com", true),
    User("王五", 28, "wangwu@example.com", false)
};

QList<User> UserRepository::getAllUsers()
{
    return m_users;
}

User UserRepository::getUserById(int id)
{
    if (id >= 0 && id < m_users.size()) {
        return m_users[id];
    }
    return User();
}

bool UserRepository::saveUser(const User &user)
{
    // 简化版：添加到列表末尾
    m_users.append(user);
    return true;
}

bool UserRepository::deleteUser(int id)
{
    if (id >= 0 && id < m_users.size()) {
        m_users.removeAt(id);
        return true;
    }
    return false;
}
