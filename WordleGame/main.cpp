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
#include "RulesDialog.h"
#include "ui_constants.h"
#include "colors.h"
#include <stdio.h>

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
HFONT CreateCustomFont(int height, bool bold, LPCTSTR fontName);
BOOL CALLBACK RulesDialogProcedure(HWND, UINT, WPARAM, LPARAM);
VOID paintBitmap(HWND hwnd, HBITMAP hBitmap, float xProp, float yProp);
VOID drawMenu(HWND hwnd, HBITMAP hWordleBitmap);
void drawGame(HWND hwnd, HDC hdc);
void drawKeyboard(HWND hwnd, int topY);
void RepositionKeyboard(HWND hwnd, int topY);
void drawLetters(HDC hdc, int cellSize, int startX, int startY);
void handleEnterButton(HWND hwnd);
void handleLetterButtons(int id);
void handleBackspaceButton();
void paintCells(HDC hdc, int startX, int startY, int cellSize);
VOID DrawTextAnywhere(
  HWND hwnd,
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

enum APP_STATE{
    STATE_MENU,
    STATE_GAME
};

struct KeyBtn {
    HWND hWnd;
    char label[8];
    int x, y, w, h;
};

KeyBtn keyboardButtons[30];
int keyboardBtnCount = 0;

char grid [6][6] = {0};
int cellColor[6][5] = {0};
int currentRow = 0;
int currentColumn = 0;
APP_STATE appState = STATE_MENU;


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
    wincl.hbrBackground = CreateSolidBrush(COLOR_BG_DEFAULT);

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
           LoadMenu(NULL, MAKEINTRESOURCE(IDM_MENU)),
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
    int x = (rcClient.right - MAIN_BTN_WIDTH) / 2;
    int y = (rcClient.bottom - MAIN_BTN_HEIGHT) / 2 + MAIN_BTN_Y_OFFSET;
    static HBITMAP hWordleBitmap = NULL;
    static HWND hPlayBtn = NULL;
    static HWND hRulesBtn = NULL;

    switch (message)                  /* handle the messages */
    {
        case WM_CREATE:


          hWordleBitmap = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_WORDLE_ICON));
          if(!hWordleBitmap){
            MessageBox(hwnd,"Failed to load Wordle Bitmap!", "Error", MB_ICONERROR);
          }
          // play button
          hPlayBtn = CreateWindow(
                "BUTTON", // predefined class; Unicode assumed
                "Play", // button text
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                x + (MAIN_BTN_WIDTH / 2), y, MAIN_BTN_WIDTH, MAIN_BTN_HEIGHT,
                hwnd, // parent window
                (HMENU) ID_PLAY_BUTTON,
                (HINSTANCE) GetWindowLong(hwnd, GWLP_HINSTANCE),
                NULL
            );

            // rules button
            hRulesBtn = CreateWindow(
                "BUTTON", // predefined class; Unicode assumed
                "Rules", // button text
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                x - (MAIN_BTN_WIDTH / 2), y, MAIN_BTN_WIDTH, MAIN_BTN_HEIGHT,
                hwnd, // parent window
                (HMENU) ID_RULES_BUTTON,
                (HINSTANCE) GetWindowLong(hwnd, GWLP_HINSTANCE),
                NULL
            );

            break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            if (appState == STATE_MENU) {
                drawMenu(hwnd, hWordleBitmap);
            } else if (appState == STATE_GAME) {
                drawGame(hwnd, hdc);
            }


            EndPaint(hwnd, &ps);
            break;
        }
        // moves objects when window is resized
        case WM_SIZE:
            {
                HWND hPlayBtn = GetDlgItem(hwnd, ID_PLAY_BUTTON);
                HWND hRulesBtn = GetDlgItem(hwnd, ID_RULES_BUTTON);
                if (hPlayBtn || hRulesBtn && appState == STATE_MENU){
                    x = (rcClient.right - MAIN_BTN_WIDTH) / 2;
                    y = (rcClient.bottom - MAIN_BTN_HEIGHT) / 2 + MAIN_BTN_Y_OFFSET;
                    MoveWindow(hPlayBtn, x + (MAIN_BTN_WIDTH / 2), y, MAIN_BTN_WIDTH, MAIN_BTN_HEIGHT, TRUE);
                    MoveWindow(hRulesBtn, x - (MAIN_BTN_WIDTH / 2), y, MAIN_BTN_WIDTH, MAIN_BTN_HEIGHT, TRUE);
                }
                if (appState == STATE_GAME) {
                    int cellSize = rcClient.bottom / 12;
                    int gridHeight = cellSize * 6;
                    int topY = cellSize + gridHeight + cellSize / 2;

                    RepositionKeyboard(hwnd, topY);
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
            {
                int id = LOWORD(wParam);
                switch(id){
                    case ID_FILE_EXIT:
                        SendMessage(hwnd, WM_CLOSE, 0, 0);
                        break;
                    case ID_PLAY_BUTTON:
                        appState = STATE_GAME;

                        ShowWindow(hPlayBtn, SW_HIDE);
                        ShowWindow(hRulesBtn, SW_HIDE);

                        InvalidateRect(hwnd, NULL, TRUE);
                        break;
                    case ID_RULES_BUTTON:
                        DialogBox(NULL, MAKEINTRESOURCE(IDD_RULES_DIALOG), hwnd, (DLGPROC)RulesDialogProcedure);
                        break;
                    case ID_READ_RULES:
                        DialogBox(NULL, MAKEINTRESOURCE(IDD_RULES_DIALOG), hwnd, (DLGPROC)RulesDialogProcedure);
                        break;
                    default:
                        // keyboard buttons
                        {
                            if (id >= 1000 && id < 1026)
                            {
                                handleLetterButtons(id);
                            }
                            else if (id == ID_BACKSPACE_BUTTON)
                            {
                                handleBackspaceButton();
                            }
                            else if (id == ID_ENTER_BUTTON)
                            {

                                handleEnterButton(hwnd);
                            }
                            InvalidateRect(hwnd, NULL, TRUE);
                            SetFocus(hwnd);
                        }
                        break;
                }
            }
            break;
            case WM_KEYDOWN:
                {
                    int vk = (int)wParam;
                    if (appState == STATE_GAME)
                    {
                        if (vk >= 'A' && vk <= 'Z')
                        {
                            char letter = (char)vk;
                            for (int i = 0; i < keyboardBtnCount; i++)
                            {
                                if (_stricmp(keyboardButtons[i].label, &letter) == 0)
                                {
                                    SendMessage(hwnd, WM_COMMAND, (1000 + i), 0);
                                    break;
                                }
                            }
                        }
                        if (vk == VK_RETURN)
                        {
                            SendMessage(hwnd, WM_COMMAND, ID_ENTER_BUTTON, 0);
                        }

                        if (vk == VK_BACK)
                        {
                            SendMessage(hwnd, WM_COMMAND, ID_BACKSPACE_BUTTON, 0);
                        }
                    }
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

void handleLetterButtons(int id){
    int index = id - 1000;
    char *label = keyboardButtons[index].label;
    char ch = label[0];

    if (currentColumn < 5){
        grid[currentRow][currentColumn] = ch;

        currentColumn++;

        grid[currentRow][currentColumn] = '\0';
    }
}

void handleBackspaceButton(){
    if (currentColumn > 0){
        currentColumn--;

        grid[currentRow][currentColumn] = '\0';
    }
}

void handleEnterButton(HWND hwnd){
    if (currentColumn < 5){
        MessageBoxA(hwnd, "Not enough letters!", "Wordle", MB_OK);
        return;
    }
    // TODO: insert logic for coloring letters
    for (int i = 0; i < 5; i++){
        cellColor[currentRow][i] = 1;
    }
    char msg[64];
    currentColumn = 0;
    currentRow++;
}

void drawGame(HWND hwnd, HDC hdc) {

    static bool isKeyboardCreated = false;

    RECT rc;
    GetClientRect(hwnd, &rc);

    int cellSize = rc.bottom / 12;
    int gridWidth = cellSize * 5;
    int gridHeight = cellSize * 6;

    int startX = (rc.right - gridWidth) / 2;
    int startY = rc.top + cellSize;

    paintCells(hdc,startX, startY, cellSize);

    drawLetters(hdc, cellSize, startX, startY);

    if (!isKeyboardCreated){
        drawKeyboard(hwnd, startY + gridHeight + cellSize/2);
        isKeyboardCreated = true;
    }
}

void paintCells(HDC hdc, int startX, int startY, int cellSize) {

    HPEN hPen = CreatePen(PS_SOLID, 5, COLOR_BLACK);

    HPEN oldPen = (HPEN)SelectObject(hdc, hPen);

    for (int r = 0; r < 6; r++) {
        for (int c = 0; c < 5; c++) {
            COLORREF fillColor = COLOR_BG_DEFAULT;
            switch (cellColor[r][c]) {
                case 1:
                    fillColor = COLOR_GREEN;
                    break;
                case 2:
                    fillColor = COLOR_YELLOW;
                    break;
                case 3:
                    fillColor = COLOR_GRAY;
                    break;
                default:
                    fillColor = COLOR_BG_DEFAULT;
                    break;
            }

            RECT rc;
            rc.left   = startX + c * cellSize;
            rc.top    = startY + r * cellSize;
            rc.right  = startX + (c + 1) * cellSize;
            rc.bottom = startY + (r + 1) * cellSize;

            Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);

            HBRUSH hFill = CreateSolidBrush(fillColor);
            FillRect(hdc, &rc, hFill);
            DeleteObject(hFill);
        }
    }

    SelectObject(hdc, oldPen);
    DeleteObject(hPen);
}

void drawLetters(HDC hdc, int cellSize, int startX, int startY){
    HFONT hFont = CreateFontA(
        cellSize * 0.6, 0, 0, 0,
        FW_BOLD, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        "Arial"
    );

    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, COLOR_BLACK);

    for (int r = 0; r < 6; r++) {
        for (int c = 0; c < 5; c++) {

            char letter = grid[r][c];
            if (letter == 0) continue;

            RECT cell = {
                startX + c * cellSize,
                startY + r * cellSize,
                startX + (c+1) * cellSize,
                startY + (r+1) * cellSize
            };

            DrawTextA(
                hdc,
                &letter, 1,
                &cell,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE
            );
        }
    }

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);

}

