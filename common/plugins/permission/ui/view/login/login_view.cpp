#include "ui/view/login/login_view.h"

#include "ui/view/perm_text.h"
#include "ui/vm/login_view_model.h"

#include "databinding.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QSizePolicy>
#include <QVBoxLayout>

namespace permission_ui {

LoginView::LoginView(QWidget *parent)
    : QWidget(parent)
{
    buildLayout();
    retranslateUi();
}

void LoginView::setViewModel(LoginViewModel *viewModel)
{
    m_viewModel = viewModel;
    setupBindings();
}

void LoginView::buildLayout()
{
    setObjectName(QStringLiteral("permLoginCard"));
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedWidth(560);
    setStyleSheet(QStringLiteral(
        "QWidget#permLoginCard{"
        "background:rgba(28,38,52,0.60);border:1px solid rgba(90,110,140,0.55);"
        "border-radius:10px;}"));

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(64, 32, 64, 40);
    root->setSpacing(0);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setFixedHeight(56);
    m_titleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_titleLabel->setStyleSheet(
        QStringLiteral("color:#ffffff;font-size:28px;font-weight:600;background:transparent;"));

    m_errorLabel = new QLabel(this);
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_errorLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_errorLabel->setStyleSheet(
        QStringLiteral("color:#fa8c16;font-size:14px;background:transparent;"));
    m_errorLabel->hide();

    const auto fieldStyle = QStringLiteral(
        "QLineEdit{background:#0f1928;color:#e8eef6;border:1px solid #2a3a4d;"
        "border-radius:8px;padding:0 18px;font-size:18px;}"
        "QLineEdit:hover{border:1px solid #556c85;}"
        "QLineEdit:focus{border:1px solid #cfd8e3;}"
        "QLineEdit::placeholder{color:#7a8699;}");

    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setStyleSheet(fieldStyle);
    m_usernameEdit->setClearButtonEnabled(false);
    m_usernameEdit->setFixedHeight(62);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setStyleSheet(fieldStyle);
    m_passwordEdit->setClearButtonEnabled(false);
    m_passwordEdit->setFixedHeight(62);

    // 输入区：用户名/密码/错误提示作为一个整体，内部间距小于分区间距
    auto *inputGroup = new QWidget(this);
    inputGroup->setAttribute(Qt::WA_StyledBackground, true);
    inputGroup->setStyleSheet(QStringLiteral("background:transparent;border:none;"));
    inputGroup->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    auto *inputLay = new QVBoxLayout(inputGroup);
    inputLay->setContentsMargins(0, 0, 0, 0);
    inputLay->setSpacing(18);
    inputLay->addWidget(m_usernameEdit);
    inputLay->addWidget(m_passwordEdit);
    inputLay->addWidget(m_errorLabel);

    auto *btnRow = new QHBoxLayout();
    btnRow->setSpacing(22);

    m_btnLogin = new QPushButton(this);
    m_btnLogin->setCursor(Qt::PointingHandCursor);
    m_btnLogin->setFixedHeight(62);
    m_btnLogin->setStyleSheet(QStringLiteral(
        "QPushButton{background:#1890ff;color:#ffffff;border:none;border-radius:8px;"
        "font-size:20px;font-weight:600;padding:0 22px;"
        "height:62px;min-height:62px;max-height:62px;}"
        "QPushButton:hover{background:#40a9ff;}"));

    m_btnExit = new QPushButton(this);
    m_btnExit->setCursor(Qt::PointingHandCursor);
    m_btnExit->setFixedHeight(62);
    m_btnExit->setStyleSheet(QStringLiteral(
        "QPushButton{background:#e8eef0;color:#1f2937;border:none;border-radius:8px;"
        "font-size:20px;font-weight:600;padding:0 22px;"
        "height:62px;min-height:62px;max-height:62px;}"
        "QPushButton:hover{background:#f3f4f6;}"));

    btnRow->addWidget(m_btnLogin, 1);
    btnRow->addWidget(m_btnExit, 1);

    root->addWidget(m_titleLabel);
    root->addSpacing(18);
    root->addWidget(inputGroup);
    root->addSpacing(46);
    root->addLayout(btnRow);

    connect(m_passwordEdit, &QLineEdit::returnPressed, m_btnLogin, &QPushButton::click);
}

void LoginView::setupBindings()
{
    if (!m_viewModel)
        return;

    DataBinding::bind(m_viewModel, LoginViewModel::PROP_USERNAME, m_usernameEdit);
    DataBinding::bind(m_viewModel, LoginViewModel::PROP_PASSWORD, m_passwordEdit);
    DataBinding::bindCommand(m_viewModel->loginCommand(), m_btnLogin);
    DataBinding::bindCommand(m_viewModel->exitCommand(), m_btnExit);

    connect(m_viewModel, &LoginViewModel::loginFailed, this, &LoginView::showError);
    connect(m_viewModel, &LoginViewModel::loginSucceeded, this, [this]() {
        if (m_errorLabel)
            m_errorLabel->hide();
    });
    connect(m_viewModel, &LoginViewModel::propertyChanged, this, [this](const QString &name) {
        if (name == QLatin1String(LoginViewModel::PROP_USERNAME)
            || name == QLatin1String(LoginViewModel::PROP_PASSWORD)) {
            if (m_errorLabel && m_errorLabel->isVisible())
                m_errorLabel->hide();
        }
    });
}

void LoginView::showError(const QString &messageKey)
{
    if (!m_errorLabel)
        return;
    m_errorLabel->setText(mapErrorMessage(messageKey));
    m_errorLabel->show();
}

QString LoginView::mapErrorMessage(const QString &key) const
{
    if (key == QStringLiteral("username_required"))
        return PTXT("perm.users.please_enter_username");
    if (key == QStringLiteral("password_required"))
        return PTXT("perm.users.please_enter_password");
    if (key == QStringLiteral("account_disabled"))
        return PTXT("perm.login.account_disabled");
    if (key == QStringLiteral("invalid_credentials"))
        return PTXT("perm.login.invalid_credentials");
    return key.isEmpty() ? PTXT("perm.login.sign_in_failed") : key;
}

void LoginView::retranslateUi()
{
    if (m_titleLabel)
        m_titleLabel->setText(PTXT("perm.login.welcome"));
    if (m_usernameEdit)
        m_usernameEdit->setPlaceholderText(PTXT("perm.users.please_enter_username"));
    if (m_passwordEdit)
        m_passwordEdit->setPlaceholderText(PTXT("perm.users.please_enter_password"));
    if (m_btnLogin)
        m_btnLogin->setText(PTXT("perm.login.sign_in"));
    if (m_btnExit)
        m_btnExit->setText(PTXT("perm.login.exit"));
}

} // namespace permission_ui
