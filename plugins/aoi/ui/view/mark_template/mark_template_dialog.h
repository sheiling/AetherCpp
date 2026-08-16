#pragma once

/**
 * @file mark_template_dialog.h
 * @brief Mark 模板制作非模态对话框（ThemedPanelDialog + 内嵌表单）
 */

#include "component/dialog/themed_panel_dialog.h"
#include "service/vision_bridge/mark_template_kind.h"

#include <QRect>
#include <QString>

class QLabel;

namespace ui {

class ThemedComboBox;
class ThemedLineEdit;
class ThemedPanelDialog;
class ThemedPushButton;

} // namespace ui

namespace aoi {

/**
 * @brief Mark 模板制作非模态对话框：填写路径/名称/类型，发出 makeRequested
 */
class MarkTemplateDialog : public ui::ThemedPanelDialog
{
    Q_OBJECT

public:
    struct ShowParams
    {
        QString       defaultPath;
        QString       defaultName;
        MarkTemplateKind modelType = MarkTemplateKind::Mark;
    };

    explicit MarkTemplateDialog(QWidget *parent = nullptr);

    /** 查找或创建弹窗、应用参数并显示；已可见时仅置顶 */
    static MarkTemplateDialog *open(QWidget *parent, const ShowParams &params = {});

    void applyShowParams(const ShowParams &params);

    void setDefaultPath(const QString &path);
    void setDefaultName(const QString &name);
    void setModelType(MarkTemplateKind type);
    void updateRoiHint(const QRect &roi, bool valid);

    QString modelPath() const;
    QString modelName() const;
    MarkTemplateKind modelType() const;

    void setMaking(bool making);
    bool isMaking() const { return m_making; }

signals:
    void makeRequested();

protected:
    void retranslateBody() override;

private:
    void buildForm();
    void browseModelPath();
    void retranslateForm();

    ui::ThemedLineEdit   *m_pathEdit  = nullptr;
    ui::ThemedPushButton *m_browseBtn = nullptr;
    ui::ThemedLineEdit   *m_nameEdit  = nullptr;
    ui::ThemedComboBox   *m_typeCombo = nullptr;
    QLabel               *m_roiLabel  = nullptr;
    ui::ThemedPushButton *m_closeBtn  = nullptr;
    ui::ThemedPushButton *m_makeBtn   = nullptr;

    bool m_making = false;
};

} // namespace aoi
