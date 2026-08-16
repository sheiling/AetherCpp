#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;

class LoginViewModel;

namespace permission_ui {

class LoginView : public QWidget
{
    Q_OBJECT

public:
    explicit LoginView(QWidget *parent = nullptr);

    void setViewModel(LoginViewModel *viewModel);
    void showError(const QString &messageKey);

protected:
    void retranslateUi();

private:
    void buildLayout();
    void setupBindings();
    QString mapErrorMessage(const QString &key) const;

    LoginViewModel *m_viewModel = nullptr;

    QLabel     *m_logoLabel   = nullptr;
    QLabel     *m_titleLabel  = nullptr;
    QLabel     *m_errorLabel  = nullptr;
    QLineEdit  *m_usernameEdit = nullptr;
    QLineEdit  *m_passwordEdit = nullptr;
    QPushButton *m_btnLogin   = nullptr;
    QPushButton *m_btnExit    = nullptr;
};

} // namespace permission_ui
