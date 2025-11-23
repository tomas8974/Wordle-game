#ifndef __MAIN_H__
#define __MAIN_H__

#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <tuple>

/*  To use this exported function of dll, include this header
 *  in your project.
 */

#ifdef BUILD_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT __declspec(dllimport)
#endif


#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_WORDS 20000
#define WORD_LENGTH 5
#define GUESS_NUMBER 6
#define _COLOR_GRAY   1
#define _COLOR_YELLOW 2
#define _COLOR_GREEN  3
#define _COLOR_BG  4
#define KEYBOARD_SIZE 26

typedef struct {
    char key[KEYBOARD_SIZE];
    int  color[KEYBOARD_SIZE];
} Keyboard;

typedef struct {
    char character;
    int color;
} LetterResult;

typedef struct {
    int letterCount[KEYBOARD_SIZE];
} PickedWordSymbols;

DLL_EXPORT void LoadWords();
DLL_EXPORT bool IsInWordList(const char* word);
DLL_EXPORT char* PickRandomWord();
DLL_EXPORT void updateKeyboard(Keyboard* kb, char character, int color);
DLL_EXPORT Keyboard initializeKeyboard();
DLL_EXPORT void CheckEnteredWord(const char* inputWord, const char* pickedWord, LetterResult* result, Keyboard* kb);

#ifdef __cplusplus
}
#endif

#endif // __MAIN_H__
