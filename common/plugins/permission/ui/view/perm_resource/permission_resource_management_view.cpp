#include "ui/view/perm_resource/permission_resource_management_view.h"

#include "data/domain/domain_types.h"
#include "model/permission_resource_management_model.h"
#include "ui/view/message_popup.h"
#include "ui/view/perm_checkbox.h"
#include "ui/view/perm_combobox.h"
#include "ui/view/perm_i18n_utils.h"
#include "ui/view/perm_resource/permission_resource_tree_delegate.h"
#include "ui/view/perm_text.h"
#include "ui/view/permission_ui_utils.h"
#include "ui/vm/permission_resource_view_model.h"

#include "command.h"
#include "service/perm_action_keys.h"
#include "service/perm_system_constants.h"
#include "uibase/permission/permservice.h"

#include "controls/button/themed_push_button.h"
#include "controls/input/themed_line_edit.h"
#include "controls/label/themed_form_label.h"
#include "controls/spinbox/themed_spinbox.h"

#include <QBrush>
#include <QDialogButtonBox>
#include <functional>
#include <QEvent>
#include <QHash>
#include <QSet>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>
#include <QVBoxLayout>

namespace permission {
namespace {

constexpr int kRoleResourceId = Qt::UserRole + 1;

/// 表单一行：左 ThemedFormLabel + 右字段（字段横向拉满）
QWidget *makeFormRow(QWidget *parent, const char *titleKey, QWidget *field)
{
    auto *row = new QWidget(parent);
    row->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *lay = new QHBoxLayout(row);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(16);

    auto *titleLbl = new ui::ThemedFormLabel(QString::fromUtf8(titleKey), row);
    permission_ui::initResourceFormFieldLabel(titleLbl);
    lay->addWidget(titleLbl);
    lay->addWidget(field, 1);
    return row;
}

/// 字段区左对齐包裹（用于排序 SpinBox、启用复选框等不需拉满的控件）
QWidget *makeFormFieldWrap(QWidget *parent, QWidget *field, int leftMargin = 0)
{
    auto *wrap = new QWidget(parent);
    wrap->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    auto *lay = new QHBoxLayout(wrap);
    lay->setContentsMargins(leftMargin, 0, 0, 0);
    lay->setSpacing(0);
    lay->addWidget(field, 0, Qt::AlignLeft | Qt::AlignVCenter);
    lay->addStretch();
    return wrap;
}

QString resourceTypeLabel(const QString &type)
{
    if (type == PermResourceType::Nav)
        return PTXT("perm.resources.type_nav");
    if (type == PermResourceType::SubPage)
        return PTXT("perm.resources.type_sub_page");
    if (type == PermResourceType::Action)
        return PTXT("perm.resources.type_action");
    return type;
}

void fillTreeItem(QStandardItem *item, const PermissionResourceTreeBranch &branch)
{
    const auto &n = branch.node;
    item->setText(permission_ui::permissionResourceDisplayText(n.key, n.displayName));
    item->setData(n.id, kRoleResourceId);
    item->setData(n.id, PermissionResourceTreeDelegate::ResourceIdRole);
    item->setData(n.resourceType, PermissionResourceTreeDelegate::ResourceTypeRole);
    item->setData(n.key, PermissionResourceTreeDelegate::ResourceKeyRole);
    item->setData(!branch.children.isEmpty(), PermissionResourceTreeDelegate::HasChildrenRole);
    item->setData(n.enabled, PermissionResourceTreeDelegate::ResourceEnabledRole);
    item->setEditable(false);
}

QStandardItem *makeTreeItem(const PermissionResourceTreeBranch &branch)
{
    auto *item = new QStandardItem(
        permission_ui::permissionResourceDisplayText(branch.node.key, branch.node.displayName));
    fillTreeItem(item, branch);
    return item;
}

void collectItemById(QStandardItem *parent, QHash<qint64, QStandardItem *> &itemById)
{
    for (int row = 0; row < parent->rowCount(); ++row) {
        QStandardItem *item = parent->child(row);
        if (!item)
            continue;
        const qint64 id = item->data(kRoleResourceId).toLongLong();
        if (id > 0)
            itemById.insert(id, item);
        collectItemById(item, itemById);
    }
}

void syncTreeChildren(QStandardItem *parent,
                      const QVector<permission::PermissionResourceTreeBranch> &branches,
                      QHash<qint64, QStandardItem *> &itemById)
{
    QSet<qint64> desired;
    desired.reserve(branches.size());
    for (const auto &branch : branches)
        desired.insert(branch.node.id);

    for (int row = parent->rowCount() - 1; row >= 0; --row) {
        QStandardItem *child = parent->child(row);
        if (!child)
            continue;
        const qint64 id = child->data(kRoleResourceId).toLongLong();
        if (!desired.contains(id)) {
            itemById.remove(id);
            parent->removeRow(row);
        }
    }

    int targetRow = 0;
    for (const auto &branch : branches) {
        const qint64 id = branch.node.id;
        QStandardItem *item = nullptr;
        int           foundRow = -1;
        for (int row = 0; row < parent->rowCount(); ++row) {
            QStandardItem *candidate = parent->child(row);
            if (candidate && candidate->data(kRoleResourceId).toLongLong() == id) {
                item      = candidate;
                foundRow  = row;
                break;
            }
        }

        if (!item) {
            item = makeTreeItem(branch);
            parent->insertRow(targetRow, item);
        } else {
            if (foundRow != targetRow) {
                const QList<QStandardItem *> moved = parent->takeRow(foundRow);
                parent->insertRow(targetRow, moved);
                item = parent->child(targetRow);
            }
            fillTreeItem(item, branch);
        }

        itemById.insert(id, item);
        syncTreeChildren(item, branch.children, itemById);
        ++targetRow;
    }
}

} // namespace

PermissionResourceManagementView::PermissionResourceManagementView(QWidget *parent)
    : PageWidget(parent)
{
}

void PermissionResourceManagementView::applyTheme()
{
    permission_ui::applyPermissionTheme(this);
    if (m_tree)
        PermissionResourceTreeDelegate::applyTreeStyle(m_tree);
}

void PermissionResourceManagementView::onLoad()
{
    permission_ui::applyPermissionTheme(this);

    m_viewModel  = new PermissionResourceViewModel(nullptr, this);
    m_treeModel  = new QStandardItemModel(this);

    buildLayout();
    setupBindings();

    retranslateUi();
    m_viewModel->refreshTree();
}

void PermissionResourceManagementView::onShow()
{
    if (!m_viewModel)
        return;
    m_viewModel->refreshCommandStates();
    m_viewModel->refreshTree();
    updateEditorActionButtons();
}

void PermissionResourceManagementView::buildLayout()
{
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(16);

    auto *leftPanel = new QWidget(this);
    leftPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(10);

    auto *header = new QFrame(leftPanel);
    permission_ui::initStackedFilterHeader(header);
    auto *headerLay = new QVBoxLayout(header);
    headerLay->setContentsMargins(0, 0, 0, 0);
    headerLay->setSpacing(permission_ui::kFilterToolbarTitleGap);

    auto *titleRow = new QHBoxLayout;
    m_pageTitle = new QLabel(header);
    permission_ui::initPageTitle(m_pageTitle);
    titleRow->addWidget(m_pageTitle);
    titleRow->addStretch();

    auto *filterRow = new QHBoxLayout;
    filterRow->setContentsMargins(0, 0, 0, 4);
    filterRow->setSpacing(8);
    filterRow->setAlignment(Qt::AlignVCenter);
    m_searchEdit  = new ui::ThemedLineEdit(header);
    m_searchEdit->setMinimumWidth(160);
    permission_ui::initSearchField(m_searchEdit);
    m_typeFilter = new permission_ui::PermComboBox(header);
    m_typeFilter->setMinimumWidth(120);
    permission_ui::initComboBox(m_typeFilter);
    m_enabledFilter = new permission_ui::PermComboBox(header);
    m_enabledFilter->setMinimumWidth(110);
    permission_ui::initComboBox(m_enabledFilter);
    filterRow->addWidget(m_searchEdit, 1);
    filterRow->addWidget(m_typeFilter);
    filterRow->addWidget(m_enabledFilter);

    headerLay->addLayout(titleRow);
    headerLay->addLayout(filterRow);

    m_tree = new QTreeView(leftPanel);
    m_tree->setModel(m_treeModel);
    m_tree->setHeaderHidden(true);
    m_tree->setFrameShape(QFrame::NoFrame);
    m_tree->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_tree->setAnimated(true);
    m_tree->setUniformRowHeights(true);
    m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tree->setSelectionBehavior(QAbstractItemView::SelectRows);
    PermissionResourceTreeDelegate::applyTreeStyle(m_tree);
    m_treeDelegate = new PermissionResourceTreeDelegate(m_tree);
    m_tree->setItemDelegate(m_treeDelegate);
    m_tree->viewport()->installEventFilter(this);

    leftLayout->addWidget(header);
    leftLayout->addWidget(m_tree, 1);

    auto *rightPanel = new QWidget(this);
    rightPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(12, 0, 8, 0);
    rightLayout->setSpacing(10);

    auto *actionRow = new QHBoxLayout;
    m_btnNewNav        = permission_ui::makeSecondaryButton(QString(), rightPanel);
    m_btnNewSub        = permission_ui::makeSecondaryButton(QString(), rightPanel);
    m_btnNewAction     = permission_ui::makeSecondaryButton(QString(), rightPanel);
    m_btnSave          = permission_ui::makePrimaryButton(QString(), rightPanel);
    m_btnReset         = permission_ui::makeSecondaryButton(QString(), rightPanel);
    m_btnDelete        = permission_ui::makeDangerButton(QString(), rightPanel);
    m_btnToggleEnabled = permission_ui::makeSecondaryButton(QString(), rightPanel);
    actionRow->addWidget(m_btnNewNav);
    actionRow->addWidget(m_btnNewSub);
    actionRow->addWidget(m_btnNewAction);
    actionRow->addStretch();
    actionRow->addWidget(m_btnToggleEnabled);
    actionRow->addWidget(m_btnReset);
    actionRow->addWidget(m_btnSave);
    actionRow->addWidget(m_btnDelete);

    m_actionBar = new QWidget(rightPanel);
    m_actionBar->setLayout(actionRow);
    if (kPermissionResourcesCatalogReadOnly)
        m_actionBar->setVisible(false);

    m_detailTitle = new QLabel(rightPanel);
    permission_ui::initPageTitle(m_detailTitle);

    auto *formFrame = new QFrame(rightPanel);
    permission_ui::initResourceFormFrame(formFrame);
    auto *formVBox = new QVBoxLayout(formFrame);
    formVBox->setContentsMargins(0, 4, 0, 0);
    formVBox->setSpacing(10);

    m_lblType      = new QLabel(formFrame);
    m_lblSource    = new QLabel(formFrame);
    m_lblParent    = new QLabel(formFrame);
    m_lblGrantRefs = new QLabel(formFrame);
    permission_ui::initResourceFormValueLabel(m_lblType);
    permission_ui::initResourceFormValueLabel(m_lblSource);
    permission_ui::initResourceFormValueLabel(m_lblParent);
    permission_ui::initResourceFormValueLabel(m_lblGrantRefs);

    m_editKey     = new ui::ThemedLineEdit(formFrame);
    m_editPageKey = new ui::ThemedLineEdit(formFrame);
    m_editDisplay = new ui::ThemedLineEdit(formFrame);
    permission_ui::initResourceFormLineEdit(m_editKey);
    permission_ui::initResourceFormLineEdit(m_editPageKey);
    permission_ui::initResourceFormLineEdit(m_editDisplay);

    m_editDesc = new ui::ThemedLineEdit(formFrame);
    permission_ui::initResourceFormLineEdit(m_editDesc);
    m_editSort = new ui::ThemedSpinBox(formFrame);
    m_editSort->setRange(0, 99999);
    permission_ui::initResourceFormSpinBox(m_editSort);

    m_editEnabled = new PermCheckBox(formFrame);
    permission_ui::initResourceFormCheckBox(m_editEnabled);

    formVBox->addWidget(makeFormRow(formFrame, "perm.resources.label_type", m_lblType));
    formVBox->addWidget(makeFormRow(formFrame, "perm.form.key", m_editKey));
    formVBox->addWidget(makeFormRow(formFrame, "perm.form.page_key", m_editPageKey));
    formVBox->addWidget(makeFormRow(formFrame, "perm.resources.label_source", m_lblSource));
    formVBox->addWidget(makeFormRow(formFrame, "perm.resources.label_parent", m_lblParent));
    formVBox->addWidget(makeFormRow(formFrame, "perm.resources.label_grant_refs", m_lblGrantRefs));
    formVBox->addWidget(makeFormRow(formFrame, "perm.users.label_display_name", m_editDisplay));
    formVBox->addWidget(makeFormRow(formFrame, "perm.resources.label_description", m_editDesc));
    formVBox->addWidget(
        makeFormRow(formFrame, "perm.resources.label_sort_order", makeFormFieldWrap(formFrame, m_editSort)));
    formVBox->addWidget(makeFormRow(
        formFrame, "perm.common.enabled", makeFormFieldWrap(formFrame, m_editEnabled, 10)));
    formVBox->addStretch();

    rightLayout->addWidget(m_actionBar);
    rightLayout->addWidget(m_detailTitle);
    rightLayout->addWidget(formFrame, 1);

    root->addWidget(leftPanel, 1);
    root->addWidget(rightPanel, 1);
}

bool PermissionResourceManagementView::eventFilter(QObject *watched, QEvent *event)
{
    if (m_tree && watched == m_tree->viewport() && event && event->type() == QEvent::MouseButtonPress) {
        auto *me = static_cast<QMouseEvent *>(event);
        if (me->button() == Qt::LeftButton) {
            const QModelIndex idx = m_tree->indexAt(me->pos());
            if (idx.isValid() && idx.data(PermissionResourceTreeDelegate::HasChildrenRole).toBool()) {
                const QRect rowRect    = m_tree->visualRect(idx);
                const QRect toggleRect =
                    PermissionResourceTreeDelegate::expandToggleRect(rowRect, true);
                if (toggleRect.contains(me->pos())) {
                    const bool expand = !m_tree->isExpanded(idx);
                    m_tree->setExpanded(idx, expand);
                    if (m_viewModel) {
                        const qint64 rid = treeItemId(idx);
                        m_viewModel->noteTreeExpanded(rid, expand);
                    }
                    return true; // 吞掉事件：避免触发选中/双击展开逻辑
                }
            }
        }
    }
    return PageWidget::eventFilter(watched, event);
}

void PermissionResourceManagementView::setupBindings()
{
    m_typeFilter->addItem(PTXT("perm.resources.filter_all_types"), QString());
    m_typeFilter->addItem(PTXT("perm.resources.type_nav"), QString::fromUtf8(PermResourceType::Nav));
    m_typeFilter->addItem(PTXT("perm.resources.type_sub_page"), QString::fromUtf8(PermResourceType::SubPage));
    m_typeFilter->addItem(PTXT("perm.resources.type_action"), QString::fromUtf8(PermResourceType::Action));
    if (m_enabledFilter) {
        m_enabledFilter->addItem(PTXT("perm.common.all_statuses"), -1);
        m_enabledFilter->addItem(PTXT("perm.common.enabled_only"), 1);
        m_enabledFilter->addItem(PTXT("perm.common.disabled_only"), 0);
        m_enabledFilter->setCurrentIndex(0);
    }

    connect(m_searchEdit, &ui::ThemedLineEdit::textChanged, this, [this](const QString &text) {
        if (!m_viewModel)
            return;
        m_viewModel->setSearchKeyword(text);
        m_viewModel->applyFilters();
    });

    connect(m_typeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int index) {
                if (!m_viewModel)
                    return;
                m_viewModel->setResourceTypeFilter(m_typeFilter->itemData(index).toString());
                m_viewModel->applyFilters();
            });
    if (m_enabledFilter) {
        connect(m_enabledFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                [this](int index) {
                    if (!m_viewModel)
                        return;
                    m_viewModel->setEnabledFilterMode(m_enabledFilter->itemData(index).toInt());
                    m_viewModel->applyFilters();
                });
    }

