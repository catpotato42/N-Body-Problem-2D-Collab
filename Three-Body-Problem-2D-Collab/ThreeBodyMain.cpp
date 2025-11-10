#include <windows.h>
#include <iostream>
#include <string>
#include <tchar.h>
#include <gdiplus.h>
#include <memory>
#include "Calculations.h"
#pragma comment (lib,"Gdiplus.lib") //tells linker to add gdiplus lib automatically

//Globals
//The main window class name.
static TCHAR szWindowClass[] = _T("NBodyApp");
//Title bar text
static TCHAR szTitle[] = _T("N Body Simulation");
//Handles to windows we create on init.
HWND hLabel1, hLabel2, hLabel3, hLabel4, hEdit1, hEdit2, hEdit3, hEdit4, hButton, hErrorMsg;
//Handles to windows for planet initial value inputs will be created later dynamically and stored here.
std::vector<HWND> planetLabels;
std::vector<HWND> planetInputBoxes; //x-pos, y-pos, x-vel, y-vel, mass
//Old edit proc for subclassed edit control. This is resized after the initial screen to numPlanets * 5
//and is never reset back down except after the initial screen on subsequent runs.
std::vector<WNDPROC> oldProcs;
HWND hStartSimButton;
//Restart
HWND hRestartSimButton;
//Change initial values
HWND hChangeInitialVals;
//Frame tracker in top left
HWND hFrameTracker;
//Unused, old implementation is commented out in WM_COMMAND under the currPhase == INITIALVALS statement,
//commenting back in or out shouldm't lead to issues as all usage is guarded currently. This is for debugging primarily.
HWND hPositionDisplay;
//conversion from meters to pixels, i.e 1 pixel = 1e6 meters
const double metersPerPixel = 1e6;
//number of planets
int numPlanets;
//vector of radius calculated by mass inputted
std::vector<int> radiusPlanets;
//length of trail behind planets
const int trailLength = 50;
//The frames per simulation second. More -> more accuracy but doesn't change the time in simulation seconds when a frame is outputted. Full explanation on the README.
float fpss;
//Sim length (seconds simulation time)
float simLength;
//Relative speed, more means faster sim. A full explanation is provided in the README.
float relativeSpeed;
//total steps outputted by our solve function (given in StartSimulation)
int totalSteps;
int currStep = 0;
//Window dimensions
int windowLength;
int windowHeight;
int centerX; //used for initial screen && error msg positioning
int centerY; //used for testing
//Current phase tracker, used for a number of functions to validate that we're on the right screen.
enum phaseTracker { CREATION, INITIALVALS, SIMULATION, PAUSED};
phaseTracker currPhase = CREATION;
//Stored instance handle for use in Win32 API calls
HINSTANCE hInst;
//Calculations class
Calculations* solveIVP = NULL;
//initial value storage
std::vector<PlanetInfo> initialVals;
//Simulation
std::vector<std::vector<std::pair<float, float>>> simulationResult;
std::vector<std::unique_ptr<Gdiplus::SolidBrush>> planetBrushes;
std::vector<std::unique_ptr<Gdiplus::Pen>> planetPens;
std::unique_ptr<Gdiplus::Bitmap> g_backBuffer;
std::unique_ptr<Gdiplus::Graphics> g_backG;
int g_backWidth = 0;
int g_backHeight = 0;

