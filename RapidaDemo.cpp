// RapidaDemo.cpp
//
#include "RapidaDemo.h"
#include "TrianglePolyDemo.h"
#pragma comment (lib,"Gdiplus.lib")
using namespace Gdiplus;


#define MAX_STRING 100

// Globals
HINSTANCE hInst;                                
const WCHAR szTitle[] = L"Rapida Demo Program";                  
const WCHAR szWindowClass[] = L"rpdemo_class";            
TrianglePolyDemo demo;

// Forward declarations 
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void DrawDemo(HWND hwnd, HDC hdc);
void World2ScreenCoords(Gdiplus::Point& pRet, const ::Point& p, const RECT& r);
void DrawHitTestResult(Graphics& g, Gdiplus::Point pts[], bool hit);
void DrawTextOutput(Graphics& g, const RECT& r);
void NudgeTriangle(HWND hwnd, Triangle& t, int index, int lr, int ud);
void RotateTriangle(HWND hwnd, Triangle& t, int index);


// entry point
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	// start GDI+
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	srand((unsigned)time(0));

	// this really should be a worker thread, to not block the UI thread
	demo.Calculate();

    // Perform window initialization; instantiation
    if (!InitInstance (hInstance, nCmdShow))
    {
		return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RAPIDADEMO));
    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

	// shut down GDI+
	Gdiplus::GdiplusShutdown(gdiplusToken);
    return 0;
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   // register window class
   WNDCLASSEXW wcex = { 0 };
   wcex.cbSize = sizeof(WNDCLASSEX);
   wcex.style = CS_HREDRAW | CS_VREDRAW;
   wcex.lpfnWndProc = WndProc;
   wcex.hInstance = hInstance;
   wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RAPIDADEMO));
   wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
   wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
   wcex.lpszClassName = szWindowClass;
   wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
   RegisterClassExW(&wcex);

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_MAXIMIZE,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
      return FALSE;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
			DrawDemo(hWnd,hdc);
            EndPaint(hWnd, &ps);
        }
        break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_LEFT:
			NudgeTriangle(hWnd, demo.triangles[demo.nSelected], demo.nSelected, -2, 0);
			break;
		case VK_RIGHT:
			NudgeTriangle(hWnd, demo.triangles[demo.nSelected], demo.nSelected, 2, 0);
			break;
		case VK_UP:
			NudgeTriangle(hWnd, demo.triangles[demo.nSelected], demo.nSelected, 0, 2);
			break;
		case VK_DOWN:
			NudgeTriangle(hWnd, demo.triangles[demo.nSelected], demo.nSelected, 0, -2);
			break;
		}
		break;
	case WM_CHAR:
		switch (wParam)
		{
		case 'r':
		case 'R':
			RotateTriangle(hWnd, demo.triangles[demo.nSelected], demo.nSelected);
			break;
		case ' ':
			if (++demo.nSelected > 4)
				demo.nSelected = 0;
			::InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// move a triangle in the lr and/or ud direction
void NudgeTriangle(HWND hwnd, Triangle& t, int index, int lr, int ud)
{
	// check location is valid before we move - code looks a bit odd 
	// because we want to allow moves back into screeen region if a 
	// rotate has shifted us outside 
	for (auto i = 0; i < 3; ++i) {
		if ( (lr<0 && (t.pt[i].x + lr < 0)) || (lr>0 && (t.pt[i].x + lr > WORLDSPACE_EXTENT))  )
			return;
		if ( (ud<0 && (t.pt[i].y + ud < 0)) || (ud>0 && (t.pt[i].y + ud > WORLDSPACE_EXTENT)) )
			return;
	}
	for (auto i = 0; i < 3; ++i) {
		t.pt[i].x += lr;
		t.pt[i].y += ud;
	}

	demo.Recalculate(index);
	::InvalidateRect(hwnd, NULL, FALSE);
}

void RotateTriangle(HWND hwnd, Triangle& t, int index)
{
	// rotate by pi/20 rads
	const double theta = 0.15708;
	double xorig = (double(t.pt[0].x + t.pt[1].x + t.pt[2].x)) / 3.0;
	double yorig = (double(t.pt[0].y + t.pt[1].y + t.pt[2].y)) / 3.0;

	for (auto i = 0; i < 3; ++i) {
		double dx = t.pt[i].x - xorig;
		double dy = t.pt[i].y - yorig;
		t.pt[i].x = static_cast<long>(round( xorig + dx * cos(theta) - dy * sin(theta) ));
		t.pt[i].y = static_cast<long>(round( yorig + dx * sin(theta) + dy * cos(theta) ));
	}

	demo.Recalculate(index);
	::InvalidateRect(hwnd, NULL, FALSE);
}


void DrawDemo(HWND hwnd, HDC hdc)
{
	// setup for double-buffering
	RECT r;
	GetClientRect(hwnd, &r);
	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hbitmap = CreateCompatibleBitmap(hdc, r.right, r.bottom);
	SelectObject(hdcMem, hbitmap);

	// basic GDI+ stuffs
	Graphics g(hdcMem);
	g.SetSmoothingMode(SmoothingModeHighQuality);
	g.FillRectangle(new Gdiplus::SolidBrush(Gdiplus::Color(255, 255, 255, 255)), 0, 0, r.right, r.bottom);

	GraphicsPath path, path2;
	Gdiplus::Point p1, p2;

	// draw the polygon
	World2ScreenCoords(p1, demo.polygon.pts[0], r);
	for (auto i = 1; i < demo.polygon.pts.size()-1; ++i) {
		World2ScreenCoords( p2, demo.polygon.pts[i], r);
		path.AddLine(p1, p2);
		p1 = p2;
	}
	g.DrawPath(new Pen(Color(255, 0, 255, 0), 2) , &path);

	LinearGradientBrush brush(
		Gdiplus::Point(0, r.bottom),
		Gdiplus::Point(0, 0),
		Color(255, 200, 255, 210),
		Color(255, 245, 255, 245));
	g.FillPath(&brush, &path); 

	// draw first 5 triangles
	for (auto i = 0; i<5; ++i) {
		/*
		Gdiplus::Point pts[3];
		for ( auto j=0; j<3 ; ++j )
			World2ScreenCoords(pts[j], demo.triangles[i].pt[j], r);
		g.DrawPolygon(new Pen(Color(255, 255, 0, 128), 2), pts, 3);*/
		Gdiplus::Point pts[3];
		for (auto j = 0; j < 3; ++j) {
			World2ScreenCoords(pts[j], demo.triangles[i].pt[j], r);
		}
		path2.AddPolygon(pts, 3);

		g.DrawPath(new Pen(Color((i==demo.nSelected)?220:120, 255, 0, 0), 1), &path2);
		g.FillPath(new SolidBrush(Color((i==demo.nSelected)?100:45, 250, 0, 0)), &path2);
		path2.Reset();

		// draw hit test result text
		DrawHitTestResult(g, pts, demo.triangle_hit_results[i]);
	} 

	DrawTextOutput(g,r);
	BitBlt(hdc, 0, 0, r.right, r.bottom, hdcMem, 0, 0, SRCCOPY);
	DeleteDC(hdcMem);
	DeleteObject(hbitmap); 
}

// calculates triangle bounds; draws a hit result within
void DrawHitTestResult(Graphics& g, Gdiplus::Point pts[], bool hit)
{
	// calculate a bounding box for text label
	Gdiplus::RectF bounds;
	auto dx = (pts[0].X + pts[1].X + pts[2].X) / 3.0f;
	auto dy = (pts[0].Y + pts[1].Y + pts[2].Y) / 3.0f;
	bounds.X = dx - 100; bounds.Width = 200;
	bounds.Y = dy - 11; bounds.Height = 22;

	Font font(L"Arial", 12);
	StringFormat fmt;
	fmt.SetAlignment(StringAlignmentCenter);
	fmt.SetLineAlignment(StringAlignmentCenter);
	g.DrawString( (hit ? L"Hit" : L"Miss"), -1, &font, bounds, &fmt, new SolidBrush(Color(255, 0, 0, 0)));

}

void DrawTextOutput(Graphics& g, const RECT& r)
{
	StringFormat fmt;
	fmt.SetAlignment(StringAlignmentNear);
	fmt.SetLineAlignment(StringAlignmentCenter);
	Font font(L"Segoe UI", 10);
	Gdiplus::RectF rTop = { 2,2,600,22 };

	wchar_t buf[256];
	swprintf(buf, 255, L"%i triangles tested in: %4.2f ms.  Displaying first 5.", NUM_TRIANGLES, ((double)demo.duration.count()) / 1000.0);
	g.DrawString(buf, -1, &font, rTop, &fmt, new SolidBrush(Color(255, 0, 0, 0)));

	rTop.Y = static_cast<Gdiplus::REAL>(r.bottom - 24);
	g.DrawString(L"<SPACE> selects triangle.  Arrow keys move selected triangle.  'R' rotates selected object.", -1, &font, rTop, &fmt, new SolidBrush(Color(255, 0, 0, 0)));
}

void World2ScreenCoords(Gdiplus::Point& pRet, const ::Point& p, const RECT& r)
{
	pRet.X = (p.x * r.right) / WORLDSPACE_EXTENT;
	pRet.Y = r.bottom - (p.y * r.bottom) / WORLDSPACE_EXTENT;
}
