#pragma once

/**
 * @file permission_ui_utils.h
 * @brief 权限插件 UI 布局约定与自绘辅助（主题对齐 AOI：标准控件用 Themed*，此处仅保留布局与 L3 自绘）
 *
 * 用法：
 *   1. 子类 override applyTheme() { permission_ui::applyPermissionTheme(this); }
 *   2. onLoad() 开头同样调用 applyPermissionTheme(this)
 *   3. 筛选栏用 QFrame + initFilterToolbar；输入用 ui::ThemedLineEdit
 *   4. 下拉用 PermComboBox（继承 ThemedComboBox）
 *   5. 日期用 PermDateTimeEdit（ui::ThemedDateTimeEdit 别名）
 *   6. 资源表单标签用 ui::ThemedFormLabel；排序用 ui::ThemedSpinBox
 */

#include "perm_combobox.h"
#include "perm_datetime_edit.h"

#include "controls/button/push_button_style.h"
#include "controls/button/themed_push_button.h"
#include "controls/input/themed_line_edit.h"
#include "controls/label/themed_form_label.h"
#include "controls/spinbox/themed_spinbox.h"
#include "table/table_style.h"
#include "theme/theme_color_math.h"
#include "theme/theme_refresh.h"
#include "theme/theme_tokens.h"
#include "uibase/thememanager.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QListWidget>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPushButton>
#include <QSizePolicy>
#include <QSpinBox>
#include <QTableView>
#include <QTextEdit>
#include <QVariant>
#include <QWidget>

