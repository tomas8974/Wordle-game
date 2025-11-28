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
#include "WordleGameLogic.h"

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
HFONT CreateCustomFont(int height, bool bold, LPCTSTR fontName);
BOOL CALLBACK RulesDialogProcedure(HWND, UINT, WPARAM, LPARAM);
void paintBitmap(HWND hwnd, HDC hdc, HBITMAP hBitmap, float xProp, float yProp);
void drawMenu(HWND hwnd, HDC hdc);
void drawGame(HWND hwnd, HDC hdc);
void drawKeyboard(HWND hwnd, int topY);
void RepositionKeyboard(HWND hwnd);
void drawLetters(HDC hdc, int cellSize, int startX, int startY);
void handleEnterButton(HWND hwnd);
void handleLetterButtons(int id);
const char* getButtonLabel(int id, int index);
COLORREF getButtonColor(int id, int index);
void paintButtonBorder(HDC hdc, RECT rc);
void paintButtonBackground(HDC hdc, RECT rc, COLORREF btnFill);
void handleBackspaceButton();
void paintButtonText(HDC hdc, RECT rc, const char* text);
void deleteKeyboard(HWND hwnd);
void drawGameFinished(HWND hwnd, HDC hdc);
void paintCells(HDC hdc, int startX, int startY, int cellSize);
void destroyButtons(HWND* buttons, int btnCount);
HWND createButton(HWND hwnd, char* text, int x, int y, int width, int height, int id);
HWND createKeyboardButton(HWND hwnd, char* text, int x, int y, int width, int height, int id);
void resetGame();
void RepositionUI(HWND hwnd);
void drawAfterGameMessage(HWND hwnd);
void DrawTextAnywhere(
  HWND hwnd,
  LPCTSTR text,
  int fontSize,
  bool bold,
  LPCTSTR fontName,
  UINT format,
  float xProp,
  float yProp
);
/*  Make the class name into a global variable  */
TCHAR szClassName[ ] = _T("CodeBlocksWindowsApp");

enum APP_STATE{
    STATE_MENU,
    STATE_GAME,
    STATE_GAME_FINISHED
};

struct KeyBtn {
    HWND hWnd;
    char label[8];
    int color;
    int x, y, w, h;
};

static const LPCSTR FONT_ARIAL = "Arial";
static const LPCSTR FONT_CASCADIA_CODE = "Cascadia Code";

KeyBtn keyboardButtons[KEYBOARD_SIZE + 1];
int keyboardBtnCount = 0;

