#pragma once

/**
 * @file themed_table_widget.h
 * @brief 主题表格：QTableWidget + ThemeTokens 动态样式
 */

#include <QTableWidget>
#include "component_global.h"

#include <QSet>

class QEvent;
class QShowEvent;

namespace ui {

/**
 * @brief 主题化 QTableWidget
 *
 * 未显式 setRowHeight 的行按所占高度均分（不低于最小行高）；
 * 显式行高或 resizeHeightToContents 后按实际高度；超出时垂直滚动。
 */
class UI_COMPONENT_EXPORT ThemedTableWidget : public QTableWidget
{
    Q_OBJECT

public:
    explicit ThemedTableWidget(QWidget *parent = nullptr);
    ThemedTableWidget(int rows, int columns, QWidget *parent = nullptr);

    void applyCompactFormDefaults();

    void setHorizontalStretchFactors(const QList<int> &factors);

    /** 清除比例列宽，改由外部 setSectionResizeMode 控制 */
    void clearHorizontalStretchFactors();

    /** 按当前各行高度收紧总高并关闭自动行高分配（用于表单型定高表格） */
    void resizeHeightToContents();

    void setRowHeight(int row, int height);

    void setMinimumRowHeight(int pixels);
    int  minimumRowHeight() const { return m_minRowHeight; }

    void setAutoRowHeightDistribution(bool enabled);
    bool autoRowHeightDistribution() const { return m_autoDistributeRowHeight; }

    /** 恢复单行为自动行高（参与均分） */
    void clearRowHeightOverride(int row);
    void clearAllRowHeightOverrides();

    void setRowCount(int rows);

protected:
    void changeEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void bindTheme();
    void refreshStyle();
    void applyHorizontalStretchFactors();
    void updateRowHeightLayout();
    int  availableRowAreaHeight() const;

    QList<int> m_hStretchFactors;
    QSet<int>  m_explicitRowHeights;
    int        m_minRowHeight            = 22;
    bool       m_autoDistributeRowHeight = true;
    bool       m_hideScrollBarWhenFits   = true;
};

UI_COMPONENT_EXPORT ThemedTableWidget *makeTableWidget(int rows, int columns, QWidget *parent = nullptr);

} // namespace ui
