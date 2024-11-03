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
#include "TS.h"
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
    int coreAssigned = -1;

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
        cout << "\nCurrent Instruction Line: " << this->instructionIndex << endl;
        cout << "Total Instruction Lines: " << this->totalInstructions << endl;
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

    bool isFinished() {
        return instructionIndex >= totalInstructions;
    }

    void assignCore(int core) {
        this->coreAssigned = core;
    }

    process(string processName, ull id, ull totalInstructions) {
        this->processName = processName;
        this->id = id;
        this->totalInstructions = totalInstructions;
        this->dateOfBirth = timeStamp();
    }

    process(process &other) {
		this->processName = other.processName;
		this->id = other.id;
		this->instructionIndex = other.instructionIndex;
		this->totalInstructions = other.totalInstructions;
		this->dateOfBirth = other.dateOfBirth;
		this->inputHistory = other.inputHistory;
		this->coreAssigned = other.coreAssigned;
    }
  
};

class screenManager {
private:
    ull maxId = 0;
    process * currentProcess = nullptr;
    mutex* m;
	TS<process *>* schedulerQueue;
public:
    vector<process *> processes;
    bool inScreen = false;

    screenManager(mutex* mutexPtr, TS<process *>* schedulerQueue) : m(mutexPtr) {
		this->schedulerQueue = schedulerQueue;
    }

    void printProcess() {
        for (auto& x : this->processes) {
            cout << "Process: " << to_string(x->getId()) << endl;
        }
    }
    void addProcess(string processName, ull totalInstructions) {
        // Create a new process and add it to the processes vector
        process* newProcess = new process(processName, maxId++, totalInstructions);
        processes.push_back(newProcess);
        schedulerQueue->push(newProcess); // Add to scheduler queue (assuming schedulerQueue accepts raw pointers)
        //cout << "Process " << newProcess->getProcessName() << " added with ID " << newProcess->getId() << endl;
    }

    void addProcessManually(string processName, ull totalInstructions) {
        // Create a new process, add it to the processes vector, and set it as the current process
        process* newProcess = new process(processName, maxId++, totalInstructions);
        processes.push_back(newProcess);
        currentProcess = newProcess;
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