    connect(PS, &PermService::permissionsChanged, this, [this]() {
        if (m_viewModel)
            m_viewModel->refreshCommandStates();
        updateEditorActionButtons();
    });
    updateEditorActionButtons();

    connect(m_viewModel, &PermissionResourceViewModel::resourceTreeChanged, this,
            &PermissionResourceManagementView::syncTree);

    connect(m_tree, &QTreeView::expanded, this, [this](const QModelIndex &index) {
        if (m_viewModel)
            m_viewModel->noteTreeExpanded(treeItemId(index), true);
    });
    connect(m_tree, &QTreeView::collapsed, this, [this](const QModelIndex &index) {
        if (m_viewModel)
            m_viewModel->noteTreeExpanded(treeItemId(index), false);
    });
    connect(m_viewModel, &PermissionResourceViewModel::editorFormChanged, this,
            &PermissionResourceManagementView::syncEditorForm);
    connect(m_viewModel, &PermissionResourceViewModel::editorDirtyChanged, this,
            &PermissionResourceManagementView::updateEditorActionButtons);
    connect(m_viewModel, &PermissionResourceViewModel::selectedResourceIdChanged, this,
            &PermissionResourceManagementView::updateEditorActionButtons);
    connect(m_viewModel, &PermissionResourceViewModel::selectedResourceIdChanged, this,
            &PermissionResourceManagementView::syncEditorForm);

