#include <windows.h>
#include <iostream>
#include <string>
#include <tchar.h>
#include <gdiplus.h>
#include "Calculations.h"
#pragma comment (lib,"Gdiplus.lib") //tells linker to add gdiplus lib automatically

//Globals
//The main window class name.
static TCHAR szWindowClass[] = _T("NBodyApp");
//Title bar text
static TCHAR szTitle[] = _T("N Body Simulation");
//Handles to windows we create on init.
HWND hLabel1, hLabel2, hLabel3, hEdit1, hEdit2, hEdit3, hButton, hErrorMsg;
//Handles to windows for planet initial value inputs will be created later dynamically and stored here.
std::vector<HWND> planetLabels;
std::vector<HWND> planetInputBoxes; //x-pos, y-pos, x-vel, y-vel, mass
//Old edit proc for subclassed edit control
std::vector<WNDPROC> oldProcs;
HWND hStartSimButton;
//number of planets
int numPlanets;
//Time between frame updates (ms), can be changed during runtime? 17 ms = ~60 fps
int frameTime;
//Sim length (ms)
float simLength;
//Window dimensions
static int windowLength;
static int windowHeight;
static int centerX; //used for initial screen && error msg positioning
//Current phase tracker
enum phaseTracker { CREATION, INITIALVALS, SIMULATION, PAUSED};
phaseTracker currPhase = CREATION;
//Stored instance handle for use in Win32 API calls
HINSTANCE hInst;
//Calculations class
Calculations* solveIVP = NULL;
//initial value storage
std::vector<PlanetInfo> initialVals;
//Simulation
std::vector<std::vector<std::pair<double, double>>> simulationResult;

//Forward declarations when funcs are called before definition
LRESULT CALLBACK ProcessMessages(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK CustomEditProc(HWND hEdit, UINT msg, WPARAM wParam, LPARAM lParam);
void CreateInitialWindows(HWND hWnd);
void OnPaint(HDC hdc);
bool DestroyIfValid();
bool IsValidInitialValues(HWND hWnd);
void CreatePlanetInitialValues(HWND hWnd);
void StartSimulation(HWND hWnd);

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
	windowLength = GetSystemMetrics(SM_CXFULLSCREEN);
	windowHeight = GetSystemMetrics(SM_CYFULLSCREEN);
	centerX = windowLength / 2 - 110; //110 is half of total width of initial boxes
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
	ShowWindow(hWnd, SW_MAXIMIZE);
	UpdateWindow(hWnd);

	HWND myconsole = GetConsoleWindow();
	//Get a handle to device context
	HDC mydc = GetDC(myconsole);

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
		CreateInitialWindows(hWnd);
		return 0;
	}
	case WM_ERASEBKGND: {
		//Redraw background in black every time InvalidateRect is called
		HDC hdcErase = (HDC)wParam;
		RECT rc;
		GetClientRect(hWnd, &rc);
		HBRUSH hBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
		FillRect(hdcErase, &rc, hBrush);
		return 1; //1 for success
	}
	case WM_PAINT:
		if (currPhase == SIMULATION) {
			hdc = BeginPaint(hWnd, &ps);
			OnPaint(hdc);
			EndPaint(hWnd, &ps);
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_TIMER:
		//Function that runs 60 fps: 17 ms is ~60 fps
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(); //calls for an immediate repaint
		//draw next frame
		return 0;
	case WM_COMMAND: {
		//The difference between the way I implemented the first two phase changes is unfortunate.
		if (LOWORD(wParam) == 7 && currPhase == CREATION) { //If Create button pressed (7 is HMENU ID)
			bool nextPhase = DestroyIfValid();
			if (nextPhase) {
				//increment phase
				currPhase = INITIALVALS;
				InvalidateRect(hWnd, NULL, TRUE);
				CreatePlanetInitialValues(hWnd);
			}
		}
		else if (LOWORD(wParam) == 500 && currPhase == INITIALVALS) {
			bool nextPhase = IsValidInitialValues(hWnd); //iterate through every edit ctrl and check if vals are valid
			if (nextPhase) {
				StartSimulation(hWnd);
			}
		}
		return 0;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
};

void OnPaint(HDC hdc)
{
}

LRESULT CALLBACK CustomEditProc(HWND hEdit, UINT msg, WPARAM wParam, LPARAM lParam) {
	int index = GetWindowLongPtr(hEdit, GWLP_USERDATA); // store index during creation

	switch (msg) {
	case WM_CHAR: {
		bool allowNeg = !(index % 5 == 0 || index % 5 == 1 || index % 5 == 4); //Negatives for velocity only
		bool allowDecimal = (index % 5 == 2 || index % 5 == 3 || index % 5 == 4); //Decimals for velocity and mass only

		if (isdigit((int)wParam) || wParam == VK_BACK)
			return CallWindowProc(oldProcs[index], hEdit, msg, wParam, lParam);

		if ((allowDecimal || oldProcs.size() < 5) && wParam == '.' && SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0) > 0)
			return CallWindowProc(oldProcs[index], hEdit, msg, wParam, lParam);

		if (allowNeg && wParam == '-' && SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0) == 0)
			return CallWindowProc(oldProcs[index], hEdit, msg, wParam, lParam);

		return 0; //Ignore character
	}
	case WM_DESTROY:
		SetWindowLongPtr(hEdit, GWLP_WNDPROC, (LONG_PTR)oldProcs[index]);
		break;
	}
	return CallWindowProc(oldProcs[index], hEdit, msg, wParam, lParam);
}

