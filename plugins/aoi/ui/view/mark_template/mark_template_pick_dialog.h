#pragma once

/**
 * @file mark_template_pick_dialog.h
 * @brief 选择已有 Mark 模板非模态对话框（ThemedPanelDialog + 内嵌表单）
 */

#include "component/dialog/themed_panel_dialog.h"

#include <QString>

class QLabel;
class QPushButton;

namespace ui {

class ThemedComboBox;
class ThemedLineEdit;
class ThemedPanelDialog;
class ThemedPushButton;

} // namespace ui

namespace aoi {

/**
 * @brief 从已有 .model 文件选择模板名称，支持确认选用或删除
 */
class MarkTemplatePickDialog : public ui::ThemedPanelDialog
{
    Q_OBJECT

public:
    struct ShowParams
    {
        QString defaultPath;
        QString defaultName;
    };

    explicit MarkTemplatePickDialog(QWidget *parent = nullptr);

    /** 查找或创建弹窗、应用参数并显示；已可见时仅置顶 */
    static MarkTemplatePickDialog *open(QWidget *parent, const ShowParams &params = {});

    void applyShowParams(const ShowParams &params);

    void setDefaultPath(const QString &path);
    void setDefaultName(const QString &name);

    QString modelPath() const;
    QString modelName() const;

    void refreshModelNames();
    void setBusy(bool busy);

signals:
    void pickConfirmed(const QString &modelPath, const QString &modelName);
    void deleteRequested(const QString &modelPath, const QString &modelName);

protected:
    void retranslateBody() override;

private:
    void buildForm();
    void retranslateForm();
    void setNameItems(const QStringList &names);
    void updateActionStates();
    void onConfirmClicked();
    void onDeleteClicked();
    void browseModelPath();

    ui::ThemedLineEdit   *m_pathEdit   = nullptr;
    ui::ThemedPushButton *m_browseBtn  = nullptr;
    ui::ThemedComboBox   *m_nameCombo  = nullptr;
    ui::ThemedPushButton *m_confirmBtn = nullptr;
    ui::ThemedPushButton *m_closeBtn   = nullptr;
    QPushButton      *m_deleteBtn  = nullptr;
};

} // namespace aoi
