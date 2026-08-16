/**
 * @file mark_template_dialog.cpp
 * @brief Mark 模板制作对话框实现：路径/名称/类型表单 + ROI 提示 + 制作请求
 */

#include "mark_template/mark_template_dialog.h"

#include "component/controls/button/push_button_style.h"
#include "component/controls/button/themed_push_button.h"
#include "component/controls/combo/themed_combo_box.h"
#include "component/controls/input/themed_line_edit.h"
#include "component/controls/label/themed_form_label.h"

#include "uibase/languagemanager.h"

#include <QCoreApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace aoi {

using namespace ui;

namespace {

constexpr int kPanelMinWidth = 540;

struct ModelTypeEntry
{
    MarkTemplateKind type;
    const char      *trKey;
};

constexpr ModelTypeEntry kModelTypes[] = {
    {MarkTemplateKind::Mark, "aoi.recipe.mark.model_type.mark"},
    {MarkTemplateKind::Chip, "aoi.recipe.mark.model_type.chip"},
    {MarkTemplateKind::Die, "aoi.recipe.mark.model_type.die"},
    {MarkTemplateKind::Solder, "aoi.recipe.mark.model_type.solder"},
    {MarkTemplateKind::Socket, "aoi.recipe.mark.model_type.socket"},
    {MarkTemplateKind::MicroCell, "aoi.recipe.mark.model_type.micro_cell"},
};

} // namespace

// ── 构造 / 单例 open ──

MarkTemplateDialog::MarkTemplateDialog(QWidget *parent)
    : ui::ThemedPanelDialog(parent)
{
    setObjectName(QStringLiteral("aoiMarkTemplateDialog"));
    applyTheme();
    setTitleKey(QStringLiteral("aoi.recipe.mark.template_dialog.title"));
    setMinimumPanelWidth(kPanelMinWidth);
    buildForm();
    finalizeLayoutSize();
}

MarkTemplateDialog *MarkTemplateDialog::open(QWidget *parent, const ShowParams &params)
{
    if (!parent)
        return nullptr;

    auto *dlg = parent->findChild<MarkTemplateDialog *>();
    if (!dlg)
        dlg = new MarkTemplateDialog(parent);

    if (dlg->isVisible()) {
        dlg->raise();
        dlg->activateWindow();
        return dlg;
    }

    dlg->applyShowParams(params);
    dlg->show();
    dlg->raise();
    dlg->activateWindow();
    return dlg;
}

void MarkTemplateDialog::applyShowParams(const ShowParams &params)
{
    setDefaultPath(params.defaultPath);
    setDefaultName(params.defaultName);
    setModelType(params.modelType);
    setMaking(false);
}

// ── 表单布局 ──

void MarkTemplateDialog::buildForm()
{
    auto *body = new QWidget(this);
    auto *layout = new QVBoxLayout(body);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    auto addRow = [&](const char *labelKey, QWidget *field) {
        layout->addWidget(makeFormLabel(QString::fromUtf8(labelKey), body, 11));
        layout->addWidget(field);
    };

    auto *pathRow = new QWidget(body);
    auto *pathHl  = new QHBoxLayout(pathRow);
    pathHl->setContentsMargins(0, 0, 0, 0);
    pathHl->setSpacing(6);
    m_pathEdit  = makeLineEdit(pathRow);
    m_browseBtn = makePushButton(TR("aoi.recipe.mark.template_browse"), PushButtonVariant::Secondary,
                                 pathRow);
    pathHl->addWidget(m_pathEdit, 1);
    pathHl->addWidget(m_browseBtn, 0);
    addRow("aoi.recipe.mark.template_path", pathRow);

    m_nameEdit = makeLineEdit(body);
    addRow("aoi.recipe.mark.template_name", m_nameEdit);

    m_typeCombo = makeComboBox(body);
    for (const auto &entry : kModelTypes)
        m_typeCombo->addItem(TR(entry.trKey), static_cast<int>(entry.type));
    addRow("aoi.recipe.mark.template_type", m_typeCombo);

    m_roiLabel = new QLabel(body);
    m_roiLabel->setObjectName(QStringLiteral("themedPanelHintLabel"));
    m_roiLabel->setWordWrap(true);
    m_roiLabel->setMinimumHeight(32);
    layout->addWidget(m_roiLabel);

    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    m_closeBtn = makePushButton(TR("aoi.recipe.add_dialog.cancel"), PushButtonVariant::Secondary,
                                body);
    m_makeBtn  = makePushButton(TR("aoi.recipe.mark.template_make"), PushButtonVariant::Primary,
                                body);
    btnRow->addWidget(m_closeBtn);
    btnRow->addWidget(m_makeBtn);
    layout->addLayout(btnRow);

    connect(m_browseBtn, &ui::ThemedPushButton::clicked, this, &MarkTemplateDialog::browseModelPath);
    connect(m_makeBtn, &ui::ThemedPushButton::clicked, this, &MarkTemplateDialog::makeRequested);
    connect(m_closeBtn, &ui::ThemedPushButton::clicked, this, &QDialog::close);

    setBodyWidget(body);
    retranslateForm();
    updateRoiHint(QRect(), false);
}

