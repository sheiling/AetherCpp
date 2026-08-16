#include "framelesswindow.h"
#include <QApplication>
#include <QPoint>
#include <QSize>
#include <QTimer>
#ifdef Q_OS_WIN

#include <windows.h>
#include <WinUser.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <objidl.h>
#include <gdiplus.h>
#include <QResizeEvent>
#include <QShowEvent>
#pragma comment (lib,"Dwmapi.lib")
#pragma comment (lib,"user32.lib")
#pragma comment (lib,"gdi32.lib")

#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
#define DWMWA_WINDOW_CORNER_PREFERENCE 33
#endif

#ifndef _DWMWINDOWCORNERPREFERENCE_DEFINED
#define _DWMWINDOWCORNERPREFERENCE_DEFINED
typedef enum {
    DWMWCP_DEFAULT_ENUM    = 0,
    DWMWCP_DONOTROUND_ENUM = 1,
    DWMWCP_ROUND_ENUM      = 2,
    DWMWCP_ROUNDSMALL_ENUM = 3
} DWMWCP_WINDOW_CORNER_PREFERENCE;
#endif

CFramelessWindow::CFramelessWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_titlebar(nullptr)
    , m_borderWidth(5)
    , m_bJustMaximized(false)
    , m_bResizeable(true)
    , m_cornerRadius(0)
{
    setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setResizeable(m_bResizeable);
    QTimer::singleShot(100, this, [this]() { applyRoundedCorners(); });
}

