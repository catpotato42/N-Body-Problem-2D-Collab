#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib") //tells linker to add gdiplus lib automatically

//Globals
//The main window class name.
static TCHAR szWindowClass[] = _T("NBodyApp");
//Title bar text
static TCHAR szTitle[] = _T("I'm a window!");
//Time between frame updates (ms), can be changed during runtime? 17 ms = ~60 fps
static const int frameTime = 17;

//Stored instance handle for use in Win32 API calls
HINSTANCE hInst;

//Forward declarations
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

	if (!RegisterClassEx(&wcex)) {
		return false;
	}

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

	SetTimer(hWnd, 0, // Handle to window, ID
		frameTime,                 // 1000/fps timer interval
		(TIMERPROC)NULL);     // no timer callback 


	MSG msg;
	//main loop
	while (GetMessage(&msg, NULL, 0, 0))
	{
		//Function that runs 60 fps: 17 ms is ~60 fps
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
	case WM_TIMER:

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

