#pragma once
#include <queue>
#include "screen.h"
#include "CPUManager.h"
#include <mutex>
#include <memory>
#include "TS.h"

using namespace std;

class FCFSScheduler 
{
private:
    CPUManager* cpuManager;
    mutex mtx;
public:
    TS<process*> *processes;
    FCFSScheduler(CPUManager* cpuManager, TS<process*> *processes) {
        this->cpuManager = cpuManager;
		this->processes = processes;
    }

    void addProcess(process* process) {
        processes->push(process);
    }
    
    void start() {
        process *currentProcess = nullptr;
        while (true) {
            vector<process *> toAdd = cpuManager->isAnyoneAvailable();
            for (auto p : toAdd) {
                processes->push(p);
            }
            currentProcess = processes->pop();
            cpuManager->startProcess(currentProcess);
            if (currentProcess->getCoreAssigned() == -1) {
                processes->push(currentProcess);
            }

        }
    }
    void getSize() {
        cout << processes->size() << endl;
    }
};