namespace permission_ui {

inline constexpr const char *kObjPageRoot      = "permPluginRoot";
inline constexpr const char *kObjFilterToolbar = "permFilterToolbar";
inline constexpr const char *kObjFooter        = "permFooter";
inline constexpr const char *kObjPageTitle     = "permPageTitle";
inline constexpr const char *kObjUsersTitle    = "permUsersPageTitle";
inline constexpr const char *kObjTable         = "permTable";
inline constexpr const char *kObjAuditTable    = "permAuditTable";
inline constexpr const char *kObjAuditFilterBtn = "permAuditFilterBtn";
inline constexpr const char *kObjUserList        = "permUserList";
inline constexpr const char *kObjUserListWrap   = "permUserListWrap";
inline constexpr const char *kObjTableActionCell = "permTableActionCell";
inline constexpr const char *kObjTableRoleCell   = "permTableRoleCell";
inline constexpr const char *kObjRoleList      = "permRoleList";
inline constexpr const char *kObjPermCheckBox  = "permCheckBox";
inline constexpr const char *kObjMutedLabel    = "permMuted";
inline constexpr const char *kObjDescLabel     = "permDesc";
inline constexpr const char *kObjTreeItem      = "permTreeItem";
inline constexpr const char *kObjSearchField   = "permSearchEdit";
inline constexpr const char *kObjResourceForm       = "permResourceForm";
inline constexpr const char *kObjResourceFormField  = "permResourceFormField";
inline constexpr const char *kObjResourceFormText   = "permResourceFormText";
inline constexpr const char *kObjResourceFormValue  = "permResourceFormValue";
inline constexpr const char *kObjResourceFormSpin   = "permResourceFormSpin";
inline constexpr const char *kObjResourceFormCheck  = "permResourceFormCheck";
inline constexpr const char *kObjResourceFormCheckCell = "permResourceFormCheckCell";

inline constexpr int kToolbarCtrlHeight      = 32;
inline constexpr int kFilterToolbarRowPad    = 18;
inline constexpr int kFilterTitleRowHeight   = 26;
inline constexpr int kFilterToolbarTitleGap  = 8;
inline constexpr int kResourceFormLabelWidth = 88;
inline constexpr int kTableRowHeight      = 48;
inline constexpr int kTableActionBtnHeight = 28;

namespace detail {

/// 页面级装饰 QSS（仅标题/次要文字，对齐 HomeView applyHomeDecorStyles）
inline QString permissionPageDecorStyleSheet()
{
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    return QStringLiteral(
        "#permPluginRoot{background:transparent;}"
        "#permPluginRoot QLabel#permPageTitle{"
        "color:%1;font-size:16px;font-weight:700;letter-spacing:0.3px;background:transparent;}"
        "#permPluginRoot QLabel#permUsersPageTitle{"
        "color:%1;font-size:18px;font-weight:700;letter-spacing:0.5px;background:transparent;}"
        "#permPluginRoot QLabel#permMuted,#permPluginRoot QLabel#permDesc{"
        "color:%2;font-size:13px;background:transparent;}"
        "#permPluginRoot QLabel#permTreeItem{"
        "color:%1;padding:4px 0;background:transparent;}"
        "#permPluginRoot QLabel#permAuditDateSep{"
        "color:%2;font-size:14px;padding:0 2px;background:transparent;}"
        "#permPluginRoot QLabel#permResourceFormValue{"
        "color:%1;font-size:12px;padding-left:10px;min-height:32px;background:transparent;}"
        "#permPluginRoot QGroupBox{"
        "border:1px solid %3;border-radius:8px;margin-top:14px;"
        "padding:12px 12px 8px;color:%1;font-weight:600;font-size:13px;}")
        .arg(t.textPrimary.name(), t.textSecondary.name(), t.surfaceBorder.name());
}

inline void refreshDateTimeEdits(QWidget *root)
{
    if (!root)
        return;
    for (auto *dt : root->findChildren<ui::ThemedDateTimeEdit *>())
        dt->refreshTheme();
}

inline void refreshItemViews(QWidget *root)
{
    if (!root)
        return;
    for (auto *view : root->findChildren<QAbstractItemView *>()) {
        if (view->viewport())
            view->viewport()->update();
    }
}

inline void refreshResourceFormTextEdits(QWidget *root)
{
    if (!root)
        return;
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    const QString         qss = QStringLiteral(
        "QTextEdit#permResourceFormText{"
        "background-color:%1;color:%2;border:1px solid %3;border-radius:4px;"
        "padding:8px 10px;font-size:12px;"
        "selection-background-color:%4;selection-color:%2;}")
                                    .arg(t.inputBackground.name(),
                                         t.textPrimary.name(),
                                         t.inputBorder.name(),
                                         t.accent.name());
    const auto apply = [&](QTextEdit *edit) {
        if (edit && edit->objectName() == QLatin1String(kObjResourceFormText))
            edit->setStyleSheet(qss);
    };
    if (auto *self = qobject_cast<QTextEdit *>(root))
        apply(self);
    for (auto *edit : root->findChildren<QTextEdit *>())
        apply(edit);
}

inline void refreshPermCheckBoxes(QWidget *root)
{
    if (!root)
        return;
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    const QString         qss = QStringLiteral(
        "QCheckBox#permCheckBox{color:%1;spacing:8px;font-size:13px;}"
        "QCheckBox#permCheckBox::indicator{width:18px;height:18px;border:none;background:transparent;}")
                                    .arg(t.textPrimary.name());
    const auto apply = [&](QCheckBox *box) {
        if (box && box->objectName() == QLatin1String(kObjPermCheckBox))
            box->setStyleSheet(qss);
    };
    if (auto *self = qobject_cast<QCheckBox *>(root))
        apply(self);
    for (auto *box : root->findChildren<QCheckBox *>())
        apply(box);
}

} // namespace detail

inline void applyAuditTableViewStyle(QTableView *table)
{
    if (!table)
        return;
    QString qss = ui::buildTableWidgetStylesheet();
    qss.replace(QStringLiteral("QTableWidget"), QStringLiteral("QTableView"));
    table->setStyleSheet(qss);
}

inline void applyUserListWrapStyle(QWidget *wrap)
{
    if (!wrap)
        return;
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    wrap->setStyleSheet(QStringLiteral(
                            "#permUserListWrap{background:%1;border:1px solid %2;border-radius:8px;}")
                            .arg(t.surface.name(), t.surfaceBorder.name()));
}

inline QString permissionDialogPanelStyleSheet()
{
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    return QStringLiteral("#permPluginRoot{background:%1;border:1px solid %2;border-radius:8px;}")
        .arg(t.surface.name(), t.surfaceBorder.name());
}

inline QString permissionDialogDividerStyleSheet()
{
    const ui::ThemeTokens t = ui::ThemeTokens::resolve();
    return QStringLiteral("color:%1;").arg(t.surfaceBorder.name());
}

inline void initComboBox(PermComboBox *combo)
{
    if (!combo)
        return;
    combo->setFixedHeight(kToolbarCtrlHeight);
}

inline void initFormLineEdit(ui::ThemedLineEdit *edit)
{
    if (!edit)
        return;
    edit->setMinimumHeight(28);
}

/// 薄壳页面/弹窗主题：根装饰 QSS + 一次性样式控件刷新；Themed* 控件自行 bindThemeRefresh
inline void applyPermissionTheme(QWidget *page)
{
    if (!page)
        return;
    page->setObjectName(QLatin1String(kObjPageRoot));
    page->setAttribute(Qt::WA_StyledBackground, true);
    page->setStyleSheet(detail::permissionPageDecorStyleSheet());
    detail::refreshDateTimeEdits(page);
    detail::refreshResourceFormTextEdits(page);
    detail::refreshPermCheckBoxes(page);
    detail::refreshItemViews(page);
    ui::ThemedFormLabel::applyFontsIn(page);
    ui::ensureThemedSpinStyles(page);
    for (auto *table : page->findChildren<QTableView *>()) {
        if (table->objectName() == QLatin1String(kObjAuditTable))
            applyAuditTableViewStyle(table);
    }
}

/// 模态弹窗：初次刷主题并在主题切换时重刷（含 QTextEdit / PermCheckBox 等）
inline void bindPermissionDialogTheme(QWidget *dlg)
{
    if (!dlg)
        return;
    applyPermissionTheme(dlg);
    ui::bindThemeRefresh(dlg, [dlg]() { applyPermissionTheme(dlg); });
}

inline void initFilterToolbar(QFrame *toolbar)
{
    toolbar->setObjectName(QLatin1String(kObjFilterToolbar));
    toolbar->setFrameShape(QFrame::NoFrame);
    toolbar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    toolbar->setMinimumHeight(kToolbarCtrlHeight + kFilterToolbarRowPad);
}

/// 标题行与筛选行上下叠放时使用（如权限资源左栏）；预留标题高度，避免 retranslate 前布局锁死导致裁切
inline void initStackedFilterHeader(QFrame *header)
{
    if (!header)
        return;
    header->setObjectName(QLatin1String(kObjFilterToolbar));
    header->setFrameShape(QFrame::NoFrame);
    header->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    header->setMinimumHeight(kFilterTitleRowHeight + kFilterToolbarTitleGap + kToolbarCtrlHeight
                               + kFilterToolbarRowPad);
}

inline void initSearchField(ui::ThemedLineEdit *edit)
{
    if (!edit)
        return;
    edit->setObjectName(QLatin1String(kObjSearchField));
    edit->setClearButtonEnabled(true);
    edit->setFixedHeight(kToolbarCtrlHeight);
}

inline void initToolbarLineEdit(ui::ThemedLineEdit *edit)
{
    initSearchField(edit);
}

inline void initPermDateTimeEdit(PermDateTimeEdit *edit)
{
    if (!edit)
        return;
    edit->setToolbarHeight(kToolbarCtrlHeight);
}

inline void initPageTitle(QLabel *label, bool usersPageLargeTitle = false)
{
    label->setObjectName(usersPageLargeTitle ? QLatin1String(kObjUsersTitle)
                                             : QLatin1String(kObjPageTitle));
}

inline void initUserList(QListView *list)
{
    if (!list)
        return;
    list->setObjectName(QLatin1String(kObjUserList));
    list->setSpacing(0);
    list->setUniformItemSizes(true);
}

inline void initAuditLogTableView(QTableView *table)
{
    if (!table)
        return;
    table->setObjectName(QLatin1String(kObjAuditTable));
    table->setShowGrid(true);
    table->setGridStyle(Qt::SolidLine);
    table->setAlternatingRowColors(false);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    table->horizontalHeader()->setHighlightSections(false);
    table->verticalHeader()->setDefaultSectionSize(kTableRowHeight);
    table->setFrameShape(QFrame::NoFrame);
}

inline void initFooter(QWidget *footer)
{
    footer->setObjectName(QLatin1String(kObjFooter));
}

inline void initMutedLabel(QLabel *label)
{
    label->setObjectName(QLatin1String(kObjMutedLabel));
}

inline void initDescLabel(QLabel *label)
{
    label->setObjectName(QLatin1String(kObjDescLabel));
}

inline void initRoleList(QListWidget *list)
{
    list->setObjectName(QLatin1String(kObjRoleList));
    list->setSpacing(6);
    list->setSelectionMode(QAbstractItemView::SingleSelection);
    list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
}

inline void paintPermissionExpandToggle(QPainter *painter, const QRect &rect, bool expanded,
                                        const ui::ThemeTokens &t = ui::ThemeTokens::resolve())
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(t.textSecondary, 1.6));
    painter->setBrush(Qt::NoBrush);

    const QPointF c = rect.center();
    QPainterPath  path;
    if (expanded) {
        path.moveTo(c.x() - 5, c.y() - 2);
        path.lineTo(c.x(), c.y() + 3);
        path.lineTo(c.x() + 5, c.y() - 2);
    } else {
        path.moveTo(c.x() - 2, c.y() - 5);
        path.lineTo(c.x() + 3, c.y());
        path.lineTo(c.x() - 2, c.y() + 5);
    }
    painter->drawPath(path);
    painter->restore();
}

