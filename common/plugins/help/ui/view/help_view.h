#pragma once

/**
 * @file help_view.h
 * @brief 帮助视图（版本信息、用户手册/维护手册/帮助文档 PDF）
 */

#include "pagewidget.h"

#include <QStackedWidget>

class QButtonGroup;
class QWidget;

namespace help {

/**
 * @brief 帮助视图 — 左侧二级导航（与系统配置 secondaryNavBar 同款）+ 右侧圆角内容区 / QStackedWidget
 */
class HelpView : public PageWidget
{
    Q_OBJECT

public:
    explicit HelpView(QWidget *parent = nullptr);

protected:
    void onLoad() override;
    void applyTheme() override;

private:
    void buildLayout();
    void applyRootStylesheet();
    void applyNavBarStyle();
    void applyMainPanelStyle();

    QWidget *buildVersionPage();
    /** 内嵌 PdfWidget 加载 help.documents 配置的 PDF 路径 */
    QWidget *buildPdfPage(const QString &pdfPath);

    QStackedWidget *m_stack      = nullptr;
    QWidget        *m_navBar     = nullptr;
    QWidget        *m_mainPanel  = nullptr;
    QButtonGroup   *m_navGroup   = nullptr;
};

} // namespace help
