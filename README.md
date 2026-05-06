# Windows API 2025
Wordle game is my final project for the Windows API subject in Vilnius University.

# About the game

This project is a simple recreation of a Wordle game according to [New York Times wordle](https://www.nytimes.com/games/wordle/index.html) game. It has a simple main menu, rules window, and interactive gameplay. You can enter guesses using either your mouse or keyboard. The on-screen keyboard is colored appropriately to indicate whether each letter is correct, present in the word, or absent.

[Demo (Youtube)](https://youtu.be/D1YmjLX2WBE)

# Downloading
You can download the game [here](https://drive.google.com/file/d/1TrYz8sq2LDNL51jp-JAp9iuIfAQw8rF7/view?usp=sharing).

# Running
First you need to extract the project's zip file.

Then to run the game you simply need to open `Wordle.exe` file.

# Technologies
- **C** - game logic compiled as a DLL
- **Windows API (WinAPI)** - The graphical user interface, keyboard and mouse input

## Building from Source
1. Open `WordleGame.cbp` with the CodeBlocks IDE
2. Build -> Build and run (or press F9)

# Examples
Below some screenshots of the game are displayed:

## Main menu
<img src="Example-pics/Wordle_menu.png" alt="Wordle menu picture" width="60%"/>

## Game Rules
<img src="Example-pics/Wordle_rules.png" alt="Wordle rules picture" width="60%"/>

## Mid game screen
<img src="Example-pics/Wordle_mid_game.png" alt="Wordle mid game picture" width="60%"/>

## Game won screen
<img src="Example-pics/Wordle_game_won.png" alt="Wordle game won picture" width="60%"/>