void drawKeyboard(HWND hwnd, int topY) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    static bool created = false;

    if (created) return;

    const char* rows[] = { "QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM" };

    int keyWidth = rc.right / 14;
    int keyHeight = keyWidth * 0.8;

    keyboardBtnCount = 0; // reset list

    for (int r = 0; r < 3; r++) {

        int rowLen = strlen(rows[r]);
        int totalKeys = rowLen;

        if (r == 2) totalKeys += 2;

        int totalWidth = totalKeys * keyWidth;
        int startX = (rc.right - totalWidth) / 2;
        int y = topY + r * (keyHeight + 5);

        int x = startX;

        // enter button
        if (r == 2) {
            keyboardButtons[ID_ENTER_BUTTON - 1000].hWnd =
                CreateWindow("BUTTON", "ENTER",
                    WS_CHILD | WS_VISIBLE,
                    x, y, keyWidth, keyHeight,
                    hwnd, (HMENU)ID_ENTER_BUTTON,
                    NULL, NULL);

            strcpy(keyboardButtons[ID_ENTER_BUTTON - 1000].label, "ENTER");
            x += keyWidth;
        }

        // letter keys
        for (int i = 0; i < rowLen; i++) {
            char txt[2] = { rows[r][i], 0 };

            keyboardButtons[keyboardBtnCount].hWnd =
                CreateWindow("BUTTON", txt,
                    WS_CHILD | WS_VISIBLE,
                    x, y, keyWidth, keyHeight,
                    hwnd, (HMENU)(1000 + keyboardBtnCount),
                    NULL, NULL);

            strcpy(keyboardButtons[keyboardBtnCount].label, txt);
            keyboardBtnCount++;

            x += keyWidth;
        }

        // backspace
        if (r == 2) {
            keyboardButtons[ID_BACKSPACE_BUTTON - 1000].hWnd =
                CreateWindow("BUTTON", "<-",
                    WS_CHILD | WS_VISIBLE,
                    x, y, keyWidth, keyHeight,
                    hwnd, (HMENU)ID_BACKSPACE_BUTTON,
                    NULL, NULL);

            strcpy(keyboardButtons[ID_BACKSPACE_BUTTON - 1000].label, "<-");
        }
        created = true;
    }
}


