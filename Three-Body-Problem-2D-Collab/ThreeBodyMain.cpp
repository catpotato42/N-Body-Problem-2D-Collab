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
HWND hLabel1, hLabel2, hLabel3, hEdit1, hEdit2, hEdit3, hButton, hErrorMsg;
//Window dimensions
static const int windowLength = 1000;
static const int windowHeight = 500;
static const int centerX = (windowLength / 2) - 110; //used for initial screen

//Stored instance handle for use in Win32 API calls
HINSTANCE hInst;

//Forward declarations when funcs are called before definition
LRESULT CALLBACK ProcessMessages(HWND, UINT, WPARAM, LPARAM);
void OnPaint(HDC hdc);
void DestroyIfValid();

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
		windowLength, windowHeight, //initial size (width, height)
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
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	Gdiplus::GdiplusShutdown(gdiplusToken);
	return (int)msg.wParam; //int
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
	{
		//Create ctrls
		hLabel1 = CreateWindowEx(0, L"STATIC", L"# of Planets", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_CENTER,
			centerX, 10, 160, 25, hWnd, (HMENU)1, nullptr, nullptr);
		hLabel2 = CreateWindowEx(0, L"STATIC", L"Fps (60 recommended)", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_CENTER,
			centerX, 45, 160, 25, hWnd, (HMENU)2, nullptr, nullptr);
		hLabel3 = CreateWindowEx(0, L"STATIC", L"Sim length (s)", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_CENTER,
			centerX, 80, 160, 25, hWnd, (HMENU)3, nullptr, nullptr);
		hEdit1 = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT,
			centerX + 160, 10, 60, 25, hWnd, (HMENU)4, nullptr, nullptr);
		hEdit2 = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT,
			centerX + 160, 45, 60, 25, hWnd, (HMENU)5, nullptr, nullptr);
		hEdit3 = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT,
			centerX + 160, 80, 60, 25, hWnd, (HMENU)6, nullptr, nullptr);
		hButton = CreateWindow(L"BUTTON", L"Create", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
			centerX + 160, 115, 60, 30, hWnd, (HMENU)7, nullptr, nullptr);
		break;
	}
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		OnPaint(hdc);
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_TIMER:
		//Function that runs 60 fps: 17 ms is ~60 fps
	case WM_COMMAND:
		if (LOWORD(wParam) == 7) { //If button pressed (7 is HMENU ID)
			DestroyIfValid();
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
};

void OnPaint(HDC hdc)
{
}

bool IsValidNumberEntry(HWND textBox) {
	int length = GetWindowTextLength(textBox);
	char szBuf[2048];
	LONG lResult;
	lResult = GetWindowTextA(textBox, szBuf, length + 1);
	//don't want any negatives or zeroes, so we can iterate through and check the string
	bool allZeroes = true;
	for (int i = 0; i < length; i++) {
		if (!std::isdigit(szBuf[i])) {
			if (hErrorMsg != NULL) {
				DestroyWindow(hErrorMsg);
			}
			hErrorMsg = CreateWindowEx(0, L"STATIC", L"Invalid entry", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_CENTER,
				centerX, 115, 150, 35, GetParent(textBox), (HMENU)1, nullptr, nullptr);
			return false;
		}
		if (szBuf[i] != '0') {
			allZeroes = false;
		}
	}
	if (length == 0) {
		if (hErrorMsg != NULL) {
			DestroyWindow(hErrorMsg);
		}
		hErrorMsg = CreateWindowEx(0, L"STATIC", L"You missed a box", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_CENTER,
			centerX, 115, 150, 35, GetParent(textBox), (HMENU)1, nullptr, nullptr);
		return false;
	}
	//This is after length so it doesn't give the error for empty box
	if (allZeroes) {
		if (hErrorMsg != NULL) {
			DestroyWindow(hErrorMsg);
		}
		hErrorMsg = CreateWindowEx(0, L"STATIC", L"Don't just put in zeroes asshole", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_CENTER,
			centerX, 115, 150, 35, GetParent(textBox), (HMENU)1, nullptr, nullptr);
		return false;
	}
	else {
		return true;
	}
}

void DestroyIfValid() {
	if (IsValidNumberEntry(hEdit1) && IsValidNumberEntry(hEdit2) && IsValidNumberEntry(hEdit3)) {
		DestroyWindow(hLabel1);
		DestroyWindow(hLabel2);
		DestroyWindow(hLabel3);
		DestroyWindow(hEdit1);
		DestroyWindow(hEdit2);
		DestroyWindow(hEdit3);
		DestroyWindow(hButton);
		if (hErrorMsg != NULL) {
			DestroyWindow(hErrorMsg);
		}
	}
}