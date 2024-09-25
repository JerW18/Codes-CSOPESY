#pragma once

#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <cstdlib>
#include "keyboard.h"
#include "timeStamp.h"
typedef unsigned long long ull;
using namespace std;

class screen {
private:
	string processName;
	ull id;
	ull instructionIndex = 0;
	ull totalInstructions = 0;
	timeStamp dateOfBirth;

	void incrementInstructionIndex() {
		while (this && this->totalInstructions > this->instructionIndex) {
			this->instructionIndex++;
			Sleep(100);
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

	void printIntstructionIndex() {
		cout << "Instruction: " << this->instructionIndex << "/" << this->totalInstructions << endl;
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
		cout << "Screen " << this->processName << " has been destroyed." << endl;
	}
};

class screenManager {
private:
	ull maxId = 0;
	keyboard* kb = new keyboard();
	thread renderThread;
	vector<screen> screens;
	screen* currentScreen = nullptr;
	//vector<shared_ptr<screen>> screens;
	//shared_ptr<screen> currentScreen = nullptr;
public:
	void printScreens() {
		for (auto& x : this->screens) {
			cout << "Screen: " << to_string(x.getId()) << endl;
		}
	}
	void addScreen(string processName, ull totalInstructions) {
		screens.emplace_back(processName, maxId++, totalInstructions);
		currentScreen = &screens.back();
		if (!renderThread.joinable()) {
			renderThread = thread(&screenManager::renderScreen, this);
			renderThread.detach();
		}
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

	void renderScreen() {
		if (this->currentScreen == nullptr) {return;}
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
			Sleep(100);
		}
	}
};