    connect(m_viewModel, &PermissionResourceViewModel::showInfo, this,
            [this](const QString &title, const QString &message) {
                permission_ui::MessagePopup::show(this, title, message, QDialogButtonBox::Ok);
            });
    connect(m_viewModel, &PermissionResourceViewModel::showWarning, this,
            [this](const QString &title, const QString &message) {
                permission_ui::MessagePopup::show(this, title.isEmpty() ? PTXT("perm.common.notice") : title,
                                                 message, QDialogButtonBox::Ok);
            });
    connect(m_viewModel, &PermissionResourceViewModel::keyConflictFound, this,
            [this](qint64 id, bool enabled) {
                m_lastKeyConflictId      = id;
                m_lastKeyConflictEnabled = enabled;
            });

    connect(m_tree->selectionModel(), &QItemSelectionModel::currentChanged, this,
            [this](const QModelIndex &current, const QModelIndex &previous) {
                Q_UNUSED(previous);
                if (m_blockTreeSelection || !m_viewModel)
                    return;

                const qint64 newId = treeItemId(current);
                if (newId == m_viewModel->selectedResourceId())
                    return;

                if (isEditorDirty()
                    && !confirmDiscardEditorEdits(PTXT(
                           "perm.resources.discard_unsaved_switch"))) {
                    QTimer::singleShot(0, this, [this]() {
                        const qint64 keepId = m_viewModel->selectedResourceId();
                        if (keepId > 0)
                            selectTreeItemById(keepId);
                        else {
                            m_blockTreeSelection = true;
                            m_tree->clearSelection();
                            m_blockTreeSelection = false;
                        }
                    });
                    return;
                }

                m_viewModel->selectResource(newId);
            });