void CreateInitialWindows(HWND hWnd) {
	//Create ctrls. ORDER OF TEXT BOXES IS IMPORTANT HERE.
	hLabel1 = CreateWindowEx(0, L"STATIC", L"# of Planets", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_CENTER,
		centerX, 10, 160, 25, hWnd, (HMENU)1, NULL, NULL);
	hLabel2 = CreateWindowEx(0, L"STATIC", L"Fps (60 recommended)", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_CENTER,
		centerX, 45, 160, 25, hWnd, (HMENU)2, NULL, NULL);
	hLabel3 = CreateWindowEx(0, L"STATIC", L"Sim length (s)", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_CENTER,
		centerX, 80, 160, 25, hWnd, (HMENU)3, NULL, NULL);
	hEdit1 = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT,
		centerX + 160, 10, 60, 25, hWnd, (HMENU)4, NULL, NULL);
	hEdit2 = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT,
		centerX + 160, 45, 60, 25, hWnd, (HMENU)5, NULL, NULL);
	hEdit3 = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT,
		centerX + 160, 80, 60, 25, hWnd, (HMENU)6, NULL, NULL);
	hButton = CreateWindow(L"BUTTON", L"Create", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		centerX + 160, 115, 60, 30, hWnd, (HMENU)7, NULL, NULL);
	SetWindowLongPtr(hEdit2, GWLP_USERDATA, 0);
	WNDPROC oldProc = (WNDPROC)SetWindowLongPtr(hEdit2, GWLP_WNDPROC, (LONG_PTR)CustomEditProc);
	oldProcs.push_back(oldProc);
	SetWindowLongPtr(hEdit3, GWLP_USERDATA, 0);
	oldProc = (WNDPROC)SetWindowLongPtr(hEdit3, GWLP_WNDPROC, (LONG_PTR)CustomEditProc);
	oldProcs.push_back(oldProc);
}

bool SpecialCaseForDecimals(HWND textBox, int startY) {
	//Custom validation for sim length box to just see if it's all 0s
	int length = GetWindowTextLength(textBox);
	char szBuf[2048];
	LONG lResult;
	lResult = GetWindowTextA(textBox, szBuf, length + 1);
	bool allZeroes = true;
	for (int i = 0; i < length; i++) {
		if (szBuf[i] != '0' && szBuf[i] != '.' && szBuf[i] != '-') {
			allZeroes = false;
		}
	}
	if (length == 0) {
		if (hErrorMsg != NULL) {
			DestroyWindow(hErrorMsg);
		}
		hErrorMsg = CreateWindowEx(0, L"STATIC", L"You missed a box", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_CENTER,
			centerX, startY, 150, 25, GetParent(textBox), (HMENU)8, NULL, NULL);
		return false;
	}
	if (allZeroes) {
		if (hErrorMsg != NULL) {
			DestroyWindow(hErrorMsg);
		}
		hErrorMsg = CreateWindowEx(0, L"STATIC", L"Don't just put in zeroes asshole", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_CENTER,
			centerX, startY, 150, 35, GetParent(textBox), (HMENU)8, NULL, NULL);
		return false;
	}
	return !allZeroes;
}

