// lab2.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "framework.h"
#include "lab2.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <map>

#define MAX_LOADSTRING 100


const double FIELD=55;
const double INTERVAL=6;
const double MARGIN=10;
std::map<HWND, std::string> map;
std::map<HWND, int> result;
std::vector<HWND> v;
std::vector<TCHAR> letters;
std::map<std::pair<HWND, char>, int> letterColors;


int xPos = 0;
int yPos = 0;


// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];    
WCHAR szTitle2[MAX_LOADSTRING];                  // The title bar text
// The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

LRESULT CALLBACK    WndProcChild(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    WndProcChild2(HWND, UINT, WPARAM, LPARAM);

void CreateEasyWindow(HWND hWnd);
void CreateMediumWindow(HWND hWnd);
void CreateHardWindow(HWND hWnd);
void PrintPuzzle(HDC hdc,int level,std::string word,HWND hWnd);
void CheckResults(std::string str);
void ColorKeboard(HDC hdc, HWND hWnd);
void ColorEasy(char c, double x, double y, HDC hdc);
void ColorMedium(char c, double x, double y, HDC hdc);
void ColorHard(char c, double x, double y, HDC hdc);

std::vector <std::string> words;
void ReadFile()
{
    std::fstream plik;
    plik.open("Wordle.txt");
    while (!plik.eof())
    {
        std::string a;
        plik >> a;
        words.push_back(a);
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
    ReadFile();
    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LAB2, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAB2));

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
    return (int) msg.wParam;
}




ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)0;
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LAB2);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

    WNDCLASS wcex2;

    LoadStringW(hInstance, IDS_APP_TITLE2, szTitle2, MAX_LOADSTRING);

    wcex2.style = CS_HREDRAW | CS_VREDRAW;
    wcex2.lpfnWndProc = WndProcChild;
    wcex2.cbClsExtra = 0;
    wcex2.cbWndExtra = 0;
    wcex2.hInstance = hInstance;
    wcex2.hIcon = NULL;
    wcex2.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex2.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(255, 255, 255)));
    wcex2.lpszMenuName = NULL;
    wcex2.lpszClassName = L"child_window";

    RegisterClass(&wcex2);
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable
   int width = 10*FIELD + 10*INTERVAL + 3*MARGIN;
   int height = 4*FIELD + 2*INTERVAL + 3*MARGIN;

   //https://forums.codeguru.com/showthread.php?500841-Set-my-window-position-at-screen-center
   int xPos = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
   int yPos = (GetSystemMetrics(SM_CYSCREEN) - height) / 2+200;
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
      xPos, yPos, width, height, nullptr, nullptr, hInstance, nullptr);

   SetWindowLong(hWnd, GWL_EXSTYLE,
   GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
   // Make this window 50% alpha
   SetLayeredWindowAttributes(hWnd, 0, (255 * 50) / 100, LWA_ALPHA);
   // Show this window
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);



   if (!hWnd)
   {
      return FALSE;
   }
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC offDC = NULL;
    static HBITMAP offOldBitmap = NULL;
    static HBITMAP offBitmap = NULL;

    const int bufSize = 256;
    TCHAR buf[bufSize];
    srand(time(NULL));
    switch (message)
    {
    case WM_SIZE:
    {
        int clientWidth = LOWORD(lParam);
        int clientHeight = HIWORD(lParam);
        HDC hdc = GetDC(hWnd);
        if (offOldBitmap != NULL) {
              SelectObject(offDC, offOldBitmap);
             
        }
        if (offBitmap != NULL) {
           DeleteObject(offBitmap);
          
        }
        offBitmap = CreateCompatibleBitmap(hdc, clientWidth, clientHeight);
        offOldBitmap = (HBITMAP)SelectObject(offDC, offBitmap);
        ReleaseDC(hWnd, hdc);
        
    }
       break;

    case WM_CREATE:
    {
        HDC hdc = GetDC(hWnd);
        offDC = CreateCompatibleDC(hdc);
        ReleaseDC(hWnd, hdc);
        srand(time(NULL));
        int glvl;
        char str[10];
        int length;
        std::string lvl = "";
        length = GetPrivateProfileStringA("wordle", "level", "", str, 10, "../settings.ini");
        if (length == 0)
        {
            glvl = rand() % 3+1;
        }
        else
        {
            for (int i = 0; i < length; ++i)
                lvl += str[i];
            glvl = atoi(lvl.c_str());
        }
        if (glvl == 1)
        {
            HMENU menu = GetMenu(hWnd);
            CheckMenuItem(menu, ID_DIFFICULTY_EASY, MF_CHECKED);
            CheckMenuItem(menu, ID_DIFFICULTY_MEDIUM, MF_UNCHECKED);
            CheckMenuItem(menu, ID_DIFFICULTY_HARD, MF_UNCHECKED);
            CreateEasyWindow(hWnd);
        }
        else if(glvl == 3)
        { 
            HMENU menu = GetMenu(hWnd);
            CheckMenuItem(menu, ID_DIFFICULTY_EASY, MF_UNCHECKED);
            CheckMenuItem(menu, ID_DIFFICULTY_MEDIUM, MF_UNCHECKED);
            CheckMenuItem(menu, ID_DIFFICULTY_HARD, MF_CHECKED);
            CreateHardWindow(hWnd);
        }
        else
        {
            HMENU menu = GetMenu(hWnd);
            CheckMenuItem(menu, ID_DIFFICULTY_EASY, MF_UNCHECKED);
            CheckMenuItem(menu, ID_DIFFICULTY_MEDIUM, MF_CHECKED);
            CheckMenuItem(menu, ID_DIFFICULTY_HARD, MF_UNCHECKED);
            CreateMediumWindow(hWnd);
        }

        return 0;
    }
    break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case ID_DIFFICULTY_EASY:
            {
                for (int i = 0; i < v.size(); ++i)
                    DestroyWindow(v[i]);
                v.clear();
                letters.clear();
                letterColors.clear();
                InvalidateRect(hWnd, NULL, TRUE);

                xPos = yPos = 0;
                HMENU menu = GetMenu(hWnd);
                CheckMenuItem(menu, ID_DIFFICULTY_EASY, MF_CHECKED);
                CheckMenuItem(menu, ID_DIFFICULTY_MEDIUM, MF_UNCHECKED);
                CheckMenuItem(menu, ID_DIFFICULTY_HARD, MF_UNCHECKED);

                CreateEasyWindow(hWnd);

            }
            break;
            case ID_DIFFICULTY_MEDIUM:
            {
                for (int i = 0; i < v.size(); ++i)
                    DestroyWindow(v[i]);
                v.clear();
                letters.clear();
                letterColors.clear();
                InvalidateRect(hWnd, NULL, TRUE);

                xPos = yPos = 0;

                HMENU menu = GetMenu(hWnd);
                CheckMenuItem(menu, ID_DIFFICULTY_EASY, MF_UNCHECKED);
                CheckMenuItem(menu, ID_DIFFICULTY_MEDIUM, MF_CHECKED);
                CheckMenuItem(menu, ID_DIFFICULTY_HARD, MF_UNCHECKED);

                CreateMediumWindow(hWnd);
                //time = 0.0;

            }
            break;
            case ID_DIFFICULTY_HARD:
            {
                for (int i = 0; i < v.size(); ++i)
                    DestroyWindow(v[i]);
                v.clear();
                letters.clear();
                letterColors.clear();
                InvalidateRect(hWnd, NULL, TRUE);

                xPos = yPos = 0;

                HMENU menu = GetMenu(hWnd);
                CheckMenuItem(menu, ID_DIFFICULTY_EASY, MF_UNCHECKED);
                CheckMenuItem(menu, ID_DIFFICULTY_MEDIUM, MF_UNCHECKED);
                CheckMenuItem(menu, ID_DIFFICULTY_HARD, MF_CHECKED);

                CreateHardWindow(hWnd);
                //time = 0.0;

            }
            break;
            //case IDM_ABOUT:
            //    DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            //    break;
            //case IDM_EXIT:
            //    DestroyWindow(hWnd);
            //    break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            HPEN pen = CreatePen(PS_SOLID, 2, RGB(164, 174, 196));
            HPEN oldPen = (HPEN)SelectObject(offDC, pen);

            RECT rc;
            GetClientRect(hWnd, &rc);

            HBRUSH backgroundBrush = CreateSolidBrush(RGB(255, 255, 255));
            RECT backgroundRect = { 0, 0, rc.right - rc.left, rc.bottom - rc.top };
            FillRect(offDC, &backgroundRect, backgroundBrush);
            DeleteObject(backgroundBrush);

            HFONT font = CreateFont(
                -MulDiv(20, GetDeviceCaps(offDC, LOGPIXELSY), 72), // Height
                0,
                0,
                0,
                FW_BOLD,
                false,
                FALSE,
                0,
                EASTEUROPE_CHARSET,
                OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY,
                DEFAULT_PITCH | FF_SWISS,
                _T(" Verdana "));
            HFONT oldFont = (HFONT)SelectObject(offDC, font);
            

            ColorKeboard(offDC, hWnd);

            BitBlt(hdc, 0, 0, rc.right, rc.bottom, offDC, 0, 0, SRCCOPY);
            SelectObject(offDC, oldFont);
            DeleteObject(font);
            SelectObject(offDC, oldPen);
            DeleteObject(pen);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_CHAR:
    {
        if (wParam == VK_BACK)
        {
            if (xPos != 0)
            {
                letters.pop_back();
                xPos--;
                for (int i = 0; i < v.size(); ++i)
                {
                    if(result[v[i]]==-1)
                        InvalidateRect(v[i], NULL, TRUE);
                }
            }
        }
        else if (wParam == VK_RETURN)
        {
            if (xPos == 5)
            {
                std::string a="";
                for (int i = 0; i < 5; ++i)
                {
                    a += tolower(letters[5 * yPos + i]);
                }
                if (std::find(words.begin(), words.end(), a)!=words.end())
                {
                    ++yPos;
                    CheckResults(a);
                    xPos = 0;
                    for (int i = 0; i < v.size(); ++i)
                    {
                          InvalidateRect(v[i], NULL, TRUE);
                    }
                    InvalidateRect(hWnd, NULL, TRUE);
                }
                else
                {
                    xPos = 0;
                    letters.erase(letters.end() - 5, letters.end());
                    for (int i = 0; i < v.size(); ++i)
                    {
                        InvalidateRect(v[i], NULL, TRUE);
                    }
                }
            }
        }
        else if (xPos<5)
        {
            letters.push_back((TCHAR)toupper(wParam));
            ++xPos;
            for (int i = 0; i < v.size(); ++i)
            {
                if(result[v[i]]==-1)
                    InvalidateRect(v[i], NULL, TRUE);
            }
        }
    }
    break;
    case WM_DESTROY:
    {
        if (offOldBitmap != NULL) {
            SelectObject(offDC, offOldBitmap);
        }
        if (offDC != NULL) {
            DeleteDC(offDC);
        }
        if (offBitmap != NULL) {
            DeleteObject(offBitmap);
        }
        if (v.size() == 1)
            WritePrivateProfileStringA("wordle", "level", "1", "../settings.ini");
        if (v.size() == 2)
            WritePrivateProfileStringA("wordle", "level", "2", "../settings.ini");
        if (v.size() == 4)
            WritePrivateProfileStringA("wordle", "level", "3", "../settings.ini");
        PostQuitMessage(0);
    }
        break;
    case WM_ERASEBKGND:
        return 1;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK WndProcChild(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HDC offDC = NULL;
    static HBITMAP offOldBitmap = NULL;
    static HBITMAP offBitmap = NULL;

    const int bufSize = 256;
    TCHAR buf[bufSize];
    switch (message)
    {
    // zrodlo: https://stackoverflow.com/questions/7773771/how-do-i-implement-dragging-a-window-using-its-client-area
    case WM_NCHITTEST: 
    {
        LRESULT hit = DefWindowProc(hWnd, message, wParam, lParam);
        if (hit == HTCLIENT) hit = HTCAPTION;
        return hit;
    }
    break;
    case WM_ERASEBKGND:
        return 1;
    case WM_CREATE:
    {
        HDC hdc = GetDC(hWnd);
        offDC = CreateCompatibleDC(hdc);
        ReleaseDC(hWnd, hdc);
        break;
     }
    case WM_SIZE:
    {
            int clientWidth = LOWORD(lParam);
            int clientHeight = HIWORD(lParam);
            HDC hdc = GetDC(hWnd);
            if (offOldBitmap != NULL) {
                SelectObject(offDC, offOldBitmap);
                
            }
            if (offBitmap != NULL) {
               DeleteObject(offBitmap);
              
            }
            offBitmap = CreateCompatibleBitmap(hdc, clientWidth, clientHeight);
            offOldBitmap = (HBITMAP)SelectObject(offDC, offBitmap);
            ReleaseDC(hWnd, hdc);
           
     }
     break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        HPEN pen = CreatePen(PS_SOLID, 2, RGB(164, 174, 196));
        HPEN oldPen = (HPEN)SelectObject(offDC, pen);
        if (result[hWnd] == 4)
        {
            EndPaint(hWnd, &ps);
            break;
        }
        if (result[hWnd] == 3)
        {
            EndPaint(hWnd, &ps);
            break;
        }

        RECT rc;
        GetClientRect(hWnd, &rc);

        HBRUSH backgroundBrush = CreateSolidBrush(RGB(255, 255, 255));
        RECT backgroundRect = { 0, 0, rc.right - rc.left, rc.bottom - rc.top };
        FillRect(offDC, &backgroundRect, backgroundBrush);
        DeleteObject(backgroundBrush);

        HFONT font = CreateFont(
            -MulDiv(20, GetDeviceCaps(offDC, LOGPIXELSY), 72), // Height
            0,
            0,
            0,
            FW_BOLD,
            false,
            FALSE,
            0,
            EASTEUROPE_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_SWISS,
            _T(" Verdana "));
        HFONT oldFont = (HFONT)SelectObject(offDC, font);

        SetBkMode(offDC, TRANSPARENT);

        std::string word = map[hWnd];

        if (v.size() == 1)
        {
            PrintPuzzle(offDC, 6, word, hWnd);
        }
        else  if (v.size() == 2)
        {
            PrintPuzzle(offDC, 8, word, hWnd);
        }
        else
        {
            PrintPuzzle(offDC, 10, word, hWnd);
        }

        std::string str = map[hWnd];
        TCHAR s[6];
        s[str.size()] = 0;
        std::copy(str.begin(), str.end(), s);
        _stprintf_s(buf, bufSize, _T("Poprawne haslo: %s"), s);
        SetWindowText(hWnd, buf);

        if (result[hWnd] == 0)
        {
            HBITMAP bitmap = LoadBitmap(hInst,
                MAKEINTRESOURCE(IDB_BITMAP1));
            HDC memDC = CreateCompatibleDC(offDC);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);
            BLENDFUNCTION bf = { AC_SRC_OVER, 0, 150, 0 };
            AlphaBlend(offDC, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, memDC, 0, 0, rc.right - rc.left, rc.bottom - rc.top, bf);
            SetTextColor(offDC, RGB(255, 255, 255));
            TCHAR s[6];
            s[map[hWnd].size()] = 0;
            for (int i = 0; i < map[hWnd].size(); ++i)
                s[i] = toupper(map[hWnd][i]);
            DrawText(offDC, s, 5, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
            SetTextColor(offDC, RGB(0,0,0));
            SelectObject(memDC, oldBitmap);
            DeleteObject(bitmap);
            result[hWnd] = 3;
        }
        if (result[hWnd] == 1)
        {
            HBITMAP bitmap = LoadBitmap(hInst,
                MAKEINTRESOURCE(IDB_BITMAP2));
            HDC memDC = CreateCompatibleDC(offDC);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);
            BLENDFUNCTION bf = { AC_SRC_OVER, 0, 150, 0 };
            AlphaBlend(offDC, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, memDC, 0, 0, rc.right - rc.left, rc.bottom - rc.top, bf);
            SetTextColor(offDC, RGB(255, 255, 255));
            DrawText(offDC, L"WYGRANA", 8, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
            SetTextColor(offDC, RGB(0, 0, 0));
            SelectObject(memDC, oldBitmap);
            DeleteObject(bitmap);
            result[hWnd] = 4;
        }


        BitBlt(hdc, 0, 0, rc.right, rc.bottom, offDC, 0, 0, SRCCOPY);
        SelectObject(offDC, oldFont);
        DeleteObject(font);
        SelectObject(offDC, oldPen);
        DeleteObject(pen);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
    {
        if (offOldBitmap != NULL) {
            SelectObject(offDC, offOldBitmap);
        
        }
        if (offDC != NULL) {
            DeleteDC(offDC);
        
        }
        if (offBitmap != NULL) {
            DeleteObject(offBitmap);
        
        }
        //PostQuitMessage(0);
    }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
void CreateEasyWindow(HWND hWnd)
{
    int height = 6 * FIELD + 5 * INTERVAL + 2 * 3 * MARGIN;
    int width = 5 * FIELD + 5 * INTERVAL + 3 * MARGIN;

    //https://forums.codeguru.com/showthread.php?500841-Set-my-window-position-at-screen-center
    int xPos = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    int yPos = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
    HWND h = CreateWindow(L"child_window", szTitle2, WS_VISIBLE,
        xPos, yPos, width, height, hWnd, NULL, NULL, NULL);
    v.push_back(h);
    int x = rand() % words.size();
    map[h] = words[x];
    result[h] = -1;
}
void CreateMediumWindow(HWND hWnd)
{
    int height = 8 * FIELD + 5 * INTERVAL + 2 * (MARGIN+25);
    int width = 5 * FIELD + 5 * INTERVAL + 3 * MARGIN;

    //https://forums.codeguru.com/showthread.php?500841-Set-my-window-position-at-screen-center
    int xPos = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    int yPos = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
    HWND h1 = CreateWindow(L"child_window", szTitle2, WS_VISIBLE,
        xPos - width, yPos, width, height, hWnd, NULL, NULL, NULL);
    v.push_back(h1);
    HWND h2 = CreateWindow(L"child_window", szTitle2, WS_VISIBLE,
        xPos + width, yPos, width, height, hWnd, NULL, NULL, NULL);
    v.push_back(h2);

    int x = rand() % words.size();
    map[h1] = words[x];
    int y = rand() % words.size();
    while (y == x)
        y = rand() % words.size();
    map[h2] = words[y];
    result[h1] = result[h2] = - 1;

}
void CreateHardWindow(HWND hWnd)
{
    int height = 10 * FIELD + 5 * INTERVAL + 2 * 4 * MARGIN;
    int width = 5 * FIELD + 5 * INTERVAL + 3 * MARGIN;

    //https://forums.codeguru.com/showthread.php?500841-Set-my-window-position-at-screen-center
    int xPos = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    int yPos = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
    HWND h[4];
    h[0] = CreateWindow(L"child_window", szTitle2, WS_VISIBLE,
        xPos - width, yPos - height/5, width, height, hWnd, NULL, NULL, NULL);
    h[2] = CreateWindow(L"child_window", szTitle2, WS_VISIBLE,
        xPos - width, yPos + height/5, width, height, hWnd, NULL, NULL, NULL);
    h[1] = CreateWindow(L"child_window", szTitle2, WS_VISIBLE,
        xPos + width, yPos - height/5, width, height, hWnd, NULL, NULL, NULL);
    h[3] = CreateWindow(L"child_window", szTitle2, WS_VISIBLE,
        xPos + width, yPos + height/5, width, height, hWnd, NULL, NULL, NULL);
    v.push_back(h[0]);
    v.push_back(h[1]);
    v.push_back(h[2]);
    v.push_back(h[3]);

    int x = rand() % words.size();
    map[h[0]] = words[x];
    int y = rand() % words.size();
    while (y == x)
        y = rand() % words.size();
    map[h[1]] = words[y];
    int z = rand() % words.size();
    while (z == x || z == y)
        z = rand() % words.size();
    map[h[2]] = words[z];
    int p = rand() % words.size();
    while (p == z || p == x || p == y)
        p = rand() % words.size();
    map[h[3]] = words[p];
    result[h[0]] = result[h[1]] = result[h[2]] = result[h[3]] = -1;
}

void PrintPuzzle(HDC hdc,int level,std::string word,HWND hWnd)
{
    for (int i = 0; i < level; ++i)
        for (int j = 0; j < 5; ++j)
        {
            HBRUSH brush, oldBrush;
            double x = MARGIN + j * (FIELD + INTERVAL);
            double y = MARGIN + i * (FIELD + INTERVAL);
            RECT rc = { x,y,x + FIELD,y + FIELD };
            int z = i * 5 + j;
            if (i >= yPos)
            {
                brush = CreateSolidBrush(RGB(251, 252, 255));
                oldBrush = (HBRUSH)SelectObject(hdc, brush);
            }
            else
            {
                if (word[j] == tolower(letters[z]))
                {
                    letterColors[std::make_pair(hWnd, letters[z])] = 1;
                    brush = CreateSolidBrush(RGB(121, 184, 81));
                    oldBrush = (HBRUSH)SelectObject(hdc, brush);
                }
                else if (word.find(tolower(letters[z])) != std::string::npos)
                {
                    letterColors[std::make_pair(hWnd, letters[z])] = 2;
                    brush = CreateSolidBrush(RGB(243, 194, 55));
                    oldBrush = (HBRUSH)SelectObject(hdc, brush);
                }
                else
                {
                    letterColors[std::make_pair(hWnd, letters[z])] = 3;
                    brush = CreateSolidBrush(RGB(164, 174, 196));
                    oldBrush = (HBRUSH)SelectObject(hdc, brush);
                }
            }
            RoundRect(hdc, x, y,
                x + FIELD,y + FIELD, 10, 10);
            if(z<letters.size())
                DrawText(hdc, &letters[z], 1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
            SelectObject(hdc, oldBrush);
            DeleteObject(brush);

        }
}
void CheckResults(std::string str)
{
    for (int i = 0; i < v.size(); ++i)
    {
        if (result[v[i]] != -1)
            continue;
        if (map[v[i]] == str)
        {
            result[v[i]] = 1;
        }
        else
        {
            if (yPos == 6 && v.size() == 1)
            {
                result[v[i]] = 0;
            }
            if (yPos == 8 && v.size() == 2)
            {
                result[v[i]] = 0;
            }
            if (yPos == 10 && v.size() == 4)
            {
                result[v[i]] = 0;
            }
        }
    }
}
void ColorKeboard(HDC hdc, HWND hWnd)
{
    WCHAR firstRow[] = { 'Q','W','E','R','T','Y','U','I','O','P' };
    WCHAR secondRow[] = { 'A','S','D','F','G','H','J','K','L' };
    WCHAR thirdRow[] = { 'Z','X','C','V','B','N','M' };
    SetBkMode(hdc, TRANSPARENT);


    RECT rc;
    GetClientRect(hWnd, &rc);
    SetDCBrushColor(hdc, RGB(251, 252, 255));

    for (int i = 0; i < 10; ++i)
    {
        double x = MARGIN + i * (FIELD + INTERVAL);
        double y = MARGIN;
        if (v.size() == 1)
        {
            ColorEasy(firstRow[i], x, y,hdc);
        }
        else if (v.size() == 2)
        {
            ColorMedium(firstRow[i], x, y,hdc);
        }
        else
        {
            ColorHard(firstRow[i], x, y,hdc);
        }
        RECT rc = { x,y,x + FIELD,y + FIELD };
        DrawText(hdc, &firstRow[i], 1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    }
    for (int i = 0; i < 9; ++i)
    {
        double x = MARGIN + i * (FIELD + INTERVAL) + FIELD / 2;
        double y = MARGIN + FIELD + INTERVAL;
        if (v.size() == 1)
        {
            ColorEasy(secondRow[i], x, y,hdc);
        }
        else if (v.size() == 2)
        {
            ColorMedium(secondRow[i], x, y,hdc);
        }
        else
        {
            ColorHard(secondRow[i], x, y,hdc);
        }
        RECT rc = { x,y,x + FIELD,y + FIELD };
        DrawText(hdc, &secondRow[i], 1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    }
    for (int i = 0; i < 7; ++i)
    {
        double x = MARGIN + i * (FIELD + INTERVAL) + FIELD / 2 + FIELD + INTERVAL;
        double y = MARGIN + 2 * FIELD + 2 * INTERVAL;
        if (v.size() == 1)
        {
            ColorEasy(thirdRow[i], x, y, hdc);
        }
        else if (v.size() == 2)
        {
            ColorMedium(thirdRow[i], x, y, hdc);
        }
        else
        {
            ColorHard(thirdRow[i], x, y, hdc);
        }
        RECT rc = { x,y,x + FIELD,y + FIELD };
        DrawText(hdc, &thirdRow[i], 1, &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
    }
}

void ColorEasy(char c,double x,double y,HDC hdc)
{
    int round = 0;
    HBRUSH brush, oldBrush;
    if (letterColors.find(std::make_pair(v[0], c)) != letterColors.end())
    {
        int z = letterColors[std::make_pair(v[0], c)];
        if (z == 1)
        {
            brush = CreateSolidBrush(RGB(121, 184, 81));
            oldBrush = (HBRUSH)SelectObject(hdc, brush);
        }
        else if (z == 2)
        {
            brush = CreateSolidBrush(RGB(243, 194, 55));
            oldBrush = (HBRUSH)SelectObject(hdc, brush);
        }
        else
        {
            brush = CreateSolidBrush(RGB(164, 174, 196));
            oldBrush = (HBRUSH)SelectObject(hdc, brush);
        }
    }
    else
    {
        brush = CreateSolidBrush(RGB(251, 252, 255));
        oldBrush = (HBRUSH)SelectObject(hdc, brush);
    }
    RoundRect(hdc, x, y,
        x+FIELD, y+FIELD, round, round);
    SelectObject(hdc, oldBrush);
    DeleteObject(brush);
}
void ColorMedium(char c, double x, double y,HDC hdc)
{
    int round=0;
    if (letterColors.find(std::make_pair(v[0], c)) == letterColors.end() && letterColors.find(std::make_pair(v[1], c)) == letterColors.end())
    {
        HBRUSH brush, oldBrush;
        HPEN pen, oldPen;
        pen = CreatePen(PS_SOLID, 2, RGB(164, 174, 196));
        oldPen = (HPEN)SelectObject(hdc, pen);
        brush = CreateSolidBrush(RGB(251, 252, 255));
        oldBrush = (HBRUSH)SelectObject(hdc, brush);
        RoundRect(hdc, x, y,
            x + FIELD, y + FIELD, round, round);
        SelectObject(hdc, oldBrush);
        DeleteObject(brush);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
        return;
    }
    for (int k = 0; k < v.size(); ++k)
    {
        HBRUSH brush, oldBrush;
        HPEN pen, oldPen;
        if (letterColors.find(std::make_pair(v[k], c)) != letterColors.end())
        {
            int x = letterColors[std::make_pair(v[k], c)];
            if (x == 1)
            {
                pen = CreatePen(PS_SOLID, 2, RGB(121, 184, 81));
                oldPen = (HPEN)SelectObject(hdc, pen);
                brush = CreateSolidBrush(RGB(121, 184, 81));
                oldBrush = (HBRUSH)SelectObject(hdc, brush);
            }
            else if (x == 2)
            {
                pen = CreatePen(PS_SOLID, 2, RGB(243, 194, 55));
                oldPen = (HPEN)SelectObject(hdc, pen);
                brush = CreateSolidBrush(RGB(243, 194, 55));
                oldBrush = (HBRUSH)SelectObject(hdc, brush);
            }
            else
            {
                pen = CreatePen(PS_SOLID, 2, RGB(164, 174, 196));
                oldPen = (HPEN)SelectObject(hdc, pen);
                brush = CreateSolidBrush(RGB(164, 174, 196));
                oldBrush = (HBRUSH)SelectObject(hdc, brush);
            }
        }
        else
        {
            pen = CreatePen(PS_SOLID, 2, RGB(164, 174, 196));
            oldPen = (HPEN)SelectObject(hdc, pen);
            brush = CreateSolidBrush(RGB(251, 252, 255));
            oldBrush = (HBRUSH)SelectObject(hdc, brush);
        }
        RoundRect(hdc, x+k*(FIELD/2),y,
            x + (k+1)*(FIELD/2), y+FIELD, round, round);
        SelectObject(hdc, oldBrush);
        DeleteObject(brush);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }
}
void ColorHard(char c, double x, double y, HDC hdc)
{
    int round = 0;
    if (letterColors.find(std::make_pair(v[0], c)) == letterColors.end() && letterColors.find(std::make_pair(v[1], c)) == letterColors.end()
        || letterColors.find(std::make_pair(v[2], c)) == letterColors.end() && letterColors.find(std::make_pair(v[3], c)) == letterColors.end())
    {
        SetDCPenColor(hdc, RGB(164, 174, 196));
        SetDCBrushColor(hdc, RGB(251, 252, 255));
        RoundRect(hdc, x, y,
            x + FIELD, y+FIELD, round, round);
        return;
    }
    for (int k = 0; k < v.size(); ++k)
    {
        HBRUSH brush, oldBrush;
        HPEN pen, oldPen;
        if (letterColors.find(std::make_pair(v[k], c)) != letterColors.end())
        {
            int x = letterColors[std::make_pair(v[k], c)];
            if (x == 1)
            {
                pen = CreatePen(PS_SOLID, 2, RGB(121, 184, 81));
                oldPen = (HPEN)SelectObject(hdc, pen);
                brush = CreateSolidBrush(RGB(121, 184, 81));
                oldBrush = (HBRUSH)SelectObject(hdc, brush);
            }
            else if (x == 2)
            {
                pen = CreatePen(PS_SOLID, 2, RGB(243, 194, 55));
                oldPen = (HPEN)SelectObject(hdc, pen);
                brush = CreateSolidBrush(RGB(243, 194, 55));
                oldBrush = (HBRUSH)SelectObject(hdc, brush);
            }
            else
            {
                pen = CreatePen(PS_SOLID, 2, RGB(164, 174, 196));
                oldPen = (HPEN)SelectObject(hdc, pen);
                brush = CreateSolidBrush(RGB(164, 174, 196));
                oldBrush = (HBRUSH)SelectObject(hdc, brush);
            }
        }
        else
        {
            pen = CreatePen(PS_SOLID, 2, RGB(164, 174, 196));
            oldPen = (HPEN)SelectObject(hdc, pen);
            brush = CreateSolidBrush(RGB(251, 252, 255));
            oldBrush = (HBRUSH)SelectObject(hdc, brush);
        }
        switch (k)
        {
        case 0:
        {
            RoundRect(hdc, x , y,
                x + (FIELD / 2), y + (FIELD/2), round, round);
            break;
        }
        case 1:
        {
            RoundRect(hdc, x + FIELD/2 ,y,
                x + FIELD, y + (FIELD / 2), round, round);
            break;
        }
        case 2:
        {
            RoundRect(hdc, x,y+FIELD/2,
                x + (FIELD / 2), y + FIELD, round, round);
            break;
        }
        case 3:
        {
            RoundRect(hdc, x+FIELD/2, y+FIELD/2,
                x + FIELD, y + FIELD, round, round);
            break;
        }
        }
        SelectObject(hdc, oldBrush);
        DeleteObject(brush);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }
}