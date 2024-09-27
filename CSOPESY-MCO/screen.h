#pragma once

#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <cstdlib>
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

	void incrementInstructionIndex() {
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
	}
};

class screenManager {
private:
	ull maxId = 0;
	screen* currentScreen = nullptr;
public:
	vector<screen> screens;
	bool inScreen = false;

	void addScreen(string processName, ull totalInstructions) {
		screens.emplace_back(processName, this->maxId++, totalInstructions);
		currentScreen = &screens.back();
		inScreen = true;
		this->maxId++;
		startScreen();
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
			this->currentScreen->incrementInstructionIndex();
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
			else {
				cout << "Invalid screen command \"" << input << "\"." << endl;
				this->currentScreen->incrementInstructionIndex();
				this->currentScreen->addInstructionToHistory(1, input);
			}
		}
	}
};
