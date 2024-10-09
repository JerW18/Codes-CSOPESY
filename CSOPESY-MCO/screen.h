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

class process {
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

	process(string processName, ull id, ull totalInstructions) {
		this->processName = processName;
		this->id = id;
		this->totalInstructions = totalInstructions;
		this->dateOfBirth = timeStamp();
		//thread t(&screen::incrementInstructionIndex, this);
		//t.detach();
	}
	~process() {
		//cout << "Screen " << this->processName << " has been destroyed." << endl;
	}
};

class screenManager {
private:
	ull maxId = 0;
	keyboard* kb = new keyboard();
	thread renderThread;
	process* currentProcess = nullptr;
	//vector<string> inputHistory;
	//vector<shared_ptr<screen>> screens;
	//shared_ptr<screen> currentScreen = nullptr;
public:
	vector<process> processes;
	bool inScreen = false;
	void printProcess() {
		for (auto& x : this->processes) {
			cout << "Process: " << to_string(x.getId()) << endl;
		}
	}
	void addProcess(string processName, ull totalInstructions) {
		processes.emplace_back(processName, maxId++, totalInstructions);
		currentProcess = &processes.back();
		// if (!renderThread.joinable()) {
		// 	renderThread = thread(&screenManager::renderScreen, this);
		// 	renderThread.detach();
		// }
		inScreen = true;
		startProcess();
	}

	void removeProcess(string processName) {
		for (ull i = 0; i < processes.size(); i++) {
			if (this->processes[i].getProcessName() == processName) {
				this->processes.erase(this->processes.begin() + i);
				return;
			}
		}
		cout << "Process " << processName << " not found." << endl;
	}

	void listProcess() {
		cout << "Processes:" << endl;
		for (auto &x : this->processes) {
			cout << "Process: " << to_string(x.getId()) << endl;
		}
	}

	void reattatchProcess(string processName, int id) {
		for (auto &x : this->processes) {
			if (x.getProcessName() == processName && x.getId() == id) {
				this->currentProcess = &x;
				inScreen = true;

				startProcess();
				return;
			}
		}
		cout << "Screen " << processName << " not found." << endl;
	}

	void startProcess() {
		if (this->currentProcess == nullptr) {return;}
		system("cls");
		cout << "Process: " << this->currentProcess->getProcessName() << endl;
		cout << "Date of birth: " << this->currentProcess->getDateOfBirth() << endl;
		cout << "Instruction: " << this->currentProcess->getInstructionIndex() << "/" << this->currentScreen->getTotalInstructions() << endl;
		this->currentProcess->printInputHistory();

		while (true){
			cout << "Enter screen command: ";
			string input;
			getline(cin, input);
			this->currentProcess->addInstructionToHistory(0, input);
			this->currentProcess->incrementInstructionIndex1();
			if(input == "exit") {
				cout << "Exiting screen " << this->currentProcess->getProcessName() << "." << endl;
				inScreen = false;
				this->currentProcess = nullptr;
				
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
					this->currentProcess->incrementInstructionIndex1();
					this->currentProcess->addInstructionToHistory(1, input);
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