void MarkTemplateDialog::retranslateBody()
{
    retranslateForm();
}

// ── 参数 / 状态 ──

void MarkTemplateDialog::setDefaultPath(const QString &path)
{
    if (m_pathEdit)
        m_pathEdit->setText(path);
}

void MarkTemplateDialog::setDefaultName(const QString &name)
{
    if (m_nameEdit)
        m_nameEdit->setText(name);
}

void MarkTemplateDialog::setModelType(MarkTemplateKind type)
{
    if (!m_typeCombo)
        return;
    const int value = static_cast<int>(type);
    for (int i = 0; i < m_typeCombo->count(); ++i) {
        if (m_typeCombo->itemData(i).toInt() == value) {
            m_typeCombo->setCurrentIndex(i);
            return;
        }
    }
}

void MarkTemplateDialog::updateRoiHint(const QRect &roi, bool valid)
{
    if (!m_roiLabel)
        return;
    if (!valid) {
        m_roiLabel->setText(TR("aoi.recipe.mark.roi_hint_empty"));
        return;
    }
    m_roiLabel->setText(TR("aoi.recipe.mark.roi_hint")
                            .arg(roi.x())
                            .arg(roi.y())
                            .arg(roi.width())
                            .arg(roi.height()));
}

QString MarkTemplateDialog::modelPath() const
{
    return m_pathEdit ? m_pathEdit->text().trimmed() : QString{};
}

QString MarkTemplateDialog::modelName() const
{
    return m_nameEdit ? m_nameEdit->text().trimmed() : QString{};
}

MarkTemplateKind MarkTemplateDialog::modelType() const
{
    if (!m_typeCombo)
        return MarkTemplateKind::Mark;
    return static_cast<MarkTemplateKind>(m_typeCombo->currentData().toInt());
}

void MarkTemplateDialog::setMaking(bool making)
{
    m_making = making;
    if (m_makeBtn)
        m_makeBtn->setEnabled(!making);
    if (m_browseBtn)
        m_browseBtn->setEnabled(!making);
    if (m_pathEdit)
        m_pathEdit->setReadOnly(making);
    if (m_nameEdit)
        m_nameEdit->setReadOnly(making);
    if (m_typeCombo)
        m_typeCombo->setEnabled(!making);
}

// ── 交互 ──

void MarkTemplateDialog::browseModelPath()
{
    const QString modelDir = QCoreApplication::applicationDirPath() + QStringLiteral("/model");
    const QString path     = QFileDialog::getSaveFileName(
        this,
        TR("aoi.recipe.mark.template_browse"),
        modelDir + QStringLiteral("/mark_template.model"),
        QStringLiteral("Model files (*.model);;All files (*)"));
    if (!path.isEmpty() && m_pathEdit)
        m_pathEdit->setText(path);
}

void MarkTemplateDialog::retranslateForm()
{
    if (m_browseBtn)
        m_browseBtn->setText(TR("aoi.recipe.mark.template_browse"));
    if (m_makeBtn)
        m_makeBtn->setText(TR("aoi.recipe.mark.template_make"));
    if (m_closeBtn)
        m_closeBtn->setText(TR("aoi.recipe.add_dialog.cancel"));

    if (QWidget *body = bodyWidget())
        ui::ThemedFormLabel::retranslateIn(body);

    if (m_typeCombo) {
        const int current = m_typeCombo->currentIndex();
        m_typeCombo->clear();
        for (const auto &entry : kModelTypes)
            m_typeCombo->addItem(TR(entry.trKey), static_cast<int>(entry.type));
        if (current >= 0 && current < m_typeCombo->count())
            m_typeCombo->setCurrentIndex(current);
    }
}

} // namespace aoi
