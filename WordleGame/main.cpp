#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include "menu.h"
#include "bitmaps.h"

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
HFONT CreateCustomFont(int height, bool bold, LPCTSTR fontName);
void paintBitmapInTheCenter(HDC hdc, const RECT* rcClient, HBITMAP hBitmap, float xProp, float yProp);
void DrawAnyText(
  HDC hdc,
  const RECT* rcClient,
  LPCTSTR text,
  int fontSize,
  bool bold,
  LPCTSTR fontName,
  COLORREF textColor,
  COLORREF bkColor,
  int xOffset,
  int yOffset,
  UINT format
);
/*  Make the class name into a global variable  */
TCHAR szClassName[ ] = _T("CodeBlocksWindowsApp");

HBITMAP hWordleBitmap = NULL;

int WINAPI WinMain (HINSTANCE hThisInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR lpszArgument,
                     int nCmdShow)
{
    HWND hwnd;               /* This is the handle for our window */
    MSG messages;            /* Here messages to the application are saved */
    WNDCLASSEX wincl;        /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
    wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
    wincl.cbSize = sizeof (WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;                 /* No menu */
    wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;                      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = CreateSolidBrush(RGB(224, 224, 224));

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx (&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx (
           0,                   /* Extended possibilites for variation */
           szClassName,         /* Classname */
           _T("Wordle"),       /* Title Text */
           WS_OVERLAPPEDWINDOW  , /* default window */
           CW_USEDEFAULT,       /* Windows decides the position */
           CW_USEDEFAULT,       /* where the window ends up on the screen */
           screenWidth,                 /* The programs width */
           screenHeight,                 /* and height in pixels */
           HWND_DESKTOP,        /* The window is a child-window to desktop */
           LoadMenu(NULL, MAKEINTRESOURCE(IDR_MYMENU)),
           hThisInstance,       /* Program Instance handler */
           NULL                 /* No Window Creation data */
           );

    /* Make the window visible on the screen */
    ShowWindow (hwnd, SW_MAXIMIZE);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (GetMessage (&messages, NULL, 0, 0))
    {
        /* Translate virtual-key messages into character messages */
        TranslateMessage(&messages);
        /* Send message to WindowProcedure */
        DispatchMessage(&messages);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);

    int btnWidth = 200;
    int btnHeight = 50;
    int x = (rcClient.right - btnWidth) / 2;
    int y = (rcClient.bottom - btnHeight) / 2 + 200;

    switch (message)                  /* handle the messages */
    {
        case WM_CREATE:

          hWordleBitmap = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_WORDLE_ICON));
          if (!hWordleBitmap){
            MessageBox(hwnd, _T("Failed to load wordle bitmap!"), _T("Error"), MB_ICONERROR);
          }

          CreateWindow(
                "BUTTON", // predefined class; Unicode assumed
                "Play", // button text
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                x, y, btnWidth, btnHeight,
                hwnd, // parent window
                (HMENU) PLAY_BUTTON,
                (HINSTANCE) GetWindowLong(hwnd, GWLP_HINSTANCE),
                NULL
            ); // pointer, not needed

            break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // draws wordle icon
            if (hWordleBitmap) {
                paintBitmapInTheCenter(hdc, &rcClient , hWordleBitmap, 0.5f, 0.2f);
            }

            // draws Wordle title
            DrawAnyText(
                hdc, &rcClient,
                _T("Wordle"),
                72, true, _T("Cascadia Code"),
                RGB(0,0,0), RGB(224,224,224),
                0, -150,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE
            );

            // draws short game description
            DrawAnyText(
                hdc, &rcClient,
                _T("Guess a 5-letter word\r\nin 6 guesses"),
                32, true, _T("Cascadia Code"),
                RGB(0,0,0), RGB(224,224,224),
                0, 0,
                DT_CENTER | DT_VCENTER
            );

            EndPaint(hwnd, &ps);
            break;
        }
        case WM_SIZE:
            {
                HWND hBtn = GetDlgItem(hwnd, PLAY_BUTTON);
                if (hBtn){
                    x = (rcClient.right - btnWidth) / 2;
                    y = (rcClient.bottom - btnHeight) / 2 + 200;
                    MoveWindow(hBtn, x, y, btnWidth, btnHeight, TRUE);
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        case WM_COMMAND:
            switch(LOWORD(wParam)){
                case ID_FILE_EXIT:
                    SendMessage(hwnd, WM_CLOSE, 0, 0);
                    break;
                case PLAY_BUTTON:
                    MessageBox(hwnd, "You started playing Wordle!", "Hooray!", MB_OK);
                    break;
            }
            break;


        case WM_DESTROY:
            if (hWordleBitmap) DeleteObject(hWordleBitmap);
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}

HFONT CreateCustomFont(int height, bool bold, LPCTSTR fontName) {
    return CreateFont(
        -height, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, fontName
    );
}

// xProp - horizontal proportion
// yProp - vertical proportion
void paintBitmapInTheCenter(HDC hdc, const RECT* rcClient, HBITMAP hBitmap, float xProp, float yProp){
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, hBitmap);

    BITMAP bm;
    GetObject(hBitmap, sizeof(bm), &bm);

    /*int bmpX = (rcClient->right - bm.bmWidth) / 2 + xOffset;
    int bmpY = (rcClient->bottom - bm.bmHeight) / 2 + yOffset;*/

    int clientWidth = rcClient->right - rcClient->left;
    int clientHeight = rcClient->bottom - rcClient->top;

    int bmpX = (int)(clientWidth * xProp) - (bm.bmWidth / 2);
    int bmpY = (int)(clientHeight * yProp) - (bm.bmHeight / 2);

    BitBlt(hdc, bmpX, bmpY, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

    SelectObject(hdcMem, hOld);
    DeleteDC(hdcMem);
}

// xOffset - horizontal offset
// yOffset - vertical offset
void DrawAnyText(
      HDC hdc,
      const RECT* rcClient,
      LPCTSTR text,
      int fontSize,
      bool bold,
      LPCTSTR fontName,
      COLORREF textColor,
      COLORREF bkColor,
      int xOffset,
      int yOffset,
      UINT format
    )
{

    RECT rcText;
    HFONT hFont = CreateCustomFont(fontSize, bold, fontName);
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    SetBkColor(hdc, bkColor);
    SetTextColor(hdc, textColor);


    // center multiline text
    if (!(format & DT_SINGLELINE) && (format & DT_VCENTER)) {
        rcText = {0, 0, rcClient->right - rcClient->left, rcClient->bottom - rcClient->top};
        DrawText(hdc, text, -1, &rcText, format | DT_CALCRECT);

        int rectWidth = rcText.right - rcText.left;
        int rectHeight = rcText.bottom - rcText.top;

        int centerX = (rcClient->left + rcClient->right) / 2 + xOffset;
        int centerY = (rcClient->top + rcClient->bottom) / 2 + yOffset;

        rcText.left   = centerX - rectWidth / 2;
        rcText.right  = centerX + rectWidth / 2;
        rcText.top    = centerY - rectHeight / 2;
        rcText.bottom = centerY + rectHeight / 2;
    }
    else {
        rcText = *rcClient;
        rcText.left  += xOffset;
        rcText.right += xOffset;
        rcText.top   += yOffset;
        rcText.bottom += yOffset;
    }

    DrawText(hdc, text, -1, &rcText, format);

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}
