#include "userview.h"
#include "databinding.h"
#include "thememanager.h"
#include "languagemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFont>
#include <QColor>

namespace {
QColor themeColorOrDefault(const QString &name, const QColor &fallback)
{
    const QColor c = TM ? TM->themeColor(name) : QColor{};
    return c.isValid() ? c : fallback;
}
}

UserView::UserView(QWidget *parent)
    : PageWidget(parent)
    , m_viewModel(nullptr)
    , m_nameEdit(nullptr)
    , m_ageSpinBox(nullptr)
    , m_emailEdit(nullptr)
    , m_activeCheckBox(nullptr)
    , m_saveButton(nullptr)
    , m_clearButton(nullptr)
    , m_loadButton(nullptr)
    , m_statusLabel(nullptr)
    , m_userCountLabel(nullptr)
    , m_titleLabel(nullptr)
    , m_descLabel(nullptr)
    , m_formGroup(nullptr)
    , m_statusGroup(nullptr)
    , m_techLabel(nullptr)
    , m_formLayout(nullptr)
{
    setPageId("mvvm_example");
    setPageTitle("MVVM示例");
}

UserView::~UserView()
{
}

void UserView::onLoad()
{
    m_viewModel = new UserViewModel(this);
    setupUi();
    setupBindings();
}

