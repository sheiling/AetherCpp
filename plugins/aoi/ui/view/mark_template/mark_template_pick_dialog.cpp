/**
 * @file mark_template_pick_dialog.cpp
 * @brief 选择已有 Mark 模板对话框实现：浏览 .model + 名称下拉 + 确认/删除
 */

#include "mark_template/mark_template_pick_dialog.h"

#include "component/controls/button/push_button_style.h"
#include "component/controls/button/themed_push_button.h"
#include "component/controls/combo/themed_combo_box.h"
#include "component/controls/input/themed_line_edit.h"
#include "component/controls/label/themed_form_label.h"
#include "service/vision_bridge/model_generator.h"

#include "uibase/languagemanager.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace aoi {

using namespace ui;

namespace {

constexpr int kPanelMinWidth = 540;

} // namespace

// ── 构造 / 单例 open ──

MarkTemplatePickDialog::MarkTemplatePickDialog(QWidget *parent)
    : ui::ThemedPanelDialog(parent)
{
    setObjectName(QStringLiteral("aoiMarkTemplatePickDialog"));
    applyTheme();
    setTitleKey(QStringLiteral("aoi.recipe.mark.pick_dialog.title"));
    setMinimumPanelWidth(kPanelMinWidth);
    buildForm();
    finalizeLayoutSize();
}

MarkTemplatePickDialog *MarkTemplatePickDialog::open(QWidget *parent, const ShowParams &params)
{
    if (!parent)
        return nullptr;

    auto *dlg = parent->findChild<MarkTemplatePickDialog *>();
    if (!dlg)
        dlg = new MarkTemplatePickDialog(parent);

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

void MarkTemplatePickDialog::applyShowParams(const ShowParams &params)
{
    setDefaultPath(params.defaultPath);
    setDefaultName(params.defaultName);
    setBusy(false);
}

// ── 表单布局 ──

void MarkTemplatePickDialog::buildForm()
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
    m_pathEdit->setReadOnly(true);
    m_browseBtn = makePushButton(TR("aoi.recipe.mark.template_browse"), PushButtonVariant::Secondary,
                                 pathRow);
    pathHl->addWidget(m_pathEdit, 1);
    pathHl->addWidget(m_browseBtn, 0);
    addRow("aoi.recipe.mark.template_file", pathRow);

    m_nameCombo = makeComboBox(body);
    addRow("aoi.recipe.mark.template_name", m_nameCombo);

    auto *btnRow = new QHBoxLayout;
    m_deleteBtn = new QPushButton(TR("aoi.recipe.mark.template_delete"), body);
    m_deleteBtn->setObjectName(QStringLiteral("themedPanelDangerBtn"));
    btnRow->addWidget(m_deleteBtn);
    btnRow->addStretch();
    m_closeBtn = makePushButton(TR("aoi.recipe.add_dialog.cancel"), PushButtonVariant::Secondary,
                                body);
    m_confirmBtn =
        makePushButton(TR("aoi.recipe.mark.template_confirm"), PushButtonVariant::Primary, body);
    btnRow->addWidget(m_closeBtn);
    btnRow->addWidget(m_confirmBtn);
    layout->addLayout(btnRow);

    connect(m_browseBtn, &ui::ThemedPushButton::clicked, this, &MarkTemplatePickDialog::browseModelPath);
    connect(m_confirmBtn, &ui::ThemedPushButton::clicked, this, &MarkTemplatePickDialog::onConfirmClicked);
    connect(m_deleteBtn, &QPushButton::clicked, this, &MarkTemplatePickDialog::onDeleteClicked);
    connect(m_closeBtn, &ui::ThemedPushButton::clicked, this, &QDialog::close);

    setBodyWidget(body);
    retranslateForm();
    updateActionStates();
}

void MarkTemplatePickDialog::retranslateBody()
{
    retranslateForm();
}

void MarkTemplatePickDialog::setDefaultPath(const QString &path)
{
    if (m_pathEdit)
        m_pathEdit->setText(path);
    refreshModelNames();
}

