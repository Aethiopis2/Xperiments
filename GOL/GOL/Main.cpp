/**
 * Breif:
 *	Program to paly/simulate Conway's Game of Life. The program takes place on a rectangular
 *	grid of size 20x20 unlike the real infinte mathematical concept of the game.
 *
 * Program Author:
 *	Rediet Worku aka Aethiops II ben Zahab
 *
 * Date Created:
 *	19th of Sepetemeber 2024, Thursday
 */
#define WIN32_LEAN_AND_MEAN


 //==========================================================================================|
 //			INCLUDES
 //==========================================================================================|
#include "life.h"






//==========================================================================================|
//			MACROS
//==========================================================================================|
#define DUMP_ERR(h, b, m)	{ _snwprintf_s(b, 256, L"%s fail with error code: %d", m, GetLastError()); \
	MessageBox(h, b, L"Error Box", MB_ICONERROR); }




//==========================================================================================|
//			GLOBALS
//==========================================================================================|
// Window properties
const wchar_t CLASS_NAME[]{ L"CellClass" };
const wchar_t WINDOW_NAME[]{ L"Conway's Game of Life: The Simulation" };
const int WINDOW_WIDTH{ 400 };			// each cell is 20x20 pixels
const int WINDOW_HEIGHT{ 400 };



wchar_t buffer[256];		// generalized buffer

HBRUSH cellColor, cellBorder;



//==========================================================================================|
//			FUNCTIONS
//==========================================================================================|
LRESULT CALLBACK mainWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_CREATE:
		cellColor = CreateSolidBrush(RGB(100, 255, 0));
		cellBorder = CreateSolidBrush(RGB(255, 255, 0));
		return 0;
		break;

	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc;
		hdc = BeginPaint(hwnd, &ps);

		EndPaint(hwnd, &ps);

		return 0;
		break;

	default:
		break;
	} // end switch

	return DefWindowProc(hwnd, msg, wparam, lparam);
} // end mainWindowProc




//==========================================================================================|
int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hprev, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX win;
	MSG msg;
	HWND hwnd;
	Life configuration;			// the life object that simulates the game
	RECT rc;					// get's the window dimensions

	ZeroMemory(&win, sizeof(win));
	win.cbSize = sizeof(win);
	win.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	win.hCursor = LoadCursor(NULL, IDC_ARROW);
	win.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	win.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	win.hInstance = hinst;
	win.lpfnWndProc = mainWindowProc;
	win.lpszClassName = CLASS_NAME;
	win.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&win))
	{
		DUMP_ERR(NULL, buffer, L"RegisterClassEx()");
		return 0;
	} // end if error


	if (!(hwnd = CreateWindowEx(0, CLASS_NAME, WINDOW_NAME, WS_CAPTION | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
		NULL, NULL, hinst, NULL)))
	{
		DUMP_ERR(NULL, buffer, L"CreateWindowEx()");
		return 0;
	} // end if window create

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	// initalize life before the main loop
	configuration.initialize();
	HDC hdc = GetDC(hwnd);
	GetClientRect(hwnd, &rc);

	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} // end if PeekMessage

		// update to the next configuration
		FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));

		configuration.draw(hdc, cellColor, cellBorder);
		configuration.update();


		Sleep(500);		// every 1/2  a second or so...
	} // end main-event loop while

	ReleaseDC(hwnd, hdc);
	return (int)msg.wParam;
} // end WinMain