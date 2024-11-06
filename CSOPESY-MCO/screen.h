#pragma once

#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <cstdlib>
#include <memory>
#include "timeStamp.h"
#include "global.h"
#include <mutex>
typedef unsigned long long ull;

using namespace std;


class process {
private:
    string processName;
    ull id;
    ull instructionIndex = 0;
    ull totalInstructions = 0;
    timeStamp dateOfBirth;
    int coreAssigned = -1;
    ull memoryRequired = 0;
    ull pages = 0;
	ull memoryAssigned = 0;
	//ull assignedPages = 0;

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
    int getCoreAssigned() {
        return this->coreAssigned;
    }
	ull getMemoryRequired() {
		return this->memoryRequired;
	}
    ull getMemoryAssigned() {
		return this->memoryAssigned;
    }

    void incrementInstructionIndex() {
        this->instructionIndex++;
    }

    void printIntstructionIndex() {
        cout << "\nCurrent Instruction Line: " << this->instructionIndex << endl;
        cout << "Total Instruction Lines: " << this->totalInstructions << endl;
    }                

    bool isFinished() {
        return instructionIndex >= totalInstructions;
    }

    void assignCore(int core) {
        this->coreAssigned = core;
    }
    
	void assignMemory(ull memory) {
		this->memoryAssigned = memory;
	}

	void assignPages(ull pages) {
		this->pages = pages;
	}

	void assignMemoryRequired(ull memory) {
		this->memoryRequired = memory;
	}

	void releaseMemory() {
		this->memoryAssigned = 0;
	}

    process(string processName, ull id, ull totalInstructions) {
        this->processName = processName;
        this->id = id;
        this->totalInstructions = totalInstructions;
        this->dateOfBirth = timeStamp();
    }

    process(string processName, ull id, ull totalInstructions, ull memoryRequired) {
        this->processName = processName;
        this->id = id;
        this->totalInstructions = totalInstructions;
        this->dateOfBirth = timeStamp();
		this->memoryRequired = memoryRequired;
    }

    process(process &other) {
		this->processName = other.processName;
		this->id = other.id;
		this->instructionIndex = other.instructionIndex;
		this->totalInstructions = other.totalInstructions;
		this->dateOfBirth = other.dateOfBirth;
		this->coreAssigned = other.coreAssigned;
		this->memoryAssigned = other.memoryAssigned;
		this->memoryRequired = other.memoryRequired;
		this->pages = other.pages;
    }
  
};

class screenManager {
private:
    ull maxId = 0;
    shared_ptr<process> currentProcess = nullptr;
    mutex* m;
public:
    vector<shared_ptr<process>> processes;
    bool inScreen = false;

    screenManager(mutex* mutexPtr) : m(mutexPtr) {}

    void printProcess() {
        for (auto& x : this->processes) {
            cout << "Process: " << to_string(x->getId()) << endl;
        }
    }
    void addProcess(string processName, ull totalInstructions) {
        processes.emplace_back(make_shared<process>(processName, maxId++, totalInstructions));
        currentProcess = processes.back();
    }

	void addProcessManually(string processName, ull totalInstructions) {
		processes.emplace_back(make_shared<process>(processName, maxId++, totalInstructions));
		currentProcess = processes.back();
		inScreen = true;
		showProcess();
	}

    void addProcess(string processName, ull totalInstructions, ull memoryRequired) {
        processes.emplace_back(make_shared<process>(processName, maxId++, totalInstructions, memoryRequired));
        currentProcess = processes.back();
    }

    void addProcessManually(string processName, ull totalInstructions, ull memoryRequired) {
        processes.emplace_back(make_shared<process>(processName, maxId++, totalInstructions, memoryRequired));
        currentProcess = processes.back();
        inScreen = true;
        showProcess();
    }


    ull getProcessCount() {
        return processes.size();
    }

    void removeProcess(string processName) {
        for (ull i = 0; i < processes.size(); i++) {
            if (this->processes[i]->getProcessName() == processName) {
                this->processes.erase(this->processes.begin() + i);
                cout << "Process " << processName << " removed." << endl;
                return;
            }
        }
        cout << "Process " << processName << " not found." << endl;
    }

    void listProcess() {
        cout << "Processes:" << endl;
        for (auto &x : this->processes) {
            cout << "Process: " << to_string(x->getId()) << endl;
        }
    }

    void reattatchProcess(string processName, ull id) {
        for (auto &x : this->processes) {
            if (x->getProcessName() == processName && x->getId() == id) {
                this->currentProcess = x;
                inScreen = true;

                showProcess();
                return;
            }
        }
        cout << "Screen " << processName << " not found." << endl;
    }

    void showProcess() {
        if (this->currentProcess == nullptr) {return;}
        system("cls");
        cout << "Process: " << this->currentProcess->getProcessName() << endl;
        cout << "ID: " << this->currentProcess->getId() << endl;
        cout << "Date of birth: " << this->currentProcess->getDateOfBirth() << endl;
        this->currentProcess->printIntstructionIndex();

        while (true){
            cout << "Enter screen command: ";
            string input;
            getline(cin, input);
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
            else if (input == "process-smi") {
				cout << "\nProcess: " << this->currentProcess->getProcessName() << endl;  
				cout << "ID: " << this->currentProcess->getId() << endl;

				if (this->currentProcess->getInstructionIndex() == this->currentProcess->getTotalInstructions()) {
					cout << "\nFinished!" << endl;
				}
				else {
                    cout << "\nCurrent Instruction Line: " << this->currentProcess->getInstructionIndex() << endl;
                    cout << "Total Instruction Lines: " << this->currentProcess->getTotalInstructions() << endl;
				}
                cout << "\n";
            }
            else
                {
                    cout << "Invalid screen command \"" << input << "\"." << endl;
                }
    }
    }
};