#include "deviceview.h"
#include "databinding.h"
#include "thememanager.h"
#include "languagemanager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QButtonGroup>
#include <QLabel>
#include <QFont>
#include <QFrame>
#include <QColor>

namespace {
QColor themeColorOrDefault(const QString &name, const QColor &fallback)
{
    const QColor c = TM ? TM->themeColor(name) : QColor{};
    return c.isValid() ? c : fallback;
}
}

DeviceView::DeviceView(QWidget *parent)
    : PageWidget(parent)
{
    setPageId("mvvm_device");
    setPageTitle(QString::fromUtf8("MVVM \u589e\u5f3a\u793a\u4f8b"));
}

DeviceView::~DeviceView()
{
}

void DeviceView::onLoad()
{
    m_viewModel = new DeviceViewModel(this);
    setupUi();
    setupBindings();
}

// ============================================================================
// UI 搭建
// ============================================================================
void DeviceView::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    mainLayout->setSpacing(16);

    // ── 标题 ──────────────────────────────────────────────────────────────────
    m_titleLabel = new QLabel(QString::fromUtf8("MVVM \u589e\u5f3a\u793a\u4f8b"), this);
    m_titleLabel->setObjectName("deviceTitleLabel");
    QFont f = m_titleLabel->font();
    f.setPointSize(16);
    f.setBold(true);
    m_titleLabel->setFont(f);
    mainLayout->addWidget(m_titleLabel);

    m_descLabel = new QLabel(
        QString::fromUtf8("\u6f14\u793a\uff1a\u7ed3\u6784\u4f53\u5b57\u6bb5\u7ed1\u5b9a\u3001QSlider\u3001QDoubleSpinBox\u3001QProgressBar\u3001QDateEdit\u3001QTimeEdit\u3001QRadioButton\u3001bindEnabled\u3001bindVisible"),
        this);
    m_descLabel->setObjectName("deviceDescLabel");
    m_descLabel->setWordWrap(true);
    mainLayout->addWidget(m_descLabel);

    // ── 上半区：DeviceInfo + SensorConfig（左右布局）─────────────────────────
    QHBoxLayout *topRow = new QHBoxLayout();
    topRow->setSpacing(16);

    // ─── DeviceInfo 结构体字段 ─────────────────────────────────────────────────
    m_deviceGroup = new QGroupBox(QString::fromUtf8("\u8bbe\u5907\u4fe1\u606f\uff08\u7ed3\u6784\u4f53 DeviceInfo \u5b57\u6bb5\u7ed1\u5b9a\uff09"), this);
    QFormLayout *devForm = new QFormLayout(m_deviceGroup);
    devForm->setSpacing(10);

    m_devNameEdit = new QLineEdit(this);
    m_devNameEdit->setPlaceholderText(QString::fromUtf8("\u8bbe\u5907\u540d\u79f0"));
    devForm->addRow(QString::fromUtf8("\u8bbe\u5907\u540d\u79f0:"), m_devNameEdit);

    m_devSnEdit = new QLineEdit(this);
    m_devSnEdit->setPlaceholderText("SN-XXXXXXXX");
    devForm->addRow(QString::fromUtf8("\u5e8f\u5217\u53f7:"), m_devSnEdit);

    m_devFwCombo = new QComboBox(this);
    m_devFwCombo->addItems({"v3.2.1", "v3.1.0", "v2.9.5", "v2.8.0"});
    devForm->addRow(QString::fromUtf8("\u56fa\u4ef6\u7248\u672c:"), m_devFwCombo);

    // 健康度：SpinBox（输入）+ ProgressBar（显示）
    QHBoxLayout *healthRow = new QHBoxLayout();
    m_devHealthSpin = new QSpinBox(this);
    m_devHealthSpin->setRange(0, 100);
    m_devHealthSpin->setSuffix("%");
    m_devHealthBar  = new QProgressBar(this);
    m_devHealthBar->setRange(0, 100);
    m_devHealthBar->setTextVisible(true);
    m_devHealthBar->setFixedHeight(20);
    healthRow->addWidget(m_devHealthSpin);
    healthRow->addWidget(m_devHealthBar);
    devForm->addRow(QString::fromUtf8("\u5065\u5eb7\u5ea6:"), healthRow);

    m_devOnlineChk = new QCheckBox(QString::fromUtf8("\u5728\u7ebf"), this);
    devForm->addRow("", m_devOnlineChk);

    topRow->addWidget(m_deviceGroup);

    // ─── SensorConfig 结构体字段 ──────────────────────────────────────────────
    m_sensorGroup = new QGroupBox(QString::fromUtf8("\u4f20\u611f\u5668\u914d\u7f6e\uff08\u7ed3\u6784\u4f53 SensorConfig \u5b57\u6bb5\u7ed1\u5b9a\uff09"), this);
    QFormLayout *sensorForm = new QFormLayout(m_sensorGroup);
    sensorForm->setSpacing(10);

    m_thresholdSpin = new QDoubleSpinBox(this);
    m_thresholdSpin->setRange(0.0, 200.0);
    m_thresholdSpin->setDecimals(2);
    m_thresholdSpin->setSuffix(" \u2103");
    sensorForm->addRow(QString::fromUtf8("\u62a5\u8b66\u9608\u5024:"), m_thresholdSpin);

    m_sampleRateSpin = new QSpinBox(this);
    m_sampleRateSpin->setRange(1, 10000);
    m_sampleRateSpin->setSuffix(" Hz");
    sensorForm->addRow(QString::fromUtf8("\u91c7\u6837\u7387:"), m_sampleRateSpin);

    m_calibDateEdit = new QDateEdit(this);
    m_calibDateEdit->setCalendarPopup(true);
    m_calibDateEdit->setDisplayFormat("yyyy-MM-dd");
    sensorForm->addRow(QString::fromUtf8("\u6821\u51c6\u65e5\u671f:"), m_calibDateEdit);

    m_triggerTimeEdit = new QTimeEdit(this);
    m_triggerTimeEdit->setDisplayFormat("HH:mm:ss");
    sensorForm->addRow(QString::fromUtf8("\u89e6\u53d1\u65f6\u95f4:"), m_triggerTimeEdit);

    topRow->addWidget(m_sensorGroup);
    mainLayout->addLayout(topRow);

    // ── 控件绑定演示区 ────────────────────────────────────────────────────────
    m_ctrlGroup = new QGroupBox(QString::fromUtf8("\u63a7\u4ef6\u7ed1\u5b9a\u6f14\u793a"), this);
    QGridLayout *ctrlGrid = new QGridLayout(m_ctrlGroup);
    ctrlGrid->setSpacing(12);

    // 音量 Slider（双向绑定）
    ctrlGrid->addWidget(new QLabel(QString::fromUtf8("\u97f3\u91cf (QSlider):"), this), 0, 0);
    m_volumeSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setTickInterval(10);
    m_volumeSlider->setTickPosition(QSlider::TicksBelow);
    ctrlGrid->addWidget(m_volumeSlider, 0, 1);
    m_volumeLabel = new QLabel("60", this);
    m_volumeLabel->setMinimumWidth(36);
    m_volumeLabel->setAlignment(Qt::AlignCenter);
    m_volumeLabel->setStyleSheet(QString("font-weight:bold; color:%1;")
                                 .arg(themeColorOrDefault("primaryColor", QColor("#0078d7")).name()));
    ctrlGrid->addWidget(m_volumeLabel, 0, 2);

    // 模拟进度 ProgressBar（单向绑定）
    ctrlGrid->addWidget(new QLabel(QString::fromUtf8("\u8fdb\u5ea6 (QProgressBar):"), this), 1, 0);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setTextVisible(true);
    ctrlGrid->addWidget(m_progressBar, 1, 1, 1, 2);

    // 报警开关 RadioButton（双向绑定）
    ctrlGrid->addWidget(new QLabel(QString::fromUtf8("\u62a5\u8b66\u5f00\u5173 (QRadioButton):"), this), 2, 0);
    QHBoxLayout *radioRow = new QHBoxLayout();
    m_alertOnRadio  = new QRadioButton(QString::fromUtf8("\u542f\u7528"), this);
    m_alertOffRadio = new QRadioButton(QString::fromUtf8("\u7981\u7528"), this);
    m_alertOnRadio->setChecked(true);
    // 使用 QButtonGroup 互斥
    QButtonGroup *bg = new QButtonGroup(this);
    bg->addButton(m_alertOnRadio);
    bg->addButton(m_alertOffRadio);
    radioRow->addWidget(m_alertOnRadio);
    radioRow->addWidget(m_alertOffRadio);
    radioRow->addStretch();
    QWidget *radioWrap = new QWidget(this);
    radioWrap->setLayout(radioRow);
    ctrlGrid->addWidget(radioWrap, 2, 1, 1, 2);

    mainLayout->addWidget(m_ctrlGroup);

    // ── 报警阈值详情面板（bindVisible 演示）────────────────────────────────────
    m_alertGroup = new QGroupBox(QString::fromUtf8("\u62a5\u8b66\u8be6\u60c5\uff08bindVisible \u6f14\u793a\uff09"), this);
    QHBoxLayout *alertLayout = new QHBoxLayout(m_alertGroup);
    QLabel *alertNote = new QLabel(
        QString::fromUtf8("\u8fd9\u4e2a\u9762\u677f\u7531 <b>useAlert</b> \u5c5e\u6027\u63a7\u5236\u53ef\u89c1\u6027\uff0c\u9009\u62e9\u300c\u7981\u7528\u300d\u540e\u81ea\u52a8\u9690\u85cf\u3002"),
        this);
    alertNote->setWordWrap(true);
    alertLayout->addWidget(alertNote);
    mainLayout->addWidget(m_alertGroup);

    // ── 操作按钮 ──────────────────────────────────────────────────────────────
    QHBoxLayout *btnRow = new QHBoxLayout();
    btnRow->setSpacing(10);

    m_applyBtn = new QPushButton(QString::fromUtf8("\u5e94\u7528\u914d\u7f6e"), this);
    m_applyBtn->setStyleSheet(QString(
        "QPushButton { background:%1; color:%2; padding:8px 20px; border-radius:4px; font-weight:bold; }"
        "QPushButton:hover { background:%3; }")
        .arg(themeColorOrDefault("primaryColor", QColor("#0078d7")).name(),
             themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name(),
             themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")).name()));
    btnRow->addWidget(m_applyBtn);

    m_resetBtn = new QPushButton(QString::fromUtf8("\u91cd\u7f6e\u9ed8\u8ba4"), this);
    m_resetBtn->setStyleSheet(QString(
        "QPushButton { background:%1; color:%2; padding:8px 20px; border-radius:4px; font-weight:bold; }"
        "QPushButton:hover { background:%3; }")
        .arg(themeColorOrDefault("secondaryDarkColor", QColor("#0f1a2c")).name(),
             themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name(),
             themeColorOrDefault("secondaryColor", QColor("#1e2a3a")).name()));
    btnRow->addWidget(m_resetBtn);

    m_simulateBtn = new QPushButton(QString::fromUtf8("\u6a21\u62df\u8fdb\u5ea6+15%"), this);
    m_simulateBtn->setStyleSheet(QString(
        "QPushButton { background:%1; color:%2; padding:8px 20px; border-radius:4px; font-weight:bold; }"
        "QPushButton:hover { background:%3; }")
        .arg(themeColorOrDefault("primaryColor", QColor("#0078d7")).name(),
             themeColorOrDefault("primaryTextColor", QColor("#e6edf7")).name(),
             themeColorOrDefault("primaryLightColor", QColor("#2f8fe2")).name()));
    btnRow->addWidget(m_simulateBtn);

    btnRow->addStretch();
    mainLayout->addLayout(btnRow);

    // ── 状态面板 ──────────────────────────────────────────────────────────────
    m_statusGroup = new QGroupBox(QString::fromUtf8("\u8fd0\u884c\u72b6\u6001"), this);
    QVBoxLayout *statusLayout = new QVBoxLayout(m_statusGroup);
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet(QString("color:%1; font-weight:bold;")
                                 .arg(themeColorOrDefault("primaryColor", QColor("#0078d7")).name()));
    m_statusLabel->setWordWrap(true);
    statusLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(m_statusGroup);

    mainLayout->addStretch();
}

// ============================================================================
// 数据绑定
// ============================================================================
void DeviceView::setupBindings()
{
    // ── DeviceInfo 结构体字段绑定（bindStructField）────────────────────────────
    using DI = DeviceInfo;
    DataBinding::bindStructField<DI>(m_viewModel, DeviceViewModel::PROP_DEVICE,
        [](const DI &d) { return d.name; },
        [](DI &d, const QString &v) { d.name = v; },
        m_devNameEdit);

    DataBinding::bindStructField<DI>(m_viewModel, DeviceViewModel::PROP_DEVICE,
        [](const DI &d) { return d.serialNumber; },
        [](DI &d, const QString &v) { d.serialNumber = v; },
        m_devSnEdit);

    DataBinding::bindStructField<DI>(m_viewModel, DeviceViewModel::PROP_DEVICE,
        [](const DI &d) { return d.firmware; },
        [](DI &d, const QString &v) { d.firmware = v; },
        m_devFwCombo);

    DataBinding::bindStructField<DI>(m_viewModel, DeviceViewModel::PROP_DEVICE,
        [](const DI &d) { return d.health; },
        [](DI &d, int v) { d.health = v; },
        m_devHealthSpin);

    // health 也单向绑定到 ProgressBar
    DataBinding::bindStructFieldOneWay<DI>(m_viewModel, DeviceViewModel::PROP_DEVICE,
        [](const DI &d) { return d.health; },
        m_devHealthBar);

    DataBinding::bindStructField<DI>(m_viewModel, DeviceViewModel::PROP_DEVICE,
        [](const DI &d) { return d.online; },
        [](DI &d, bool v) { d.online = v; },
        m_devOnlineChk);

    // ── SensorConfig 结构体字段绑定 ────────────────────────────────────────────
    using SC = SensorConfig;
    DataBinding::bindStructField<SC>(m_viewModel, DeviceViewModel::PROP_SENSOR,
        [](const SC &s) { return s.threshold; },
        [](SC &s, double v) { s.threshold = v; },
        m_thresholdSpin);

    DataBinding::bindStructField<SC>(m_viewModel, DeviceViewModel::PROP_SENSOR,
        [](const SC &s) { return s.sampleRate; },
        [](SC &s, int v) { s.sampleRate = v; },
        m_sampleRateSpin);

    // QDateEdit 和 QTimeEdit 直接绑定 SensorConfig 内的 QDate/QTime 字段
    // 使用 SensorConfig 专属的 QDate/QTime 字段：bindStructField 未提供 QDateEdit 重载，
    // 此处通过 propertyChanged 手动连接，演示灵活扩展能力。
    {
        // 初始化
        SensorConfig sc = m_viewModel->sensor();
        if (sc.calibDate.isValid())
            m_calibDateEdit->setDate(sc.calibDate);
        if (sc.triggerTime.isValid())
            m_triggerTimeEdit->setTime(sc.triggerTime);

        // ViewModel -> View
        QObject::connect(m_viewModel, &ViewModel::propertyChanged,
            this, [this](const QString &name, const QVariant &val) {
                if (name == DeviceViewModel::PROP_SENSOR) {
                    SensorConfig sc = val.value<SensorConfig>();
                    if (sc.calibDate.isValid())
                        m_calibDateEdit->setDate(sc.calibDate);
                    if (sc.triggerTime.isValid())
                        m_triggerTimeEdit->setTime(sc.triggerTime);
                }
            });

        // View -> ViewModel（modify 字段）
        QObject::connect(m_calibDateEdit, &QDateEdit::dateChanged,
            m_viewModel, [this](const QDate &d) {
                m_viewModel->modifySensor([d](SensorConfig &s) { s.calibDate = d; });
            });
        QObject::connect(m_triggerTimeEdit, &QTimeEdit::timeChanged,
            m_viewModel, [this](const QTime &t) {
                m_viewModel->modifySensor([t](SensorConfig &s) { s.triggerTime = t; });
            });
    }

    // ── QSlider 双向绑定（音量）────────────────────────────────────────────────
    DataBinding::bind(m_viewModel, DeviceViewModel::PROP_VOLUME, m_volumeSlider);

    // 音量 Label 实时同步（监听 propertyChanged 实现单向绑定）
    QObject::connect(m_viewModel, &ViewModel::propertyChanged, this,
        [this](const QString &name, const QVariant &val) {
            if (name == DeviceViewModel::PROP_VOLUME)
                m_volumeLabel->setText(QString::number(val.toInt()));
        });
    m_volumeLabel->setText(QString::number(m_viewModel->volume()));

    // ── QProgressBar 单向绑定（进度）──────────────────────────────────────────
    DataBinding::bindOneWay(m_viewModel, DeviceViewModel::PROP_PROGRESS, m_progressBar);

    // ── QRadioButton 双向绑定（useAlert）──────────────────────────────────────
    // alertOnRadio  <-> useAlert == true
    // alertOffRadio <-> useAlert == false（取反绑定，手动处理）
    DataBinding::bind(m_viewModel, DeviceViewModel::PROP_USE_ALERT, m_alertOnRadio);
    // alertOffRadio 监听
    QObject::connect(m_viewModel, &ViewModel::propertyChanged, this,
        [this](const QString &name, const QVariant &val) {
            if (name == DeviceViewModel::PROP_USE_ALERT)
                m_alertOffRadio->setChecked(!val.toBool());
        });
    m_alertOffRadio->setChecked(!m_viewModel->useAlert());
    QObject::connect(m_alertOffRadio, &QRadioButton::toggled, m_viewModel,
        [this](bool checked) {
            if (checked)
                m_viewModel->setUseAlert(false);
        });

    // ── bindVisible：报警详情面板跟随 useAlert ────────────────────────────────
    DataBinding::bindVisible(m_viewModel, DeviceViewModel::PROP_ALERT_VISIBLE, m_alertGroup);

    // ── 状态标签（单向绑定）──────────────────────────────────────────────────
    DataBinding::bindOneWay(m_viewModel, DeviceViewModel::PROP_STATUS, m_statusLabel);

    // ── 命令绑定 ───────────────────────────────────────────────────────────────
    DataBinding::bindCommand(m_viewModel->applyCommand(),    m_applyBtn);
    DataBinding::bindCommand(m_viewModel->resetCommand(),    m_resetBtn);
    DataBinding::bindCommand(m_viewModel->simulateCommand(), m_simulateBtn);
}

void DeviceView::retranslateUi()
{
    if (m_titleLabel)  m_titleLabel->setText(QString::fromUtf8("MVVM \u589e\u5f3a\u793a\u4f8b"));
    if (m_descLabel)   m_descLabel->setText(
        QString::fromUtf8("\u6f14\u793a\uff1a\u7ed3\u6784\u4f53\u5b57\u6bb5\u7ed1\u5b9a\u3001QSlider\u3001QDoubleSpinBox\u3001QProgressBar\u3001QDateEdit\u3001QTimeEdit\u3001QRadioButton\u3001bindEnabled\u3001bindVisible"));
}

void DeviceView::applyTheme()
{
    // 语义色按钮不随主题变化
}
