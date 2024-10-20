#pragma once
#include "screen.h"
#include <queue>
#include <thread>
#include <iostream>
#include <atomic>
#include "CPUManager.h"

using namespace std;

class RRScheduler {
private:
    queue<process*> processes;
    CPUManager* cpuManager;

public:
    RRScheduler(CPUManager* cpuManager) {
        this->cpuManager = cpuManager;
    }

    void addProcess(process* proc) {
        processes.push(proc);
    }

    void start() {
        while (!processes.empty()) {
            process* currentProcess = processes.front();
            processes.pop();

            // Assign the process to a CPU worker for the quantum time
            cpuManager->startProcess(currentProcess);

            // Check if the process is still not complete
            if (currentProcess->getInstructionIndex() < currentProcess->getTotalInstructions()) {
                // If not finished, re-queue it
                processes.push(currentProcess);
            }
        }
    }
};