bool IsValidNumberEntry(HWND textBox) {
	if (textBox == hEdit3 || textBox == hEdit2) {
		return SpecialCaseForDecimals(textBox, 115);
	}
	int length = GetWindowTextLength(textBox);
	char szBuf[2048];
	LONG lResult;
	lResult = GetWindowTextA(textBox, szBuf, length + 1);
	//don't want any negatives or decimals, so we can iterate through and check the string to make sure it's all digits
	bool allZeroes = true;
	for (int i = 0; i < length; i++) {
		if (!std::isdigit(szBuf[i])) {
			if (hErrorMsg != NULL) {
				DestroyWindow(hErrorMsg);
			}
			hErrorMsg = CreateWindowEx(0, L"STATIC", L"Invalid entry", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_CENTER,
				centerX, 115, 150, 25, GetParent(textBox), (HMENU)8, NULL, NULL);
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
		hErrorMsg = CreateWindowEx(0, L"STATIC", L"You missed a box", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_CENTER,
			centerX, 115, 150, 25, GetParent(textBox), (HMENU)8, NULL, NULL);
		return false;
	}
	//This is after length so it doesn't give the error for empty box
	if (allZeroes) {
		if (hErrorMsg != NULL) {
			DestroyWindow(hErrorMsg);
		}
		hErrorMsg = CreateWindowEx(0, L"STATIC", L"Don't just put in zeroes asshole", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_CENTER,
			centerX, 115, 150, 35, GetParent(textBox), (HMENU)8, NULL, NULL);
		return false;
	}
	if (textBox == hEdit1) {
		//number of planets special case
		int numPlanetsCheck = std::stoi(std::string(szBuf));
		if (numPlanetsCheck < 2 || numPlanetsCheck > 10) {
			if (hErrorMsg != NULL) {
				DestroyWindow(hErrorMsg);
			}
			hErrorMsg = CreateWindowEx(0, L"STATIC", L"# of planets must be 2-10", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_CENTER,
				centerX, 115, 150, 35, GetParent(textBox), (HMENU)8, NULL, NULL);
			return false;
		}
	}
	return true;

}

bool DestroyIfValid() {
	if (IsValidNumberEntry(hEdit1) && IsValidNumberEntry(hEdit2) && IsValidNumberEntry(hEdit3)) {
		//construct calculations class here
		char szBuf1[2048];
		char szBuf2[2048];
		char szBuf3[2048];
		int throwaway1 = GetWindowTextA(hEdit1, szBuf1, GetWindowTextLength(hEdit1) + 1);
		int throwaway2 = GetWindowTextA(hEdit2, szBuf2, GetWindowTextLength(hEdit2) + 1);
		int throwaway3 = GetWindowTextA(hEdit3, szBuf3, GetWindowTextLength(hEdit3) + 1);
		numPlanets = std::stoi(std::string(szBuf1));
		frameTime = 1000 / std::stof(std::string(szBuf2));
		simLength = 1000 * std::stof(std::string(szBuf3));
		solveIVP = new Calculations(
			numPlanets, frameTime, simLength
		);
		DestroyWindow(hLabel1);
		DestroyWindow(hLabel2);
		DestroyWindow(hLabel3);
		DestroyWindow(hEdit1);
		DestroyWindow(hEdit2);
		DestroyWindow(hEdit3);
		oldProcs.erase(oldProcs.begin());
		oldProcs.erase(oldProcs.begin());
		DestroyWindow(hButton);
		if (hErrorMsg != NULL) {
			DestroyWindow(hErrorMsg);
		}
		return true;
	}
	return false;
}

void CreatePlanetInitialValues(HWND hWnd) {
	//In case window was resized
	RECT clientRect;
	int clientWidth;
	int clientHeight;
	if (GetClientRect(hWnd, &clientRect)) {
		clientWidth = clientRect.right;
		clientHeight = clientRect.bottom;
	}

	//Labels and text boxes, 10 max planets so no need for fancy formatting
	planetLabels.resize(numPlanets*5);
	planetInputBoxes.resize(numPlanets*5);
	oldProcs.resize(numPlanets * 5);

	//Condition: window must be tall enough to fit one column of planet values + 30 for the button and wide enough to fit 10 columns.
	std::pair<int, int> startingPixel = { 0,0 };
	wchar_t buffer[256];
	swprintf(buffer, 256, L"Enter initial values for each planet below. Positions are in pixels where top left is {0,0}, width = %d px and height = %d px. Vel in m/s.", windowLength, windowHeight);
	hLabel1 = CreateWindowEx(0, L"STATIC", buffer, WS_CHILD | WS_VISIBLE | ES_CENTER,
		0, 0, 220, 80, hWnd, (HMENU)1, NULL, NULL);
	startingPixel.second += 80; //move down for the inputs

	for (int i = 0; i < numPlanets*5; i+=5) {
		std::wstring number = std::to_wstring(i/5 + 1);
		if (startingPixel.first == 0 && i == 5) { //if second planet, force to first column
			startingPixel = { 0, startingPixel.second };
		}
		else {
			startingPixel = (startingPixel.second + 205 < clientHeight) ? std::pair<int, int> {startingPixel.first, startingPixel.second}
			: std::pair<int, int>{ startingPixel.first + 220, 0 }; //205 = 35*5 + 30 pixels button, 220 = length of label + input box + 10
		}
		planetLabels[i] = CreateWindowEx(0, L"STATIC", (L"Planet " + number + L" x-position:").c_str(), WS_CHILD | WS_BORDER | WS_VISIBLE | ES_CENTER,
			startingPixel.first, startingPixel.second, 150, 35, hWnd, (HMENU)i, NULL, NULL);
		planetLabels[i + 1] = CreateWindowEx(0, L"STATIC", (L"Planet " + number + L" y-position:").c_str(), WS_CHILD | WS_BORDER | WS_VISIBLE | ES_CENTER,
			startingPixel.first, startingPixel.second + 35, 150, 35, hWnd, (HMENU)(i + 1), NULL, NULL);
		planetLabels[i + 2] = CreateWindowEx(0, L"STATIC", (L"Planet " + number + L" x-velocity:").c_str(), WS_CHILD | WS_BORDER | WS_VISIBLE | ES_CENTER,
			startingPixel.first, startingPixel.second + 70, 150, 35, hWnd, (HMENU)(i + 2), NULL, NULL);
		planetLabels[i + 3] = CreateWindowEx(0, L"STATIC", (L"Planet " + number + L" y-velocity:").c_str(), WS_CHILD | WS_BORDER | WS_VISIBLE | ES_CENTER,
			startingPixel.first, startingPixel.second + 105, 150, 35, hWnd, (HMENU)(i + 3), NULL, NULL);
		planetLabels[i + 4] = CreateWindowEx(0, L"STATIC", (L"Planet " + number + L" mass (in 10^24 kgs) (recommended 1-5 range):").c_str(), WS_CHILD | WS_BORDER | WS_VISIBLE | ES_CENTER,
			startingPixel.first, startingPixel.second + 140, 150, 35, hWnd, (HMENU)(i + 4), NULL, NULL);
		planetInputBoxes[i] = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT,
			startingPixel.first + 150, startingPixel.second, 60, 35, hWnd, (HMENU)(i + 5), NULL, NULL);
		planetInputBoxes[i + 1] = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT,
			startingPixel.first + 150, startingPixel.second + 35, 60, 35, hWnd, (HMENU)(i + 6), NULL, NULL);
		planetInputBoxes[i + 2] = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT,
			startingPixel.first + 150, startingPixel.second + 70, 60, 35, hWnd, (HMENU)(i + 7), NULL, NULL);
		planetInputBoxes[i + 3] = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT,
			startingPixel.first + 150, startingPixel.second + 105, 60, 35, hWnd, (HMENU)(i + 8), NULL, NULL);
		planetInputBoxes[i + 4] = CreateWindow(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT,
			startingPixel.first + 150, startingPixel.second + 140, 60, 35, hWnd, (HMENU)(i + 9), NULL, NULL);
		//subclass each edit box
		for (int j = 0; j < 5; j++) {
			SetWindowLongPtr(planetInputBoxes[i + j], GWLP_USERDATA, i + j);
			WNDPROC oldProc = (WNDPROC)SetWindowLongPtr(planetInputBoxes[i + j], GWLP_WNDPROC, (LONG_PTR)CustomEditProc);
			oldProcs[i + j] = oldProc;
		}
		startingPixel.second += 175; //move down for next planet.
	}
	hStartSimButton = CreateWindow(L"BUTTON", L"Start Simulation", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON | ES_CENTER,
		startingPixel.first, startingPixel.second + 5, 150, 25, hWnd, (HMENU)500, NULL, NULL);
}

