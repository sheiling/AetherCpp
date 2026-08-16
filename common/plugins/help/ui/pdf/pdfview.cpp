#include "pdfview.h"

#include "mupdf_engine.h"
#include "mupdfdef.h"

#include <QPainter>
#include <QKeyEvent>
#include <QWheelEvent>

PdfView::PdfView(QWidget *parent)
    : QWidget(parent)
    , m_engine(new MuPdfEngine())
{
    setObjectName(QStringLiteral("pdfView"));
    setAttribute(Qt::WA_StyledBackground, true);
    setFocusPolicy(Qt::ClickFocus);
}

PdfView::~PdfView()
{
    delete m_engine;
    m_engine = nullptr;
}

void PdfView::setPdfPath(const QString &fileName)
{
    m_fileName = fileName;
}

void PdfView::setPage(int page)
{
    if (!loadPagePixmap(page)) {
        return;
    }

    m_scrollOffsetY = 0.0;
    update();
    notifyPageChanged();
}

void PdfView::nextPage()
{
    if (!loadPagePixmap(m_currentPage + 1)) {
        return;
    }

    m_scrollOffsetY = 0.0;
    update();
    notifyPageChanged();
}

void PdfView::previousPage()
{
    if (!loadPagePixmap(m_currentPage - 1)) {
        return;
    }

    m_scrollOffsetY = 0.0;
    update();
    notifyPageChanged();
}

void PdfView::zoomIn()
{
    if (m_pixmap.isNull()) {
        return;
    }

    m_scale *= MUPDF_READER_SCALE;
    if (m_scale > 4.0) {
        m_scale = 4.0;
    }

    refreshPixmap();
}

void PdfView::zoomOut()
{
    if (m_pixmap.isNull()) {
        return;
    }

    m_scale /= MUPDF_READER_SCALE;
    if (m_scale < 1.0) {
        m_scale = 1.0;
    }

    refreshPixmap();
}

void PdfView::loadPdf()
{
    if (!m_engine->open(m_fileName, m_pageCount)) {
        return;
    }

    m_currentPage = 0;
    m_scrollOffsetY = 0.0;
    refreshPixmap();
    notifyPageChanged();
}

void PdfView::dropPdf()
{
    m_engine->close();
    m_pixmap = {};
    m_pageCount = 0;
    m_currentPage = 0;
    m_scrollOffsetY = 0.0;
    update();
}

bool PdfView::isEmpty() const
{
    return m_pageCount <= 0 || m_pixmap.isNull();
}

void PdfView::keyPressEvent(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);

    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Up:
        previousPage();
        break;
    case Qt::Key_Right:
    case Qt::Key_Down:
        nextPage();
        break;
    default:
        break;
    }
}

void PdfView::wheelEvent(QWheelEvent *event)
{
    if (m_pixmap.isNull()) {
        return;
    }

    const int delta = event->angleDelta().y();
    if (delta > 0) {
        m_scrollOffsetY += MUPDF_READER_STEP;
    } else if (delta < 0) {
        m_scrollOffsetY -= MUPDF_READER_STEP;
    } else {
        return;
    }

    update();
}

void PdfView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    if (m_pixmap.isNull()) {
        return;
    }

    const int scrollExtent = pageScrollExtent();

    // 滚轮向上越过页顶：进入上一页并落在页底，与向下进入下一页落在页顶对称
    if (m_scrollOffsetY > MUPDF_READER_STEP) {
        if (loadPagePixmap(m_currentPage - 1)) {
            m_scrollOffsetY = -static_cast<double>(pageScrollExtent());
            notifyPageChanged();
        } else {
            m_scrollOffsetY = 0.0;
        }
    } else if (scrollExtent + m_scrollOffsetY < -MUPDF_READER_STEP) {
        if (loadPagePixmap(m_currentPage + 1)) {
            m_scrollOffsetY = 0.0;
            notifyPageChanged();
        } else {
            m_scrollOffsetY = -static_cast<double>(scrollExtent);
        }
    }

    QPainter painter(this);
    painter.drawPixmap((width() - m_pixmap.width()) / 2,
                       static_cast<int>(m_scrollOffsetY),
                       m_pixmap);
}

int PdfView::pageScrollExtent() const
{
    const int extent = m_pixmap.height() - height();
    return extent > 0 ? extent : 0;
}

bool PdfView::loadPagePixmap(int page)
{
    const QPixmap pixmap = m_engine->renderPage(page, m_scale);
    if (pixmap.isNull()) {
        return false;
    }

    m_pixmap = pixmap;
    m_currentPage = page;
    return true;
}

void PdfView::refreshPixmap()
{
    loadPagePixmap(m_currentPage);
    update();
}

void PdfView::notifyPageChanged()
{
    emit pageChanged(m_currentPage, m_pageCount);
}