//Forward declarations when funcs are called before definition
LRESULT CALLBACK ProcessMessages(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK CustomEditProc(HWND hEdit, UINT msg, WPARAM wParam, LPARAM lParam);
void CreateInitialWindows(HWND hWnd);
void OnPaint(HDC hdc);
bool DestroyIfValid();
bool IsValidInitialValues(HWND hWnd);
void CreatePlanetInitialValues(HWND hWnd);
void ReCreatePlanetInitialValues(HWND hWnd);
void StartSimulation(HWND hWnd);
void EndSimulation(HWND hWnd);

//stole this buffer from online somewhere
void CreateBackBuffer(int width, int height)
{
	if (width <= 0 || height <= 0) return;
	if (g_backBuffer && g_backWidth == width && g_backHeight == height) return;

	g_backBuffer.reset(new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB));
	g_backG.reset(Gdiplus::Graphics::FromImage(g_backBuffer.get()));
	g_backG->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
	g_backG->SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed);
	g_backG->SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);

	g_backWidth = width;
	g_backHeight = height;
}

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
) {
	//All standard, uses gdiplus.
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
	//registers our window
	if (!RegisterClassEx(&wcex)) {
		return false;
	}

	//this all depends on the window being created as full screen. Again, centerX and centerY are used only on the first screen (CREATION).
	//HOWEVER, windowLength and windowHeight are used in CreatePlanetInitialValues to let the user know how big their screen is.
	//All of this means right now you shouldn't resize your screen as it's gonna f up that output.
	windowLength = GetSystemMetrics(SM_CXFULLSCREEN);
	windowHeight = GetSystemMetrics(SM_CYFULLSCREEN);
	centerX = windowLength / 2;
	centerY = windowHeight / 2;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	HWND hWnd = CreateWindowEx(
		//these comments are for us as we haven't used Win32 API before
		WS_EX_OVERLAPPEDWINDOW, //style
		szWindowClass, //name of application
		szTitle, //text in title bar
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, //type of window to create
		CW_USEDEFAULT, CW_USEDEFAULT, //initial position (x, y)
		1000, 500, //initial size (width, height)
		NULL, //parent window
		NULL, //menu
		hInstance, //instance handle
		NULL //creation parameters
	);
	ShowWindow(hWnd, SW_MAXIMIZE);
	UpdateWindow(hWnd);
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
		//Redraw background in black every time InvalidateRect(..., ..., TRUE) is called.
		//The if is an extra guard to make sure we don't slow down our simulation.
		if (currPhase != SIMULATION) {
			HDC hdcErase = (HDC)wParam;
			RECT rc;
			GetClientRect(hWnd, &rc);
			HBRUSH hBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
			FillRect(hdcErase, &rc, hBrush);
		}
		return 1; //1 for success i guess
	}
	case WM_PAINT:
		//pretty standard
		hdc = BeginPaint(hWnd, &ps);
		OnPaint(hdc);
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_TIMER:
		//Function that runs __ fps: 1000/fps = ms between frames. Timer set at the end of StartSimulation.
		currStep++;
		if (currStep < totalSteps) {
			wchar_t buf[128];
			swprintf(buf, _countof(buf), L"Current Frame: %d / %d", currStep, totalSteps);
			SetWindowTextW(hFrameTracker, buf);
			if (simulationResult.size() >= 2 &&
				currStep < (int)simulationResult[0].size() &&
				currStep < (int)simulationResult[1].size() && 
				hPositionDisplay != NULL)
			{
				swprintf(buf, _countof(buf), L"Current pos: {%d, %d}, {%d, %d}",
					(int)simulationResult[0][currStep].first, (int)simulationResult[0][currStep].second,
					(int)simulationResult[1][currStep].first, (int)simulationResult[1][currStep].second);
				SetWindowTextW(hPositionDisplay, buf);
			}
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else {
			currPhase = PAUSED;
			EndSimulation(hWnd);
		}
		//draw next frame
		return 0;
	case WM_COMMAND: {
		//The difference between the way I implemented the first two phase changes is unfortunate.
		if (LOWORD(wParam) == 8 && currPhase == CREATION) { //If Create button pressed (7 is HMENU ID)
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
				hFrameTracker = CreateWindowEx(0, L"STATIC", L"Current Frame: " + (char)currStep, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_CENTER,
					0, 0, 170, 25, hWnd, (HMENU)1, NULL, NULL);
				//hPositionDisplay = CreateWindowEx(0, L"STATIC", L"Current pos: ", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_CENTER,
				//	0, 35, 200, 70, hWnd, (HMENU)1, NULL, NULL);
				currPhase = SIMULATION; //after bc can't draw w/out knowing output values
			}
		}
		else if (LOWORD(wParam) == 1000 && currPhase == PAUSED) {
			DestroyWindow(hRestartSimButton);
			DestroyWindow(hChangeInitialVals);
			currPhase = CREATION;
			InvalidateRect(hWnd, NULL, TRUE);
			CreateInitialWindows(hWnd);
		}
		else if (LOWORD(wParam) == 1001 && currPhase == PAUSED) {
			DestroyWindow(hRestartSimButton);
			DestroyWindow(hChangeInitialVals);
			currPhase = INITIALVALS;
			InvalidateRect(hWnd, NULL, TRUE);
			ReCreatePlanetInitialValues(hWnd);
		}
		return 0;
	case WM_SIZE:
		int w = LOWORD(lParam);
		int h = HIWORD(lParam);
		CreateBackBuffer(w, h);
	return 0;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
};