void RepositionKeyboard(HWND hwnd, int topY) {
    RECT rc;
    GetClientRect(hwnd, &rc);

    const char* rows[] = { "QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM" };

    int keyWidth  = rc.right / 14;
    int keyHeight = keyWidth * 0.8;

    int index = 0; // walks through letter buttons

    for (int r = 0; r < 3; r++) {

        int rowLen = strlen(rows[r]);
        int totalKeys = rowLen;

        if (r == 2) totalKeys += 2; // enter and backspace

        int totalWidth = totalKeys * keyWidth;
        int startX = (rc.right - totalWidth) / 2;
        int y = topY + r * (keyHeight + 5);

        int x = startX;

        // enter button
        if (r == 2) {
            MoveWindow(GetDlgItem(hwnd, ID_ENTER_BUTTON), x, y, keyWidth, keyHeight, TRUE);
            x += keyWidth;
        }

        // letter keys
        for (int i = 0; i < rowLen; i++) {
            MoveWindow(keyboardButtons[index].hWnd,x, y, keyWidth, keyHeight, TRUE);

            index++;
            x += keyWidth;
        }

        // backspace
        if (r == 2) {
            MoveWindow(GetDlgItem(hwnd, ID_BACKSPACE_BUTTON), x, y, keyWidth, keyHeight, TRUE);
        }
    }
}