bool IsValidInitialValues(HWND hWnd) {
	RECT simButtonRect;
	GetWindowRect(hStartSimButton, &simButtonRect);
	MapWindowPoints(HWND_DESKTOP, hWnd, (LPPOINT)&simButtonRect, 2);
	int startErrorX = simButtonRect.right;
	int startErrorY = simButtonRect.top;
	for (int i = 0; i < planetInputBoxes.size(); i++) {
		HWND textBox = planetInputBoxes[i];
		int length = GetWindowTextLength(textBox);
		char szBuf[2048];
		LONG lResult;
		lResult = GetWindowTextA(textBox, szBuf, length + 1);
		bool allZeroes = true;
		for (int i = 0; i < length; i++) {
			if (szBuf[i] != '0' && szBuf[i] != '.' && szBuf[i] != '-') {
				allZeroes = false;
			}
		}
		if (length == 0) {
			if (hErrorMsg != NULL) {
				DestroyWindow(hErrorMsg);
			}
			hErrorMsg = CreateWindowEx(0, L"STATIC", L"You missed a box", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_CENTER,
				startErrorX, startErrorY, 150, 25, GetParent(textBox), (HMENU)501, NULL, NULL);
			return false;
		}
		if (allZeroes) {
			if (hErrorMsg != NULL) {
				DestroyWindow(hErrorMsg);
			}
			hErrorMsg = CreateWindowEx(0, L"STATIC", L"Don't just put in zeroes asshole", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_CENTER,
				startErrorX, startErrorY, 150, 35, GetParent(textBox), (HMENU)501, NULL, NULL);
			return false;
		}
	}

	return true;
}

