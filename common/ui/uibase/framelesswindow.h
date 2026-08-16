#pragma once

#include "uibase_global.h"
#include "qsystemdetection.h"
#include <QObject>
#include <QMainWindow>

#ifdef Q_OS_WIN
#include <QWidget>
#include <QList>
#include <QMargins>
#include <QRect>

class UIBASE_EXPORT CFramelessWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit CFramelessWindow(QWidget *parent = nullptr);

    void setResizeable(bool resizeable = true);
    bool isResizeable() { return m_bResizeable; }
    void setResizeableAreaWidth(int width = 5);
    void setCornerRadius(int radius = 0);
    int cornerRadius() const { return m_cornerRadius; }

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void setTitleBar(QWidget *titlebar);
    void addIgnoreWidget(QWidget *widget);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
#else
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
#endif

private slots:
    void onTitleBarDestroyed();

public:
    void setContentsMargins(const QMargins &margins);
    void setContentsMargins(int left, int top, int right, int bottom);
    QMargins contentsMargins() const;
    QRect contentsRect() const;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void getContentsMargins(int *left, int *top, int *right, int *bottom) const;
#endif

public slots:
    void showFullScreen();

private:
    QWidget     *m_titlebar;
    QList<QWidget *> m_whiteList;
    int          m_borderWidth;
    QMargins     m_margins;
    QMargins     m_frames;
    bool         m_bJustMaximized;
    bool         m_bResizeable;
    int          m_cornerRadius;

    void applyRoundedCorners();
};

#elif defined Q_OS_MAC
#include <QMouseEvent>
#include <QResizeEvent>
#include <QPoint>

class UIBASE_EXPORT CFramelessWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit CFramelessWindow(QWidget *parent = nullptr);

private:
    void initUI();

public:
    void setDraggableAreaHeight(int height = 0);
    bool isNativeStyleOK() { return m_bNativeSystemBtn; }
    void setCloseBtnQuit(bool bQuit = true);
    void setCloseBtnEnabled(bool bEnable = true);
    void setMinBtnEnabled(bool bEnable = true);
    void setZoomBtnEnabled(bool bEnable = true);
    bool isCloseBtnEnabled() { return m_bIsCloseBtnEnabled; }
    bool isMinBtnEnabled()   { return m_bIsMinBtnEnabled;   }
    bool isZoomBtnEnabled()  { return m_bIsZoomBtnEnabled;  }

    // stubs to match Windows API
    void setResizeable(bool = true) {}
    void setCornerRadius(int = 0) {}
    void addIgnoreWidget(QWidget *) {}
    void setTitleBar(QWidget *) {}

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    int    m_draggableHeight;
    bool   m_bWinMoving;
    bool   m_bMousePressed;
    QPoint m_MousePos;
    QPoint m_WindowPos;
    bool   m_bCloseBtnQuit;
    bool   m_bNativeSystemBtn;
    bool   m_bIsCloseBtnEnabled, m_bIsMinBtnEnabled, m_bIsZoomBtnEnabled;
    bool   m_bTitleBarVisible;

    void setTitlebarVisible(bool bTitlebarVisible = false);
    bool isTitlebarVisible() { return m_bTitleBarVisible; }

private slots:
    void onRestoreFromFullScreen();

signals:
    void restoreFromFullScreen();

protected:
    void resizeEvent(QResizeEvent *event);
};
#endif
