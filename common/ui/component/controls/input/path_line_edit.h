#pragma once

/**
 * @file path_line_edit.h
 * @brief 路径选择行：标签 + 路径输入框 + 浏览按钮
 *
 * 用于系统配置等场景；目录选择逻辑见 pickExistingDirectory / browseForDirectory。
 */

#include "component_global.h"

#include <QString>
#include <QWidget>

class QLineEdit;
class QLabel;

namespace ui {

class ThemedPushButton;

/** 弹出目录选择对话框，选中后写入 edit */
UI_COMPONENT_EXPORT void pickExistingDirectory(QLineEdit *edit, QWidget *dialogParent = nullptr);

/**
 * 路径选择行：标签 + 路径 ThemedLineEdit + Secondary 浏览按钮
 *
 * labelKey 为翻译键，展示为「TR(labelKey):」；浏览按钮使用 `aoi.sys.path.browse`。
 */
class UI_COMPONENT_EXPORT PathLineEdit final : public QWidget
{
    Q_OBJECT

public:
    explicit PathLineEdit(const QString &labelKey,
                          QWidget       *parent = nullptr,
                          const QString &path   = {});

    QString labelKey() const { return m_labelKey; }
    void    setLabelKey(const QString &key);

    int  labelWidth() const { return m_labelWidth; }
    void setLabelWidth(int width);

    QString path() const;
    void    setPath(const QString &path);

    QLabel           *label() const { return m_label; }
    QLineEdit        *lineEdit() const { return m_edit; }
    ThemedPushButton *browseButton() const { return m_browseBtn; }

    void browseForDirectory(QWidget *dialogParent = nullptr);
    void retranslateRow();

protected:
    void changeEvent(QEvent *event) override;

private:
    void buildUi(const QString &path);
    void applyLabelWidth();

    QString           m_labelKey;
    int               m_labelWidth  = 150;
    QLabel           *m_label     = nullptr;
    QLineEdit        *m_edit      = nullptr;
    ThemedPushButton *m_browseBtn = nullptr;
};

UI_COMPONENT_EXPORT PathLineEdit *makePathLineEdit(const QString &labelKey,
                               QWidget       *parent = nullptr,
                               const QString &path   = {});

} // namespace ui
