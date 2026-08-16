#include "io_point_widget.h"

#include "io_input_point_widget.h"
#include "io_output_point_widget.h"
#include "motion_ui_common.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QRadioButton>
#include <QVBoxLayout>

namespace motion {

IoPointWidget *IoPointWidget::create(const QString &title, bool isOutput, QWidget *parent)
{
    if (isOutput)
        return new IoOutputPointWidget(title, parent);
    return new IoInputPointWidget(title, parent);
}

IoPointWidget::IoPointWidget(const QString &title, bool isOutput, QWidget *parent)
    : QFrame(parent)
    , m_title(title)
    , m_isOutput(isOutput)
{
    setObjectName(QStringLiteral("motionIoPoint"));
    setAttribute(Qt::WA_StyledBackground, true);
    initCommonUi();
}

void IoPointWidget::initCommonUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(12, 10, 12, 10);
    root->setSpacing(kMotionIoSectionSpacing);

    m_headLayout = new QHBoxLayout();
    m_headLayout->setSpacing(8);
    m_headLayout->setAlignment(Qt::AlignVCenter);

    m_portHeaderLbl = new QLabel(this);
    m_portHeaderLbl->setObjectName(QStringLiteral("motionIoPointPort"));
    updatePortHeaderLabel(0);
    m_headLayout->addWidget(m_portHeaderLbl, 0, Qt::AlignVCenter);

    auto *titleLbl = new QLabel(m_title, this);
    titleLbl->setObjectName(QStringLiteral("motionIoPointTitle"));
    titleLbl->setAlignment(Qt::AlignCenter);
    m_headLayout->addWidget(titleLbl, 1, Qt::AlignVCenter);

    root->addLayout(m_headLayout);

    m_detailSection = new QWidget(this);
    auto *detailOuter = new QHBoxLayout(m_detailSection);
    detailOuter->setContentsMargins(0, 0, 0, 0);
    detailOuter->addStretch();

    auto *formBlock = new QWidget(m_detailSection);
    auto *formBlockLay = new QHBoxLayout(formBlock);
    formBlockLay->setContentsMargins(0, 0, 0, kMotionIoFormBottomSpacing);
    formBlockLay->setSpacing(10);

    auto *labelCol = new QVBoxLayout();
    labelCol->setContentsMargins(0, 0, 0, 0);
    labelCol->setSpacing(kMotionIoFormRowSpacing);

    auto *ctrlCol = new QVBoxLayout();
    ctrlCol->setContentsMargins(0, 0, 0, 0);
    ctrlCol->setSpacing(kMotionIoFormRowSpacing);

    auto *levelLbl = new QLabel(QStringLiteral("有效电平"), formBlock);
    styleMotionIoFormLabel(levelLbl);

    auto *levelRow = new QHBoxLayout();
    levelRow->setContentsMargins(0, 0, 0, 0);
    levelRow->setSpacing(kMotionIoFormRadioSpacing);
    m_lowActive = new QRadioButton(QStringLiteral("低有效"), formBlock);
    m_lowActive->setObjectName(QStringLiteral("motionIoRadio"));
    m_highActive = new QRadioButton(QStringLiteral("高有效"), formBlock);
    m_highActive->setObjectName(QStringLiteral("motionIoRadio"));

    auto *activeLevelGroup = new QButtonGroup(formBlock);
    activeLevelGroup->setExclusive(true);
    activeLevelGroup->addButton(m_lowActive, 0);
    activeLevelGroup->addButton(m_highActive, 1);
    m_lowActive->setChecked(true);

    levelRow->addWidget(m_lowActive, 0, Qt::AlignVCenter);
    levelRow->addWidget(m_highActive, 0, Qt::AlignVCenter);
    levelRow->addStretch();

    auto *levelWrap = new QWidget(formBlock);
    levelWrap->setLayout(levelRow);

    auto *portLbl = new QLabel(QStringLiteral("端口号"), formBlock);
    styleMotionIoFormLabel(portLbl);

    m_portEdit = new QLineEdit(formBlock);
    m_portEdit->setObjectName(QStringLiteral("motionIoPortEdit"));
    m_portEdit->setValidator(new QIntValidator(0, 999, m_portEdit));
    m_portEdit->setAlignment(Qt::AlignCenter);

    const int ctrlWidth = calcMotionIoFormControlWidth(m_lowActive, m_highActive, m_portEdit);
    levelWrap->setFixedSize(ctrlWidth, kMotionIoFormRowHeight);
    m_portEdit->setFixedSize(ctrlWidth, kMotionIoFormRowHeight);

    labelCol->addWidget(levelLbl, 0, Qt::AlignRight | Qt::AlignVCenter);
    labelCol->addWidget(portLbl, 0, Qt::AlignRight | Qt::AlignVCenter);
    ctrlCol->addWidget(levelWrap, 0, Qt::AlignLeft | Qt::AlignVCenter);
    ctrlCol->addWidget(m_portEdit, 0, Qt::AlignLeft | Qt::AlignVCenter);

    formBlockLay->addLayout(labelCol);
    formBlockLay->addLayout(ctrlCol);
    detailOuter->addWidget(formBlock, 0, Qt::AlignHCenter);
    detailOuter->addStretch();

    root->addWidget(m_detailSection);
    applyDetailsVisibility();
}

void IoPointWidget::insertStateWidget(QWidget *widget)
{
    if (m_headLayout && widget)
        m_headLayout->addWidget(widget, 0, Qt::AlignVCenter);
}

int IoPointWidget::port() const
{
    if (!m_portEdit)
        return 0;
    bool ok = false;
    const int value = m_portEdit->text().trimmed().toInt(&ok);
    return ok ? value : 0;
}

bool IoPointWidget::highActive() const
{
    return m_highActive && m_highActive->isChecked();
}

void IoPointWidget::updatePortHeaderLabel(int port)
{
    if (m_portHeaderLbl)
        m_portHeaderLbl->setText(formatIoPointPortHeader(m_isOutput, port));
}

void IoPointWidget::applyFromConfig(int port, bool highActive)
{
    updatePortHeaderLabel(port);
    m_portEdit->setText(QString::number(port));
    if (highActive)
        m_highActive->setChecked(true);
    else
        m_lowActive->setChecked(true);
}

void IoPointWidget::setShowFullDetails(bool show)
{
    if (m_showFullDetails == show)
        return;
    m_showFullDetails = show;
    applyDetailsVisibility();
}

void IoPointWidget::applyDetailsVisibility()
{
    if (m_detailSection)
        m_detailSection->setVisible(m_showFullDetails);
    updateGeometry();
}

} // namespace motion
