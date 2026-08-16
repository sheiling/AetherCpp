#pragma once

#include <QWidget>

class PdfView;
class QFrame;
class QLineEdit;
class QLabel;
class QPushButton;
class QShowEvent;

/**
 * @brief PDF 阅读控件（工具栏 + 页面视图，纯代码布局，无 .ui 文件）
 */
class PdfWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PdfWidget(QWidget *parent = nullptr);
    ~PdfWidget() override = default;

    void setPdf(const QString &fileName);

protected:
    /** 首次显示时加载 PDF，切换标签不再重复 load/drop */
    void showEvent(QShowEvent *event) override;

private slots:
    void onPageChanged(int currentPage, int pageCount);

private:
    void applyToolbarStyle();

    QFrame      *m_toolBar     = nullptr;
    PdfView     *m_view        = nullptr;
    QPushButton *m_btnPrevious = nullptr;
    QPushButton *m_btnNext     = nullptr;
    QLineEdit   *m_editPage    = nullptr;
    QLabel      *m_lblSep      = nullptr;
    QLabel      *m_lblTotal    = nullptr;
    QPushButton *m_btnZoomIn   = nullptr;
    QPushButton *m_btnZoomOut  = nullptr;
    QString      m_fileName;
};