void OnPaint(HDC hdc)
{
	RECT rc;
	GetClientRect(WindowFromDC(hdc), &rc);
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;
	CreateBackBuffer(w, h);
	if (!g_backG || !g_backBuffer) return;

	g_backG->Clear(Gdiplus::Color::Black);

	if (currPhase == SIMULATION) {
		int radius;
		float cx, cy;
		for (int i = 0; i < numPlanets; i++) {

			int start = (currStep >= trailLength) ? currStep - trailLength : 1; //if currStep is smaller, start at step 1
			int count = currStep - start; //# of pts to draw
			//draw a line from the current step position to the next step position. malloca instead of alloca bc vs insisted, but shouldn't fall back most likely
			if (count > 1) {
				Gdiplus::PointF* pts = (Gdiplus::PointF*)_malloca(sizeof(Gdiplus::PointF) * count);
				for (int j = start, k = 0; j < currStep; j++, k++) {
					pts[k].X = simulationResult[i][j].first;
					pts[k].Y = simulationResult[i][j].second;
				}
				if (count > 1) {
					g_backG->DrawLines(planetPens[i].get(), pts, count);
				}
				_freea(pts);
			}

			radius = radiusPlanets[i];
			cx = simulationResult[i][currStep].first;
			cy = simulationResult[i][currStep].second;
			//have to cast to REAL to get the correct overload i guess
			if ((size_t)i < planetBrushes.size() && planetBrushes[i]) {
				g_backG->FillEllipse(planetBrushes[i].get(), static_cast<Gdiplus::REAL>(cx - radius), static_cast<Gdiplus::REAL>(cy - radius), static_cast<Gdiplus::REAL>(radius * 2), static_cast<Gdiplus::REAL>(radius * 2));
			} else {
				Gdiplus::SolidBrush fallback(Gdiplus::Color::White);
				g_backG->FillEllipse(&fallback, static_cast<Gdiplus::REAL>(cx - radius), static_cast<Gdiplus::REAL>(cy - radius), static_cast<Gdiplus::REAL>(radius * 2), static_cast<Gdiplus::REAL>(radius * 2));
			}
		}
		Gdiplus::Graphics screen(hdc);
		screen.DrawImage(g_backBuffer.get(), 0, 0);
	}
}

