#include "motion_catalog_dialogs.h"

#include "component/controls/button/push_button_style.h"
#include "component/controls/button/themed_push_button.h"
#include "component/controls/label/themed_form_label.h"
#include "component/controls/input/themed_line_edit.h"
#include "component/dialog/themed_panel_dialog.h"
#include "uibase/languagemanager.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

namespace motion_control_plugin {

namespace {

QString trKey(const char *key)
{
    return LanguageManager::instance()->t(QString::fromLatin1(key));
}

} // namespace

bool promptNameModuleDialog(QWidget *parent,
                            const QString &titleKey,
                            const QString &nameLabelKey,
                            const QString &moduleLabelKey,
                            const QString &namePlaceholderKey,
                            const QString &modulePlaceholderKey,
                            const QString &defaultName,
                            const QString &defaultModuleName,
                            QString *nameOut,
                            QString *moduleOut)
{
    if (!nameOut || !moduleOut)
        return false;

    ui::ThemedPanelDialog dlg(parent ? parent->window() : nullptr);
    dlg.setModal(true);
    dlg.setTitleKey(titleKey);
    dlg.setMinimumPanelWidth(360);

    auto *body = new QWidget;
    auto *bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(10);

    auto *nameLabel = ui::makeFormLabel(nameLabelKey, body, 11);
    auto *nameEdit  = ui::makeLineEdit(body);
    nameEdit->setText(defaultName);
    nameEdit->setMinimumWidth(280);
    nameEdit->setMinimumHeight(30);
    if (!namePlaceholderKey.isEmpty())
        nameEdit->setPlaceholderText(trKey(namePlaceholderKey.toLatin1().constData()));

    auto *moduleLabel = ui::makeFormLabel(moduleLabelKey, body, 11);
    auto *moduleEdit  = ui::makeLineEdit(body);
    moduleEdit->setText(defaultModuleName);
    moduleEdit->setMinimumWidth(280);
    moduleEdit->setMinimumHeight(30);
    if (!modulePlaceholderKey.isEmpty())
        moduleEdit->setPlaceholderText(trKey(modulePlaceholderKey.toLatin1().constData()));

    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(8);
    btnRow->addStretch();
    auto *cancelBtn = ui::makePushButton(trKey("motion.dialog.cancel"),
                                         ui::PushButtonVariant::Secondary, body);
    auto *confirmBtn = ui::makePushButton(trKey("motion.dialog.confirm"),
                                          ui::PushButtonVariant::Primary, body);
    btnRow->addWidget(cancelBtn);
    btnRow->addWidget(confirmBtn);

    bodyLayout->addWidget(nameLabel);
    bodyLayout->addWidget(nameEdit);
    bodyLayout->addWidget(moduleLabel);
    bodyLayout->addWidget(moduleEdit);
    bodyLayout->addLayout(btnRow);

    dlg.setBodyWidget(body);
    dlg.finalizeLayoutSize();

    QObject::connect(cancelBtn, &ui::ThemedPushButton::clicked, &dlg, &QDialog::reject);
    QObject::connect(confirmBtn, &ui::ThemedPushButton::clicked, &dlg, &QDialog::accept);
    QObject::connect(nameEdit, &ui::ThemedLineEdit::returnPressed, moduleEdit,
                     static_cast<void (QWidget::*)()>(&QWidget::setFocus));
    QObject::connect(moduleEdit, &ui::ThemedLineEdit::returnPressed, &dlg, &QDialog::accept);

    nameEdit->setFocus();
    nameEdit->selectAll();

    if (dlg.exec() != QDialog::Accepted)
        return false;

    *nameOut   = nameEdit->text().trimmed();
    *moduleOut = moduleEdit->text().trimmed();
    return !nameOut->isEmpty();
}

} // namespace motion_control_plugin
