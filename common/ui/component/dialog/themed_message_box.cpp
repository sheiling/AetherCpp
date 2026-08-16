/**
 * @file themed_message_box.cpp
 * @brief ThemedMessageBox 实现：组装消息正文与按钮行，委托 ThemedPanelDialog 展示
 */

#include "dialog/themed_message_box.h"

#include "controls/button/themed_push_button.h"
#include "dialog/themed_panel_dialog.h"

#include "uibase/languagemanager.h"

#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QObject>
#include <QVBoxLayout>

namespace ui {

namespace {

/** 创建正文标签，objectName=themedPanelMessageLabel，支持换行与鼠标选中 */
QLabel *makeMessageLabel(const QString &text, QWidget *parent)
{
    auto *lbl = new QLabel(text, parent);
    lbl->setObjectName(QStringLiteral("themedPanelMessageLabel"));
    lbl->setWordWrap(true);
    lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
    return lbl;
}

/**
 * 构建弹窗主体：正文 + 右对齐按钮行。
 * 取消钮连接 reject，确认钮连接 accept；按 confirmIsDefault 设置初始焦点。
 */
QWidget *buildMessageBody(const QString &text,
                          const QString &confirmButtonText,
                          const QString &cancelButtonText,
                          PushButtonVariant confirmVariant,
                          bool confirmIsDefault,
                          ThemedPanelDialog *dialog)
{
    auto *body = new QWidget;
    auto *vl   = new QVBoxLayout(body);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(12);

    vl->addWidget(makeMessageLabel(text, body));

    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(8);
    btnRow->addStretch();

    ThemedPushButton *cancelBtn = nullptr;
    if (!cancelButtonText.isEmpty()) {
        cancelBtn = makePushButton(cancelButtonText, PushButtonVariant::Secondary, body);
        btnRow->addWidget(cancelBtn);
        QObject::connect(cancelBtn, &ThemedPushButton::clicked, dialog, &QDialog::reject);
    }

    auto *confirmBtn = makePushButton(confirmButtonText, confirmVariant, body);
    btnRow->addWidget(confirmBtn);
    QObject::connect(confirmBtn, &ThemedPushButton::clicked, dialog, &QDialog::accept);

    if (confirmIsDefault)
        confirmBtn->setFocus();
    else if (cancelBtn)
        cancelBtn->setFocus();

    vl->addLayout(btnRow);

    Q_UNUSED(cancelBtn);
    return body;
}

} // namespace

bool ThemedMessageBox::ask(QWidget *parent,
                           const QString &title,
                           const QString &text,
                           const QString &confirmButtonText,
                           const QString &cancelButtonText,
                           const ThemedMessageBoxIcon icon,
                           const bool confirmIsDefault,
                           const PushButtonVariant confirmVariant)
{
    Q_UNUSED(icon);

    ThemedPanelDialog dlg(parent ? parent->window() : nullptr);
    dlg.setModal(true);
    dlg.setTitleText(title);
    dlg.setMinimumPanelWidth(360);
    dlg.setBodyWidget(buildMessageBody(text, confirmButtonText, cancelButtonText, confirmVariant,
                                       confirmIsDefault, &dlg));
    dlg.applyTheme();
    dlg.finalizeLayoutSize();

    return dlg.exec() == QDialog::Accepted;
}

void ThemedMessageBox::information(QWidget *parent,
                                   const QString &title,
                                   const QString &text,
                                   const QString &buttonText,
                                   const ThemedMessageBoxIcon icon,
                                   const PushButtonVariant buttonVariant)
{
    Q_UNUSED(icon);

    const QString okText =
        buttonText.isEmpty() ? TR("aoi.recipe.add_dialog.confirm") : buttonText;

    ThemedPanelDialog dlg(parent ? parent->window() : nullptr);
    dlg.setModal(true);
    dlg.setTitleText(title);
    dlg.setMinimumPanelWidth(360);
    dlg.setBodyWidget(
        buildMessageBody(text, okText, QString(), buttonVariant, true, &dlg));
    dlg.applyTheme();
    dlg.finalizeLayoutSize();
    dlg.exec();
}

} // namespace ui
