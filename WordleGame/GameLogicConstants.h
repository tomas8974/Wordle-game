#include <stdio.h>

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
