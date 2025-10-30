#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <gdiplus.h>
#include "Calculations.h"
#pragma comment (lib,"Gdiplus.lib") //tells linker to add gdiplus lib automatically

//Globals
//The main window class name.
static TCHAR szWindowClass[] = _T("NBodyApp");
//Title bar text
static TCHAR szTitle[] = _T("I'm a window!");
//Time between frame updates (ms), can be changed during runtime? 17 ms = ~60 fps
static const int frameTime = 17;
//Handles to windows we create on init.
HWND hLabel1, hLabel2, hLabel3, hEdit1, hEdit2, hEdit3, hButton;


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
	case WM_CREATE:
		hLabel1 = CreateWindowEx(0, L"STATIC", L"# of Planets", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
			10, 10, 50, 25, hWnd, (HMENU)1, nullptr, nullptr);
		hLabel2 = CreateWindowEx(0, L"STATIC", L"Fps (recommended 60)", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
			10, 45, 50, 25, hWnd, (HMENU)2, nullptr, nullptr);
		hLabel3 = CreateWindowEx(0, L"STATIC", L"Sim length (s)", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
			10, 80, 50, 25, hWnd, (HMENU)3, nullptr, nullptr);
		hEdit1 = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
			60, 10, 150, 25, hWnd, (HMENU)4, nullptr, nullptr);
		hEdit2 = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
			60, 45, 150, 25, hWnd, (HMENU)5, nullptr, nullptr);
		hEdit3 = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
			60, 80, 150, 25, hWnd, (HMENU)6, nullptr, nullptr);
		hButton = CreateWindow(L"BUTTON", L"Create", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
			10, 115, 100, 30, hWnd, (HMENU)7, nullptr, nullptr);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		OnPaint(hdc);
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_TIMER:

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case 4:
			// Destroy first screen
			DestroyWindow(hLabel1);
			DestroyWindow(hLabel2);
			DestroyWindow(hLabel3);
			DestroyWindow(hEdit1);
			DestroyWindow(hEdit2);
			DestroyWindow(hEdit3);
			DestroyWindow(hButton);
			break;
		default:
			//error
			MessageBox(hWnd, L"Button command not recognized", L"Error", MB_OK | MB_ICONERROR);
			break;
		}
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