LRESULT CALLBACK CustomEditProc(HWND hEdit, UINT msg, WPARAM wParam, LPARAM lParam) {
	int index = GetWindowLongPtr(hEdit, GWLP_USERDATA); // store index during creation

	switch (msg) {
	case WM_CHAR: {
		bool allowNeg = !(index % 5 == 0 || index % 5 == 1 || index % 5 == 4); //Negatives for velocity only
		bool allowDecimal = (index % 5 == 2 || index % 5 == 3 || index % 5 == 4); //Decimals for velocity and mass only

		if (isdigit((int)wParam) || wParam == VK_BACK)
			return CallWindowProc(oldProcs[index], hEdit, msg, wParam, lParam);

		if ((allowDecimal || currPhase == CREATION) && wParam == '.' && SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0) > 0)
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
	//I know these locations should be variables in the global context, as the error msg positioning y value depends on how large these windows are (it's equal to hButton y).
	//However just don't add new controls and it will all be fine.
	hLabel1 = CreateWindowEx(0, L"STATIC", L"# of Planets (2-15)", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_CENTER,
		centerX - 110, 10, 220, 20, hWnd, (HMENU)1, NULL, NULL);
	hLabel2 = CreateWindowEx(0, L"STATIC", L"Frames per simulation second (more increases accuracy and time to load, .1 - 5 recommended)", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_CENTER,
		centerX - 110, 40, 220, 70, hWnd, (HMENU)2, NULL, NULL);
	hLabel3 = CreateWindowEx(0, L"STATIC", L"Length of time to simulate (in 10,000 * relative speed seconds)", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_CENTER,
		centerX - 110, 120, 220, 35, hWnd, (HMENU)3, NULL, NULL);
	hLabel4 = CreateWindowEx(0, L"STATIC", L"Relative speed multiplier (more means a faster simulation, 1 is a good speed)", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_CENTER,
		centerX - 110, 165, 220, 55, hWnd, (HMENU)2, NULL, NULL);
	hEdit1 = CreateWindowEx(0, L"EDIT", L"3", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP |  ES_AUTOHSCROLL | ES_RIGHT,
		centerX + 110, 10, 60, 20, hWnd, (HMENU)4, NULL, NULL);
	hEdit2 = CreateWindowEx(0, L"EDIT", L"0.5", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT,
		centerX + 110, 40, 60, 20, hWnd, (HMENU)5, NULL, NULL);
	hEdit3 = CreateWindowEx(0, L"EDIT", L"200", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT,
		centerX + 110, 120, 60, 20, hWnd, (HMENU)6, NULL, NULL);
	hEdit4 = CreateWindowEx(0, L"EDIT", L"1", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL | ES_RIGHT,
		centerX + 110, 165, 60, 20, hWnd, (HMENU)7, NULL, NULL);
	hButton = CreateWindow(L"BUTTON", L"Create", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
		centerX + 110, 230, 60, 30, hWnd, (HMENU)8, NULL, NULL); //ID 8 used in WM_BUTTON
	SetWindowLongPtr(hEdit2, GWLP_USERDATA, 0);
	WNDPROC oldProc = (WNDPROC)SetWindowLongPtr(hEdit2, GWLP_WNDPROC, (LONG_PTR)CustomEditProc);
	//if it's our second run, oldProcs will be resized, so we just set index 0 to these.
	if (oldProcs.size() > 0) oldProcs[0] = oldProc;
	else oldProcs.push_back(oldProc);
	SetWindowLongPtr(hEdit3, GWLP_USERDATA, 0);
	oldProc = (WNDPROC)SetWindowLongPtr(hEdit3, GWLP_WNDPROC, (LONG_PTR)CustomEditProc);
	oldProcs.push_back(oldProc);
	if (oldProcs.size() > 0) oldProcs[1] = oldProc;
	else oldProcs.push_back(oldProc);
	SetWindowLongPtr(hEdit4, GWLP_USERDATA, 0);
	oldProc = (WNDPROC)SetWindowLongPtr(hEdit4, GWLP_WNDPROC, (LONG_PTR)CustomEditProc);
	oldProcs.push_back(oldProc);
	if (oldProcs.size() > 0) oldProcs[2] = oldProc;
	else oldProcs.push_back(oldProc);
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
			centerX - 110, startY, 150, 25, GetParent(textBox), (HMENU)9, NULL, NULL);
		return false;
	}
	if (allZeroes) {
		if (hErrorMsg != NULL) {
			DestroyWindow(hErrorMsg);
		}
		hErrorMsg = CreateWindowEx(0, L"STATIC", L"Zeroes are not valid", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_CENTER,
			centerX - 110, startY, 150, 25, GetParent(textBox), (HMENU)9, NULL, NULL);
		return false;
	}
	return !allZeroes;
}

