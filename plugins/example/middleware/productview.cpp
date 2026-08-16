#include "productview.h"
#include "databinding.h"
#include "thememanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFont>
#include <QDateTime>
#include <QColor>

namespace {
QColor themeColorOrDefault(const QString &name, const QColor &fallback)
{
    const QColor c = TM ? TM->themeColor(name) : QColor{};
    return c.isValid() ? c : fallback;
}
}

ProductView::ProductView(QWidget *parent)
    : PageWidget(parent)
{
    setPageId("middleware_example");
    setPageTitle("Middleware \xe7\xa4\xba\xe4\xbe\x8b");   // "Middleware 示例"
}

void ProductView::onLoad()
{
    m_viewModel = new ProductViewModel(this);
    setupUi();
    setupBindings();
}

// ============================================================================
// setupUi
// ============================================================================
void ProductView::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 20);
    root->setSpacing(12);

    // ── 标题 ─────────────────────────────────────────────────────────────
    m_titleLabel = new QLabel(this);
    m_titleLabel->setText(QString::fromUtf8(
        "Middleware \xe6\xa8\xa1\xe5\xbc\x8f\xe7\xa4\xba\xe4\xbe\x8b\xef\xbc\x9a\xe4\xba\xa7\xe5\x93\x81\xe7\xae\xa1\xe7\x90\x86"));
    QFont tf = m_titleLabel->font();
    tf.setPointSize(15);
    tf.setBold(true);
    m_titleLabel->setFont(tf);
    root->addWidget(m_titleLabel);

    // 说明文字
    auto *descLabel = new QLabel(this);
    descLabel->setText(QString::fromUtf8(
        "\xe5\xb1\x9e\xe6\x80\xa7\xe7\xae\xa1\xe9\x81\x93 (around \xe6\xa8\xa1\xe5\xbc\x8f)\xef\xbc\x9a  "
        "LoggingMiddleware \xe2\x86\x92 AuditLogMiddleware \xe2\x86\x92 \xe2\x98\x85\xe5\xb1\x9e\xe6\x80\xa7\xe5\x86\x99\xe5\x85\xa5 "
        "\xe2\x86\x90 AuditLogMiddleware \xe2\x86\x90 LoggingMiddleware\n"
        "\xe5\x91\xbd\xe4\xbb\xa4\xe7\xae\xa1\xe9\x81\x93 (around \xe6\xa8\xa1\xe5\xbc\x8f)\xef\xbc\x9a  "
        "TimingMiddleware \xe2\x86\x92 AuthMiddleware \xe2\x86\x92 LogMiddleware \xe2\x86\x92 \xe2\x98\x85" "execute() "
        "\xe2\x86\x90 LogMiddleware \xe2\x86\x90 AuthMiddleware \xe2\x86\x90 TimingMiddleware\n"
        "\xe5\x8b\xbe\xe9\x80\x89\xe2\x80\x9c\xe7\xa6\x81\xe7\x94\xa8\xe6\x9d\x83\xe9\x99\x90\xe2\x80\x9d\xe5\x8f\xaf\xe7\x9c\x8b\xe5\x88\xb0 AuthCommandMiddleware \xe9\x98\xbb\xe6\xad\xa2\xe5\x91\xbd\xe4\xbb\xa4\xe5\x8f\x8a\xe5\x90\x8e\xe7\xbb\xad\xe4\xb8\xad\xe9\x97\xb4\xe4\xbb\xb6\xe4\xb8\x8d\xe5\x86\x8d\xe8\xa2\xab\xe8\xb0\x83\xe7\x94\xa8\xe3\x80\x82"));
    descLabel->setWordWrap(true);
    descLabel->setStyleSheet(QString("color:%1; font-size:12px;")
                             .arg(themeColorOrDefault("secondaryTextColor", QColor("#9fb0c7")).name()));
    root->addWidget(descLabel);

    // ── 主体：水平分割（左:表单 / 右:日志）─────────────────────────────
    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(6);

    // ─────────────────── 左侧：产品表单 ────────────────────────────────
    auto *leftWidget = new QWidget(splitter);
    auto *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 8, 0);

    m_formGroup  = new QGroupBox(QString::fromUtf8("\xe4\xba\xa7\xe5\x93\x81\xe4\xbf\xa1\xe6\x81\xaf"), leftWidget);
    m_formLayout = new QFormLayout(m_formGroup);
    m_formLayout->setSpacing(10);

    m_nameEdit = new QLineEdit(leftWidget);
    m_nameEdit->setPlaceholderText(QString::fromUtf8("\xe4\xbe\x8b\xef\xbc\x9a iPhone 16"));
    m_formLayout->addRow(QString::fromUtf8("\xe4\xba\xa7\xe5\x93\x81\xe5\x90\x8d\xe7\xa7\xb0 *"), m_nameEdit);

    m_priceSpinBox = new QDoubleSpinBox(leftWidget);
    m_priceSpinBox->setRange(0.0, 999999.0);
    m_priceSpinBox->setDecimals(2);
    m_priceSpinBox->setSuffix(QString::fromUtf8(" \xe5\x85\x83"));
    m_formLayout->addRow(QString::fromUtf8("\xe4\xbb·\xe6\xa0\xbc *"), m_priceSpinBox);

    m_stockSpinBox = new QSpinBox(leftWidget);
    m_stockSpinBox->setRange(0, 99999);
    m_formLayout->addRow(QString::fromUtf8("\xe5\xba\x93\xe5\xad\x98 *"), m_stockSpinBox);

    m_categoryCombo = new QComboBox(leftWidget);
    m_categoryCombo->addItems({
        QString::fromUtf8("\xe7\x94\xb5\xe8\x84\x91"),
        QString::fromUtf8("\xe6\x89\x8b\xe6\x9c\xba"),
        QString::fromUtf8("\xe5\xa4\x96\xe8\xae\xbe"),
        QString::fromUtf8("\xe6\x95\xb0\xe7\xa0\x81"),
        QString::fromUtf8("\xe5\xae\xb6\xe7\x94\xb5"),
        QString::fromUtf8("\xe5\x85\xb6\xe4\xbb\x96")
    });
    m_formLayout->addRow(QString::fromUtf8("\xe5\x88\x86\xe7\xb1\xbb"), m_categoryCombo);

    m_descEdit = new QLineEdit(leftWidget);
    m_descEdit->setPlaceholderText(QString::fromUtf8("\xe7\xae\x80\xe7\x9f\xad\xe6\x8f\x8f\xe8\xbf\xb0\xef\xbc\x88\xe5\x8f\xaf\xe9\x80\x89\xef\xbc\x89"));
    m_formLayout->addRow(QString::fromUtf8("\xe6\x8f\x8f\xe8\xbf\xb0"), m_descEdit);

    leftLayout->addWidget(m_formGroup);

    // 权限模拟开关
    auto *authGroup = new QGroupBox(QString::fromUtf8("\xe6\x9d\x83\xe9\x99\x90\xe6\xa8\xa1\xe6\x8b\x9f"), leftWidget);
    auto *authLayout = new QVBoxLayout(authGroup);
    m_denyCheckBox = new QCheckBox(
        QString::fromUtf8("\xe7\xa6\x81\xe7\x94\xa8\xe6\x8c\x89\xe9\x92\xae\xe6\x9d\x83\xe9\x99\x90\xef\xbc\x88\xe8\xa7\x82\xe5\xaf\x9f AuthCommandMiddleware \xe6\x8b\xa6\xe6\x88\xaa\xef\xbc\x89"),
        authGroup);
    authLayout->addWidget(m_denyCheckBox);
    leftLayout->addWidget(authGroup);

    // 按钮区
    auto *btnLayout = new QHBoxLayout();
    m_addButton = new QPushButton(QString::fromUtf8("\xe6\xb7\xbb\xe5\x8a\xa0\xe4\xba\xa7\xe5\x93\x81"), leftWidget);
    m_addButton->setStyleSheet(QString(
        "QPushButton { background:%1; color:%2; padding:8px 18px; border-radius:4px; font-weight:bold; }"
        "QPushButton:hover { background:%3; }"
        "QPushButton:disabled { background:%4; color:%5; }")
        .arg(themeColorOrDefault("primaryColor", QColor("#0078d7")).name(),
             themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name(),
             themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")).name(),
             themeColorOrDefault("secondaryColor", QColor("#1e2a3a")).name(),
             themeColorOrDefault("secondaryTextColor", QColor("#9fb0c7")).name()));
    btnLayout->addWidget(m_addButton);

    m_clearButton = new QPushButton(QString::fromUtf8("\xe6\xb8\x85\xe7\xa9\xba\xe8\xa1\xa8\xe5\x8d\x95"), leftWidget);
    m_clearButton->setStyleSheet(QString(
        "QPushButton { background:%1; color:%2; padding:8px 18px; border-radius:4px; font-weight:bold; }"
        "QPushButton:hover { background:%3; }")
        .arg(themeColorOrDefault("secondaryColor", QColor("#1e2a3a")).name(),
             themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name(),
             themeColorOrDefault("secondaryLightColor", QColor("#263447")).name()));
    btnLayout->addWidget(m_clearButton);

    m_loadButton = new QPushButton(QString::fromUtf8("\xe5\x88\xb7\xe6\x96\xb0\xe6\x95\xb0\xe6\x8d\xae"), leftWidget);
    m_loadButton->setStyleSheet(QString(
        "QPushButton { background:%1; color:%2; padding:8px 18px; border-radius:4px; font-weight:bold; }"
        "QPushButton:hover { background:%3; }")
        .arg(themeColorOrDefault("primaryColor", QColor("#0078d7")).name(),
             themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name(),
             themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")).name()));
    btnLayout->addWidget(m_loadButton);
    btnLayout->addStretch();
    leftLayout->addLayout(btnLayout);

    // 状态 & 数量
    m_statusLabel = new QLabel(leftWidget);
    m_statusLabel->setStyleSheet(QString("color:%1; font-weight:bold;")
                                 .arg(themeColorOrDefault("primaryColor", QColor("#0078d7")).name()));
    leftLayout->addWidget(m_statusLabel);

    m_countLabel = new QLabel(leftWidget);
    leftLayout->addWidget(m_countLabel);

    leftLayout->addStretch();

    // ─────────────────── 右侧：日志面板 ──────────────────────────────────
    auto *rightWidget = new QWidget(splitter);
    auto *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(8, 0, 0, 0);

    // 实时日志
    m_logGroup = new QGroupBox(
        QString::fromUtf8("\xf0\x9f\x93\x9d  Middleware \xe5\xae\x9e\xe6\x97\xb6\xe6\x97\xa5\xe5\xbf\x97"),
        rightWidget);
    auto *logLayout = new QVBoxLayout(m_logGroup);
    m_logEdit = new QTextEdit(m_logGroup);
    m_logEdit->setReadOnly(true);
    m_logEdit->setFont(QFont("Consolas", 9));
    m_logEdit->setStyleSheet(QString(
        "QTextEdit { background:%1; color:%2; border:1px solid %3; }")
        .arg(themeColorOrDefault("secondaryColor", QColor("#1e2a3a")).name(),
             themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name(),
             themeColorOrDefault("secondaryLightColor", QColor("#263447")).name()));
    m_logEdit->setMinimumHeight(160);
    logLayout->addWidget(m_logEdit);

    m_clearLogButton = new QPushButton(QString::fromUtf8("\xe6\xb8\x85\xe7\xa9\xba\xe6\x97\xa5\xe5\xbf\x97"), m_logGroup);
    m_clearLogButton->setFixedWidth(80);
    logLayout->addWidget(m_clearLogButton);
    rightLayout->addWidget(m_logGroup);

    // 审计历史
    m_auditGroup = new QGroupBox(
        QString::fromUtf8("\xf0\x9f\x93\x8b  \xe5\xae\xa1\xe8\xae\xa1\xe5\x8f\x98\xe6\x9b\xb4\xe5\x8e\x86\xe5\x8f\xb2"),
        rightWidget);
    auto *auditLayout = new QVBoxLayout(m_auditGroup);
    m_auditEdit = new QTextEdit(m_auditGroup);
    m_auditEdit->setReadOnly(true);
    m_auditEdit->setFont(QFont("Consolas", 9));
    m_auditEdit->setStyleSheet(QString(
        "QTextEdit { background:%1; color:%2; border:1px solid %3; }")
        .arg(themeColorOrDefault("secondaryDarkColor", QColor("#0f1a2c")).name(),
             themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")).name(),
             themeColorOrDefault("secondaryLightColor", QColor("#263447")).name()));
    m_auditEdit->setMinimumHeight(140);
    auditLayout->addWidget(m_auditEdit);

    m_clearAuditButton = new QPushButton(QString::fromUtf8("\xe6\xb8\x85\xe7\xa9\xba\xe5\x8e\x86\xe5\x8f\xb2"), m_auditGroup);
    m_clearAuditButton->setFixedWidth(80);
    auditLayout->addWidget(m_clearAuditButton);
    rightLayout->addWidget(m_auditGroup);

    // ── 组合分割器 ─────────────────────────────────────────────────────────
    splitter->addWidget(leftWidget);
    splitter->addWidget(rightWidget);
    splitter->setStretchFactor(0, 4);
    splitter->setStretchFactor(1, 5);

    root->addWidget(splitter, 1);
}

// ============================================================================
// setupBindings
// ============================================================================
void ProductView::setupBindings()
{
    // ── 表单双向绑定 ─────────────────────────────────────────────────────
    // 名称：QLineEdit <-> ViewModel.name
    DataBinding::bind(m_viewModel, ProductViewModel::PROP_NAME, m_nameEdit);

    // 价格：手动绑定（QDoubleSpinBox 无专用 bind 重载）
    m_priceSpinBox->setValue(m_viewModel->price());
    connect(m_viewModel, &ViewModel::propertyChanged, m_priceSpinBox,
            [this](const QString &name, const QVariant &val) {
        if (name == ProductViewModel::PROP_PRICE)
            m_priceSpinBox->setValue(val.toDouble());
    });
    connect(m_priceSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            m_viewModel, [this](double v) {
        m_viewModel->setPrice(v);
    });

    // 库存
    DataBinding::bind(m_viewModel, ProductViewModel::PROP_STOCK, m_stockSpinBox);

    // 分类
    m_categoryCombo->setCurrentText(m_viewModel->category());
    connect(m_viewModel, &ViewModel::propertyChanged, m_categoryCombo,
            [this](const QString &name, const QVariant &val) {
        if (name == ProductViewModel::PROP_CATEGORY)
            m_categoryCombo->setCurrentText(val.toString());
    });
    connect(m_categoryCombo, &QComboBox::currentTextChanged,
            m_viewModel, [this](const QString &text) {
        m_viewModel->setCategory(text);
    });

    // 描述
    DataBinding::bind(m_viewModel, ProductViewModel::PROP_DESCRIPTION, m_descEdit);

    // 状态 & 数量（单向）
    DataBinding::bindOneWay(m_viewModel, ProductViewModel::PROP_STATUS, m_statusLabel);
    connect(m_viewModel, &ViewModel::propertyChanged, m_countLabel,
            [this](const QString &name, const QVariant &val) {
        if (name == ProductViewModel::PROP_COUNT)
            m_countLabel->setText(QString::fromUtf8("\xe5\xba\x93\xe5\xad\x98\xe4\xba\xa7\xe5\x93\x81\xef\xbc\x9a%1 \xe4\xbb\xb6").arg(val.toInt()));
    });
    m_countLabel->setText(
        QString::fromUtf8("\xe5\xba\x93\xe5\xad\x98\xe4\xba\xa7\xe5\x93\x81\xef\xbc\x9a%1 \xe4\xbb\xb6")
            .arg(m_viewModel->count()));

    // ── 命令绑定 ─────────────────────────────────────────────────────────
    DataBinding::bindCommand(m_viewModel->addCommand(),   m_addButton);
    DataBinding::bindCommand(m_viewModel->clearCommand(), m_clearButton);
    DataBinding::bindCommand(m_viewModel->loadCommand(),  m_loadButton);

    // ── Middleware 回调设置 ───────────────────────────────────────────────

    // 1. LoggingMiddleware → 实时日志面板
    m_viewModel->loggingMiddleware()->setLogCallback([this](const QString &msg) {
        m_logEdit->append(
            QString("<span style='color:#9cdcfe;'>[%1]</span> %2")
                .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
                .arg(msg.toHtmlEscaped()));
    });

    // 2. AuditLogMiddleware → 审计历史面板
    m_viewModel->auditMiddleware()->setOnEntryAdded(
        [this](const AuditLogMiddleware::Entry &e) {
            m_auditEdit->append(
                QString("<span style='color:#8b949e;'>[%1]</span>"
                        " <span style='color:#79c0ff;'>%2</span>"
                        " <span style='color:#ffa657;'>%3</span>"
                        " <span style='color:#8b949e;'>\xe2\x86\x92</span>"
                        " <span style='color:#7ee787;'>%4</span>")
                    .arg(e.timestamp.toString("HH:mm:ss"))
                    .arg(e.propertyName.toHtmlEscaped())
                    .arg(e.oldValue.toString().toHtmlEscaped())
                    .arg(e.newValue.toString().toHtmlEscaped()));
        });

    // 3. AuthCommandMiddleware → 日志面板显示拒绝信息
    m_viewModel->authMiddleware()->setOnDenied([this](const QString &cmd) {
        m_logEdit->append(
            QString("<span style='color:#f85149;'>[AUTH] \xe5\x91\xbd\xe4\xbb\xa4 '%1' \xe8\xa2\xab\xe6\x8b\x92\xe7\xbb\x9d\xef\xbc\x9a\xe6\x9d\x83\xe9\x99\x90\xe4\xb8\x8d\xe8\xb6\xb3</span>")
                .arg(cmd.toHtmlEscaped()));
    });

    // 4. LoggingCommandMiddleware → 日志面板
    m_viewModel->cmdLogMiddleware()->setLogCallback([this](const QString &msg) {
        m_logEdit->append(
            QString("<span style='color:#d2a8ff;'>[%1]</span> %2")
                .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
                .arg(msg.toHtmlEscaped()));
    });

    // 5. TimingCommandMiddleware → 日志面板（最外层，包裹全链路）
    m_viewModel->timingMiddleware()->setLogCallback([this](const QString &msg) {
        m_logEdit->append(
            QString("<span style='color:#ffd700;'>[%1]</span> %2")
                .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
                .arg(msg.toHtmlEscaped()));
    });

    // 7. 权限开关 → AuthCommandMiddleware
    connect(m_denyCheckBox, &QCheckBox::toggled, this, [this](bool denied) {
        m_viewModel->authMiddleware()->setPermitted(!denied);
        QString tip = denied
            ? QString::fromUtf8("\xf0\x9f\x94\x92 \xe6\x9d\x83\xe9\x99\x90\xe5\xb7\xb2\xe7\xa6\x81\xe7\x94\xa8\xef\xbc\x9a\xe6\x8c\x89\xe9\x92\xae\xe5\x91\xbd\xe4\xbb\xa4\xe5\xb0\x86\xe8\xa2\xab AuthCommandMiddleware \xe9\x98\xbb\xe6\xad\xa2\xe3\x80\x82")
            : QString::fromUtf8("\xf0\x9f\x94\x93 \xe6\x9d\x83\xe9\x99\x90\xe5\xb7\xb2\xe5\x85\x81\xe8\xae\xb8\xef\xbc\x9a\xe5\x91\xbd\xe4\xbb\xa4\xe5\x8f\xaf\xe6\xad\xa3\xe5\xb8\xb8\xe6\x89\xa7\xe8\xa1\x8c\xe3\x80\x82");
        m_logEdit->append(
            QString("<span style='color:#f0883e;'>%1</span>").arg(tip));
    });

    // ── 日志清空按钮 ──────────────────────────────────────────────────────
    connect(m_clearLogButton,   &QPushButton::clicked, m_logEdit,   &QTextEdit::clear);
    connect(m_clearAuditButton, &QPushButton::clicked, this, [this]() {
        m_auditEdit->clear();
        m_viewModel->auditMiddleware()->clearHistory();
    });
}

void ProductView::retranslateUi()
{
    // 静态文本已通过 UTF-8 字面量硬编码，此处仅刷新动态计数
    if (m_countLabel && m_viewModel) {
        m_countLabel->setText(
            QString::fromUtf8("\xe5\xba\x93\xe5\xad\x98\xe4\xba\xa7\xe5\x93\x81\xef\xbc\x9a%1 \xe4\xbb\xb6")
                .arg(m_viewModel->count()));
    }
}

void ProductView::applyTheme()
{
    // 语义色按钮不随主题变化
}