char currentWord[GUESS_NUMBER][WORD_LENGTH + 1] = {0};
int cellColor[GUESS_NUMBER][WORD_LENGTH] = {0};
int currentRow = 0;
int currentColumn = 0;
APP_STATE appState = STATE_MENU;
HWND hPlayBtn = NULL;
HWND hRulesBtn = NULL;
HWND hReplayBtn = NULL;
HWND hBackBtn = NULL;
HBITMAP hWordleBitmap = NULL;
bool areMenuItemsCreated = false;
bool isKeyboardCreated = false;
bool finishedButtonsCreated = false;
char* selectedWord = NULL;
LetterResult result[WORD_LENGTH];
Keyboard keyboard;
bool gameWon = false;

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
    switch (message)                  /* handle the messages */
    {
        case WM_CREATE:
        {
            LoadWords();
        }
        break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            if (appState == STATE_MENU) {
                drawMenu(hwnd, hdc);
            }
            else if (appState == STATE_GAME) {
                drawGame(hwnd, hdc);
            }
            else if (appState == STATE_GAME_FINISHED){
                drawGameFinished(hwnd, hdc);
            }
            EndPaint(hwnd, &ps);
            break;
        }
        // moves objects when window is resized
        case WM_SIZE:
            {
                RepositionUI(hwnd);
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
                        {
                            resetGame();
                            HWND finishedButtons[] = { hPlayBtn, hRulesBtn };
                            destroyButtons(finishedButtons, 2);
                            InvalidateRect(hwnd, NULL, TRUE);
                        }
                        break;
                    case ID_MAIN_MENU_BUTTON:
                        {
                            if (isKeyboardCreated){
                                deleteKeyboard(hwnd);
                            }
                            appState = STATE_MENU;
                            HWND finishedButtons[] = { hReplayBtn, hBackBtn };
                            destroyButtons(finishedButtons, 2);
                            InvalidateRect(hwnd, NULL, TRUE);
                        }

                        break;
                    case ID_REPLAY_BUTTON:
                        {
                            resetGame();
                            HWND finishedButtons[] = { hReplayBtn, hBackBtn };
                            destroyButtons(finishedButtons, 2);
                            InvalidateRect(hwnd, NULL, TRUE);
                        }
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
            case WM_DRAWITEM:
            {
                DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
                int id = dis->CtlID;

                int index = id - 1000;
                if (id != ID_ENTER_BUTTON &&
                    id != ID_BACKSPACE_BUTTON &&
                    (index < 0 || index >= keyboardBtnCount))
                    break;

                const char* text = getButtonLabel(id, index);
                COLORREF color = getButtonColor(id, index);

                paintButtonBackground(dis->hDC, dis->rcItem, color);
                paintButtonBorder(dis->hDC, dis->rcItem);
                paintButtonText(dis->hDC, dis->rcItem, text);

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


void RepositionUI(HWND hwnd)
{
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);

    int x = (rcClient.right - MAIN_BTN_WIDTH) / 2;
    int y = (rcClient.bottom - MAIN_BTN_HEIGHT) / 2 + MAIN_BTN_Y_OFFSET;

    switch (appState)
    {
        case STATE_MENU:
        {
            if (hPlayBtn && hRulesBtn)
            {
                MoveWindow(hPlayBtn, x + (MAIN_BTN_WIDTH / 2), y,
                           MAIN_BTN_WIDTH, MAIN_BTN_HEIGHT, TRUE);

                MoveWindow(hRulesBtn, x - (MAIN_BTN_WIDTH / 2), y,
                           MAIN_BTN_WIDTH, MAIN_BTN_HEIGHT, TRUE);
            }
        }
        break;

        case STATE_GAME:
        {
            RepositionKeyboard(hwnd);
        }
        break;

        case STATE_GAME_FINISHED:
        {
            if (hReplayBtn && hBackBtn)
            {
                MoveWindow(hReplayBtn, x + (MAIN_BTN_WIDTH / 2), y,
                           MAIN_BTN_WIDTH, MAIN_BTN_HEIGHT, TRUE);

                MoveWindow(hBackBtn, x - (MAIN_BTN_WIDTH / 2), y,
                           MAIN_BTN_WIDTH, MAIN_BTN_HEIGHT, TRUE);
            }
        }
        break;
    }
}

void handleLetterButtons(int id){
    int index = id - 1000;
    char *label = keyboardButtons[index].label;
    char ch = label[0];

    if (currentColumn < WORD_LENGTH){
        currentWord[currentRow][currentColumn] = ch;

        currentColumn++;

        currentWord[currentRow][currentColumn] = '\0';
    }
}

void handleBackspaceButton(){
    if (currentColumn > 0){
        currentColumn--;

        currentWord[currentRow][currentColumn] = '\0';
    }
}

void handleEnterButton(HWND hwnd){
    if (currentColumn < WORD_LENGTH){
        MessageBoxA(hwnd, "Not enough letters!", "Wordle", MB_OK);
        return;
    }
    if (!IsInWordList(currentWord[currentRow])){
        MessageBoxA(hwnd, "Word is not in the word list!", "Wordle", MB_OK);
        return;
    }
    CheckEnteredWord(currentWord[currentRow], selectedWord, result, &keyboard);
    for (int i = 0; i < WORD_LENGTH; i++){
        cellColor[currentRow][i] = result[i].color;
    }
    if (strcmp(currentWord[currentRow], selectedWord) == 0 && currentRow < GUESS_NUMBER){;
        gameWon = true;
    }
    else if (currentRow < GUESS_NUMBER - 1){
        currentColumn = 0;
        currentRow++;
        return;
    }
    InvalidateRect(hwnd, NULL, TRUE);
    appState = STATE_GAME_FINISHED;
    deleteKeyboard(hwnd);
}

void drawGame(HWND hwnd, HDC hdc) {
    RECT rc;
    GetClientRect(hwnd, &rc);

    int cellSize = rc.bottom / 12;
    int gridWidth = cellSize * WORD_LENGTH;
    int gridHeight = cellSize * GUESS_NUMBER;

    int startX = (rc.right - gridWidth) / 2;
    int startY = rc.top + cellSize;

    paintCells(hdc,startX, startY, cellSize);
    drawLetters(hdc, cellSize, startX, startY);

    if (!isKeyboardCreated){
        drawKeyboard(hwnd, startY + gridHeight + cellSize/2);
    }
}

void paintCells(HDC hdc, int startX, int startY, int cellSize) {

    HPEN hPen = CreatePen(PS_SOLID, 5, COLOR_BLACK);

    HPEN oldPen = (HPEN)SelectObject(hdc, hPen);

    for (int r = 0; r < GUESS_NUMBER; r++) {
        for (int c = 0; c < WORD_LENGTH; c++) {
            COLORREF fillColor = COLOR_BG_DEFAULT;
            switch (cellColor[r][c]) {
                case _COLOR_GRAY:
                    fillColor = COLOR_GRAY;
                    break;
                case _COLOR_YELLOW:
                    fillColor = COLOR_YELLOW;
                    break;
                case _COLOR_GREEN:
                    fillColor = COLOR_GREEN;
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

void drawGameFinished(HWND hwnd, HDC hdc){

    RECT rc;
    GetClientRect(hwnd, &rc);

    int cellSize = rc.bottom / 12;
    int gridWidth = cellSize * WORD_LENGTH;
    int gridHeight = cellSize * GUESS_NUMBER;

    int startX = (rc.right - gridWidth) / 2;
    int startY = rc.top + cellSize;

    paintCells(hdc,startX, startY, cellSize);

    drawLetters(hdc, cellSize, startX, startY);

    drawAfterGameMessage(hwnd);

    if (!finishedButtonsCreated) {
        int x = (rc.right - MAIN_BTN_WIDTH) / 2;
        int y = (rc.bottom - MAIN_BTN_HEIGHT) / 2 + MAIN_BTN_Y_OFFSET;
        hReplayBtn = createButton(hwnd, "Play again", x + (MAIN_BTN_WIDTH / 2), y, MAIN_BTN_WIDTH, MAIN_BTN_HEIGHT, ID_REPLAY_BUTTON);
        hBackBtn = createButton(hwnd, "Back to Main Menu", x - (MAIN_BTN_WIDTH / 2), y, MAIN_BTN_WIDTH, MAIN_BTN_HEIGHT, ID_MAIN_MENU_BUTTON);
        finishedButtonsCreated = true;
    }
}
void drawAfterGameMessage(HWND hwnd){
    char msg[128];
    if (gameWon){
        sprintf(msg, "You won!\r\nThe word was: %s", selectedWord);
    }
    else{
        sprintf(msg, "You Lost!\r\nThe word was: %s", selectedWord);
    }
    DrawTextAnywhere(hwnd, msg, 28, true, FONT_ARIAL, DT_CENTER, 0, 0.6);
}
void resetGame(){
    if (selectedWord != NULL)
    {
        free(selectedWord);
    }
    memset(currentWord, 0, sizeof(currentWord));
    memset(cellColor, 0, sizeof(cellColor));
    currentRow = 0;
    currentColumn = 0;
    gameWon = false;

    appState = STATE_GAME;

    areMenuItemsCreated = false;
    finishedButtonsCreated = false;
    keyboard = initializeKeyboard();
    selectedWord = PickRandomWord();
}

HWND createButton(HWND hwnd, char* text, int x, int y, int width, int height, int id){
    HWND hButton = CreateWindow(
        "BUTTON", // predefined class; Unicode assumed
        text, // button text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        x, y, width, height,
        hwnd, // parent window
        (HMENU)id,
        NULL,
        NULL
    );
    return hButton;
}

HWND createKeyboardButton(HWND hwnd, char* text, int x, int y, int width, int height, int id){
    HWND hButton = CreateWindow(
        "BUTTON", // predefined class; Unicode assumed
        text, // button text
        WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
        x, y, width, height,
        hwnd, // parent window
        (HMENU)id,
        NULL,
        NULL
    );
    return hButton;
}
void drawLetters(HDC hdc, int cellSize, int startX, int startY){
    HFONT hFont = CreateFontA(
        cellSize * 0.6, 0, 0, 0,
        FW_BOLD, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        FONT_ARIAL
    );

    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, COLOR_BLACK);

    for (int r = 0; r < GUESS_NUMBER; r++) {
        for (int c = 0; c < WORD_LENGTH; c++) {

            char letter = currentWord[r][c];
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

    if (isKeyboardCreated) return;

    const char* rows[] = { "QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM" };
    size_t rows_size = sizeof(rows) / sizeof(rows[0]);

    int keyWidth = rc.right / 14;
    int keyHeight = keyWidth * 0.8;

    keyboardBtnCount = 0; // reset list

    for (int r = 0; r < rows_size; r++) {

        int rowLen = strlen(rows[r]);
        int totalKeys = rowLen;

        if (r == 2) totalKeys += 2;

        int totalWidth = totalKeys * keyWidth;
        int startX = (rc.right - totalWidth) / 2;
        int y = topY + r * (keyHeight + 5);

        int x = startX;

        // enter button
        if (r == 2) {
            createKeyboardButton(hwnd, "ENTER", x, y, keyWidth, keyHeight, ID_ENTER_BUTTON);
            x += keyWidth;
        }

        // letter keys
        for (int i = 0; i < rowLen; i++) {
            char txt[2] = { rows[r][i], 0 };
            keyboardButtons[keyboardBtnCount].hWnd = createKeyboardButton(hwnd, txt, x, y, keyWidth, keyHeight, 1000 + keyboardBtnCount);
            strcpy(keyboardButtons[keyboardBtnCount].label, txt);
            keyboardBtnCount++;
            x += keyWidth;
        }

        // backspace
        if (r == 2) {
            createKeyboardButton(hwnd, "<-", x, y, keyWidth, keyHeight, ID_BACKSPACE_BUTTON);
        }
    }
    isKeyboardCreated = true;
}

const char* getButtonLabel(int id, int index) {
    if (id == ID_ENTER_BUTTON) return "ENTER";
    if (id == ID_BACKSPACE_BUTTON) return "<-";
    return keyboardButtons[index].label;
}

COLORREF getButtonColor(int id, int index) {
    if (id == ID_ENTER_BUTTON || id == ID_BACKSPACE_BUTTON)
        return COLOR_WHITISH;

    char key = keyboardButtons[index].label[0];
    int colorCode = _COLOR_BG;

    for (int i = 0; i < KEYBOARD_SIZE; i++) {
        if (keyboard.key[i] == key) {
            colorCode = keyboard.color[i];
            break;
        }
    }
    switch (colorCode) {
        case _COLOR_GRAY:
            return COLOR_GRAY;
        case _COLOR_YELLOW:
            return COLOR_YELLOW;
        case _COLOR_GREEN:
            return COLOR_GREEN;
        default:
            return COLOR_WHITISH;
    }
}

void paintButtonBackground(HDC hdc, RECT rc, COLORREF btnFill) {
    HBRUSH brush = CreateSolidBrush(btnFill);
    FillRect(hdc, &rc, brush);
    DeleteObject(brush);
}

void paintButtonBorder(HDC hdc, RECT rc) {
    HPEN hPen = CreatePen(PS_SOLID, 2, COLOR_BLACK);
    HPEN oldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

    Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(hPen);
}

void paintButtonText(HDC hdc, RECT rc, const char* text) {
    SetBkMode(hdc, TRANSPARENT);

    int height = (rc.bottom - rc.top) * 0.3;
    HFONT hFont = CreateFontA(height, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                              ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                              CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                              DEFAULT_PITCH | FF_DONTCARE, FONT_ARIAL);

    HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

    DrawTextA(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, oldFont);
    DeleteObject(hFont);
}

void RepositionKeyboard(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);

    int cellSize = rc.bottom / 12;
    int gridHeight = cellSize * GUESS_NUMBER;
    int topY = cellSize + gridHeight + cellSize / 2;

    const char* rows[] = { "QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM" };
    size_t rowsSize = sizeof(rows) / sizeof(rows[0]);


    int keyWidth  = rc.right / 14;
    int keyHeight = keyWidth * 0.8;

    int index = 0;

    for (int r = 0; r < rowsSize; r++) {

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

void destroyButtons(HWND* buttons, int btnCount) {
    for (int i = 0; i < btnCount; i++) {
        if (buttons[i]) {
            DestroyWindow(buttons[i]);
            buttons[i] = NULL;
        }
    }
}

void deleteKeyboard(HWND hwnd) {
    for (int i = 0; i < keyboardBtnCount; i++) {
        if (keyboardButtons[i].hWnd) {
            DestroyWindow(keyboardButtons[i].hWnd);
            keyboardButtons[i].hWnd = NULL;
        }
    }

    HWND hEnterBtn = GetDlgItem(hwnd, ID_ENTER_BUTTON);
    HWND hBackSpaceBtn = GetDlgItem(hwnd, ID_BACKSPACE_BUTTON);
    if (hEnterBtn) {
        DestroyWindow(hEnterBtn);
    }
    if (hBackSpaceBtn) {
        DestroyWindow(hBackSpaceBtn);
    }

    isKeyboardCreated = false;
    keyboardBtnCount = 0;
}


HFONT CreateCustomFont(int height, bool bold, LPCTSTR fontName) {
    return CreateFont(
        -height, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, fontName
    );
}

void drawMenu(HWND hwnd, HDC hdc){
    RECT rcClient;
    GetClientRect(hwnd, &rcClient);

    if (!areMenuItemsCreated){
        int x = (rcClient.right - MAIN_BTN_WIDTH) / 2;
        int y = (rcClient.bottom - MAIN_BTN_HEIGHT) / 2 + MAIN_BTN_Y_OFFSET;

        // loads wordle icon
        hWordleBitmap = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_WORDLE_ICON));
          if(!hWordleBitmap){
            MessageBox(hwnd,"Failed to load Wordle Bitmap!", "Error", MB_ICONERROR);
          }

        // creates buttons
        hPlayBtn = createButton(hwnd, "Play", x + (MAIN_BTN_WIDTH / 2), y, MAIN_BTN_WIDTH, MAIN_BTN_HEIGHT, ID_PLAY_BUTTON);
        hRulesBtn = createButton(hwnd, "Rules", x - (MAIN_BTN_WIDTH / 2), y, MAIN_BTN_WIDTH, MAIN_BTN_HEIGHT, ID_RULES_BUTTON);

        areMenuItemsCreated = true;
    }

    // draws wordle icon
    if (hWordleBitmap){
        paintBitmap(hwnd, hdc, hWordleBitmap, 0.5f, 0.19f);
    }
    // draws Wordle title
    DrawTextAnywhere(
        hwnd,
        _T("Wordle"),
        72, true, FONT_CASCADIA_CODE,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE,
        0, -0.2f
    );

    // draws short game description
    DrawTextAnywhere(
        hwnd,
        _T("Guess a 5-letter word\r\nin 6 guesses"),
        32, true, FONT_CASCADIA_CODE,
        DT_CENTER | DT_VCENTER,
        0, 0.5f
    );
}

// xProp - horizontal proportion
// yProp - vertical proportion
void paintBitmap(HWND hwnd, HDC hdc, HBITMAP hBitmap, float xProp = 0.0f, float yProp = 0.0f){
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
}

// xProp - horizontal proportion
// yProp - vertical proportion
void DrawTextAnywhere(
      HWND hwnd,
      LPCTSTR text,
      int fontSize,
      bool bold,
      LPCTSTR fontName,
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

    SetBkColor(hdc, COLOR_BG_DEFAULT);
    SetTextColor(hdc, COLOR_BLACK);

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
                    40, true, FONT_CASCADIA_CODE,
                    DT_VCENTER | DT_SINGLELINE,
                    0.05f, -0.8f
                );

                DrawTextAnywhere(
                    hDlg,
                    _T("Guess the Wordle in 6 tries."),
                    24, false, FONT_CASCADIA_CODE,
                    DT_VCENTER | DT_SINGLELINE ,
                    0.05f, -0.7f
                );

                DrawTextAnywhere(
                    hDlg,
                    _T("• Each guess must be a valid 5-letter word.\r\n\n"
                       "• The color of the tiles will change to show\r\n  how close your guess was to the word."),
                    18, false, FONT_CASCADIA_CODE,
                    DT_VCENTER | DT_LEFT,
                    0.05f, -2.2f
                );

                DrawTextAnywhere(
                    hDlg,
                    _T("Examples"),
                    18, true, FONT_CASCADIA_CODE,
                    DT_VCENTER | DT_SINGLELINE,
                    0.05f, -0.29f
                );

                if (hExample1) {
                    paintBitmap(hDlg, hdc, hExample1, 0.195f, 0.405f);
                }

                DrawTextAnywhere(
                    hDlg,
                    _T("W is in the word and in the correct spot."),
                    18, false, FONT_CASCADIA_CODE,
                    DT_VCENTER | DT_SINGLELINE,
                    0.05f, -0.11f
                );

                if (hExample2) {
                    paintBitmap(hDlg, hdc, hExample2, 0.195f, 0.51f);
                }

                DrawTextAnywhere(
                    hDlg,
                    _T("I is in the word but in the wrong spot."),
                    18, false, FONT_CASCADIA_CODE,
                    DT_VCENTER | DT_SINGLELINE,
                    0.05f, 0.09f
                );

                if (hExample3) {
                    paintBitmap(hDlg, hdc, hExample3, 0.195f, 0.61f);
                }

                DrawTextAnywhere(
                    hDlg,
                    _T("U is not in the word in any spot."),
                    18, false, FONT_CASCADIA_CODE,
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
            return TRUE;
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