//oldProcs.clear(); when destroying the edit boxes later
void StartSimulation(HWND hWnd) {
	SetTimer(hWnd, 1, frameTime, (TIMERPROC)NULL);
	DestroyWindow(hLabel1);
	//store our intial values in initialVals vector then destroy the input boxes.
	//Can't resize without constructing each PlanetInfo, so assign default balances
	initialVals.assign(numPlanets, PlanetInfo(0, 0, 0.0f, 0.0f, 0.0f));
	for (int i = 0; i < planetInputBoxes.size(); i++) {
		int currInputIndex = i * 5;
		char szBufXPos[2048];
		char szBufYPos[2048];
		char szBufXVel[2048];
		char szBufYVel[2048];
		char szBufMass[2048];
		GetWindowTextA(planetInputBoxes[currInputIndex], szBufXPos, GetWindowTextLength(planetInputBoxes[currInputIndex]) + 1);
		GetWindowTextA(planetInputBoxes[currInputIndex + 1], szBufXPos, GetWindowTextLength(planetInputBoxes[currInputIndex + 1]) + 1);
		GetWindowTextA(planetInputBoxes[currInputIndex + 2], szBufXPos, GetWindowTextLength(planetInputBoxes[currInputIndex + 2]) + 1);
		GetWindowTextA(planetInputBoxes[currInputIndex + 3], szBufXPos, GetWindowTextLength(planetInputBoxes[currInputIndex + 3]) + 1);
		GetWindowTextA(planetInputBoxes[currInputIndex + 4], szBufXPos, GetWindowTextLength(planetInputBoxes[currInputIndex + 4]) + 1);
		int xPos = std::stoi(szBufXPos);
		int yPos = std::stoi(szBufXPos);
		float xVel = std::stof(szBufXPos);
		float yVel = std::stof(szBufXPos);
		float mass = std::stof(szBufXPos);
		initialVals[i] = PlanetInfo(xPos, yPos, xVel, yVel, mass);
		for (int j = 0; j < 5; j++) {
			DestroyWindow(planetInputBoxes[currInputIndex + j]);
			DestroyWindow(planetLabels[currInputIndex + j]);
		}
	}
	if (hErrorMsg != NULL) {
		DestroyWindow(hErrorMsg);
	}
	DestroyWindow(hStartSimButton);
	solveIVP->setInitialValues(initialVals);
	simulationResult = solveIVP->solve();
}

void EndSimulation(HWND hWnd) {
	currPhase = PAUSED;
	KillTimer(hWnd, 1);
}