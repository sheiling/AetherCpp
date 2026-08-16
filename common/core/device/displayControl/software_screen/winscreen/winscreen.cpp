#include "winscreen.h"
#include <thread>
#include <functional>

namespace common
{
namespace device
{

bool WinScreen::mouseInWnd_ = false;

bool WinScreen::Init()
{
    std::thread th = std::thread(std::bind(&WinScreen::initWindow, this));
    th.detach();
    return true;
}

void WinScreen::Release()
{
    if (hwnd_ && ::IsWindow(hwnd_))
    {
        ::DestroyWindow(hwnd_);
    }  
}

void WinScreen::Display(const SoftScreenParam &screenParams)
{
    screenParam_ = screenParams;
    // createWindow();
    ::SetWindowPos(hwnd_, HWND_TOPMOST, 0, 0, screenParam_.width, screenParam_.height, SWP_FRAMECHANGED);
    painted_ = true;
    RECT rect;
    SetRect(&rect, 0, 0, screenParam_.width, screenParam_.height);
    HRGN rgn = CreateRectRgnIndirect(&rect);
    RedrawWindow(hwnd_, &rect, rgn, RDW_INVALIDATE | RDW_NOERASE | RDW_UPDATENOW);
    ::ShowWindow(hwnd_, SW_SHOW);
    DeleteObject(rgn);
}


void WinScreen::initWindow()
{
    // 1.窗口注册
    registerWindow();
    // 2.创建窗口
    createWindow();
    // 3.消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

ATOM WinScreen::registerWindow()
{
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WinScreen::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hIns_;
    wcex.hIcon = NULL;
    wcex.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "CusWnd";
    wcex.hIconSm = NULL;
    return RegisterClassEx(&wcex);
}

bool WinScreen::createWindow()
{
    if (!hwnd_)
    {
        hwnd_ = CreateWindowEx(WS_EX_TOPMOST, "CusWnd", "display", WS_BORDER,0, 0, screenParam_.width, screenParam_.height, NULL, NULL, hIns_, (LPVOID)this);
        //hwnd_ = ::CreateWindow("CusWnd", "display", WS_BORDER,0, 0, screenParam_.width, screenParam_.height, NULL, NULL, hIns_, (LPVOID)this);
        if (!hwnd_)
        {
            ::MessageBox(NULL, "create windows failed!", "错误", MB_OK);
            return false;
        }
        Display(screenParam_);
        return true;
        ::SetWindowPos(hwnd_, HWND_TOPMOST, 0, 0, screenParam_.width, screenParam_.height, SWP_FRAMECHANGED);
        ::ShowWindow(hwnd_, SW_SHOW);
        ::UpdateWindow(hwnd_);
    }
    else
    {
        hwnd_ = CreateWindowEx(WS_EX_TOPMOST, "CusWnd", "display", WS_BORDER,0, 0, screenParam_.width, screenParam_.height, NULL, NULL, hIns_, (LPVOID)this);
        //hwnd_ = ::CreateWindow("CusWnd", "display", WS_BORDER,0, 0, screenParam_.width, screenParam_.height, NULL, NULL, hIns_, (LPVOID)this);
        if (!hwnd_)
        {
            ::MessageBox(NULL, "create windows failed!", "错误", MB_OK);
            return false;
        }
        Display(screenParam_);
        return true;
        ::SetWindowPos(hwnd_, HWND_TOPMOST, 0, 0, screenParam_.width, screenParam_.height, SWP_FRAMECHANGED);
        ::ShowWindow(hwnd_, SW_SHOW);
        ::UpdateWindow(hwnd_);
    }
    return true;
}

void WinScreen::draw(HDC hdc)
{
    switch (screenParam_.mode)
    {
    case SoftScreenMode::FULL_MODE:
        drawPureColor(hdc);
        break;
    case SoftScreenMode::ODD_MODE:
        drawChessBoard(hdc);
        break;
    case SoftScreenMode::EVEN_MODE:
        drawChessBoard(hdc);
        break;
    default:
        break;
    }
}

void WinScreen::drawPureColor(HDC hdc)
{
    HBRUSH hBrush;
    RECT rect;
    SetRect(&rect, 0, 0, screenParam_.width, screenParam_.height);
	int brightness = screenParam_.brightness;

	switch (screenParam_.color)
	{
	case SoftScreenColor::COLOR_RED:
		hBrush = CreateSolidBrush(RGB(brightness, 0, 0));
		break;
	case SoftScreenColor::COLOR_GREEN:
		hBrush = CreateSolidBrush(RGB(0, brightness, 0));
		break;
	case SoftScreenColor::COLOR_BLUE:
		hBrush = CreateSolidBrush(RGB(0, 0, brightness));
		break;
    case SoftScreenColor::COLOR_BLACK:
		hBrush = CreateSolidBrush(RGB(0, 0, 0));
		break;
	case SoftScreenColor::COLOR_WHITE:
		hBrush = CreateSolidBrush(RGB(brightness, brightness, brightness));
		break;
	default:
		hBrush = CreateSolidBrush(RGB(0, 0, 0));
		break;
	}

    FillRect(hdc, &rect, hBrush);
    DeleteObject(hBrush);
    hBrush = NULL;
}

void WinScreen::drawChessBoard(HDC hdc)
{
    // 1.填充背景
    HBRUSH hBrush;
    RECT rect;
    SetRect(&rect, 0, 0, screenParam_.width, screenParam_.height);
    hBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, &rect, hBrush);
    DeleteObject(hBrush);
    hBrush = NULL;

    // 2.创建画笔
    HPEN hPen = nullptr;
    int brightness = screenParam_.brightness;
	switch (screenParam_.color)
	{
	case SoftScreenColor::COLOR_RED:
		hPen = CreatePen(PS_SOLID, 1, RGB(brightness, 0, 0));
		break;
	case SoftScreenColor::COLOR_GREEN:
		hPen = CreatePen(PS_SOLID, 1, RGB(0, brightness, 0));
		break;
	case SoftScreenColor::COLOR_BLUE:
		hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, brightness));
		break;
	default:
		hPen = CreatePen(PS_SOLID, 1, RGB(brightness, brightness, brightness));
		break;
	}     