void CFramelessWindow::setResizeable(bool resizeable)
{
    bool visible = isVisible();
    m_bResizeable = resizeable;
    if (m_bResizeable) {
        setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
        HWND hwnd = (HWND)this->winId();
        DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
        ::SetWindowLong(hwnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
    } else {
        setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
        HWND hwnd = (HWND)this->winId();
        DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
        ::SetWindowLong(hwnd, GWL_STYLE, style & ~WS_MAXIMIZEBOX & ~WS_CAPTION);
    }
    const MARGINS shadow = { 1, 1, 1, 1 };
    DwmExtendFrameIntoClientArea(HWND(winId()), &shadow);
    setVisible(visible);
}

void CFramelessWindow::setResizeableAreaWidth(int width)
{
    if (1 > width) width = 1;
    m_borderWidth = width;
}

void CFramelessWindow::setTitleBar(QWidget *titlebar)
{
    m_titlebar = titlebar;
    if (!titlebar) return;
    connect(titlebar, SIGNAL(destroyed(QObject*)), this, SLOT(onTitleBarDestroyed()));
}

void CFramelessWindow::onTitleBarDestroyed()
{
    if (m_titlebar == QObject::sender())
        m_titlebar = nullptr;
}

void CFramelessWindow::addIgnoreWidget(QWidget *widget)
{
    if (!widget) return;
    if (m_whiteList.contains(widget)) return;
    m_whiteList.append(widget);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
bool CFramelessWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
#else
bool CFramelessWindow::nativeEvent(const QByteArray &eventType, void *message, long *result)
#endif
{
#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
    MSG *msg = *reinterpret_cast<MSG **>(message);
#else
    MSG *msg = reinterpret_cast<MSG *>(message);
#endif

    switch (msg->message) {
    case WM_NCCALCSIZE: {
        NCCALCSIZE_PARAMS &params = *reinterpret_cast<NCCALCSIZE_PARAMS *>(msg->lParam);
        if (params.rgrc[0].top != 0)
            params.rgrc[0].top -= 1;
        *result = WVR_REDRAW;
        return true;
    }
    case WM_NCHITTEST: {
        *result = 0;
        const LONG border_width = m_borderWidth;
        RECT winrect;
        GetWindowRect(HWND(winId()), &winrect);
        long x = GET_X_LPARAM(msg->lParam);
        long y = GET_Y_LPARAM(msg->lParam);

        if (m_bResizeable) {
            bool resizeWidth  = minimumWidth()  != maximumWidth();
            bool resizeHeight = minimumHeight() != maximumHeight();
            if (resizeWidth) {
                if (x >= winrect.left && x < winrect.left + border_width) *result = HTLEFT;
                if (x < winrect.right && x >= winrect.right - border_width) *result = HTRIGHT;
            }
            if (resizeHeight) {
                if (y < winrect.bottom && y >= winrect.bottom - border_width) *result = HTBOTTOM;
                if (y >= winrect.top   && y < winrect.top + border_width)     *result = HTTOP;
            }
            if (resizeWidth && resizeHeight) {
                if (x >= winrect.left && x < winrect.left + border_width   && y < winrect.bottom && y >= winrect.bottom - border_width) *result = HTBOTTOMLEFT;
                if (x < winrect.right && x >= winrect.right - border_width && y < winrect.bottom && y >= winrect.bottom - border_width) *result = HTBOTTOMRIGHT;
                if (x >= winrect.left && x < winrect.left + border_width   && y >= winrect.top   && y < winrect.top + border_width)     *result = HTTOPLEFT;
                if (x < winrect.right && x >= winrect.right - border_width && y >= winrect.top   && y < winrect.top + border_width)     *result = HTTOPRIGHT;
            }
        }
        if (0 != *result) return true;

        if (!m_titlebar) return false;
        double dpr = this->devicePixelRatioF();
        QPoint pos = m_titlebar->mapFromGlobal(QPoint(x / dpr, y / dpr));
        if (!m_titlebar->rect().contains(pos)) return false;
        QWidget *child = m_titlebar->childAt(pos);
        if (!child) {
            *result = HTCAPTION;
            return true;
        } else {
            if (m_whiteList.contains(child)) {
                *result = HTCAPTION;
                return true;
            }
        }
        return false;
    }
    case WM_GETMINMAXINFO: {
        if (::IsZoomed(msg->hwnd)) {
            RECT frame = { 0, 0, 0, 0 };
            AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);
            double dpr = this->devicePixelRatioF();
            m_frames.setLeft(abs(frame.left) / dpr + 0.5);
            m_frames.setTop(abs(frame.bottom) / dpr + 0.5);
            m_frames.setRight(abs(frame.right) / dpr + 0.5);
            m_frames.setBottom(abs(frame.bottom) / dpr + 0.5);
            QMainWindow::setContentsMargins(m_frames.left() + m_margins.left(),
                                            m_frames.top()  + m_margins.top(),
                                            m_frames.right()+ m_margins.right(),
                                            m_frames.bottom()+m_margins.bottom());
            m_bJustMaximized = true;
            SetWindowRgn(msg->hwnd, NULL, TRUE);
            DWMWCP_WINDOW_CORNER_PREFERENCE cp = DWMWCP_DONOTROUND_ENUM;
            DwmSetWindowAttribute(msg->hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &cp, sizeof(cp));
        } else {
            if (m_bJustMaximized) {
                QMainWindow::setContentsMargins(m_margins);
                m_frames = QMargins();
                m_bJustMaximized = false;
                QTimer::singleShot(10, this, [this]() { applyRoundedCorners(); });
            }
        }
        return false;
    }
    default:
        return QMainWindow::nativeEvent(eventType, message, result);
    }
}

void CFramelessWindow::setContentsMargins(const QMargins &margins)
{
    QMainWindow::setContentsMargins(margins + m_frames);
    m_margins = margins;
}

void CFramelessWindow::setContentsMargins(int left, int top, int right, int bottom)
{
    QMainWindow::setContentsMargins(left + m_frames.left(), top + m_frames.top(),
                                    right + m_frames.right(), bottom + m_frames.bottom());
    m_margins.setLeft(left);
    m_margins.setTop(top);
    m_margins.setRight(right);
    m_margins.setBottom(bottom);
}

QMargins CFramelessWindow::contentsMargins() const
{
    QMargins margins = QMainWindow::contentsMargins();
    margins -= m_frames;
    return margins;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void CFramelessWindow::getContentsMargins(int *left, int *top, int *right, int *bottom) const
{
    QMainWindow::getContentsMargins(left, top, right, bottom);
    if (!(left && top && right && bottom)) return;
    if (isMaximized()) {
        *left   -= m_frames.left();
        *top    -= m_frames.top();
        *right  -= m_frames.right();
        *bottom -= m_frames.bottom();
    }
}
#endif

QRect CFramelessWindow::contentsRect() const
{
    QRect rect = QMainWindow::contentsRect();
    int w = rect.width(), h = rect.height();
    rect.setLeft(rect.left() - m_frames.left());
    rect.setTop(rect.top()   - m_frames.top());
    rect.setWidth(w);
    rect.setHeight(h);
    return rect;
}

void CFramelessWindow::setCornerRadius(int radius)
{
    if (radius < 0) radius = 0;
    m_cornerRadius = radius;
    HWND hwnd = (HWND)this->winId();
    if (hwnd) {
        applyRoundedCorners();
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
    } else {
        QTimer::singleShot(0, this, [this]() { applyRoundedCorners(); });
    }
}

void CFramelessWindow::applyRoundedCorners()
{
    HWND hwnd = (HWND)this->winId();
    if (!hwnd) return;

    if (isMaximized() || isFullScreen()) {
        SetWindowRgn(hwnd, NULL, TRUE);
        DWMWCP_WINDOW_CORNER_PREFERENCE cp = DWMWCP_DONOTROUND_ENUM;
        DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &cp, sizeof(cp));
        return;
    }

    DWMWCP_WINDOW_CORNER_PREFERENCE cp = DWMWCP_DONOTROUND_ENUM;
    DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &cp, sizeof(cp));

    if (m_cornerRadius <= 0) {
        SetWindowRgn(hwnd, NULL, TRUE);
        return;
    }

    RECT rect;
    GetWindowRect(hwnd, &rect);
    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;
    HRGN hRgn = CreateRoundRectRgn(0, 0, w, h, m_cornerRadius * 2, m_cornerRadius * 2);
    if (hRgn) SetWindowRgn(hwnd, hRgn, TRUE);
}

void CFramelessWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (!isMaximized() && !isFullScreen())
        applyRoundedCorners();
}

void CFramelessWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    QTimer::singleShot(0, this, [this]() { applyRoundedCorners(); });
}

void CFramelessWindow::showFullScreen()
{
    if (isMaximized()) {
        QMainWindow::setContentsMargins(m_margins);
        m_frames = QMargins();
    }
    HWND hwnd = (HWND)this->winId();
    if (hwnd) {
        SetWindowRgn(hwnd, NULL, TRUE);
        DWMWCP_WINDOW_CORNER_PREFERENCE cp = DWMWCP_DONOTROUND_ENUM;
        DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &cp, sizeof(cp));
    }
    QMainWindow::showFullScreen();
}

#endif // Q_OS_WIN
