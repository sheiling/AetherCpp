#include "ui/vm/login_view_model.h"

#include "model/auth_service.h"
#include "service/audit_log_writer.h"
#include "ui/view/perm_text.h"

#include "uibase/permission/permservice.h"

LoginViewModel::LoginViewModel(QObject *parent)
    : ViewModel(parent)
{
    registerProperty(PROP_USERNAME, QString());
    registerProperty(PROP_PASSWORD, QString());
    bindCommands();
}

void LoginViewModel::bindCommands()
{
    m_loginCommand = new FunctionalCommand([this]() { doLogin(); }, nullptr, this);

    m_exitCommand = new FunctionalCommand([this]() { emit requestExit(); }, nullptr, this);
}

void LoginViewModel::doLogin()
{
    const QString attemptedUser = username().trimmed();
    permission::AuthService auth;
    const permission::AuthResult result = auth.login(attemptedUser, password());
    if (!result.ok) {
        permission::AuditLogWriter::appendLoginFailure(attemptedUser, result.message);
        emit loginFailed(result.message);
        return;
    }

    permission::AuthService::setCurrentSession(result.session);
    permission::AuditLogWriter::appendLoginSuccess(result.session);

    if (PermService *ps = PermService::instance(); ps->isReady())
        ps->reloadForUser(result.session.userId);

    setPassword(QString());
    emit loginSucceeded(result.session);
}
