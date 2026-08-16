#pragma once

#include "ui/view/perm_text.h"
#include "ui/view/permission_ui_utils.h"

#include "controls/button/themed_push_button.h"

#include <QApplication>
#include <QFontMetrics>
#include <QDialogButtonBox>
#include <QEventLoop>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QScreen>
#include <QScrollArea>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

namespace permission_ui {

inline QString messagePopupStandardButtonText(QDialogButtonBox::StandardButton b)
{
    switch (b) {
    case QDialogButtonBox::Ok:
        return PTXT("perm.common.ok");
    case QDialogButtonBox::Cancel:
        return PTXT("perm.common.cancel");
    case QDialogButtonBox::Yes:
        return PTXT("perm.common.yes");
    case QDialogButtonBox::No:
        return PTXT("perm.common.no");
    default:
        return QString();
    }
}

/// QWidget 版轻量弹窗：标题 / 正文 / 按钮行（按钮使用 QDialogButtonBox::StandardButtons）
class MessagePopup
{
public:
    static inline QDialogButtonBox::StandardButton show(
        QWidget *parent,
        const QString &title,
        const QString &text,
        QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok,
        QDialogButtonBox::StandardButton defaultButton = QDialogButtonBox::Ok)
    {
        auto *dlg = new QWidget(parent, Qt::Dialog | Qt::FramelessWindowHint);
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->setWindowModality(Qt::ApplicationModal);
        dlg->setFixedSize(360, 160);

        applyPermissionTheme(dlg);
        dlg->setStyleSheet(dlg->styleSheet() + permissionDialogPanelStyleSheet());

        auto *root = new QVBoxLayout(dlg);
        root->setContentsMargins(20, 16, 20, 16);
        root->setSpacing(12);

        auto *lblTitle = new QLabel(title, dlg);
        initPageTitle(lblTitle);

        auto *lblText = new QLabel(text, dlg);
        lblText->setWordWrap(true);
        initDescLabel(lblText);

        auto *line = new QFrame(dlg);
        line->setFrameShape(QFrame::HLine);
        line->setStyleSheet(permissionDialogDividerStyleSheet());

        auto *btnRow = new QHBoxLayout;
        btnRow->addStretch();

        QDialogButtonBox::StandardButton clicked = QDialogButtonBox::NoButton;
        ui::ThemedPushButton *defaultBtn = nullptr;
        const auto addBtn = [&](QDialogButtonBox::StandardButton b) {
            auto *btn = makeSecondaryButton(messagePopupStandardButtonText(b), dlg);
            btnRow->addWidget(btn);
            if (b == defaultButton)
                defaultBtn = btn;
            QObject::connect(btn, &ui::ThemedPushButton::clicked, dlg, [dlg, &clicked, b] {
                clicked = b;
                dlg->close();
            });
        };

        const QVector<QDialogButtonBox::StandardButton> order = {
            QDialogButtonBox::Cancel,
            QDialogButtonBox::No,
            QDialogButtonBox::Yes,
            QDialogButtonBox::Ok,
        };
        for (auto b : order) {
            if (buttons.testFlag(b))
                addBtn(b);
        }

        root->addWidget(lblTitle);
        root->addWidget(lblText, 1);
        root->addWidget(line);
        root->addLayout(btnRow);

        if (defaultBtn)
            defaultBtn->setDefault(true);

        if (QWidget *host = parent ? parent->window() : nullptr) {
            const QPoint c = host->geometry().center();
            dlg->move(c.x() - dlg->width() / 2, c.y() - dlg->height() / 2);
        }

        QEventLoop loop;
        QObject::connect(dlg, &QWidget::destroyed, &loop, &QEventLoop::quit);
        dlg->show();
        loop.exec();
        return clicked == QDialogButtonBox::NoButton ? defaultButton : clicked;
    }

    /// 长文本弹窗：可滚动、按内容自适应高度（审计详情等）
    static inline QDialogButtonBox::StandardButton showScrollable(
        QWidget *parent,
        const QString &title,
        const QString &text,
        QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Ok,
        QDialogButtonBox::StandardButton defaultButton = QDialogButtonBox::Ok)
    {
        auto *dlg = new QWidget(parent, Qt::Dialog | Qt::FramelessWindowHint);
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->setWindowModality(Qt::ApplicationModal);
        dlg->setMinimumSize(520, 280);

        applyPermissionTheme(dlg);
        dlg->setStyleSheet(dlg->styleSheet() + permissionDialogPanelStyleSheet());

        auto *root = new QVBoxLayout(dlg);
        root->setContentsMargins(20, 16, 20, 16);
        root->setSpacing(12);

        auto *lblTitle = new QLabel(title, dlg);
        initPageTitle(lblTitle);

        auto *scroll = new QScrollArea(dlg);
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scroll->setStyleSheet(QStringLiteral("QScrollArea{background:transparent;}"));

        auto *lblText = new QLabel(text, dlg);
        lblText->setWordWrap(true);
        lblText->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        lblText->setTextInteractionFlags(Qt::TextSelectableByMouse);
        initDescLabel(lblText);
        scroll->setWidget(lblText);

        auto *line = new QFrame(dlg);
        line->setFrameShape(QFrame::HLine);
        line->setStyleSheet(permissionDialogDividerStyleSheet());

        auto *btnRow = new QHBoxLayout;
        btnRow->addStretch();

        QDialogButtonBox::StandardButton clicked = QDialogButtonBox::NoButton;
        ui::ThemedPushButton *defaultBtn = nullptr;
        const auto addBtn = [&](QDialogButtonBox::StandardButton b) {
            auto *btn = makeSecondaryButton(messagePopupStandardButtonText(b), dlg);
            btnRow->addWidget(btn);
            if (b == defaultButton)
                defaultBtn = btn;
            QObject::connect(btn, &ui::ThemedPushButton::clicked, dlg, [dlg, &clicked, b] {
                clicked = b;
                dlg->close();
            });
        };

        const QVector<QDialogButtonBox::StandardButton> order = {
            QDialogButtonBox::Cancel,
            QDialogButtonBox::No,
            QDialogButtonBox::Yes,
            QDialogButtonBox::Ok,
        };
        for (auto b : order) {
            if (buttons.testFlag(b))
                addBtn(b);
        }

        root->addWidget(lblTitle);
        root->addWidget(scroll, 1);
        root->addWidget(line);
        root->addLayout(btnRow);

        if (defaultBtn)
            defaultBtn->setDefault(true);

        const QFontMetrics fm(lblText->font());
        const int contentH =
            qMin(fm.boundingRect(QRect(0, 0, 480, 10000), Qt::TextWordWrap, text).height() + 24, 520);
        int dlgH = 72 + contentH + 56;
        if (QScreen *screen = QApplication::primaryScreen()) {
            dlgH = qMin(dlgH, static_cast<int>(screen->availableGeometry().height() * 0.78));
        }
        dlg->resize(560, qMax(320, dlgH));

        if (QWidget *host = parent ? parent->window() : nullptr) {
            const QPoint c = host->geometry().center();
            dlg->move(c.x() - dlg->width() / 2, c.y() - dlg->height() / 2);
        }

        QEventLoop loop;
        QObject::connect(dlg, &QWidget::destroyed, &loop, &QEventLoop::quit);
        dlg->show();
        loop.exec();
        return clicked == QDialogButtonBox::NoButton ? defaultButton : clicked;
    }
};

} // namespace permission_ui
