#include "mupdf_engine.h"

#include <QDebug>
#include <QImage>

#include "mupdf/fitz.h"
#include "mupdf/pdf.h"

MuPdfEngine::MuPdfEngine() = default;

MuPdfEngine::~MuPdfEngine()
{
    close();
}

bool MuPdfEngine::open(const QString &fileName, int &pageCount)
{
    if (m_ctx) {
        qDebug() << "[MuPdfEngine] context already created";
        return false;
    }

    m_ctx = fz_new_context(nullptr, nullptr, FZ_STORE_UNLIMITED);
    if (!m_ctx) {
        qDebug() << "[MuPdfEngine] cannot create mupdf context";
        return false;
    }

    fz_try(m_ctx)
    {
        fz_register_document_handlers(m_ctx);
    }
    fz_catch(m_ctx)
    {
        qDebug() << "[MuPdfEngine] cannot register document handlers:"
                 << fz_caught_message(m_ctx);
        fz_drop_context(m_ctx);
        m_ctx = nullptr;
        return false;
    }

    pageCount = 0;
    if (m_doc) {
        qDebug() << "[MuPdfEngine] document already opened";
        return false;
    }

    fz_try(m_ctx)
    {
        m_doc = fz_open_document(m_ctx, fileName.toUtf8().constData());
    }
    fz_catch(m_ctx)
    {
        qDebug() << "[MuPdfEngine] cannot open document:" << fz_caught_message(m_ctx);
        m_doc = nullptr;
        return false;
    }

    fz_try(m_ctx)
    {
        m_pageCount = fz_count_pages(m_ctx, m_doc);
    }
    fz_catch(m_ctx)
    {
        qDebug() << "[MuPdfEngine] cannot count pages:" << fz_caught_message(m_ctx);
        fz_drop_document(m_ctx, m_doc);
        m_doc = nullptr;
        m_pageCount = 0;
        return false;
    }

    pageCount = m_pageCount;
    return true;
}

void MuPdfEngine::close()
{
    if (m_doc) {
        fz_drop_document(m_ctx, m_doc);
        m_doc = nullptr;
        m_pageCount = 0;
    }

    if (m_ctx) {
        fz_drop_context(m_ctx);
        m_ctx = nullptr;
    }
}

QPixmap MuPdfEngine::renderPage(int &page, double scale)
{
    if (m_pageCount <= 0 || !m_ctx || !m_doc) {
        return {};
    }

    if (page < 0) {
        page = 0;
    }
    if (page >= m_pageCount) {
        page = m_pageCount - 1;
    }

    fz_matrix ctm = fz_scale(static_cast<float>(scale), static_cast<float>(scale));
    ctm = fz_pre_rotate(ctm, 0.0f);

    fz_pixmap *pix = nullptr;
    fz_try(m_ctx)
    {
        pix = fz_new_pixmap_from_page_number(m_ctx, m_doc, page, ctm, fz_device_rgb(m_ctx), 0);
    }
    fz_catch(m_ctx)
    {
        qDebug() << "[MuPdfEngine] cannot render page:" << fz_caught_message(m_ctx);
        return {};
    }

    const int width = fz_pixmap_width(m_ctx, pix);
    const int height = fz_pixmap_height(m_ctx, pix);
    const int stride = static_cast<int>(pix->stride);
    QImage image(pix->samples, width, height, stride, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(image.copy());

    fz_drop_pixmap(m_ctx, pix);
    return pixmap;
}
