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
    }

    void start() {
        shared_ptr<process> currentProcess = nullptr;
        while (true) {
            if (processes.empty()) {
                this_thread::sleep_for(chrono::milliseconds(100));
                continue;
            }
            {
                lock_guard<mutex> lock(mtx);
                currentProcess = processes.front();
                processes.pop_front();
            }
            cpuManager->startProcess(currentProcess);
            if (currentProcess->getInstructionIndex() < currentProcess->getTotalInstructions()) {
                lock_guard<mutex> lock(mtx);
                processes.push_back(currentProcess);
            }
        }
    }

    void getSize() {
        cout << processes.size() << endl;
    }
};