bool IsValidNumberEntry(HWND textBox) {
	if (textBox == hEdit3 || textBox == hEdit2 || textBox == hEdit4) {
		return SpecialCaseForDecimals(textBox, 230);
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
				centerX - 110, 230, 150, 25, GetParent(textBox), (HMENU)9, NULL, NULL);
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
			centerX - 110, 230, 150, 25, GetParent(textBox), (HMENU)9, NULL, NULL);
		return false;
	}
	//This is after length so it doesn't give the error for empty box
	if (allZeroes) {
		if (hErrorMsg != NULL) {
			DestroyWindow(hErrorMsg);
		}
		hErrorMsg = CreateWindowEx(0, L"STATIC", L"Zeroes are not valid", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_CENTER,
			centerX - 110, 230, 150, 25, GetParent(textBox), (HMENU)9, NULL, NULL);
		return false;
	}
	if (textBox == hEdit1) {
		//number of planets special case
		int numPlanetsCheck = std::stoi(std::string(szBuf));
		if (numPlanetsCheck < 2 || numPlanetsCheck > 15) {
			if (hErrorMsg != NULL) {
				DestroyWindow(hErrorMsg);
			}
			hErrorMsg = CreateWindowEx(0, L"STATIC", L"# of planets must be 2-15", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_CENTER,
				centerX -110, 230, 150, 35, GetParent(textBox), (HMENU)9, NULL, NULL);
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
		char szBuf4[2048];
		int throwaway1 = GetWindowTextA(hEdit1, szBuf1, GetWindowTextLength(hEdit1) + 1);
		int throwaway2 = GetWindowTextA(hEdit2, szBuf2, GetWindowTextLength(hEdit2) + 1);
		int throwaway3 = GetWindowTextA(hEdit3, szBuf3, GetWindowTextLength(hEdit3) + 1);
		int throwaway4 = GetWindowTextA(hEdit4, szBuf4, GetWindowTextLength(hEdit4) + 1);
		numPlanets = std::stoi(std::string(szBuf1));
		fpss = std::stof(std::string(szBuf2));
		simLength = std::stof(std::string(szBuf3)); //in seconds
		relativeSpeed = std::stof(std::string(szBuf4));
		solveIVP = new Calculations(
			numPlanets, fpss, simLength, relativeSpeed
		);
		DestroyWindow(hLabel1);
		DestroyWindow(hLabel2);
		DestroyWindow(hLabel3);
		DestroyWindow(hLabel4);
		DestroyWindow(hEdit1);
		DestroyWindow(hEdit2);
		DestroyWindow(hEdit3);
		DestroyWindow(hEdit4);
		oldProcs.erase(oldProcs.begin());
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
	wchar_t szBufXPos[256];
	wchar_t szBufYPos[256];
	wchar_t szBufXVel[256];
	wchar_t szBufYVel[256];
	for (int i = 0; i < numPlanets*5; i+=5) {
		std::wstring number = std::to_wstring(i/5 + 1);
		if (startingPixel.first == 0 && i == 5) { //if second planet, force to first column
			startingPixel = { 0, startingPixel.second };
		}
		else {
			startingPixel = (startingPixel.second + 205 < clientHeight) ? std::pair<int, int> {startingPixel.first, startingPixel.second}
			: std::pair<int, int>{ startingPixel.first + 220, 0 }; //205 = 35*5 + 30 pixels button, 220 = length of label + input box + 10
		}
		int yVel = ((i / 5) % 3 == 0) ? 1000 : (((i / 5) % 3 == 1) ? 0 : -1000); //0 = + vel, 1 = no vel, 2 = - vel, 3 = + vel...
		swprintf(szBufXPos, 256, L"%d", 400 + (i / 5) * 100);
		swprintf(szBufYPos, 256, L"%d", 400);
		swprintf(szBufXVel, 256, L"%d", 0);
		swprintf(szBufYVel, 256, L"%d", yVel);
		planetLabels[i] = CreateWindowEx(0, L"STATIC", (L"Planet " + number + L" x-position (10^6 m per px):").c_str(), WS_CHILD | WS_BORDER | WS_VISIBLE | ES_CENTER,
			startingPixel.first, startingPixel.second, 150, 35, hWnd, (HMENU)i, NULL, NULL);
		planetLabels[i + 1] = CreateWindowEx(0, L"STATIC", (L"Planet " + number + L" y-position (10^6 m per px):").c_str(), WS_CHILD | WS_BORDER | WS_VISIBLE | ES_CENTER,
			startingPixel.first, startingPixel.second + 35, 150, 35, hWnd, (HMENU)(i + 1), NULL, NULL);
		planetLabels[i + 2] = CreateWindowEx(0, L"STATIC", (L"Planet " + number + L" x-velocity (m/s):").c_str(), WS_CHILD | WS_BORDER | WS_VISIBLE | ES_CENTER,
			startingPixel.first, startingPixel.second + 70, 150, 35, hWnd, (HMENU)(i + 2), NULL, NULL);
		planetLabels[i + 3] = CreateWindowEx(0, L"STATIC", (L"Planet " + number + L" y-velocity (m/s):").c_str(), WS_CHILD | WS_BORDER | WS_VISIBLE | ES_CENTER,
			startingPixel.first, startingPixel.second + 105, 150, 35, hWnd, (HMENU)(i + 3), NULL, NULL);
		planetLabels[i + 4] = CreateWindowEx(0, L"STATIC", (L"Planet " + number + L" mass (in 10^24 kgs):").c_str(), WS_CHILD | WS_BORDER | WS_VISIBLE | ES_CENTER,
			startingPixel.first, startingPixel.second + 140, 150, 35, hWnd, (HMENU)(i + 4), NULL, NULL);
		planetInputBoxes[i] = CreateWindow(L"EDIT", szBufXPos, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT,
			startingPixel.first + 150, startingPixel.second, 70, 35, hWnd, (HMENU)(i + 5), NULL, NULL);
		planetInputBoxes[i + 1] = CreateWindow(L"EDIT", szBufYPos, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT,
			startingPixel.first + 150, startingPixel.second + 35, 70, 35, hWnd, (HMENU)(i + 6), NULL, NULL);
		planetInputBoxes[i + 2] = CreateWindow(L"EDIT", szBufXVel, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT,
			startingPixel.first + 150, startingPixel.second + 70, 70, 35, hWnd, (HMENU)(i + 7), NULL, NULL);
		planetInputBoxes[i + 3] = CreateWindow(L"EDIT", szBufYVel, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT,
			startingPixel.first + 150, startingPixel.second + 105, 70, 35, hWnd, (HMENU)(i + 8), NULL, NULL);
		planetInputBoxes[i + 4] = CreateWindow(L"EDIT", L"3", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT,
			startingPixel.first + 150, startingPixel.second + 140, 70, 35, hWnd, (HMENU)(i + 9), NULL, NULL);
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

void ReCreatePlanetInitialValues(HWND hWnd) {
	//In case window was resized
	RECT clientRect;
	int clientWidth;
	int clientHeight;
	if (GetClientRect(hWnd, &clientRect)) {
		clientWidth = clientRect.right;
		clientHeight = clientRect.bottom;
	}

	//Labels and text boxes, 10 max planets so no need for fancy formatting
	planetLabels.resize(numPlanets * 5);
	planetInputBoxes.resize(numPlanets * 5);
	oldProcs.resize(numPlanets * 5);

	//Condition: window must be tall enough to fit one column of planet values + 30 for the button and wide enough to fit 10 columns.
	std::pair<int, int> startingPixel = { 0,0 };
	wchar_t buffer[256];
	swprintf(buffer, 256, L"Enter initial values for each planet below. Positions are in pixels where top left is {0,0}, width = %d px and height = %d px. Vel in m/s.", windowLength, windowHeight);
	hLabel1 = CreateWindowEx(0, L"STATIC", buffer, WS_CHILD | WS_VISIBLE | ES_CENTER,
		0, 0, 220, 80, hWnd, (HMENU)1, NULL, NULL);
	startingPixel.second += 80; //move down for the inputs

	wchar_t szBufXPos[256];
	wchar_t szBufYPos[256];
	wchar_t szBufXVel[256];
	wchar_t szBufYVel[256];
	wchar_t szBufMass[256];
	for (int i = 0; i < numPlanets * 5; i += 5) {
		std::wstring number = std::to_wstring(i / 5 + 1);
		if (startingPixel.first == 0 && i == 5) { //if second planet, force to first column
			startingPixel = { 0, startingPixel.second };
		}
		else {
			startingPixel = (startingPixel.second + 205 < clientHeight) ? std::pair<int, int> {startingPixel.first, startingPixel.second}
			: std::pair<int, int>{ startingPixel.first + 220, 0 }; //205 = 35*5 + 30 pixels button, 220 = length of label + input box + 10
		}
		swprintf(szBufXPos, 256, L"%f", initialVals[i / 5].xPos / metersPerPixel);
		swprintf(szBufYPos, 256, L"%f", initialVals[i / 5].yPos / metersPerPixel);
		swprintf(szBufXVel, 256, L"%f", initialVals[i / 5].xVel);
		swprintf(szBufYVel, 256, L"%f", initialVals[i / 5].yVel);
		swprintf(szBufMass, 256, L"%f", initialVals[i / 5].mass / 1e24);

		//swprintf(buffer, 256, L"%d", 50000);
		planetLabels[i] = CreateWindowEx(0, L"STATIC", (L"Planet " + number + L" x-position (10^6 meters per px):").c_str(), WS_CHILD | WS_BORDER | WS_VISIBLE | ES_CENTER,
			startingPixel.first, startingPixel.second, 150, 35, hWnd, (HMENU)i, NULL, NULL);
		planetLabels[i + 1] = CreateWindowEx(0, L"STATIC", (L"Planet " + number + L" y-position (10^6 meters per px):").c_str(), WS_CHILD | WS_BORDER | WS_VISIBLE | ES_CENTER,
			startingPixel.first, startingPixel.second + 35, 150, 35, hWnd, (HMENU)(i + 1), NULL, NULL);
		planetLabels[i + 2] = CreateWindowEx(0, L"STATIC", (L"Planet " + number + L" x-velocity (m/s):").c_str(), WS_CHILD | WS_BORDER | WS_VISIBLE | ES_CENTER,
			startingPixel.first, startingPixel.second + 70, 150, 35, hWnd, (HMENU)(i + 2), NULL, NULL);
		planetLabels[i + 3] = CreateWindowEx(0, L"STATIC", (L"Planet " + number + L" y-velocity (m/s):").c_str(), WS_CHILD | WS_BORDER | WS_VISIBLE | ES_CENTER,
			startingPixel.first, startingPixel.second + 105, 150, 35, hWnd, (HMENU)(i + 3), NULL, NULL);
		planetLabels[i + 4] = CreateWindowEx(0, L"STATIC", (L"Planet " + number + L" mass (10^24 kgs):").c_str(), WS_CHILD | WS_BORDER | WS_VISIBLE | ES_CENTER,
			startingPixel.first, startingPixel.second + 140, 150, 35, hWnd, (HMENU)(i + 4), NULL, NULL);
		planetInputBoxes[i] = CreateWindow(L"EDIT", szBufXPos, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT,
			startingPixel.first + 150, startingPixel.second, 70, 35, hWnd, (HMENU)(i + 5), NULL, NULL);
		planetInputBoxes[i + 1] = CreateWindow(L"EDIT", szBufYPos, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT,
			startingPixel.first + 150, startingPixel.second + 35, 70, 35, hWnd, (HMENU)(i + 6), NULL, NULL);
		planetInputBoxes[i + 2] = CreateWindow(L"EDIT", szBufXVel, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT,
			startingPixel.first + 150, startingPixel.second + 70, 70, 35, hWnd, (HMENU)(i + 7), NULL, NULL);
		planetInputBoxes[i + 3] = CreateWindow(L"EDIT", szBufYVel, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT,
			startingPixel.first + 150, startingPixel.second + 105, 70, 35, hWnd, (HMENU)(i + 8), NULL, NULL);
		planetInputBoxes[i + 4] = CreateWindow(L"EDIT", szBufMass, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_RIGHT,
			startingPixel.first + 150, startingPixel.second + 140, 70, 35, hWnd, (HMENU)(i + 9), NULL, NULL);
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
		if (allZeroes && (i%5) % 4 == 0) { //velocities and pixels can be 0, only mass can't (pixels shouldn't but I'm not gonna stop anyone).
			if (hErrorMsg != NULL) {
				DestroyWindow(hErrorMsg);
			}
			hErrorMsg = CreateWindowEx(0, L"STATIC", L"Zeroes are not valid", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_CENTER,
				startErrorX, startErrorY, 150, 35, GetParent(textBox), (HMENU)501, NULL, NULL);
			return false;
		}
	}

	return true;
}

//oldProcs.clear(); when destroying the edit boxes later
void StartSimulation(HWND hWnd) {
	DestroyWindow(hLabel1);
	//store our initial values in initialVals vector then destroy the input boxes.
	initialVals.resize(numPlanets);
	radiusPlanets.resize(numPlanets);
	for (int i = 0; i < numPlanets; i++) {
		int currInputIndex = i * 5;
		char szBufXPos[2048];
		char szBufYPos[2048];
		char szBufXVel[2048];
		char szBufYVel[2048];
		char szBufMass[2048];
		GetWindowTextA(planetInputBoxes[currInputIndex], szBufXPos, GetWindowTextLength(planetInputBoxes[currInputIndex]) + 1);
		GetWindowTextA(planetInputBoxes[currInputIndex + 1], szBufYPos, GetWindowTextLength(planetInputBoxes[currInputIndex + 1]) + 1);
		GetWindowTextA(planetInputBoxes[currInputIndex + 2], szBufXVel, GetWindowTextLength(planetInputBoxes[currInputIndex + 2]) + 1);
		GetWindowTextA(planetInputBoxes[currInputIndex + 3], szBufYVel, GetWindowTextLength(planetInputBoxes[currInputIndex + 3]) + 1);
		GetWindowTextA(planetInputBoxes[currInputIndex + 4], szBufMass, GetWindowTextLength(planetInputBoxes[currInputIndex + 4]) + 1);
		int xPos = std::stoi(szBufXPos);
		int yPos = std::stoi(szBufYPos);
		float xVel = std::stof(szBufXVel);
		float yVel = std::stof(szBufYVel);
		float mass = std::stof(szBufMass);
		if (mass < .001) {
			radiusPlanets[i] = 2;
		}
		if (mass < .01) {
			radiusPlanets[i] = 4;
		}
		else if (mass < .05) {
			radiusPlanets[i] = 6;
		}
		else if (mass <= 1) {
			radiusPlanets[i] = 7;
		}
		else if (mass <= 10) {
			radiusPlanets[i] = 8;
		}
		else if (mass <= 30) {
			radiusPlanets[i] = 15;
		}
		else if (mass > 30) {
			radiusPlanets[i] = 20;
		}
		double xPosMeters = xPos * metersPerPixel;
		double yPosMeters = yPos * metersPerPixel;
		double massKG = mass * 1e24;
		initialVals[i] = PlanetInfo(xPosMeters, yPosMeters, xVel, yVel, massKG);
		for (int j = 0; j < 5; j++) {
			DestroyWindow(planetInputBoxes[currInputIndex + j]);
			DestroyWindow(planetLabels[currInputIndex + j]);
		}
	}
	if (hErrorMsg != NULL) {
		DestroyWindow(hErrorMsg);
	}
	DestroyWindow(hStartSimButton);
	std::vector<COLORREF> rgbValues =
	{ RGB(0, 102, 204),  RGB(204, 0, 0), RGB(0, 255, 0), RGB(0, 204, 204), RGB(204, 0, 204), RGB(255, 255, 255),
		RGB(204, 0, 102), RGB(102, 0, 204), RGB(0, 204, 102), RGB(0, 0, 204), RGB(102, 0, 0), RGB(0, 102, 51),
		RGB(153, 255, 255), RGB(255, 102, 178), RGB(76, 0, 153)};
	for (int i = 0; i < numPlanets; i++) {
		//create pens and brushes for each planet
		COLORREF cref = rgbValues[i];
		Gdiplus::Color c(255, GetRValue(cref), GetGValue(cref), GetBValue(cref));
		planetBrushes.push_back(std::make_unique<Gdiplus::SolidBrush>(c));
		planetPens.push_back(std::make_unique<Gdiplus::Pen>(c, 1.5f));
	}
	currStep = 0;
	solveIVP->setMetersPerPixel(metersPerPixel);
	solveIVP->setInitialValues(initialVals);
	simulationResult = solveIVP->solve();
	totalSteps = simulationResult[0].size();
	SetTimer(hWnd, 1, 34, (TIMERPROC)NULL); //34 ms -> 30 fps
}

void EndSimulation(HWND hWnd) {
	KillTimer(hWnd, 1);
	DestroyWindow(hFrameTracker);
	if (hPositionDisplay != NULL) {
		DestroyWindow(hPositionDisplay);
	}
	hRestartSimButton = CreateWindow(L"BUTTON", L"Restart", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		centerX - 30, 115, 60, 30, hWnd, (HMENU)1000, NULL, NULL);
	hChangeInitialVals = CreateWindow(L"BUTTON", L"Change Initial Values", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		centerX - 100, 75, 200, 30, hWnd, (HMENU)1001, NULL, NULL);
}