#pragma once

#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <cstdlib>
#include "keyboard.h"
#include "timeStamp.h"
#include "global.h"
typedef unsigned long long ull;

using namespace std;

class screen {
private:
	string processName;
	ull id;
	ull instructionIndex = 0;
	ull totalInstructions = 0;
	timeStamp dateOfBirth;
	vector<pair<int, string>> inputHistory;

	void incrementInstructionIndex() {
		while (this && this->totalInstructions > this->instructionIndex) {
			//this->instructionIndex++;
			Sleep(1000);
		}
	}

public:
	string getProcessName() {
		return this->processName;
	}
	string getDateOfBirth() {
		return this->dateOfBirth.getTimeStamp();
	}
	ull getId() {
		return this->id;
	}
	ull getInstructionIndex() {
		return this->instructionIndex;
	}
	ull getTotalInstructions() {
		return this->totalInstructions;
	}

	void incrementInstructionIndex1() {
		this->instructionIndex++;
	}

	void addInstructionToHistory(int state, string instruction) {
		if(state == 0) {
			this->inputHistory.push_back(make_pair(state, instruction));
		}
		else {
			this->inputHistory.push_back(make_pair(state, instruction));
		}
	}

	void printIntstructionIndex() {
		cout << "Instruction: " << this->instructionIndex << "/" << this->totalInstructions << endl;
	}

	void printInputHistory() {
		for (auto& x : this->inputHistory) {
			if(x.first == 0) {
				cout << "<History> Command: " << x.second << endl;
			}
			else {
				cout << "<History> Invalid Command: " << x.second << endl;
			}
		}
	}

	screen(string processName, ull id, ull totalInstructions) {
		this->processName = processName;
		this->id = id;
		this->totalInstructions = totalInstructions;
		this->dateOfBirth = timeStamp();
		thread t(&screen::incrementInstructionIndex, this);
		t.detach();
	}
	~screen() {
		//cout << "Screen " << this->processName << " has been destroyed." << endl;
	}
};

class screenManager {
private:
	ull maxId = 0;
	keyboard* kb = new keyboard();
	thread renderThread;
	screen* currentScreen = nullptr;
	//vector<string> inputHistory;
	//vector<shared_ptr<screen>> screens;
	//shared_ptr<screen> currentScreen = nullptr;
public:
	vector<screen> screens;
	bool inScreen = false;
	void printScreens() {
		for (auto& x : this->screens) {
			cout << "Screen: " << to_string(x.getId()) << endl;
		}
	}
	void addScreen(string processName, ull totalInstructions) {
		screens.emplace_back(processName, maxId++, totalInstructions);
		currentScreen = &screens.back();
		// if (!renderThread.joinable()) {
		// 	renderThread = thread(&screenManager::renderScreen, this);
		// 	renderThread.detach();
		// }
		inScreen = true;
		startScreen();
	}

	void removeScreen(string processName) {
		for (ull i = 0; i < screens.size(); i++) {
			if (this->screens[i].getProcessName() == processName) {
				this->screens.erase(this->screens.begin() + i);
				return;
			}
		}
		cout << "Screen " << processName << " not found." << endl;
	}

	void listScreens() {
		cout << "Screens:" << endl;
		for (auto &x : this->screens) {
			cout << "Screen: " << to_string(x.getId()) << endl;
		}
	}

	void reattatchScreen(string processName, int id) {
		for (auto &x : this->screens) {
			if (x.getProcessName() == processName && x.getId() == id) {
				this->currentScreen = &x;
				inScreen = true;

				startScreen();
				return;
			}
		}
		cout << "Screen " << processName << " not found." << endl;
	}

	void startScreen() {
		if (this->currentScreen == nullptr) {return;}
		system("cls");
		cout << "Process: " << this->currentScreen->getProcessName() << endl;
		cout << "Date of birth: " << this->currentScreen->getDateOfBirth() << endl;
		cout << "Instruction: " << this->currentScreen->getInstructionIndex() << "/" << this->currentScreen->getTotalInstructions() << endl;
		this->currentScreen->printInputHistory();

		while (true){
			cout << "Enter screen command: ";
			string input;
			getline(cin, input);
			this->currentScreen->addInstructionToHistory(0, input);
			this->currentScreen->incrementInstructionIndex1();
			if(input == "exit") {
				cout << "Exiting screen " << this->currentScreen->getProcessName() << "." << endl;
				inScreen = false;
				this->currentScreen = nullptr;
				
				#ifdef _WIN32
					system("cls");
				#else
					system("clear");
				#endif
				g.printHeader();
	
				return;
			}
			else
				{
					cout << "Invalid screen command \"" << input << "\"." << endl;
					this->currentScreen->incrementInstructionIndex1();
					this->currentScreen->addInstructionToHistory(1, input);
				}
		}
		
		/*
		cout << "Enter command: " << this->kb->getInput();
		string input = this->kb->getInput();
		if(input == "exit1") {
			this->currentScreen = nullptr;
			return;
		}
		*/
		/*
		while (true) {
			#ifdef _WIN32
				system("cls");
			#else
				system("clear");
			#endif
			cout << "Process: " << this->currentScreen->getProcessName() << endl;
			cout << "Date of birth: " << this->currentScreen->getDateOfBirth() << endl;
			cout << "Instruction: " << this->currentScreen->getInstructionIndex() << "/" << this->currentScreen->getTotalInstructions() << endl;
			cout << "Enter command: " << this->kb->getInput();

			// Print the input history
            for (const auto& input : inputHistory) {
                cout << input << endl;
            }

            // Get and store the new input
            string input = this->kb->getInput();
            inputHistory.push_back("Enter command: " + input);

            // Sleep for a while
            Sleep(100);

            // Check for exit command
            if (input == "exit") {
                this->currentScreen = nullptr;
                return;
            }
		}
		*/
	}
};