void UserView::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);

    // 标题
    m_titleLabel = new QLabel(TR("user.title"), this);
    m_titleLabel->setObjectName("userTitleLabel");
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    mainLayout->addWidget(m_titleLabel);

    // 说明文字
    m_descLabel = new QLabel(TR("user.desc"), this);
    m_descLabel->setObjectName("userDescLabel");
    mainLayout->addWidget(m_descLabel);

    // 用户信息表单
    m_formGroup = new QGroupBox(TR("user.form.group"), this);
    m_formLayout = new QFormLayout(m_formGroup);
    m_formLayout->setSpacing(15);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText(TR("user.form.name.hint"));
    m_formLayout->addRow(TR("user.form.name"), m_nameEdit);

    m_ageSpinBox = new QSpinBox(this);
    m_ageSpinBox->setRange(0, 150);
    m_formLayout->addRow(TR("user.form.age"), m_ageSpinBox);

    m_emailEdit = new QLineEdit(this);
    m_emailEdit->setPlaceholderText("example@email.com");
    m_formLayout->addRow(TR("user.form.email"), m_emailEdit);

    m_activeCheckBox = new QCheckBox(TR("user.form.active"), this);
    m_formLayout->addRow("", m_activeCheckBox);

    mainLayout->addWidget(m_formGroup);

    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    m_saveButton = new QPushButton(TR("user.button.save"), this);
    m_saveButton->setStyleSheet(QString(
        "QPushButton { background:%1; color:%2; padding:8px 20px; border-radius:4px; font-weight:bold; }"
        "QPushButton:hover { background:%3; }"
        "QPushButton:disabled { background:%4; color:%5; }")
        .arg(themeColorOrDefault("primaryColor", QColor("#0078d7")).name(),
             themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name(),
             themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")).name(),
             themeColorOrDefault("secondaryColor", QColor("#1e2a3a")).name(),
             themeColorOrDefault("secondaryTextColor", QColor("#9fb0c7")).name()));
    buttonLayout->addWidget(m_saveButton);

    m_clearButton = new QPushButton(TR("user.button.clear"), this);
    m_clearButton->setStyleSheet(QString(
        "QPushButton { background:%1; color:%2; padding:8px 20px; border-radius:4px; font-weight:bold; }"
        "QPushButton:hover { background:%3; }")
        .arg(themeColorOrDefault("secondaryDarkColor", QColor("#0f1a2c")).name(),
             themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name(),
             themeColorOrDefault("secondaryColor", QColor("#1e2a3a")).name()));
    buttonLayout->addWidget(m_clearButton);

    m_loadButton = new QPushButton(TR("user.button.load"), this);
    m_loadButton->setStyleSheet(QString(
        "QPushButton { background:%1; color:%2; padding:8px 20px; border-radius:4px; font-weight:bold; }"
        "QPushButton:hover { background:%3; }")
        .arg(themeColorOrDefault("primaryColor", QColor("#0078d7")).name(),
             themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name(),
             themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")).name()));
    buttonLayout->addWidget(m_loadButton);

    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    // 状态信息
    m_statusGroup = new QGroupBox(TR("user.status.group"), this);
    QVBoxLayout *statusLayout = new QVBoxLayout(m_statusGroup);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet(QString("color:%1; font-weight:bold;")
                                 .arg(themeColorOrDefault("primaryColor", QColor("#0078d7")).name()));
    statusLayout->addWidget(m_statusLabel);

    m_userCountLabel = new QLabel(this);
    m_userCountLabel->setObjectName("userCountLabel");
    statusLayout->addWidget(m_userCountLabel);

    mainLayout->addWidget(m_statusGroup);

    // 技术说明
    m_techLabel = new QLabel(TR("user.tech.note"), this);
    m_techLabel->setObjectName("userTechLabel");
    mainLayout->addWidget(m_techLabel);

    mainLayout->addStretch();
}

void UserView::retranslateUi()
{
    if (m_titleLabel)    m_titleLabel->setText(TR("user.title"));
    if (m_descLabel)     m_descLabel->setText(TR("user.desc"));
    if (m_formGroup)     m_formGroup->setTitle(TR("user.form.group"));
    if (m_statusGroup)   m_statusGroup->setTitle(TR("user.status.group"));
    if (m_techLabel)     m_techLabel->setText(TR("user.tech.note"));
    if (m_activeCheckBox) m_activeCheckBox->setText(TR("user.form.active"));
    if (m_saveButton)    m_saveButton->setText(TR("user.button.save"));
    if (m_clearButton)   m_clearButton->setText(TR("user.button.clear"));
    if (m_loadButton)    m_loadButton->setText(TR("user.button.load"));
    if (m_nameEdit)      m_nameEdit->setPlaceholderText(TR("user.form.name.hint"));

    // 更新 FormLayout 行标签
    if (m_formLayout) {
        auto updateRowLabel = [&](int row, const QString &key) {
            QLayoutItem *item = m_formLayout->itemAt(row, QFormLayout::LabelRole);
            if (item && item->widget()) {
                if (QLabel *lbl = qobject_cast<QLabel*>(item->widget())) {
                    lbl->setText(TR(key));
                }
            }
        };
        updateRowLabel(0, "user.form.name");
        updateRowLabel(1, "user.form.age");
        updateRowLabel(2, "user.form.email");
    }

    // 刷新用户数量显示（保持当前计数）
    if (m_userCountLabel && m_viewModel) {
        int count = m_viewModel->getProperty(UserViewModel::PROP_USER_COUNT).toInt();
        m_userCountLabel->setText(TR("user.status.count").arg(count));
    }
}

void UserView::applyTheme()
{
    // 语义色按钮（保存/清空/加载）不随主题变化，无需处理
    // 其余控件已通过 objectName 由应用级 QSS 自动命中
}

void UserView::setupBindings()
{
    DataBinding::bind(m_viewModel, UserViewModel::PROP_USER_NAME,  m_nameEdit);
    DataBinding::bind(m_viewModel, UserViewModel::PROP_USER_AGE,   m_ageSpinBox);
    DataBinding::bind(m_viewModel, UserViewModel::PROP_USER_EMAIL, m_emailEdit);
    DataBinding::bind(m_viewModel, UserViewModel::PROP_IS_ACTIVE,  m_activeCheckBox);

    DataBinding::bindOneWay(m_viewModel, UserViewModel::PROP_STATUS_MESSAGE, m_statusLabel);

    connect(m_viewModel, &ViewModel::propertyChanged, this,
            [this](const QString &name, const QVariant &value) {
        if (name == UserViewModel::PROP_USER_COUNT) {
            m_userCountLabel->setText(TR("user.status.count").arg(value.toInt()));
        }
    });

    DataBinding::bindCommand(m_viewModel->saveCommand(),  m_saveButton);
    DataBinding::bindCommand(m_viewModel->clearCommand(), m_clearButton);
    DataBinding::bindCommand(m_viewModel->loadCommand(),  m_loadButton);
}
