#ifndef _WINSCRREN_HH
#define _WINSCRREN_HH

#include "../software_screen_base.h"
#include "../software_screen_global.h"
#include <stdint.h>
#include <string>
#include <Windows.h>
#include <windowsx.h>

namespace common
{
namespace device
{

class SOFTWARE_SCREEN_EXPORT WinScreen : public SoftwareScreenBase {
public:
    WinScreen() = default;
    ~WinScreen() = default;
public:
	bool Init() override;
	void Release() override;
	void Display(const SoftScreenParam &screenParams) override;
private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
	static void onCreate(HWND hwnd, LPARAM lparam);
	static void onContextMenu(HWND hwnd, LPARAM lparam);
	static void onDestroy(HWND hwnd);
	static void onPaint(HWND hwnd);
	static void onCommand(HWND hwnd, WPARAM wparam);
private:
	void initWindow();
	ATOM registerWindow();
    bool createWindow();
	void draw(HDC hdc);
	void drawPureColor(HDC hdc);
	void drawChessBoard(HDC hdc);
	void drawOddBoard(HDC hdc);
	void drawEvenBoard(HDC hdc);
private:
    HWND hwnd_ = nullptr;
    HINSTANCE hIns_ = nullptr;
    bool painted_ = false;
	SoftScreenParam screenParam_;
	static bool mouseInWnd_;
};

} // end of namespace device
} // end of namespace common

#endif
