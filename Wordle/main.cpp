#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <tuple>
#include <windows.h>

#define WORD_LENGTH 5

#define COLOR_BLACK 0
#define COLOR_GREEN 2
#define COLOR_YELLOW 6
#define COLOR_WHITE 7
#define COLOR_GRAY 8

using namespace std;

vector<string> SaveWordsToMemory();
string PickRandomWord(vector<string> wordList);
bool IsWordInWordList(string word, vector<string> wordList);
int CountSymbolByTypeInWord(char symbol, string word);
bool HasWonGame(string inputWord, string pickedWord);
vector<tuple<char, int>> CheckEnteredWord(string inputWord, string pickedWord, map<char, int>& keyboard);
unordered_map<char, int> CountSameCharactersInWord(string word);
map<char, int> InitiateKeyboard();
map<char, int>& UpdateKeyboard(char character, int color, map<char, int>& keyboard);
void printResult(vector<tuple<char, int>> result);
void printKeyboard(map<char, int>& keyboard);
void SetColor(int textColor, int bgColor);

int main() {
  string inputWord;
  vector<string> wordList;
  wordList = SaveWordsToMemory();

  string pickedWord = PickRandomWord(wordList);
  map<char, int> keyboardCharacters = InitiateKeyboard();

  for (int i = 0; i < 6; i++){
    cout << "Enter a word: ";
    cin >> inputWord;
    if (IsWordInWordList(inputWord, wordList)){
        vector<tuple<char, int>> result = CheckEnteredWord(inputWord, pickedWord, keyboardCharacters);
        printResult(result);
        printKeyboard(keyboardCharacters);

        if (HasWonGame(inputWord, pickedWord)){
            break;
        }
    }
    else {
        cout << "Word is not in the word list!" << endl;
        i--;
    }
  }
  if (HasWonGame(inputWord, pickedWord)){
        cout << "You won the game!" << endl;
  }
  cout << "The word was: " << pickedWord << endl;



  return 0;
}

vector<string> SaveWordsToMemory(){
    vector<string> wordList;
    ifstream WordFile("words.txt");
    string word;
    if (WordFile.is_open()){

        while(getline(WordFile, word)){
            wordList.push_back(word);
        }
        WordFile.close();
        return wordList;
    }
    else {
        return {};
    }
}

string PickRandomWord(vector<string> wordList){
     random_device rd;
    if (!wordList.empty()){
        uniform_int_distribution<int> dist(0, wordList.size() -1);
        string word = wordList.at(dist(rd));
        return word;
    }
    else {
        return "";
    }
}

bool IsWordInWordList(string word, vector<string> wordList){
    if (find(wordList.begin(), wordList.end(), word) != wordList.end()){
        return true;
    }
    else return false;
}

unordered_map<char, int> CountSameCharactersInWord(string word){

    // Hash map to store frequency of each character
    unordered_map<char, int> frequency;
    // Count frequency of each character
    for (char c : word) {
        frequency[c]++;
    }
    return frequency;
}

map<char, int> InitiateKeyboard(){
    string keyboard = "qwertyuiopasdfghjklzxcvbnm";

    map<char, int> keyboardCharacters;

    for(char c : keyboard){
        keyboardCharacters[c] = COLOR_BLACK;
    }
    return keyboardCharacters;
}


bool HasWonGame(string inputWord, string pickedWord){
    if (inputWord.compare(pickedWord) == 0){ // winning condition, words match
        return true;
    }
    else return false;
}
vector<tuple<char, int>> CheckEnteredWord(string inputWord, string pickedWord, map<char, int>& keyboard){

    vector<tuple<char, int>> result(WORD_LENGTH);

    // initiating, setting all to gray
    for (int i = 0; i < WORD_LENGTH; i++){
        //inputWord[i] = (char) tolower(inputWord[i]); // sets character to lower
        result[i] = make_tuple(inputWord[i], COLOR_GRAY);
        UpdateKeyboard(inputWord[i], COLOR_GRAY, keyboard);
    }

    if (HasWonGame(inputWord, pickedWord)){ // winning condition, words match
        // all letters green
        for (int i = 0; i < WORD_LENGTH; i++){
            result[i] = make_tuple(inputWord[i], COLOR_GREEN);
            UpdateKeyboard(inputWord[i], COLOR_GREEN, keyboard);
        }
    }
    else {
        // looking for greens
        unordered_map<char, int> pickedWordSymbols = CountSameCharactersInWord(pickedWord);
        for(int i = 0; i < WORD_LENGTH; i++){
            if (inputWord[i] == pickedWord[i]){
                result[i] = make_tuple(inputWord[i], COLOR_GREEN);
                UpdateKeyboard(inputWord[i], COLOR_GREEN, keyboard);
                pickedWordSymbols[inputWord[i]]--;
            }
        }
        // looking for yellows
        for (int i = 0; i < WORD_LENGTH; i++) {
            auto& [letter, color] = result[i];
            if (color == COLOR_GRAY && pickedWordSymbols[letter] > 0) {
                color = COLOR_YELLOW;
                UpdateKeyboard(inputWord[i], COLOR_YELLOW, keyboard);
                pickedWordSymbols[letter]--;
            }
        }
    }
    return result;
}



map<char, int>& UpdateKeyboard(char character, int color, map<char, int>& keyboard){
    if(keyboard[character] != color){
        keyboard[character] = color;
    }
    return keyboard;
}

void printResult(vector<tuple<char, int>> result){
    for (const auto& [letter, color] : result) {
            SetColor(COLOR_WHITE, color);
            cout << letter;
    }
    SetColor(COLOR_WHITE, COLOR_BLACK);
    cout << endl;
}

void printKeyboard(map<char, int>& keyboard){
    for (const auto& kb : keyboard) {
        SetColor(COLOR_WHITE,kb.second);
        cout << kb.first << " ";
    }
    SetColor(COLOR_WHITE, COLOR_BLACK);
    cout << endl;
}

void SetColor(int textColor, int bgColor){
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole,(bgColor << 4) | textColor);
}