void MarkTemplatePickDialog::setDefaultName(const QString &name)
{
    if (!m_nameCombo || name.isEmpty())
        return;
    const int idx = m_nameCombo->findText(name);
    if (idx >= 0)
        m_nameCombo->setCurrentIndex(idx);
}

QString MarkTemplatePickDialog::modelPath() const
{
    return m_pathEdit ? m_pathEdit->text().trimmed() : QString{};
}

QString MarkTemplatePickDialog::modelName() const
{
    return m_nameCombo ? m_nameCombo->currentText().trimmed() : QString{};
}

// ── 模型列表 / 按钮状态 ──

void MarkTemplatePickDialog::refreshModelNames()
{
    const QString path = modelPath();
    if (path.isEmpty() || !QFileInfo::exists(path)) {
        setNameItems({});
        return;
    }

    QStringList names;
    if (!ModelGenerator::listModelNames(path, names).ok) {
        setNameItems({});
        return;
    }

    setNameItems(names);
}

void MarkTemplatePickDialog::setBusy(bool busy)
{
    if (m_browseBtn)
        m_browseBtn->setEnabled(!busy);
    if (m_nameCombo)
        m_nameCombo->setEnabled(!busy);
    if (m_confirmBtn)
        m_confirmBtn->setEnabled(!busy);
    if (m_deleteBtn)
        m_deleteBtn->setEnabled(!busy);
    if (m_closeBtn)
        m_closeBtn->setEnabled(!busy);
}

void MarkTemplatePickDialog::setNameItems(const QStringList &names)
{
    if (!m_nameCombo)
        return;
    const QString previous = m_nameCombo->currentText();
    m_nameCombo->clear();
    if (!names.isEmpty())
        m_nameCombo->addItems(names);
    const int idx = m_nameCombo->findText(previous);
    if (idx >= 0)
        m_nameCombo->setCurrentIndex(idx);
    updateActionStates();
}

void MarkTemplatePickDialog::updateActionStates()
{
    const bool hasPath = !modelPath().isEmpty() && QFileInfo::exists(modelPath());
    const bool hasName = m_nameCombo && m_nameCombo->count() > 0;
    if (m_confirmBtn)
        m_confirmBtn->setEnabled(hasPath && hasName);
    if (m_deleteBtn)
        m_deleteBtn->setEnabled(hasPath && hasName);
}

// ── 确认 / 删除 / 浏览 ──

void MarkTemplatePickDialog::onConfirmClicked()
{
    if (modelPath().isEmpty() || modelName().isEmpty())
        return;
    emit pickConfirmed(modelPath(), modelName());
}

void MarkTemplatePickDialog::onDeleteClicked()
{
    if (modelPath().isEmpty() || modelName().isEmpty())
        return;
    emit deleteRequested(modelPath(), modelName());
}

void MarkTemplatePickDialog::browseModelPath()
{
    const QString modelDir = ModelGenerator::defaultModelDir();
    const QString path     = QFileDialog::getOpenFileName(
        this,
        TR("aoi.recipe.mark.template_browse"),
        modelDir,
        QStringLiteral("Model files (*.model);;All files (*)"));
    if (path.isEmpty())
        return;

    if (m_pathEdit)
        m_pathEdit->setText(path);
    refreshModelNames();
}

void MarkTemplatePickDialog::retranslateForm()
{
    if (m_browseBtn)
        m_browseBtn->setText(TR("aoi.recipe.mark.template_browse"));
    if (m_confirmBtn)
        m_confirmBtn->setText(TR("aoi.recipe.mark.template_confirm"));
    if (m_deleteBtn)
        m_deleteBtn->setText(TR("aoi.recipe.mark.template_delete"));
    if (m_closeBtn)
        m_closeBtn->setText(TR("aoi.recipe.add_dialog.cancel"));

    if (QWidget *body = bodyWidget())
        ui::ThemedFormLabel::retranslateIn(body);
}

} // namespace aoi
