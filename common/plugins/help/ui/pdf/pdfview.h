#pragma once

#include <QPixmap>
#include <QString>
#include <QWidget>

class MuPdfEngine;

/**
 * @brief PDF 页面绘制区（翻页、缩放、滚轮/键盘交互）
 */
class PdfView : public QWidget
{
    Q_OBJECT

public:
    explicit PdfView(QWidget *parent = nullptr);
    ~PdfView() override;

    void setPdfPath(const QString &fileName);
    void setPage(int page);
    void nextPage();
    void previousPage();
    void zoomIn();
    void zoomOut();

    void loadPdf();
    void dropPdf();

    /** 当前是否已成功加载文档 */
    bool isEmpty() const;

signals:
    void pageChanged(int currentPage, int pageCount);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    /** 当前页在视口内可纵向滚动的最大像素（页高不超过视口时为 0） */
    int pageScrollExtent() const;
    /** 渲染指定页并更新缓存；失败时不改动当前页 */
    bool loadPagePixmap(int page);
    void refreshPixmap();
    void notifyPageChanged();

    MuPdfEngine *m_engine = nullptr;
    QPixmap m_pixmap;
    int m_pageCount = 0;
    int m_currentPage = 0;
    double m_scrollOffsetY = 0.0;
    double m_scale = 2.0;
    QString m_fileName;
};
