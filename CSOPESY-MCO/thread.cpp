#include <iostream>
#include <string>
#include <conio.h>
#include <windows.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>

using namespace std;

mutex mutexConsole;

int delay1 = 50;
int delay2 = 10;

void printHeader() {
    cout << "******************************************" << endl;
    cout << "*  Displaying a moving marquee console!  *" << endl;
    cout << "******************************************" << endl;
}

void runMarquee(const string& text, int areaWidth, int areaHeight, atomic<bool>& running) {
    int textLength = text.size();
    int xPos = 0, yPos = 0;
    int xDir = 1, yDir = 1;

    while (running) {
        system("cls");
        COORD cursorPosition;
        cursorPosition.X = 0;
        cursorPosition.Y = 0;
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursorPosition);

        printHeader();

        cursorPosition.X = xPos;
        cursorPosition.Y = 3 + yPos;
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
    this_thread::sleep_for(chrono::milliseconds(delay1));
    }
};

void handleUserInput(atomic<bool>& running, int inputYPos) {
    string userInput;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    bool hasInput = false;
    string command;

    while (running) {
        lock_guard<mutex> lock(mutexConsole);
        COORD cursorPosition;
        cursorPosition.X = 0;
        cursorPosition.Y = inputYPos;
        SetConsoleCursorPosition(hConsole, cursorPosition);
        cout << string(100, ' ');  

        SetConsoleCursorPosition(hConsole, cursorPosition);  
        cout << "Enter a command: " << userInput;  

        if (hasInput) {
            cursorPosition.X = 0;
            cursorPosition.Y = inputYPos + 1;
            SetConsoleCursorPosition(hConsole, cursorPosition);

            cout << string(100, ' ');  
            SetConsoleCursorPosition(hConsole, cursorPosition);
            cout << "Command processed: " << command;
        }

        cursorPosition.X = 17 + userInput.length();  
        cursorPosition.Y = inputYPos;
        SetConsoleCursorPosition(hConsole, cursorPosition);

        if (_kbhit()) {
            char ch = _getch();
            if (ch == 27) {  
                running = false;
                break;
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

        this_thread::sleep_for(chrono::milliseconds(delay2));
    }
}


int main() {
    system("cls");
    string marqueeText = "Hello world in marquee!";
    atomic<bool> running(true);

    int areaWidth = 50;   
    int areaHeight = 10;   

    thread marqueeThread = thread(runMarquee, marqueeText, areaWidth, areaHeight, ref(running));

    printHeader();

    int inputYPos = 15;

    //thread userInputThread = thread(handleUserInput, ref(running), inputYPos);

    handleUserInput(running, inputYPos);

    marqueeThread.join();
    //userInputThread.join();

    return 0;
}
