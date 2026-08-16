#pragma once

/**
 * @file defect_type_filter_row.h
 * @brief 缺陷 MAP 侧栏单行：勾选框 + 色块 + 文案（整行可点，对齐原型图 2）
 */

#include <QWidget>

#include <Qt>

class QLabel;

class QMouseEvent;

namespace aoi::review {

class DefectTypeFilterRow : public QWidget
{
    Q_OBJECT

public:
  explicit DefectTypeFilterRow(const QColor &swatchColor, QWidget *parent = nullptr,
                                 bool showSwatch = true);

    void setLabelText(const QString &text);
    void setChecked(bool checked);
    void setCheckState(Qt::CheckState state);
    void setEnabled(bool enabled);
    bool isChecked() const;
    Qt::CheckState checkState() const;

signals:
    void toggled(bool checked);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void updateCheckIndicator();
    void toggle();

    QColor         m_swatchColor;
    bool           m_checked = false;
    Qt::CheckState m_checkState = Qt::Unchecked;
    bool           m_enabled      = true;
    bool           m_showSwatch   = true;
    QLabel *m_checkIndicator = nullptr;
    QLabel *m_swatch         = nullptr;
    QLabel *m_label          = nullptr;
};

} // namespace aoi::review