    // 3.画笔与设备绑定
    SelectObject(hdc, hPen);   

    // 4.棋盘格绘制
    switch (screenParam_.mode)
    {
    case SoftScreenMode::ODD_MODE:
        drawOddBoard(hdc);
        break;
    case SoftScreenMode::EVEN_MODE:
        drawEvenBoard(hdc);
        break;
    default:
        break;
    }

    DeleteObject(hPen);
}

void WinScreen::drawOddBoard(HDC hdc)
{
    int intervalX = screenParam_.interval <= 0 ? 1 : screenParam_.interval;
    POINT pt1, pt2;
    // 绘制左上半区
    for (int i = 0; i < screenParam_.width; i++)
    {
        if (i % intervalX == 0)
        {
            pt1.x = i;
            pt1.y = 0;
            pt2.x = screenParam_.width;
            pt2.y = screenParam_.width - i;
            MoveToEx(hdc, pt1.x, pt1.y, NULL);
            LineTo(hdc, pt2.x, pt2.y);
        }
    }

    // 绘制右下半区
    for (int i = 1; i < screenParam_.height; i++)
    {
        if (i % intervalX == 0)
        {
            pt1.x = 0;
            pt1.y = i;
            pt2.x = screenParam_.height - i;
            pt2.y = screenParam_.height;
            MoveToEx(hdc, pt1.x, pt1.y, NULL);
            LineTo(hdc, pt2.x, pt2.y);
        }
    }
}

