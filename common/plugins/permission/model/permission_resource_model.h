#pragma once

#include "data/domain/domain_types.h"
#include "model/permission_resource_management_model.h"
#include "model/permission_result.h"

#include <QMap>
#include <QString>
#include <QStringList>
#include <QVector>

namespace permission {

class IDataStore;

/// 权限资源业务 Model（无 Q_OBJECT）
class PermissionResourceModel
{
public:
    explicit PermissionResourceModel(IDataStore *store = nullptr);

    QVector<PermissionRecord> loadAll() const;
    bool                      findById(qint64 id, PermissionRecord &out) const;
    bool findByKey(const QString &key, PermissionRecord &out) const;

    int childCount(qint64 parentId) const;
    int grantReferenceCount(qint64 permissionId) const;

    int nextSortOrder(qint64 parentId) const;

    OperationResult validateEditorForm(const PermissionResourceEditorForm &form) const;
    PermissionResourceEditorForm toEditorForm(const PermissionRecord &record,
                                              const QString &parentDisplayName = {}) const;

    OperationResult createResource(const PermissionResourceEditorForm &form,
                                   PermissionRecord *createdOut = nullptr);
    OperationResult updateResource(const PermissionResourceEditorForm &form);
    OperationResult deleteResource(qint64 id);
    OperationResult setEnabled(qint64 id, bool enabled);

    QVector<qint64> collectAncestorIds(qint64 id) const;
    QVector<qint64> collectDescendantIds(qint64 id) const;

    QVector<PermissionResourceTreeBranch> buildVisibleForest(
        const QVector<PermissionRecord> &all, const QString &keyword,
        const QString &resourceTypeFilter, int enabledFilterMode = -1) const;

private:
    OperationResult validateParentChain(const PermissionResourceEditorForm &form) const;
    OperationResult applyEnabledCascade(qint64 id, bool enabled, QString *cascadeInfo = nullptr);
    void            enableAncestors(qint64 id, QStringList *touchedNames);
    void            disableDescendants(qint64 id, QStringList *touchedNames);

    IDataStore *m_store = nullptr;
};

} // namespace permission