    auto hookEditor = [this]() {
        if (m_blockEditorSignals)
            return;
        applyEditorFormToViewModel();
        updateEditorActionButtons();
    };
    if (!kPermissionResourcesCatalogReadOnly) {
        connect(m_editKey, &ui::ThemedLineEdit::textChanged, this, hookEditor);
        connect(m_editPageKey, &ui::ThemedLineEdit::textChanged, this, hookEditor);
        connect(m_editDisplay, &ui::ThemedLineEdit::textChanged, this, hookEditor);
        connect(m_editDesc, &ui::ThemedLineEdit::textChanged, this, hookEditor);
        connect(m_editSort, QOverload<int>::of(&QSpinBox::valueChanged), this, hookEditor);
        connect(m_editEnabled, &QCheckBox::toggled, this, hookEditor);

        connect(m_btnNewNav, &ui::ThemedPushButton::clicked, m_viewModel,
                &PermissionResourceViewModel::beginCreateNav);
        connect(m_btnNewSub, &ui::ThemedPushButton::clicked, m_viewModel,
                &PermissionResourceViewModel::beginCreateSubPage);
        connect(m_btnNewAction, &ui::ThemedPushButton::clicked, m_viewModel,
                &PermissionResourceViewModel::beginCreateAction);
        // 重置/保存：可用状态由 updateEditorActionButtons（须 dirty）控制，点击走 Command（含权限校验）
        connect(m_btnSave, &ui::ThemedPushButton::clicked, this, [this]() {
            if (!m_viewModel || !m_viewModel->saveCommand())
                return;
            applyEditorFormToViewModel();
            m_lastKeyConflictId = 0;
            m_viewModel->saveCommand()->run();
            if (m_lastKeyConflictId <= 0)
                return;

            const auto clicked = permission_ui::MessagePopup::show(
                this, PTXT("perm.err.key_duplicate"),
                PTXT("perm.resources.duplicate_key_prompt"),
                QDialogButtonBox::Yes | QDialogButtonBox::No | QDialogButtonBox::Cancel,
                QDialogButtonBox::Yes);

            if (clicked == QDialogButtonBox::Yes) {
                if (m_enabledFilter)
                    m_enabledFilter->setCurrentIndex(0);
                m_viewModel->applyFilters();
                selectTreeItemById(m_lastKeyConflictId);
            } else if (clicked == QDialogButtonBox::No) {
                if (m_enabledFilter)
                    m_enabledFilter->setCurrentIndex(2);
                m_viewModel->applyFilters();
                selectTreeItemById(m_lastKeyConflictId);
            }
            m_lastKeyConflictId = 0;
        });
        connect(m_btnReset, &ui::ThemedPushButton::clicked, m_viewModel->resetCommand(), &Command::run);
        connect(m_btnDelete, &ui::ThemedPushButton::clicked, this, [this]() {
            if (!m_viewModel || m_viewModel->selectedResourceId() <= 0)
                return;
            if (permission_ui::MessagePopup::show(
                    this, PTXT("perm.common.confirm_deletion"),
                    PTXT("perm.resources.delete_confirm"),
                    QDialogButtonBox::Yes | QDialogButtonBox::No, QDialogButtonBox::No)
                != QDialogButtonBox::Yes)
                return;
            m_viewModel->deleteSelected();
        });
        connect(m_btnToggleEnabled, &ui::ThemedPushButton::clicked, m_viewModel,
                &PermissionResourceViewModel::toggleSelectedEnabled);
    }
}