void WinScreen::drawEvenBoard(HDC hdc)
{
    int intervalX = screenParam_.interval <= 0 ? 1 : screenParam_.interval;
    POINT pt1, pt2;
    // 绘制左上半区
    for (int i = 0; i < screenParam_.width; i++)
    {
        if (i % intervalX == 1)
        {
            pt1.x = i;
            pt1.y = 0;
            pt2.x = screenParam_.width;
            pt2.y = screenParam_.width - i;
            MoveToEx(hdc, pt1.x, pt1.y, NULL);
            LineTo(hdc, pt2.x, pt2.y);
        }
    }
    // 绘制右下半区
    for (int i = 1; i < screenParam_.height; i++)
    {
        if (i % intervalX == 1)
        {
            pt1.x = 0;
            pt1.y = i;
            pt2.x = screenParam_.height - i;
            pt2.y = screenParam_.height;
            MoveToEx(hdc, pt1.x, pt1.y, NULL);
            LineTo(hdc, pt2.x, pt2.y);
        }
    }
}

LRESULT WinScreen::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    ShowCursor(false);
    switch (msg)
    {
    case WM_CREATE:
        onCreate(hwnd, lparam);
        break;
    case WM_DESTROY:
        onDestroy(hwnd);
        break;
    case WM_CONTEXTMENU:
        onContextMenu(hwnd, lparam);
        break;
    case WM_PAINT:
        onPaint(hwnd);
        break;
	case WM_ERASEBKGND:
		break;
    case WM_COMMAND:
        onCommand(hwnd, wparam);
        break;
    case WM_WINDOWPOSCHANGING:
        ::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        break;
    case WM_ACTIVATE:
        if (LOWORD(wparam) != WA_INACTIVE) {
            ::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }
        break;
    default:
        return ::DefWindowProc(hwnd, msg, wparam, lparam);
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

void WinScreen::onCreate(HWND hwnd, LPARAM lparam)
{
    WinScreen *winScreen = (WinScreen*)(((LPCREATESTRUCT)lparam)->lpCreateParams);
    winScreen->hwnd_ = hwnd;
    LONG styleValue = ::GetWindowLong(hwnd, GWL_STYLE);
    styleValue &= ~WS_CAPTION;
    ::SetWindowLongPtr(hwnd, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

    RECT rect;
    ::GetClientRect(hwnd, &rect);
    ::SetWindowPos(hwnd, HWND_TOPMOST, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_FRAMECHANGED);
    ::SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(winScreen));
}

void WinScreen::onContextMenu(HWND hwnd, LPARAM lparam)
{
    HMENU hpopup = ::CreatePopupMenu();
    ::AppendMenu(hpopup, MF_STRING, 1001, "Hide"); // 隐藏打屏
    ::TrackPopupMenu(hpopup, TPM_LEFTALIGN | TPM_TOPALIGN, LOWORD(lparam), HIWORD(lparam), 0, hwnd, NULL);
}

void WinScreen::onDestroy(HWND hwnd)
{
    WinScreen *winScreen = reinterpret_cast<WinScreen*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (!winScreen)
    {
        PostQuitMessage(0);
        winScreen->hwnd_ = nullptr;
    }
}

void WinScreen::onPaint(HWND hwnd)
{
    WinScreen *winScreen = reinterpret_cast<WinScreen*>(::GetWindowLongPtr(hwnd, GWLP_USERDATA));

    PAINTSTRUCT ps;
    HDC hdc;
    hdc = ::BeginPaint(hwnd, &ps);
    /* 使用双缓存解决刷新闪烁 */
    if (winScreen && winScreen->painted_)
    {
        HDC hdc2 = CreateCompatibleDC(hdc);
        HBITMAP bitMap = CreateCompatibleBitmap(hdc, winScreen->screenParam_.width, winScreen->screenParam_.height);
        SelectObject(hdc2, bitMap);
        winScreen->draw(hdc2);
        BitBlt(hdc, 0, 0, winScreen->screenParam_.width, winScreen->screenParam_.height, hdc2, 0, 0, SRCCOPY);
        DeleteDC(hdc2);
        DeleteBitmap(bitMap);
    }
    /* --- */
    ::EndPaint(hwnd, &ps);
}

void WinScreen::onCommand(HWND hwnd, WPARAM wparam)
{
    switch (LOWORD(wparam))
    {
    case 1001:
        ::ShowWindow(hwnd, SW_HIDE);
        break;
    default:
        break;
    }
}

}
}
