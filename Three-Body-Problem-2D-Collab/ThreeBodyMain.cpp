#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")

//Globals
// The main window class name.
static TCHAR szWindowClass[] = _T("NBodyApp");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("I'm a window!");

// Stored instance handle for use in Win32 API calls such as FindResource
HINSTANCE hInst;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK ProcessMessages(HWND, UINT, WPARAM, LPARAM);
VOID OnPaint(HDC hdc);

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
) {
	WNDCLASSEX wcex;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = ProcessMessages;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	RegisterClassEx(&wcex);
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	HWND hWnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW, //style
		szWindowClass, //name of application
		szTitle, //text in title bar
		WS_OVERLAPPEDWINDOW, //type of window to create
		CW_USEDEFAULT, CW_USEDEFAULT, //initial position (x, y)
		1000, 500, //initial size (width, height)
		NULL, //parent window
		NULL, //menu
		hInstance, //instance handle
		NULL //creation parameters
	);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	HWND myconsole = GetConsoleWindow();
	//Get a handle to device context
	HDC mydc = GetDC(myconsole);


	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	Gdiplus::GdiplusShutdown(gdiplusToken);
	return msg.wParam; //int
};

LRESULT CALLBACK ProcessMessages(
	_In_ HWND   hWnd,
	_In_ UINT   message,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
) {
	HDC          hdc;
	PAINTSTRUCT  ps;
	std::pair<int, int> center = { 500,250 };
	int radius = 100;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		OnPaint(hdc);
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
};

VOID OnPaint(HDC hdc)
{
	Gdiplus::Graphics graphics(hdc);
	Gdiplus::Pen      pen(Gdiplus::Color(255, 0, 0, 255));
	graphics.DrawLine(&pen, 0, 0, 200, 100);
}