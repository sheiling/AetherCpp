#include "ui/view/login/login_dialog.h"

#include "ui/view/login/login_view.h"
#include "ui/vm/login_view_model.h"
#include "uibase/thememanager.h"

#include <QPainter>
#include <QLabel>
#include <QVBoxLayout>

namespace permission_ui {

namespace {
constexpr int kLogoToCardTop = 40;   // Logo 下方到操作区上方固定留白
constexpr int kCardToBottom  = 40;   // 操作区下方到弹窗底部固定留白
} // namespace

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent, Qt::Dialog | Qt::FramelessWindowHint)
{
    setModal(true);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setObjectName(QStringLiteral("permLoginDialog"));
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QStringLiteral(
        "QDialog#permLoginDialog{border:1px solid rgba(90,110,140,0.55);border-radius:10px;}"));

    m_background = QPixmap(QStringLiteral(":/resources/image/loginBG.png"));

    m_viewModel = new LoginViewModel(this);
    m_loginView = new LoginView(this);
    m_loginView->setViewModel(m_viewModel);

    connect(m_viewModel, &LoginViewModel::loginSucceeded, this, &QDialog::accept);
    connect(m_viewModel, &LoginViewModel::requestExit, this, &QDialog::reject);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 18, 0, 22);
    root->setSpacing(0);

    m_logoLabel = new QLabel(this);
    m_logoLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_logoLabel->setFixedHeight(76);
    {
        const QString logoPath = (TM && TM->isDark())
                                     ? QStringLiteral(":/resources/image/white_logo.png")
                                     : QStringLiteral(":/resources/image/black_logo.png");
        const QPixmap logo(logoPath);
        if (!logo.isNull())
            m_logoLabel->setPixmap(
                logo.scaled(320, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    root->addWidget(m_logoLabel, 0, Qt::AlignHCenter);
    root->addSpacing(kLogoToCardTop);
    root->addWidget(m_loginView, 0, Qt::AlignHCenter);
    root->addSpacing(kCardToBottom);

    // 弹窗大小：能装下登录卡片并留出背景留白
    const QSize card = m_loginView->sizeHint().isValid() ? m_loginView->sizeHint()
                                                         : QSize(360, 320);
    resize(qMax(980, card.width() + 360),
           qMax(720, 18 + 76 + kLogoToCardTop + card.height() + kCardToBottom + 22));
}

void LoginDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    if (!m_background.isNull()) {
        const QPixmap scaled =
            m_background.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        const int x = (width() - scaled.width()) / 2;
        const int y = (height() - scaled.height()) / 2;
        p.drawPixmap(x, y, scaled);
        p.fillRect(rect(), QColor(0, 0, 0, 80));
    } else {
        p.fillRect(rect(), QColor(QStringLiteral("#0b1520")));
    }
}

} // namespace permission_ui
