#pragma once

#include "model/permission_resource_management_model.h"
#include "pagewidget.h"

#include <QModelIndex>

class PermCheckBox;
class QLabel;
class PermCheckBox;

namespace ui {
class ThemedLineEdit;
class ThemedPushButton;
class ThemedSpinBox;
}
class QStandardItemModel;
class QTreeView;

class PermissionResourceTreeDelegate;
class PermissionResourceViewModel;

namespace permission_ui {
class PermComboBox;
}

namespace permission {

class PermissionResourceManagementView : public PageWidget
{
    Q_OBJECT

public:
    explicit PermissionResourceManagementView(QWidget *parent = nullptr);

protected:
    void onLoad() override;
    void applyTheme() override;
    void retranslateUi() override;
    void onShow() override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void buildLayout();
    void setupBindings();
    void syncTree();
    void syncEditorForm();
    void updateEditorActionButtons();
    bool isEditorDirty() const;
    void applyEditorFormToViewModel();
    PermissionResourceEditorForm editorFormFromView() const;
    bool confirmDiscardEditorEdits(const QString &message);
    void selectTreeItemById(qint64 id);
    qint64 treeItemId(const QModelIndex &index) const;
    QModelIndex findTreeIndexById(qint64 id) const;

    void captureExpandedStateToViewModel();
    void restoreExpandedStateFromViewModel();
    void expandAncestorsInView(const QModelIndex &index);
    void expandAllVisibleNodes();
    void applyTreeVisualSelection(const QModelIndex &index);
    void reapplyTreeSelectionVisual();
    /// 树刷新后：以树当前选中为准同步 ViewModel 与右侧编辑区（避免 block 导致 currentChanged 未触发）
    void syncTreeSelectionToViewModel();

    PermissionResourceViewModel *m_viewModel = nullptr;

    QLabel                     *m_pageTitle      = nullptr;
    ui::ThemedLineEdit         *m_searchEdit     = nullptr;
    permission_ui::PermComboBox *m_typeFilter    = nullptr;
    permission_ui::PermComboBox *m_enabledFilter = nullptr;
    QTreeView                       *m_tree           = nullptr;
    QStandardItemModel              *m_treeModel      = nullptr;
    PermissionResourceTreeDelegate *m_treeDelegate  = nullptr;

    QLabel      *m_detailTitle    = nullptr;
    QLabel      *m_lblType        = nullptr;
    QLabel      *m_lblSource      = nullptr;
    QLabel      *m_lblParent      = nullptr;
    QLabel      *m_lblGrantRefs   = nullptr;
    ui::ThemedLineEdit *m_editKey        = nullptr;
    ui::ThemedLineEdit *m_editPageKey    = nullptr;
    ui::ThemedLineEdit *m_editDisplay    = nullptr;
    ui::ThemedLineEdit *m_editDesc       = nullptr;
    ui::ThemedSpinBox  *m_editSort       = nullptr;
    PermCheckBox *m_editEnabled   = nullptr;
    ui::ThemedPushButton *m_btnNewNav         = nullptr;
    ui::ThemedPushButton *m_btnNewSub         = nullptr;
    ui::ThemedPushButton *m_btnNewAction      = nullptr;
    ui::ThemedPushButton *m_btnSave           = nullptr;
    ui::ThemedPushButton *m_btnReset          = nullptr;
    ui::ThemedPushButton *m_btnDelete         = nullptr;
    ui::ThemedPushButton *m_btnToggleEnabled  = nullptr;
    QWidget     *m_actionBar        = nullptr;

    bool m_blockTreeSelection      = false;
    bool m_blockEditorSignals      = false;
    bool m_initialTreeExpandDone   = false;

    qint64 m_lastKeyConflictId = 0;
    bool   m_lastKeyConflictEnabled = true;
};

} // namespace permission
