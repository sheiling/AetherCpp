#include "pdfwidget.h"

#include "pdfview.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QShowEvent>
#include <QVBoxLayout>

PdfWidget::PdfWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName(QStringLiteral("pdfWidget"));
    setAttribute(Qt::WA_StyledBackground, true);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(6);

    m_toolBar = new QFrame(this);
    m_toolBar->setObjectName(QStringLiteral("pdfToolBar"));
    m_toolBar->setAttribute(Qt::WA_StyledBackground, true);
    auto *toolBarLayout = new QHBoxLayout(m_toolBar);
    toolBarLayout->setContentsMargins(8, 6, 8, 6);
    toolBarLayout->setSpacing(6);

    m_btnPrevious = new QPushButton(QStringLiteral("<"), m_toolBar);
    m_btnPrevious->setObjectName(QStringLiteral("pdfNavBtn"));
    m_btnPrevious->setFlat(true);
    m_btnNext = new QPushButton(QStringLiteral(">"), m_toolBar);
    m_btnNext->setObjectName(QStringLiteral("pdfNavBtn"));
    m_btnNext->setFlat(true);
    m_editPage = new QLineEdit(m_toolBar);
    m_editPage->setObjectName(QStringLiteral("pdfPageEdit"));
    m_editPage->setFixedWidth(48);
    m_editPage->setAlignment(Qt::AlignCenter);
    m_lblSep = new QLabel(QStringLiteral("/"), m_toolBar);
    m_lblSep->setObjectName(QStringLiteral("pdfPageLabel"));
    m_lblTotal = new QLabel(QStringLiteral("0"), m_toolBar);
    m_lblTotal->setObjectName(QStringLiteral("pdfPageLabel"));
    m_btnZoomIn = new QPushButton(QStringLiteral("+"), m_toolBar);
    m_btnZoomIn->setObjectName(QStringLiteral("pdfToolBtn"));
    m_btnZoomOut = new QPushButton(QStringLiteral("-"), m_toolBar);
    m_btnZoomOut->setObjectName(QStringLiteral("pdfToolBtn"));

    toolBarLayout->addWidget(m_btnPrevious);
    toolBarLayout->addWidget(m_btnNext);
    toolBarLayout->addWidget(m_editPage);
    toolBarLayout->addWidget(m_lblSep);
    toolBarLayout->addWidget(m_lblTotal);
    toolBarLayout->addStretch();
    toolBarLayout->addWidget(m_btnZoomIn);
    toolBarLayout->addWidget(m_btnZoomOut);

    m_view = new PdfView(this);
    rootLayout->addWidget(m_toolBar);
    rootLayout->addWidget(m_view, 1);

    connect(m_btnPrevious, &QPushButton::clicked, m_view, &PdfView::previousPage);
    connect(m_btnNext, &QPushButton::clicked, m_view, &PdfView::nextPage);
    connect(m_btnZoomIn, &QPushButton::clicked, m_view, &PdfView::zoomIn);
    connect(m_btnZoomOut, &QPushButton::clicked, m_view, &PdfView::zoomOut);
    connect(m_editPage, &QLineEdit::editingFinished, this, [this]() {
        const int page = m_editPage->text().trimmed().toInt() - 1;
        m_view->setPage(page);
    });
    connect(m_view, &PdfView::pageChanged, this, &PdfWidget::onPageChanged);

    applyToolbarStyle();
}

void PdfWidget::applyToolbarStyle()
{
    setStyleSheet(QStringLiteral(
        "QWidget#pdfWidget {"
        "  background-color: transparent;"
        "}"
        "QFrame#pdfToolBar {"
        "  background-color: #152a3d;"
        "  border: 1px solid #4a5f79;"
        "  border-radius: 6px;"
        "}"
        "QFrame#pdfToolBar QLabel#pdfPageLabel {"
        "  color: #f0f6ff;"
        "  font-size: 13px;"
        "  background-color: transparent;"
        "  padding: 0 2px;"
        "}"
        "QFrame#pdfToolBar QLineEdit#pdfPageEdit {"
        "  background-color: #1f2d40;"
        "  color: #ffffff;"
        "  border: 1px solid #6b8cae;"
        "  border-radius: 4px;"
        "  padding: 2px 4px;"
        "  font-size: 13px;"
        "  selection-background-color: #1976d2;"
        "  selection-color: #ffffff;"
        "}"
        "QFrame#pdfToolBar QLineEdit#pdfPageEdit:focus {"
        "  border-color: #90caf9;"
        "}"
        "QFrame#pdfToolBar QPushButton#pdfNavBtn {"
        "  min-width: 24px;"
        "  min-height: 26px;"
        "  background-color: transparent;"
        "  color: #f0f6ff;"
        "  border: none;"
        "  border-radius: 4px;"
        "  font-size: 13px;"
        "  font-weight: 600;"
        "  padding: 2px 6px;"
        "}"
        "QFrame#pdfToolBar QPushButton#pdfNavBtn:hover {"
        "  background-color: #263a52;"
        "  color: #ffffff;"
        "}"
        "QFrame#pdfToolBar QPushButton#pdfNavBtn:pressed {"
        "  background-color: #1a3550;"
        "}"
        "QFrame#pdfToolBar QPushButton#pdfToolBtn {"
        "  min-width: 28px;"
        "  min-height: 26px;"
        "  background-color: #1e2d40;"
        "  color: #f0f6ff;"
        "  border: 1px solid #6b8cae;"
        "  border-radius: 4px;"
        "  font-size: 13px;"
        "  font-weight: 600;"
        "  padding: 2px 8px;"
        "}"
        "QFrame#pdfToolBar QPushButton#pdfToolBtn:hover {"
        "  background-color: #263a52;"
        "  border-color: #90caf9;"
        "  color: #ffffff;"
        "}"
        "QFrame#pdfToolBar QPushButton#pdfToolBtn:pressed {"
        "  background-color: #1a3550;"
        "}"
        "QWidget#pdfView {"
        "  background-color: #0f1620;"
        "  border: 1px solid #4a5f79;"
        "  border-radius: 4px;"
        "}"
    ));
}

void PdfWidget::setPdf(const QString &fileName)
{
    m_fileName = fileName;
    m_view->setPdfPath(m_fileName);
}

void PdfWidget::showEvent(QShowEvent *event)
{
    if (!m_fileName.isEmpty() && m_view->isEmpty()) {
        m_view->loadPdf();
    }
    QWidget::showEvent(event);
}

void PdfWidget::onPageChanged(int currentPage, int pageCount)
{
    m_editPage->setText(QString::number(currentPage + 1));
    m_lblTotal->setText(QString::number(pageCount));
}
