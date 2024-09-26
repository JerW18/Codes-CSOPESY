// #pragma once

// #include <windows.h>
// #include <iostream>
// #include <string>
// #include <thread>
// #include <vector>
// #include <cstdlib>
// #include "keyboard.h"
// #include "timeStamp.h"
// typedef unsigned long long ull;
// using namespace std;

// class screen {
// private:
//     string processName;
//     ull id;
//     ull instructionIndex = 0;
//     ull totalInstructions = 0;
//     timeStamp dateOfBirth;
//     // need some way to keep track of the inputted commands? maybe a 2d vector of strings of the command and the line?
//     //vector<vector<string>> inputHistory;

// public:
//     string getProcessName() {
//         return this->processName;
//     }
//     string getDateOfBirth() {
//         return this->dateOfBirth.getTimeStamp();
//     }
//     ull getId() {
//         return this->id;
//     }
//     ull getInstructionIndex() {
//         return this->instructionIndex;
//     }
//     ull getTotalInstructions() {
//         return this->totalInstructions;
//     }

//     void printIntstructionIndex() {
//         cout << "Instruction: " << this->instructionIndex << "/" << this->totalInstructions << endl;
//     }
//     screen(string processName, ull id, ull totalInstructions) {
//         this->processName = processName;
//         this->id = id;
//         this->totalInstructions = totalInstructions;
//         this->dateOfBirth = timeStamp();
//     }
//     ~screen() {
//         cout << "Screen " << this->processName << " has been destroyed." << endl;
//     }
// };

// class screenManager {
// private:
//     ull maxId = 0;
//     keyboard* kb = new keyboard();
//     thread renderThread;
//     vector<screen> screens;
//     screen* currentScreen = nullptr;
//     vector<string> inputHistory;
//     //vector<shared_ptr<screen>> screens;
//     //shared_ptr<screen> currentScreen = nullptr;
// public:
//     vector<shared_ptr<screen>> screens;
//     shared_ptr<screen> currentScreen = nullptr;
//     vector<screen> getScreen() {
//         return this->screens;
//     }
//     void printScreens() {
//         for (auto& x : this->screens) {
//             cout << "Screen: " << to_string(x->getId()) << endl;
//         }
//     }
//     void addScreen(string processName, ull totalInstructions) {
//         screens.emplace_back(make_shared<screen>(processName, maxId++, totalInstructions));
//         currentScreen = screens.back();
//         if (!renderThread.joinable()) {
//             renderThread = thread(&screenManager::renderScreen, this);
//             renderThread.detach();
//         }
//     }
//     void renderScreen() {
//         while (true) {
//             system("cls");
//             cout << "Current Screen: " << currentScreen->getProcessName() << endl;
//             cout << "Screen ID: " << currentScreen->getId() << endl;
//             cout << "Date of Birth: " << currentScreen->getDateOfBirth() << endl;
//             cout << "Instruction Index: " << currentScreen->getInstructionIndex() << "/" << currentScreen->getTotalInstructions() << endl;
//             cout << "Enter a command: ";
//             string input = kb->pollKeyboard();
//             inputHistory.push_back(input);
//             if (input == "exit") {
//                 break;
//             }
//         }
//     }
// };
