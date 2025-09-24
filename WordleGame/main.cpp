#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include "menu.h"
#include "bitmaps.h"
#include "buttons.h"
#include "dialog.h"

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
HFONT CreateCustomFont(int height, bool bold, LPCTSTR fontName);
BOOL CALLBACK RulesDialogProcedure(HWND, UINT, WPARAM, LPARAM);
VOID paintBitmap(HDC hdc, const RECT* rcClient, HBITMAP hBitmap, float xProp, float yProp);
VOID DrawTextAnywhere(
  HDC hdc,
  const RECT* rcClient,
  LPCTSTR text,
  int fontSize,
  bool bold,
  LPCTSTR fontName,
  COLORREF textColor,
  COLORREF bkColor,
  UINT format,
  float xProp,
  float yProp
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

          // play button
          CreateWindow(
                "BUTTON", // predefined class; Unicode assumed
                "Play", // button text
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                x + (btnWidth / 2), y, btnWidth, btnHeight,
                hwnd, // parent window
                (HMENU) PLAY_BUTTON,
                (HINSTANCE) GetWindowLong(hwnd, GWLP_HINSTANCE),
                NULL
            );

            // rules button
            CreateWindow(
                "BUTTON", // predefined class; Unicode assumed
                "Rules", // button text
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                x - (btnWidth / 2), y, btnWidth, btnHeight,
                hwnd, // parent window
                (HMENU) RULES_BUTTON,
                (HINSTANCE) GetWindowLong(hwnd, GWLP_HINSTANCE),
                NULL
            );

            break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // draws wordle icon
            if (hWordleBitmap) {
                paintBitmap(hdc, &rcClient , hWordleBitmap, 0.5f, 0.19f);
            }

            // draws Wordle title
            DrawTextAnywhere(
                hdc, &rcClient,
                _T("Wordle"),
                72, true, _T("Cascadia Code"),
                RGB(0,0,0), RGB(224,224,224),
                DT_CENTER | DT_VCENTER | DT_SINGLELINE,
                0, -0.2f
            );

            // draws short game description
            DrawTextAnywhere(
                hdc, &rcClient,
                _T("Guess a 5-letter word\r\nin 6 guesses"),
                32, true, _T("Cascadia Code"),
                RGB(0,0,0), RGB(224,224,224),
                DT_CENTER | DT_VCENTER,
                0, 0.5f
            );

            EndPaint(hwnd, &ps);
            break;
        }
        case WM_SIZE:
            {
                HWND hPlayBtn = GetDlgItem(hwnd, PLAY_BUTTON);
                HWND hRulesBtn = GetDlgItem(hwnd, RULES_BUTTON);
                if (hPlayBtn || hRulesBtn){
                    x = (rcClient.right - btnWidth) / 2;
                    y = (rcClient.bottom - btnHeight) / 2 + 200;
                    MoveWindow(hPlayBtn, x + (btnWidth / 2), y, btnWidth, btnHeight, TRUE);
                    MoveWindow(hRulesBtn, x - (btnWidth / 2), y, btnWidth, btnHeight, TRUE);
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        case WM_GETMINMAXINFO:
            {
                MINMAXINFO* pMinMax = (MINMAXINFO*)lParam;
                int screenWidth = GetSystemMetrics(SM_CXSCREEN);
                int screenHeight = GetSystemMetrics(SM_CYSCREEN);

                pMinMax->ptMinTrackSize.x = screenWidth * 0.5;
                pMinMax->ptMinTrackSize.y = screenHeight * 0.75;
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
                case RULES_BUTTON:
                    DialogBox(NULL, MAKEINTRESOURCE(RULES_DIALOG), hwnd, (DLGPROC)RulesDialogProcedure);
                    break;
                case ID_READ_RULES:
                    DialogBox(NULL, MAKEINTRESOURCE(RULES_DIALOG), hwnd, (DLGPROC)RulesDialogProcedure);
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
VOID paintBitmap(HDC hdc, const RECT* rcClient, HBITMAP hBitmap, float xProp = 0.0f, float yProp = 0.0f){
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, hBitmap);

    BITMAP bm;
    GetObject(hBitmap, sizeof(bm), &bm);

    int clientWidth = rcClient->right - rcClient->left;
    int clientHeight = rcClient->bottom - rcClient->top;

    int bmpX = (int)(clientWidth * xProp) - (bm.bmWidth / 2);
    int bmpY = (int)(clientHeight * yProp) - (bm.bmHeight / 2);

    BitBlt(hdc, bmpX, bmpY, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

    SelectObject(hdcMem, hOld);
    DeleteDC(hdcMem);
}

// xProp - horizontal proportion
// yProp - vertical proportion
VOID DrawTextAnywhere(
      HDC hdc,
      const RECT* rcClient,
      LPCTSTR text,
      int fontSize,
      bool bold,
      LPCTSTR fontName,
      COLORREF textColor,
      COLORREF bkColor,
      UINT format,
      float xProp = 0.0f,
      float yProp = 0.0f
    )
{
    RECT rcText;
    HFONT hFont = CreateCustomFont(fontSize, bold, fontName);
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    SetBkColor(hdc, bkColor);
    SetTextColor(hdc, textColor);

    rcText = {0, 0, rcClient->right - rcClient->left, rcClient->bottom - rcClient->top};
    DrawText(hdc, text, -1, &rcText, format | DT_CALCRECT);

    int textWidth = rcText.right - rcText.left;
    int textHeight = rcText.bottom - rcText.top;

    int clientWidth  = rcClient->right - rcClient->left;
    int clientHeight = rcClient->bottom - rcClient->top;


    // horizontal allignment
    UINT horizontal = format & (DT_LEFT | DT_CENTER | DT_RIGHT);
    if (horizontal == DT_CENTER) {
        int centerX = (clientWidth / 2) + (int)(textWidth * xProp);
        rcText.left   = centerX - textWidth / 2;
        rcText.right  = centerX + textWidth / 2;
    }
    else if (horizontal == DT_RIGHT) {
        rcText.right = rcClient->right + (int)(clientWidth * xProp);
        rcText.left  = rcText.right - textWidth;
    }
    else { // DT_LEFT or default (DT_LEFT == 0)
        rcText.left  = rcClient->left + (int)(clientWidth * xProp);
        rcText.right = rcText.left + textWidth;
    }

    // vertical allignment
    UINT vertical = format & (DT_TOP | DT_VCENTER | DT_BOTTOM);
    if (vertical == DT_VCENTER) {
        int centerY = (clientHeight / 2) + (int)(textHeight * yProp);
        rcText.top    = centerY - textHeight / 2;
        rcText.bottom = centerY + textHeight / 2;
    } else if (vertical == DT_BOTTOM) {
        rcText.bottom = rcClient->bottom + (int)(clientHeight * yProp);
        rcText.top    = rcText.bottom - textHeight;
    } else { // DT_TOP or default
        rcText.top    = rcClient->top + (int)(clientHeight * yProp);
        rcText.bottom = rcText.top + textHeight;
    }

    DrawText(hdc, text, -1, &rcText, format);

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

BOOL CALLBACK RulesDialogProcedure(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam){
    static HBRUSH hDialogBrush = NULL;
    static HBITMAP hExample1 = NULL;
    static HBITMAP hExample2 = NULL;
    static HBITMAP hExample3 = NULL;

    switch(uMsg){
        case WM_INITDIALOG:
            {
                // changes dialog background color
                if (!hDialogBrush){
                    hDialogBrush = CreateSolidBrush(RGB(224, 224, 224));
                }

                hExample1 = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_EXAMPLE_1));
                hExample2 = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_EXAMPLE_2));
                hExample3 = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_EXAMPLE_3));

                // changes dialogs position to the center
                RECT rc, rcOwner, rcClient;
                HWND hwndOwner = GetParent(hDlg);
                if (!hwndOwner){
                    hwndOwner = GetDesktopWindow();
                }
                GetWindowRect(hwndOwner, &rcOwner);
                GetWindowRect(hDlg, &rc);
                int dlgWidth = rc.right - rc.left;
                int dlgHeight = rc.bottom - rc.top;

                int x = rcOwner.left + ((rcOwner.right - rcOwner.left) - dlgWidth) / 2;
                int y = rcOwner.top + ((rcOwner.bottom - rcOwner.top) - dlgHeight) / 2;

                SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

                // changes close button position
                HWND hCloseBtn = GetDlgItem(hDlg, DIALOG_CLOSE_BUTTON);
                GetClientRect(hDlg, &rcClient);
                int btnWidth = 200;
                int btnHeight = 50;
                if (hCloseBtn){
                    int x = (rcClient.right - btnWidth) / 2;
                    int y = (rcClient.bottom - btnHeight) / 2 + 300;
                    MoveWindow(hCloseBtn, x, y, btnWidth, btnHeight, TRUE);
                }
            }
            return TRUE;
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hDlg, &ps);
                RECT rc;
                GetClientRect(hDlg, &rc);

                // draws Rules title
                DrawTextAnywhere(
                    hdc, &rc,
                    _T("How To Play"),
                    40, true, _T("Cascadia Code"),
                    RGB(0,0,0), RGB(224,224,224),
                    DT_VCENTER | DT_SINGLELINE,
                    0.05f, -0.8f
                );

                DrawTextAnywhere(
                    hdc, &rc,
                    _T("Guess the Wordle in 6 tries."),
                    24, false, _T("Cascadia Code"),
                    RGB(0,0,0), RGB(224,224,224),
                    DT_VCENTER | DT_SINGLELINE ,
                    0.05f, -0.7f
                );

                DrawTextAnywhere(
                    hdc, &rc,
                    _T("• Each guess must be a valid 5-letter word.\r\n\n"
                       "• The color of the tiles will change to show\r\n  how close your guess was to the word."),
                    18, false, _T("Cascadia Code"),
                    RGB(0,0,0), RGB(224,224,224),
                    DT_VCENTER | DT_LEFT,
                    0.05f, -2.2f
                );

                DrawTextAnywhere(
                    hdc, &rc,
                    _T("Examples"),
                    18, true, _T("Cascadia Code"),
                    RGB(0,0,0), RGB(224,224,224),
                    DT_VCENTER | DT_SINGLELINE,
                    0.05f, -0.29f
                );

                if (hExample1) {
                    paintBitmap(hdc, &rc , hExample1, 0.195f, 0.405f);
                }

                DrawTextAnywhere(
                    hdc, &rc,
                    _T("W is in the word and in the correct spot."),
                    18, false, _T("Cascadia Code"),
                    RGB(0,0,0), RGB(224,224,224),
                    DT_VCENTER | DT_SINGLELINE,
                    0.05f, -0.11f
                );

                if (hExample2) {
                    paintBitmap(hdc, &rc , hExample2, 0.195f, 0.51f);
                }

                DrawTextAnywhere(
                    hdc, &rc,
                    _T("I is in the word but in the wrong spot."),
                    18, false, _T("Cascadia Code"),
                    RGB(0,0,0), RGB(224,224,224),
                    DT_VCENTER | DT_SINGLELINE,
                    0.05f, 0.09f
                );

                if (hExample3) {
                    paintBitmap(hdc, &rc , hExample3, 0.195f, 0.61f);
                }

                DrawTextAnywhere(
                    hdc, &rc,
                    _T("U is not in the word in any spot."),
                    18, false, _T("Cascadia Code"),
                    RGB(0,0,0), RGB(224,224,224),
                    DT_VCENTER | DT_SINGLELINE,
                    0.05f, 0.29f
                );

                EndPaint(hDlg, &ps);
                break;
            }
            return TRUE;
        case WM_CTLCOLORDLG:
            return (INT_PTR)hDialogBrush;
        case WM_COMMAND:
            switch(LOWORD(wParam)){
                case DIALOG_CLOSE_BUTTON:
                    {
                        EndDialog(hDlg, 0);
                        if (hExample1) DeleteObject(hExample1);
                        if (hExample2) DeleteObject(hExample2);
                        if (hExample3) DeleteObject(hExample3);
                    }
                    return TRUE;
            }
            return TRUE;
        case WM_CLOSE:
            EndDialog(hDlg, 0);
            if (hExample1) DeleteObject(hExample1);
            if (hExample2) DeleteObject(hExample2);
            if (hExample3) DeleteObject(hExample3);
            return TRUE;
    }
    return FALSE;
}
