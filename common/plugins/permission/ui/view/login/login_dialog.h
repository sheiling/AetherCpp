#pragma once

#include <QDialog>

class LoginViewModel;
class QLabel;

namespace permission_ui {

class LoginView;

/**
 * @brief 登录弹窗（背景图自适应 + 居中登录卡片）
 */
class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);

    LoginViewModel *viewModel() const { return m_viewModel; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    LoginViewModel *m_viewModel = nullptr;
    LoginView      *m_loginView = nullptr;
    QLabel         *m_logoLabel = nullptr;
    QPixmap         m_background;
};

} // namespace permission_ui
