#ifndef USERMODEL_H
#define USERMODEL_H

#include <QString>
#include <QList>

/**
 * @brief 用户数据模型（Model层）
 * 
 * 纯数据类，不包含UI逻辑
 * 代表业务实体
 */
struct User
{
    QString name;
    int age;
    QString email;
    bool isActive;
    
    User() : age(0), isActive(true) {}
    
    User(const QString &n, int a, const QString &e, bool active = true)
        : name(n), age(a), email(e), isActive(active) {}
};

/**
 * @brief 用户仓储（数据访问层）
 * 
 * 模拟数据持久化操作
 */
class UserRepository
{
public:
    static QList<User> getAllUsers();
    static User getUserById(int id);
    static bool saveUser(const User &user);
    static bool deleteUser(int id);
    
private:
    static QList<User> m_users; // 模拟数据库
};

#endif // USERMODEL_H
