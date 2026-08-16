/**
 * @file defect_review_shortcut_panel.cpp
 */

#include "review/defect_review_shortcut_panel.h"

#include "component/theme/theme_refresh.h"
#include "component/theme/theme_tokens.h"
#include "uibase/languagemanager.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace aoi::review {
namespace {

constexpr int kRowSpacing      = 6;
constexpr int kGroupSpacing    = 12;
constexpr int kKeyTextSpacing  = 4;
constexpr int kMultiKeySpacing = 4;
constexpr int kTitleBottomGap  = 6;

QString keyCapStyleSheet(const ui::ThemeTokens &t)
{
    const QColor border = t.accent.isValid() ? t.accent : QColor(0x64, 0x96, 0xff);
    const QColor bg     = t.inputBackground.isValid() ? t.inputBackground : t.surface;
    return QStringLiteral(
        "color: %1; background: %2; border: 1px solid %3;"
        " border-radius: 4px; padding: 1px 6px; font-size: 12px; font-weight: 600;")
        .arg(t.textPrimary.name(QColor::HexRgb), bg.name(QColor::HexRgb), border.name(QColor::HexRgb));
}

QString descPillStyleSheet(const ui::ThemeTokens &t)
{
    const QColor fg = t.textSecondary.isValid() ? t.textSecondary : t.textPrimary;
    const QColor bg =
        t.surfaceElevated.isValid() ? t.surfaceElevated : t.inputBackground;
    return QStringLiteral(
        "color: %1; font-size: 12px; background: %2; border: none;"
        " border-radius: 4px; padding: 2px 8px;")
        .arg(fg.name(QColor::HexRgb), bg.name(QColor::HexRgb));
}

} // namespace

DefectReviewShortcutPanel::DefectReviewShortcutPanel(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("defectReviewShortcutPanel"));
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    buildUi();
    ui::bindThemeRefresh(this, [this]() {
        applyTheme();
        retranslateUi();
    });
    applyTheme();
    retranslateUi();
}

void DefectReviewShortcutPanel::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 4);
    root->setSpacing(0);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setObjectName(QStringLiteral("defectReviewShortcutTitle"));
    root->addWidget(m_titleLabel);
    root->addSpacing(kTitleBottomGap);

    auto *row1 = new QHBoxLayout();
    row1->setContentsMargins(0, 0, 0, 0);
    row1->setSpacing(kGroupSpacing);
    row1->addWidget(makeShortcutGroup({QStringLiteral("↑"), QStringLiteral("↓")}, &m_prevNextDesc));
    row1->addWidget(makeShortcutGroup({QStringLiteral("Enter")}, &m_enterDesc));
    root->addLayout(row1);
    root->addSpacing(kRowSpacing);

    auto *row2 = new QHBoxLayout();
    row2->setContentsMargins(0, 0, 0, 0);
    row2->setSpacing(kGroupSpacing);
    row2->addWidget(makeShortcutGroup({QStringLiteral("O")}, &m_okDesc));
    row2->addWidget(makeShortcutGroup({QStringLiteral("F1–F11")}, &m_defectToggleDesc));
    row2->addWidget(makeShortcutGroup({QStringLiteral("R")}, &m_resetDesc));
    root->addLayout(row2);
}

QLabel *DefectReviewShortcutPanel::makeKeyCap(const QString &text)
{
    auto *cap = new QLabel(text, this);
    cap->setObjectName(QStringLiteral("defectReviewShortcutKeyCap"));
    cap->setAlignment(Qt::AlignCenter);
    cap->setMinimumHeight(22);
    cap->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    return cap;
}

QWidget *DefectReviewShortcutPanel::makeShortcutGroup(const QStringList &keyTexts, QLabel **outDesc)
{
    auto *group = new QWidget(this);
    group->setObjectName(QStringLiteral("defectReviewShortcutRow"));
    auto *layout = new QHBoxLayout(group);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(kKeyTextSpacing);

    auto *keysWrap = new QWidget(group);
    auto *keysLayout = new QHBoxLayout(keysWrap);
    keysLayout->setContentsMargins(0, 0, 0, 0);
    keysLayout->setSpacing(kMultiKeySpacing);
    for (const QString &keyText : keyTexts)
        keysLayout->addWidget(makeKeyCap(keyText));

    auto *desc = new QLabel(group);
    desc->setObjectName(QStringLiteral("defectReviewShortcutDesc"));
    desc->setWordWrap(false);
    desc->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    layout->addWidget(keysWrap, 0, Qt::AlignVCenter);
    layout->addWidget(desc, 0, Qt::AlignVCenter);

    if (outDesc)
        *outDesc = desc;
    return group;
}

void DefectReviewShortcutPanel::applyTheme()
{
    const ui::ThemeTokens t         = ui::ThemeTokens::resolve();
    const QString         capStyle  = keyCapStyleSheet(t);
    const QString         descStyle = descPillStyleSheet(t);

    if (m_titleLabel) {
        m_titleLabel->setStyleSheet(QStringLiteral(
            "color: %1; font-size: 13px; font-weight: 600; background: transparent; border: none;")
                                        .arg(t.textPrimary.name(QColor::HexRgb)));
    }

    for (QLabel *cap : findChildren<QLabel *>(QStringLiteral("defectReviewShortcutKeyCap")))
        cap->setStyleSheet(capStyle);

    for (QLabel *desc : findChildren<QLabel *>(QStringLiteral("defectReviewShortcutDesc")))
        desc->setStyleSheet(descStyle);
}

void DefectReviewShortcutPanel::retranslateUi()
{
    if (m_titleLabel)
        m_titleLabel->setText(TR("aoi.review.shortcuts.title"));
    if (m_prevNextDesc)
        m_prevNextDesc->setText(TR("aoi.review.shortcut.prev_next"));
    if (m_defectToggleDesc)
        m_defectToggleDesc->setText(TR("aoi.review.shortcut.toggle_defect"));
    if (m_enterDesc)
        m_enterDesc->setText(TR("aoi.review.shortcut.confirm_next"));
    if (m_okDesc)
        m_okDesc->setText(TR("aoi.review.shortcut.rejudge_ok"));
    if (m_resetDesc)
        m_resetDesc->setText(TR("aoi.review.shortcut.reset_pending"));
}

} // namespace aoi::review