void PermissionResourceManagementView::retranslateUi()
{
    if (m_pageTitle)
        m_pageTitle->setText(PTXT("perm.resources.page_title"));
    if (m_searchEdit)
        m_searchEdit->setPlaceholderText(PTXT("perm.resources.search_placeholder"));
    if (m_btnNewNav)
        m_btnNewNav->setText(PTXT("perm.resources.new_nav"));
    if (m_btnNewSub)
        m_btnNewSub->setText(PTXT("perm.resources.new_sub_page"));
    if (m_btnNewAction)
        m_btnNewAction->setText(PTXT("perm.resources.new_action"));
    if (m_btnSave)
        m_btnSave->setText(PTXT("perm.common.save"));
    if (m_btnReset)
        m_btnReset->setText(PTXT("perm.common.reset"));
    if (m_btnDelete)
        m_btnDelete->setText(PTXT("perm.common.delete"));
    if (m_btnToggleEnabled)
        m_btnToggleEnabled->setText(PTXT("perm.resources.toggle_enabled"));

    if (auto *formFrame = findChild<QFrame *>(permission_ui::kObjResourceForm))
        ui::ThemedFormLabel::retranslateIn(formFrame);

    syncTree();
    syncEditorForm();
}

void PermissionResourceManagementView::syncTree()
{
    if (!m_viewModel || !m_treeModel || !m_tree)
        return;

    captureExpandedStateToViewModel();

    const qint64 selectedId = m_viewModel->selectedResourceId();

    QHash<qint64, QStandardItem *> itemById;
    auto *root = m_treeModel->invisibleRootItem();
    collectItemById(root, itemById);

    if (root->rowCount() == 0 && !m_viewModel->visibleForest().isEmpty()) {
        for (const auto &branch : m_viewModel->visibleForest()) {
            QStandardItem *item = makeTreeItem(branch);
            root->appendRow(item);
            itemById.insert(branch.node.id, item);
            syncTreeChildren(item, branch.children, itemById);
        }
    } else {
        syncTreeChildren(root, m_viewModel->visibleForest(), itemById);
    }

    restoreExpandedStateFromViewModel();
    if (!m_initialTreeExpandDone) {
        expandAllVisibleNodes();
        m_initialTreeExpandDone = true;
    }

    QModelIndex targetIdx;
    if (selectedId > 0)
        targetIdx = findTreeIndexById(selectedId);

    if (targetIdx.isValid()) {
        applyTreeVisualSelection(targetIdx);
    } else {
        // 原节点已不在树中（如「仅禁用」下启用后消失）：沿用 Qt 已定位到的当前行
        const QModelIndex cur = m_tree->currentIndex();
        if (cur.isValid()) {
            applyTreeVisualSelection(cur);
        } else if (QItemSelectionModel *sel = m_tree->selectionModel()) {
            m_blockTreeSelection = true;
            sel->clearSelection();
            m_tree->setCurrentIndex(QModelIndex());
            m_blockTreeSelection = false;
        }
    }

    syncTreeSelectionToViewModel();
    reapplyTreeSelectionVisual();
}

