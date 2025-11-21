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

#define MAX_WORDS 14000
#define WORD_LENGTH 5
#define _COLOR_GRAY   1
#define _COLOR_YELLOW 2
#define _COLOR_GREEN  3

typedef struct {
    char character;
    int color;
} LetterResult;

typedef struct {
    int letterCount[26];
} PickedWordSymbols;


DLL_EXPORT void LoadWords();
DLL_EXPORT bool IsInWordList(const char* word);
DLL_EXPORT char* PickRandomWord();
DLL_EXPORT void CheckEnteredWord(const char* inputWord, const char* pickedWord, LetterResult* result);

#ifdef __cplusplus
}
#endif

#endif // __MAIN_H__
