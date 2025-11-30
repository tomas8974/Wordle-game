#define MAX_WORDS 20000
#define WORD_LENGTH 5
#define GUESS_NUMBER 6
#define KEYBOARD_SIZE 26

typedef struct {
    char key[KEYBOARD_SIZE];
    int  color[KEYBOARD_SIZE];
} Keyboard;

typedef struct {
    char character;
    int color;
} LetterResult;
