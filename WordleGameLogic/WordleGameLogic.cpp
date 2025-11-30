#include "WordleGameLogic.h"
#include <cstring>
#include <cstdlib>
#include <ctime>

// a sample exported function

bool IsInWordList(const char* word, char words[][WORD_LENGTH + 1], int wordCount)
{
    for (int i = 0; i < wordCount; i++)
    {
        if (_stricmp(words[i], word) == 0)
            return true;
    }
    return false;
}

char* PickRandomWord(char words[][WORD_LENGTH + 1], int wordCount) {
    if (wordCount == 0) return NULL;

    srand((unsigned)time(NULL));
    int i = rand() % wordCount;

    char* buffer = (char*)malloc(WORD_LENGTH + 1);
    if (!buffer) return NULL;

    strncpy(buffer, words[i], WORD_LENGTH);
    buffer[WORD_LENGTH] = '\0';

    return buffer;
}

Keyboard initializeKeyboard(){
    static const char layout[KEYBOARD_SIZE + 1] = "QWERTYUIOPASDFGHJKLZXCVBNM";

    Keyboard kb;

    for (int i = 0; i < KEYBOARD_SIZE; i++) {
        kb.key[i] = layout[i];
        kb.color[i] = _COLOR_BG;
    }

    return kb;
}

void updateKeyboard(Keyboard* kb, char character, int color){
    if (!kb) return;

    for (int i = 0; i < KEYBOARD_SIZE; i++) {
        if (kb->key[i] == character) {
            kb->color[i] = color;
            return;
        }
    }
}

static inline int letterIndex(char c) {
    return c - 'A';
}

PickedWordSymbols CountSameCharactersInWord(const char* word) {
    PickedWordSymbols freq = {0};

    for (int i = 0; i < WORD_LENGTH; i++) {
        char c = word[i];
        int index = letterIndex(c);
        if (index >= 0 && index < KEYBOARD_SIZE) {
            freq.letterCount[index]++;
        }
    }
    return freq;
}

void CheckEnteredWord(const char* inputWord, const char* pickedWord, LetterResult* result, Keyboard* kb)
{
    // iniating, setting all to gray
    for(int i = 0; i < WORD_LENGTH; i++){
        result[i].character = inputWord[i];
        result[i].color = _COLOR_GRAY;
        updateKeyboard(kb, inputWord[i], _COLOR_GRAY);
    }

    // winning condition
    if (strcmp(inputWord, pickedWord) == 0){
        // all letters green
        for (int i = 0; i < WORD_LENGTH; i++){
            result[i].character = inputWord[i];
            result[i].color = _COLOR_GREEN;
            updateKeyboard(kb, inputWord[i], _COLOR_GREEN);
        }
    }
    else{
        // looking for greens
        PickedWordSymbols pickedWordSymbols = CountSameCharactersInWord(pickedWord);
        for (int i = 0; i < WORD_LENGTH; i++){
            if (inputWord[i] == pickedWord[i]){
                result[i].character = inputWord[i];
                result[i].color = _COLOR_GREEN;
                updateKeyboard(kb, inputWord[i], _COLOR_GREEN);
                pickedWordSymbols.letterCount[letterIndex(inputWord[i])]--;
            }
        }
        // looking for yellows
        for (int i = 0; i < WORD_LENGTH; i++){
            char letter = result[i].character;
            int color = result[i].color;
            if (color == _COLOR_GRAY && pickedWordSymbols.letterCount[letterIndex(letter)] > 0){
                result[i].color = _COLOR_YELLOW;
                updateKeyboard(kb, inputWord[i], _COLOR_YELLOW);
                pickedWordSymbols.letterCount[letterIndex(letter)]--;
            }
        }

    }
}

extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // attach to process
            // return FALSE to fail DLL load
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}
