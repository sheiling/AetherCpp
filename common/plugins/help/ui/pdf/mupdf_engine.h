#pragma once

#include <QPixmap>
#include <QString>

/**
 * @brief MuPDF 文档渲染封装（打开/关闭/按页渲染为 QPixmap）
 */
class MuPdfEngine
{
public:
    MuPdfEngine();
    ~MuPdfEngine();

    bool open(const QString &fileName, int &pageCount);
    void close();

    QPixmap renderPage(int &page, double scale);

private:
    struct fz_context *m_ctx = nullptr;
    struct fz_document *m_doc = nullptr;
    int m_pageCount = 0;
};
