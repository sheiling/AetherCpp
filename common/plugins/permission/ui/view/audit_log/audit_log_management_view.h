#pragma once

#include "pagewidget.h"
#include "perm_datetime_edit.h"

class QLabel;
class QTableView;

class AuditLogManagementViewModel;

namespace permission_ui {
class PermComboBox;
}

namespace ui {
class ThemedLineEdit;
class ThemedPushButton;
}

namespace permission {

class AuditLogTableModel;

class AuditLogManagementView : public PageWidget
{
    Q_OBJECT

public:
    explicit AuditLogManagementView(QWidget *parent = nullptr);

protected:
    void onLoad() override;
    void applyTheme() override;
    void retranslateUi() override;
    void onShow() override;

private:
    void buildLayout();
    void setupBindings();
    void syncDraftFiltersToView();
    void syncDraftTimePickersToView();
    void syncDateTimePickerTexts();
    void applyRangePreset(int presetIndex);
    void onPickerTimeFromChanged(const QDateTime &value);
    void onPickerTimeToChanged(const QDateTime &value);
    void syncTable();
    void syncPagerLabel();
    void syncActionTypeCombo();
    void syncRangePresetCombo();
    void showLogDetail(qint64 logId);

    AuditLogManagementViewModel *m_viewModel = nullptr;

    QLabel                       *m_pageTitle      = nullptr;
    ui::ThemedLineEdit           *m_operatorEdit   = nullptr;
    permission_ui::PermComboBox  *m_actionFilter   = nullptr;
    permission_ui::PermComboBox  *m_rangePreset    = nullptr;
    QLabel                       *m_lblDateSep     = nullptr;
    permission_ui::PermDateTimeEdit *m_timeFrom    = nullptr;
    permission_ui::PermDateTimeEdit *m_timeTo      = nullptr;
    ui::ThemedPushButton         *m_btnSearch      = nullptr;
    ui::ThemedPushButton         *m_btnReset       = nullptr;
    ui::ThemedPushButton         *m_btnExport      = nullptr;
    QTableView                   *m_tableView      = nullptr;
    AuditLogTableModel           *m_tableModel     = nullptr;
    QLabel                       *m_totalLabel     = nullptr;
    ui::ThemedPushButton         *m_btnPrevPage    = nullptr;
    ui::ThemedPushButton         *m_btnNextPage    = nullptr;
    QLabel                       *m_pageLabel      = nullptr;

    bool m_syncingPickers = false;
};

} // namespace permission