void PermissionResourceManagementView::syncTreeSelectionToViewModel()
{
    if (!m_viewModel || !m_tree)
        return;

    const qint64 treeId = treeItemId(m_tree->currentIndex());
    if (treeId != m_viewModel->selectedResourceId())
        m_viewModel->selectResource(treeId);
    else if (treeId > 0)
        m_viewModel->selectResource(treeId); // 同一节点也刷新表单（启用状态等可能已变）
}

void PermissionResourceManagementView::expandAllVisibleNodes()
{
    if (!m_tree || !m_treeModel)
        return;

    std::function<void(const QModelIndex &)> walk;
    walk = [&](const QModelIndex &parent) {
        const int rows = m_treeModel->rowCount(parent);
        for (int row = 0; row < rows; ++row) {
            const QModelIndex idx = m_treeModel->index(row, 0, parent);
            if (!idx.isValid())
                continue;
            if (m_treeModel->hasChildren(idx)) {
                m_tree->setExpanded(idx, true);
                const qint64 id = treeItemId(idx);
                if (m_viewModel && id > 0)
                    m_viewModel->noteTreeExpanded(id, true);
            }
            walk(idx);
        }
    };
    walk({});
}

void PermissionResourceManagementView::applyTreeVisualSelection(const QModelIndex &index)
{
    if (!m_tree || !index.isValid())
        return;

    QItemSelectionModel *selModel = m_tree->selectionModel();
    if (!selModel)
        return;

    m_blockTreeSelection = true;
    const QItemSelection selection(index, index);
    selModel->select(selection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    m_tree->setCurrentIndex(index);
    expandAncestorsInView(index);
    m_tree->scrollTo(index, QAbstractItemView::EnsureVisible);
    m_tree->viewport()->update();
    m_blockTreeSelection = false;
}

void PermissionResourceManagementView::reapplyTreeSelectionVisual()
{
    if (!m_tree)
        return;

    QModelIndex idx = m_tree->currentIndex();
    if (!idx.isValid() && m_viewModel && m_viewModel->selectedResourceId() > 0)
        idx = findTreeIndexById(m_viewModel->selectedResourceId());

    if (idx.isValid()) {
        applyTreeVisualSelection(idx);
        return;
    }

    if (!m_treeModel || m_treeModel->rowCount() <= 0)
        return;

    const QModelIndex first = m_treeModel->index(0, 0);
    if (!first.isValid())
        return;

    applyTreeVisualSelection(first);
    if (m_viewModel) {
        const qint64 id = treeItemId(first);
        if (id != m_viewModel->selectedResourceId())
            m_viewModel->selectResource(id);
    }
}

void PermissionResourceManagementView::captureExpandedStateToViewModel()
{
    if (!m_viewModel || !m_tree || !m_treeModel)
        return;

    std::function<void(const QModelIndex &)> walk;
    walk = [&](const QModelIndex &parent) {
        const int rows = m_treeModel->rowCount(parent);
        for (int row = 0; row < rows; ++row) {
            const QModelIndex idx = m_treeModel->index(row, 0, parent);
            if (!idx.isValid())
                continue;
            const qint64 id = treeItemId(idx);
            if (id > 0 && m_tree->isExpanded(idx))
                m_viewModel->noteTreeExpanded(id, true);
            walk(idx);
        }
    };
    walk({});
}

void PermissionResourceManagementView::restoreExpandedStateFromViewModel()
{
    if (!m_viewModel || !m_tree)
        return;

    m_blockTreeSelection = true;
    for (qint64 id : m_viewModel->expandedResourceIds()) {
        const QModelIndex idx = findTreeIndexById(id);
        if (idx.isValid())
            m_tree->setExpanded(idx, true);
    }
    m_blockTreeSelection = false;
}

void PermissionResourceManagementView::expandAncestorsInView(const QModelIndex &index)
{
    if (!m_tree || !m_viewModel || !index.isValid())
        return;

    QModelIndex parent = index.parent();
    while (parent.isValid()) {
        m_tree->setExpanded(parent, true);
        m_viewModel->noteTreeExpanded(treeItemId(parent), true);
        parent = parent.parent();
    }
}

void PermissionResourceManagementView::syncEditorForm()
{
    if (!m_viewModel)
        return;

    m_blockEditorSignals = true;
    const auto &form = m_viewModel->editorForm();
    const bool creating = form.editingId <= 0;

    if (creating && form.resourceType.isEmpty()) {
        m_detailTitle->setText(PTXT("perm.resources.details_title"));
        m_lblType->clear();
        m_lblSource->clear();
        m_lblParent->clear();
        m_lblGrantRefs->clear();
        m_editKey->clear();
        m_editPageKey->clear();
        m_editDisplay->clear();
        m_editDesc->clear();
        m_editSort->setValue(0);
        m_editEnabled->setChecked(true);
        m_editKey->setEnabled(false);
        m_editPageKey->setEnabled(false);
        m_editDisplay->setEnabled(false);
        m_editDesc->setEnabled(false);
        m_editSort->setEnabled(false);
        m_editEnabled->setEnabled(false);
        m_blockEditorSignals = false;
        updateEditorActionButtons();
        return;
    }

    const QString localizedDisplay =
        permission_ui::permissionResourceDisplayText(form.key, form.displayName);
    m_detailTitle->setText(creating ? PTXT("perm.resources.new_resource")
                                    : PTXT("perm.resources.edit_title").arg(localizedDisplay));
    m_lblType->setText(resourceTypeLabel(form.resourceType));
    m_lblSource->setText(form.source.isEmpty() ? QStringLiteral("manual") : form.source);
    m_lblParent->setText(form.parentId <= 0
                             ? PTXT("perm.resources.root_parent")
                             : m_viewModel->parentDisplayName(form.parentId));
    m_lblGrantRefs->setText(QString::number(form.grantRefCount));

    m_editKey->setText(form.key);
    m_editPageKey->setText(form.pageKey);
    m_editDisplay->setText(localizedDisplay);
    m_editDesc->setText(form.description);
    m_editSort->setValue(form.sortOrder);
    m_editEnabled->setChecked(form.enabled);

    const bool syncSource = form.source == QStringLiteral("sync_menu")
                            || form.source == QStringLiteral("sync_code");
    const bool keyEditable = creating;
    m_editKey->setEnabled(keyEditable);
    m_editKey->setReadOnly(!keyEditable);
    m_editPageKey->setEnabled(!syncSource || creating);
    m_editDisplay->setEnabled(true);
    m_editDesc->setEnabled(true);
    m_editSort->setEnabled(true);
    m_editEnabled->setEnabled(form.editingId > 0 || creating);

    if (kPermissionResourcesCatalogReadOnly) {
        if (!creating && form.editingId > 0)
            m_detailTitle->setText(PTXT("perm.resources.details_title"));
        m_editKey->setEnabled(false);
        m_editKey->setReadOnly(true);
        m_editPageKey->setEnabled(false);
        m_editPageKey->setReadOnly(true);
        m_editDisplay->setEnabled(false);
        m_editDisplay->setReadOnly(true);
        m_editDesc->setEnabled(false);
        m_editDesc->setReadOnly(true);
        m_editSort->setEnabled(false);
        m_editEnabled->setEnabled(false);
    }

    if (creating) {
        m_blockTreeSelection = true;
        m_tree->clearSelection();
        m_blockTreeSelection = false;
    }

    m_blockEditorSignals = false;
    updateEditorActionButtons();
}

namespace {

bool canResourceAction(const QString &actionKey)
{
    if (PS->canAction(actionKey))
        return true;
    // 兼容旧库仅有 perm.resources.edit 的情况
    return PS->canAction(QStringLiteral("perm.resources.edit"));
}

} // namespace

void PermissionResourceManagementView::updateEditorActionButtons()
{
    if (kPermissionResourcesCatalogReadOnly)
        return;

    const bool hasSelection = m_viewModel && m_viewModel->selectedResourceId() > 0;
    const bool creating     = m_viewModel && m_viewModel->editorForm().editingId <= 0
                          && !m_viewModel->editorForm().resourceType.isEmpty();
    const bool dirty        = isEditorDirty();
    const bool canCreateNav     = PS->canAction(permission::PermKeys::ResourcesCreateNav);
    const bool canCreateSub     = PS->canAction(permission::PermKeys::ResourcesCreateSubPage);
    const bool canCreateAction  = PS->canAction(permission::PermKeys::ResourcesCreateAction);
    const bool canToggleEnabled = PS->canAction(permission::PermKeys::ResourcesToggleEnabled);
    const bool canReset         = canResourceAction(permission::PermKeys::ResourcesReset);
    const bool canSave          = canResourceAction(permission::PermKeys::ResourcesSave);
    const bool canDelete        = PS->canAction(permission::PermKeys::ResourcesDelete);

    if (m_btnSave)
        m_btnSave->setEnabled(dirty && canSave);
    if (m_btnReset)
        m_btnReset->setEnabled(dirty && canReset);
    if (m_btnDelete)
        m_btnDelete->setEnabled(hasSelection && !creating && canDelete);
    if (m_btnToggleEnabled) {
        m_btnToggleEnabled->setEnabled(hasSelection && !creating && canToggleEnabled);
        if (hasSelection && !creating && m_viewModel) {
            m_btnToggleEnabled->setText(m_viewModel->editorForm().enabled
                                            ? PTXT("perm.common.disabled")
                                            : PTXT("perm.common.enabled"));
        } else {
            m_btnToggleEnabled->setText(PTXT("perm.resources.toggle_enabled"));
        }
    }
    if (m_btnNewNav)
        m_btnNewNav->setEnabled(canCreateNav);
    if (m_btnNewSub)
        m_btnNewSub->setEnabled((hasSelection || creating) && canCreateSub);
    if (m_btnNewAction)
        m_btnNewAction->setEnabled((hasSelection || creating) && canCreateAction);
}

bool PermissionResourceManagementView::isEditorDirty() const
{
    return m_viewModel && m_viewModel->isEditorDirty();
}

void PermissionResourceManagementView::applyEditorFormToViewModel()
{
    if (!m_viewModel)
        return;
    m_viewModel->updateEditorForm(editorFormFromView());
}

permission::PermissionResourceEditorForm PermissionResourceManagementView::editorFormFromView()
    const
{
    permission::PermissionResourceEditorForm form = m_viewModel->editorForm();
    form.key         = m_editKey->text().trimmed();
    form.pageKey     = m_editPageKey->text().trimmed();
    const auto &baseline = m_viewModel->editorForm();
    const QString editedDisplay = m_editDisplay->text().trimmed();
    const QString localizedDisplay =
        permission_ui::permissionResourceDisplayText(baseline.key, baseline.displayName);
    form.displayName =
        (editedDisplay == localizedDisplay) ? baseline.displayName : editedDisplay;
    form.description = m_editDesc->text().trimmed();
    form.sortOrder   = m_editSort->value();
    form.enabled     = m_editEnabled->isChecked();
    return form;
}

bool PermissionResourceManagementView::confirmDiscardEditorEdits(const QString &message)
{
    return permission_ui::MessagePopup::show(
               this, PTXT("perm.common.notice"), message, QDialogButtonBox::Yes | QDialogButtonBox::No,
               QDialogButtonBox::No)
           == QDialogButtonBox::Yes;
}

QModelIndex PermissionResourceManagementView::findTreeIndexById(qint64 id) const
{
    if (!m_treeModel || id <= 0)
        return {};

    std::function<QModelIndex(QStandardItem *, qint64)> findIndex;
    findIndex = [&](QStandardItem *parent, qint64 targetId) -> QModelIndex {
        for (int r = 0; r < parent->rowCount(); ++r) {
            QStandardItem *item = parent->child(r);
            if (!item)
                continue;
            if (item->data(kRoleResourceId).toLongLong() == targetId)
                return item->index();
            const QModelIndex nested = findIndex(item, targetId);
            if (nested.isValid())
                return nested;
        }
        return {};
    };

    return findIndex(m_treeModel->invisibleRootItem(), id);
}

void PermissionResourceManagementView::selectTreeItemById(qint64 id)
{
    if (!m_tree || !m_treeModel || id <= 0)
        return;

    if (m_viewModel)
        m_viewModel->ensureExpandedOnPathTo(id);

    const QModelIndex idx = findTreeIndexById(id);
    if (!idx.isValid())
        return;

    applyTreeVisualSelection(idx);
}

qint64 PermissionResourceManagementView::treeItemId(const QModelIndex &index) const
{
    if (!index.isValid() || !m_treeModel)
        return 0;
    QStandardItem *item = m_treeModel->itemFromIndex(index);
    return item ? item->data(kRoleResourceId).toLongLong() : 0;
}

} // namespace permission
