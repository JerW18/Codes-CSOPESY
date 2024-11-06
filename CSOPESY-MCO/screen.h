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
#include "MemoryAllocator.h"


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
	//ull memoryAssigned = 0;
	void* memoryAddress = nullptr;
	//ull assignedPages = 0;
	bool hasMemory = false;

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
	
	bool hasMemoryAssigned() {
		return this->hasMemory;
	}
	void setMemoryAssigned(bool hasMemory) {
		this->hasMemory = hasMemory;
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
    
	void assignMemory(void *address, ull size) {
		this->memoryAddress = address;
		this->memoryRequired = size;

	}

	void releaseMemory() {
		this->memoryAddress = nullptr;
	}

	void assignPages(ull pages) {
		this->pages = pages;
	}

	void assignMemoryRequired(ull memory) {
		this->memoryRequired = memory;
	}

	void assignMemoryAddress(void* address) {
		this->memoryAddress = address;
	}

	void* getMemoryAddress() {
		return this->memoryAddress;
	}


    process(string processName, ull id, ull totalInstructions) {
        this->processName = processName;
        this->id = id;
        this->totalInstructions = totalInstructions;
        this->dateOfBirth = timeStamp();
    }

    process(string processName, ull id, ull totalInstructions, ull memoryRequired)
        : processName(processName), id(id), totalInstructions(totalInstructions), dateOfBirth(timeStamp()), memoryRequired(memoryRequired) {}
  
};

class screenManager {
private:
    ull maxId = 0;
    shared_ptr<process> currentProcess = nullptr;
    mutex* m;
    MemoryAllocator& allocator;
public:
    vector<shared_ptr<process>> processes;
    bool inScreen = false;

    screenManager(mutex* mutexPtr, MemoryAllocator& allocator)
        : m(mutexPtr), allocator(allocator) {}

    void printProcess() {
        for (auto& x : this->processes) {
            cout << "Process: " << to_string(x->getId()) << endl;
        }
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



    void addProcess(string processName, ull totalInstructions, ull memoryRequired, string strategy) {
        void* allocatedMemory = allocator.allocate(memoryRequired, strategy);
        if (allocatedMemory) {
            auto proc = make_shared<process>(processName, maxId++, totalInstructions, memoryRequired);
            proc->assignMemory(allocatedMemory, memoryRequired);
            processes.push_back(proc);
            currentProcess = processes.back();
        }
        else {
            //cout << "Failed to allocate memory for process " << processName << endl;
        }
    }

    void addProcessManually(string processName, ull totalInstructions, ull memoryRequired, string strategy) {
        void* allocatedMemory = allocator.allocate(memoryRequired, strategy);
        if (allocatedMemory) {
            auto proc = make_shared<process>(processName, maxId++, totalInstructions, memoryRequired);
            proc->assignMemory(allocatedMemory, memoryRequired);
            processes.push_back(proc);
            currentProcess = proc;
            inScreen = true;
            showProcess();
        }
        else {
            //cout << "Failed to manually allocate memory for process " << processName << endl;
        }
    }

    ull getProcessCount() {
        return processes.size();
    }

    void removeProcess(string processName) {
        for (auto it = processes.begin(); it != processes.end(); ++it) {
            if ((*it)->getProcessName() == processName) {
                allocator.deallocate((*it)->getMemoryAddress(), (*it)->getMemoryRequired());
                processes.erase(it);
                //cout << "Process " << processName << " removed and memory deallocated." << endl;
                return;
            }
        }
        cout << "Process " << processName << " not found." << endl;
    }

    void reattachProcess(string processName, ull id) {
        for (const auto& x : this->processes) {
            if (x->getProcessName() == processName && x->getId() == id) {
                this->currentProcess = x;
                inScreen = true;

                // Display additional information about the process, including memory details
                cout << "Reattaching to process: " << x->getProcessName() << endl;
                cout << "Process ID: " << x->getId() << endl;
                cout << "Memory Required: " << x->getMemoryRequired() << " bytes" << endl;
                cout << "Memory Address: " << x->getMemoryAddress() << endl;

                showProcess();
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