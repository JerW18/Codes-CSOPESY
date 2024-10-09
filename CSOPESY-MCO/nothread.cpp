#include <iostream>
#include <string>
#include <conio.h>
#include <windows.h>
#include <chrono>
#include <thread>

using namespace std;

int delay1 = 25;

void printHeader() {
    cout << "******************************************" << endl;
    cout << "*  Displaying a moving marquee console!  *" << endl;
    cout << "******************************************" << endl;
}

void runMarquee(const string& text, int areaWidth, int areaHeight, int& xPos, int& yPos, int& xDir, int& yDir) {
    int textLength = text.size();

    system("cls");
    printHeader();

    COORD cursorPosition;
    cursorPosition.X = xPos;
    cursorPosition.Y = yPos + 3; 
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);
    cout << text;

    xPos += xDir;
    yPos += yDir;

    if (xPos + textLength > areaWidth || xPos < 0) {
        xDir = -xDir; 
        xPos = max(0, min(xPos, areaWidth - textLength));
    }
    
    if (yPos >= areaHeight || yPos < 0) {
        yDir = -yDir;  
        yPos = max(0, min(yPos, areaHeight - 1));
    }
}

void handleUserInput(int inputYPos, string& userInput, bool& hasInput, string& command) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    COORD cursorPosition;
    cursorPosition.X = 0;
    cursorPosition.Y = inputYPos;
    SetConsoleCursorPosition(hConsole, cursorPosition);

    cout << "Enter a command: " << userInput;
    if (hasInput) {
        cout << "\nCommand processed: " << command;
    }

    cursorPosition.X = 17 + userInput.length();
    cursorPosition.Y = inputYPos;
    SetConsoleCursorPosition(hConsole, cursorPosition);

    if (_kbhit()) {
        char ch = _getch();
        if (ch == 27) {  
            exit(0);  
        } else if (ch == '\r') {
            hasInput = true;
            command = userInput;
            cursorPosition.X = 0;
            cursorPosition.Y = inputYPos + 1;
            SetConsoleCursorPosition(hConsole, cursorPosition);

            cout << string(100, ' ');

            SetConsoleCursorPosition(hConsole, cursorPosition);
            cout << "Command processed: " << userInput << endl;
            userInput.clear();  
        } else if (ch == '\b' && !userInput.empty()) {  
            userInput.pop_back();
        } else if (ch != '\b') {  
            userInput += ch;
        }
    }
}

int main() {
    system("cls");
    string marqueeText = "Hello world in marquee!";
    int areaWidth = 50;   
    int areaHeight = 10;   
    int xPos = 0; 
    int yPos = 0; 
    int xDir = 1; 
    int yDir = 1;
    int inputYPos = 15; 
    bool hasInput = false;
    string command;
    string userInput;

    printHeader();

    while (true) {
        runMarquee(marqueeText, areaWidth, areaHeight, xPos, yPos, xDir, yDir);
        handleUserInput(inputYPos, userInput, hasInput, command);
        this_thread::sleep_for(chrono::milliseconds(delay1));
    }

    return 0;
}
