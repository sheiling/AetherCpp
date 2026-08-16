#pragma once

#ifdef interface
#undef interface
#endif

#include "command.h"
#include "model/permission_resource_management_model.h"
#include "model/permission_resource_model.h"
#include "viewmodel.h"

#include <QSet>
#include <QVector>

#include <memory>

class PermissionResourceViewModel : public ViewModel
{
    Q_OBJECT

public:
    static constexpr const char *PROP_SEARCH_KEYWORD        = "searchKeyword";
    static constexpr const char *PROP_RESOURCE_TYPE_FILTER  = "resourceTypeFilter";
    static constexpr const char *PROP_ENABLED_FILTER_MODE   = "enabledFilterMode";
    static constexpr const char *PROP_SELECTED_RESOURCE_ID  = "selectedResourceId";

    VIEWMODEL_PROPERTY(QString, searchKeyword, SearchKeyword)
    VIEWMODEL_PROPERTY(QString, resourceTypeFilter, ResourceTypeFilter)
    VIEWMODEL_PROPERTY(int,     enabledFilterMode, EnabledFilterMode) // -1=全部，1=仅启用，0=仅禁用
    VIEWMODEL_PROPERTY(qint64, selectedResourceId, SelectedResourceId)

signals:
    void searchKeywordChanged();
    void resourceTypeFilterChanged();
    void enabledFilterModeChanged();
    void selectedResourceIdChanged();

    void resourceTreeChanged();
    void editorFormChanged();
    void editorDirtyChanged();

    void showInfo(const QString &title, const QString &message);
    void showWarning(const QString &title, const QString &message);
    void keyConflictFound(qint64 id, bool enabled);

public:
    explicit PermissionResourceViewModel(permission::PermissionResourceModel *model = nullptr,
                                         QObject *parent                        = nullptr);

    const QVector<permission::PermissionResourceTreeBranch> &visibleForest() const
    {
        return m_visibleForest;
    }
    const permission::PermissionResourceEditorForm &editorForm() const { return m_editorForm; }
    const permission::PermissionResourceEditorForm &editorBaseline() const
    {
        return m_editorBaseline;
    }
    bool isEditorDirty() const;

    const QSet<qint64> &expandedResourceIds() const { return m_expandedResourceIds; }
    void noteTreeExpanded(qint64 id, bool expanded);
    void ensureExpandedOnPathTo(qint64 id);

    QString validationMessage(const QString &key) const;

    Command *saveCommand() const { return m_saveCommand; }
    Command *resetCommand() const { return m_resetCommand; }

public slots:
    void refreshCommandStates();
    void refreshTree();
    void applyFilters();
    void selectResource(qint64 id);
    void beginCreateNav();
    void beginCreateSubPage();
    void beginCreateAction();
    void updateEditorForm(const permission::PermissionResourceEditorForm &form);
    permission::OperationResult saveEditor();
    void resetEditor();
    permission::OperationResult deleteSelected();
    permission::OperationResult toggleSelectedEnabled();

    /// 按当前语言解析父节点展示名（资源 key → 各插件 i18n，否则 DB display_name）
    QString parentDisplayName(qint64 parentId) const;

private:
    void bindCommands();
    void updateCommandStates();

    void rebuildVisibleForest();
    void loadEditorForId(qint64 id);
    void loadEditorForCreate(const permission::PermissionResourceEditorForm &draft);
    permission::PermissionResourceEditorForm defaultCreateForm(
        const QString &resourceType, qint64 parentId) const;

    std::unique_ptr<permission::PermissionResourceModel> m_ownedModel;
    permission::PermissionResourceModel                 *m_model = nullptr;

    QVector<permission::PermissionRecord>              m_allRecords;
    QVector<permission::PermissionResourceTreeBranch> m_visibleForest;

    permission::PermissionResourceEditorForm m_editorForm;
    permission::PermissionResourceEditorForm m_editorBaseline;
    bool                                     m_creating = false;

    QSet<qint64> m_expandedResourceIds;

    FunctionalCommand *m_saveCommand  = nullptr;
    FunctionalCommand *m_resetCommand = nullptr;
};
