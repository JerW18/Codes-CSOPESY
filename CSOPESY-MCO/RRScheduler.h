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
    deque<shared_ptr<process>> processes;
    CPUManager* cpuManager;
    mutex mtx;
public:
    RRScheduler(CPUManager* cpuManager) {
        this->cpuManager = cpuManager;
    }

    void addProcess(shared_ptr<process> process) {
        lock_guard<mutex> lock(mtx);
        processes.push_back(process);
        cout << "RR Scheduler: Added process " << process->getProcessName() << " to queue" << endl;
        cout << processes.size() << endl;

        for (auto p : processes) {
            cout << p->getProcessName() << endl;
        }
    }

    void start() {
        shared_ptr<process> currentProcess = nullptr;
        while (!processes.empty()) {
            currentProcess = processes.front();
            processes.pop_front();

            // Assign the process to a CPU worker for the quantum time
            cpuManager->startProcess(currentProcess);

            // Check if the process is still not complete
            if (currentProcess->getInstructionIndex() < currentProcess->getTotalInstructions()) {
                // If not finished, re-queue it
                processes.push_back(currentProcess);
            }
        }
    }
};
