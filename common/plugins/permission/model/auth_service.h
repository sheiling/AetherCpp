#pragma once

#include "model/auth_session.h"

#include <QString>

namespace permission {

struct UserRecord;

struct AuthResult
{
    bool        ok      = false;
    QString     message;
    AuthSession session;
};

class IDataStore;

/**
 * @brief 登录认证（校验用户名/密码、账号状态，生成会话）
 */
class AuthService
{
public:
    explicit AuthService(IDataStore *store = nullptr);

    AuthResult login(const QString &username, const QString &plainPassword) const;

    static AuthSession currentSession();
    static void        setCurrentSession(const AuthSession &session);
    static void        clearSession();

private:
    bool verifyPassword(const QString &username, const QString &plainPassword,
                        UserRecord &userOut) const;

    IDataStore *m_store = nullptr;
};

} // namespace permission