inline void paintPermissionActionTag(QPainter *painter, const QRect &rect, const QString &label,
                                    bool selected, bool enabled,
                                    const ui::ThemeTokens &t = ui::ThemeTokens::resolve())
{
    painter->save();
    painter->setOpacity(enabled ? 1.0 : 0.38);
    painter->setRenderHint(QPainter::Antialiasing, true);

    const int radius = rect.height() / 2;
    if (selected && enabled) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(t.accent);
        painter->drawRoundedRect(rect, radius, radius);
        painter->setPen(t.highlightText);
    } else {
        painter->setBrush(t.inputBackground);
        painter->setPen(QPen(t.inputBorder, 1));
        painter->drawRoundedRect(rect, radius, radius);
        painter->setPen(enabled ? t.textPrimary : t.textSecondary);
    }

    QFont f = painter->font();
    f.setPixelSize(12);
    painter->setFont(f);
    painter->drawText(rect, Qt::AlignCenter, label);
    painter->restore();
}

inline void paintPermissionCheckbox(QPainter *painter, const QRect &rect, bool checked,
                                    const ui::ThemeTokens &t = ui::ThemeTokens::resolve())
{
    constexpr int kSize = 18;
    const QRect   box(rect.center().x() - kSize / 2, rect.center().y() - kSize / 2, kSize, kSize);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    if (checked) {
        painter->setBrush(t.accent);
        painter->setPen(QPen(t.accent, 1));
        painter->drawRoundedRect(box, 3, 3);
        QPen checkPen(t.highlightText, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter->setPen(checkPen);
        QPainterPath mark;
        mark.moveTo(box.left() + 4, box.center().y() + 1);
        mark.lineTo(box.left() + 8, box.bottom() - 5);
        mark.lineTo(box.right() - 4, box.top() + 5);
        painter->drawPath(mark);
    } else {
        painter->setBrush(t.surfaceElevated);
        painter->setPen(QPen(t.inputBorder, 1));
        painter->drawRoundedRect(box, 3, 3);
    }
    painter->restore();
}

inline void initPermCheckBox(QCheckBox *box)
{
    box->setObjectName(QLatin1String(kObjPermCheckBox));
    box->setCursor(Qt::PointingHandCursor);
    detail::refreshPermCheckBoxes(box);
}

inline void initResourceFormFrame(QFrame *frame)
{
    if (!frame)
        return;
    frame->setObjectName(QLatin1String(kObjResourceForm));
    frame->setFrameShape(QFrame::NoFrame);
    frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

inline void initResourceFormLineEdit(ui::ThemedLineEdit *edit)
{
    if (!edit)
        return;
    edit->setObjectName(QLatin1String(kObjResourceFormField));
    edit->setMinimumHeight(32);
    edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

inline void initResourceFormTextEdit(QTextEdit *edit)
{
    if (!edit)
        return;
    edit->setObjectName(QLatin1String(kObjResourceFormText));
    edit->setMinimumHeight(72);
    edit->setMaximumHeight(72);
    edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    detail::refreshResourceFormTextEdits(edit);
}

inline void initResourceFormFieldLabel(ui::ThemedFormLabel *label)
{
    if (!label)
        return;
    label->setPixelSize(13);
    label->setFixedWidth(kResourceFormLabelWidth);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

inline void initResourceFormValueLabel(QLabel *label)
{
    if (!label)
        return;
    label->setObjectName(QLatin1String(kObjResourceFormValue));
    label->setMinimumHeight(32);
    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

inline void initResourceFormSpinBox(ui::ThemedSpinBox *spin)
{
    if (!spin)
        return;
    spin->setObjectName(QLatin1String(kObjResourceFormSpin));
    spin->setFixedWidth(140);
    spin->setFixedHeight(32);
    spin->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

inline void initResourceFormCheckBox(QCheckBox *box)
{
    if (!box)
        return;
    box->setObjectName(QLatin1String(kObjResourceFormCheck));
    box->setCursor(Qt::PointingHandCursor);
    box->setText(QString());
    box->setFixedSize(24, 24);
}

inline void initTreeItemLabel(QLabel *label)
{
    label->setObjectName(QLatin1String(kObjTreeItem));
}

inline void initTableActionCell(QWidget *widget, QHBoxLayout *lay)
{
    if (!widget || !lay)
        return;
    widget->setObjectName(QLatin1String(kObjTableActionCell));
    widget->setAutoFillBackground(false);
    widget->setAttribute(Qt::WA_StyledBackground, true);
    widget->setStyleSheet(QStringLiteral(
        "QWidget#permTableActionCell{margin:0;padding:0;border:none;background:transparent;}"));
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(6);
    lay->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
}

inline QLabel *makeRoleBadge(const QString &text, const QString &colorHex, QWidget *parent)
{
    auto *lbl = new QLabel(text, parent);
    lbl->setAlignment(Qt::AlignCenter);
    const QString bg = colorHex.isEmpty() ? QStringLiteral("#6b7280") : colorHex;
    lbl->setStyleSheet(QStringLiteral(
                           "QLabel{background:%1;color:#fff;padding:3px 12px;border-radius:12px;"
                           "font-size:12px;font-weight:500;}")
                           .arg(bg));
    return lbl;
}

inline QWidget *makeRoleBadgesRow(const QStringList &names,
                                  const QList<QString> &colors,
                                  QWidget *parent)
{
    auto *wrap = new QWidget(parent);
    wrap->setObjectName(QLatin1String(kObjTableRoleCell));
    wrap->setAutoFillBackground(false);
    wrap->setAttribute(Qt::WA_StyledBackground, true);
    wrap->setStyleSheet(QStringLiteral(
        "QWidget#permTableRoleCell{margin:0;padding:0;border:none;background:transparent;}"));

    auto *lay = new QHBoxLayout(wrap);
    lay->setContentsMargins(8, 0, 8, 0);
    lay->setSpacing(6);
    lay->setAlignment(Qt::AlignCenter);
    lay->addStretch();
    for (int i = 0; i < names.size(); ++i) {
        const QString color = i < colors.size() ? colors.at(i) : QStringLiteral("#6b7280");
        lay->addWidget(makeRoleBadge(names.at(i), color, wrap));
    }
    lay->addStretch();
    return wrap;
}

inline ui::ThemedPushButton *makePrimaryButton(const QString &text, QWidget *parent)
{
    auto *btn = new ui::ThemedPushButton(text, ui::PushButtonVariant::Primary, parent);
    btn->setFixedHeight(kToolbarCtrlHeight);
    return btn;
}

inline ui::ThemedPushButton *makeSecondaryButton(const QString &text, QWidget *parent)
{
    auto *btn = new ui::ThemedPushButton(text, ui::PushButtonVariant::Secondary, parent);
    btn->setFixedHeight(kToolbarCtrlHeight);
    return btn;
}

inline ui::ThemedPushButton *makeDangerButton(const QString &text, QWidget *parent)
{
    auto *btn = new ui::ThemedPushButton(text, ui::PushButtonVariant::Danger, parent);
    btn->setFixedHeight(28);
    return btn;
}

} // namespace permission_ui