HFONT CreateCustomFont(int height, bool bold, LPCTSTR fontName) {
    return CreateFont(
        -height, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, fontName
    );
}

VOID drawMenu(HWND hwnd, HBITMAP hWordleBitmap){
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);

    // draws wordle icon
    if (hWordleBitmap){
        paintBitmap(hwnd, hWordleBitmap, 0.5f, 0.19f);
    }
    // draws Wordle title
    DrawTextAnywhere(
        hwnd,
        _T("Wordle"),
        72, true, _T("Cascadia Code"),
        COLOR_TITLE, COLOR_BG_DEFAULT,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE,
        0, -0.2f
    );

    // draws short game description
    DrawTextAnywhere(
        hwnd,
        _T("Guess a 5-letter word\r\nin 6 guesses"),
        32, true, _T("Cascadia Code"),
        COLOR_DESCRIPTION, COLOR_BG_DEFAULT,
        DT_CENTER | DT_VCENTER,
        0, 0.5f
    );
}

// xProp - horizontal proportion
// yProp - vertical proportion
VOID paintBitmap(HWND hwnd, HBITMAP hBitmap, float xProp = 0.0f, float yProp = 0.0f){
    HDC hdc;
    hdc = GetDC(hwnd);

    RECT rcClient;
    GetClientRect(hwnd, &rcClient);

    HDC hdcMem = CreateCompatibleDC(hdc);;
    HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, hBitmap);

    BITMAP bm;
    GetObject(hBitmap, sizeof(bm), &bm);

    int clientWidth = rcClient.right - rcClient.left;
    int clientHeight = rcClient.bottom - rcClient.top;

    int bmpX = (int)(clientWidth * xProp) - (bm.bmWidth / 2);
    int bmpY = (int)(clientHeight * yProp) - (bm.bmHeight / 2);

    BitBlt(hdc, bmpX, bmpY, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

    SelectObject(hdcMem, hOld);
    DeleteDC(hdcMem);
    ReleaseDC(hwnd, hdc);
}

