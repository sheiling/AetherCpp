#pragma once

#include "command.h"
#include "model/auth_session.h"
#include "viewmodel.h"

class LoginViewModel : public ViewModel
{
    Q_OBJECT

public:
    static constexpr const char *PROP_USERNAME = "username";
    static constexpr const char *PROP_PASSWORD = "password";

    VIEWMODEL_PROPERTY(QString, username, Username)
    VIEWMODEL_PROPERTY(QString, password, Password)

signals:
    void usernameChanged();
    void passwordChanged();

    void loginSucceeded(const permission::AuthSession &session);
    void loginFailed(const QString &message);
    void requestExit();

public:
    explicit LoginViewModel(QObject *parent = nullptr);

    Command *loginCommand() const { return m_loginCommand; }
    Command *exitCommand() const { return m_exitCommand; }

private:
    void bindCommands();
    void doLogin();

    FunctionalCommand *m_loginCommand = nullptr;
    FunctionalCommand *m_exitCommand  = nullptr;
};