// xProp - horizontal proportion
// yProp - vertical proportion
VOID DrawTextAnywhere(
      HWND hwnd,
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
    HDC hdc;
    hdc = GetDC(hwnd);

    RECT rcClient;
    GetClientRect(hwnd, &rcClient);


    RECT rcText;
    HFONT hFont = CreateCustomFont(fontSize, bold, fontName);
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    SetBkColor(hdc, bkColor);
    SetTextColor(hdc, textColor);

    rcText = {0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top};
    DrawText(hdc, text, -1, &rcText, format | DT_CALCRECT);

    int textWidth = rcText.right - rcText.left;
    int textHeight = rcText.bottom - rcText.top;

    int clientWidth  = rcClient.right - rcClient.left;
    int clientHeight = rcClient.bottom - rcClient.top;


    // horizontal allignment
    UINT horizontal = format & (DT_LEFT | DT_CENTER | DT_RIGHT);
    if (horizontal == DT_CENTER) {
        int centerX = (clientWidth / 2) + (int)(textWidth * xProp);
        rcText.left   = centerX - textWidth / 2;
        rcText.right  = centerX + textWidth / 2;
    }
    else if (horizontal == DT_RIGHT) {
        rcText.right = rcClient.right + (int)(clientWidth * xProp);
        rcText.left  = rcText.right - textWidth;
    }
    else { // DT_LEFT or default (DT_LEFT == 0)
        rcText.left  = rcClient.left + (int)(clientWidth * xProp);
        rcText.right = rcText.left + textWidth;
    }

    // vertical allignment
    UINT vertical = format & (DT_TOP | DT_VCENTER | DT_BOTTOM);
    if (vertical == DT_VCENTER) {
        int centerY = (clientHeight / 2) + (int)(textHeight * yProp);
        rcText.top    = centerY - textHeight / 2;
        rcText.bottom = centerY + textHeight / 2;
    } else if (vertical == DT_BOTTOM) {
        rcText.bottom = rcClient.bottom + (int)(clientHeight * yProp);
        rcText.top    = rcText.bottom - textHeight;
    } else { // DT_TOP or default
        rcText.top    = rcClient.top + (int)(clientHeight * yProp);
        rcText.bottom = rcText.top + textHeight;
    }

    DrawText(hdc, text, -1, &rcText, format);

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
    ReleaseDC(hwnd, hdc);
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
                    hDialogBrush = CreateSolidBrush(COLOR_BG_DEFAULT);
                }

                // loads bitmaps
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
                HWND hCloseBtn = GetDlgItem(hDlg, ID_DIALOG_CLOSE_BUTTON);
                GetClientRect(hDlg, &rcClient);
                if (hCloseBtn){
                    int x = (rcClient.right - DIALOG_BTN_WIDTH) / 2;
                    int y = (rcClient.bottom - DIALOG_BTN_HEIGHT) / 2 + DIALOG_BTN_Y_OFFSET;
                    MoveWindow(hCloseBtn, x, y, DIALOG_BTN_WIDTH, DIALOG_BTN_HEIGHT, TRUE);
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
                    hDlg,
                    _T("How To Play"),
                    40, true, _T("Cascadia Code"),
                    COLOR_TITLE, COLOR_BG_DEFAULT,
                    DT_VCENTER | DT_SINGLELINE,
                    0.05f, -0.8f
                );

                DrawTextAnywhere(
                    hDlg,
                    _T("Guess the Wordle in 6 tries."),
                    24, false, _T("Cascadia Code"),
                    COLOR_TEXT_PRIMARY, COLOR_BG_DEFAULT,
                    DT_VCENTER | DT_SINGLELINE ,
                    0.05f, -0.7f
                );

                DrawTextAnywhere(
                    hDlg,
                    _T("• Each guess must be a valid 5-letter word.\r\n\n"
                       "• The color of the tiles will change to show\r\n  how close your guess was to the word."),
                    18, false, _T("Cascadia Code"),
                    COLOR_TEXT_PRIMARY, COLOR_BG_DEFAULT,
                    DT_VCENTER | DT_LEFT,
                    0.05f, -2.2f
                );

                DrawTextAnywhere(
                    hDlg,
                    _T("Examples"),
                    18, true, _T("Cascadia Code"),
                    COLOR_TITLE, COLOR_BG_DEFAULT,
                    DT_VCENTER | DT_SINGLELINE,
                    0.05f, -0.29f
                );

                if (hExample1) {
                    paintBitmap(hDlg, hExample1, 0.195f, 0.405f);
                }

                DrawTextAnywhere(
                    hDlg,
                    _T("W is in the word and in the correct spot."),
                    18, false, _T("Cascadia Code"),
                    COLOR_TEXT_PRIMARY, COLOR_BG_DEFAULT,
                    DT_VCENTER | DT_SINGLELINE,
                    0.05f, -0.11f
                );

                if (hExample2) {
                    paintBitmap(hDlg, hExample2, 0.195f, 0.51f);
                }

                DrawTextAnywhere(
                    hDlg,
                    _T("I is in the word but in the wrong spot."),
                    18, false, _T("Cascadia Code"),
                    COLOR_TEXT_PRIMARY, COLOR_BG_DEFAULT,
                    DT_VCENTER | DT_SINGLELINE,
                    0.05f, 0.09f
                );

                if (hExample3) {
                    paintBitmap(hDlg, hExample3, 0.195f, 0.61f);
                }

                DrawTextAnywhere(
                    hDlg,
                    _T("U is not in the word in any spot."),
                    18, false, _T("Cascadia Code"),
                    COLOR_TEXT_PRIMARY, COLOR_BG_DEFAULT,
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
                case ID_DIALOG_CLOSE_BUTTON:
                    {
                        EndDialog(hDlg, 0);
                    }
                    return TRUE;
            }
            return TRUE;
        case WM_CLOSE:
            /*EndDialog(hDlg, 0);
            if (hExample1) DeleteObject(hExample1);
            if (hExample2) DeleteObject(hExample2);
            if (hExample3) DeleteObject(hExample3);
            if (hDialogBrush) DeleteObject(hDialogBrush);
            */return TRUE;
        case WM_DESTROY:
            EndDialog(hDlg, 0);
            if (hExample1) DeleteObject(hExample1);
            if (hExample2) DeleteObject(hExample2);
            if (hExample3) DeleteObject(hExample3);
            //if (hDialogBrush) DeleteObject(hDialogBrush);
            return TRUE;
    }
    return FALSE;
}
