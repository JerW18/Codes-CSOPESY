#pragma once
#include <queue>
#include "screen.h"
#include "CPUManager.h"
#include <mutex>
#include <memory>

using namespace std;

class FCFSScheduler 
{
private:
    deque<shared_ptr<process>> processes;
    CPUManager* cpuManager;
    mutex mtx;
public:
    FCFSScheduler(CPUManager* cpuManager) {
        this->cpuManager = cpuManager;
    }

    void addProcess(shared_ptr<process> process) {
        lock_guard<mutex> lock(mtx);
        processes.push_back(process);
        cout << "FCFS Scheduler: Added process " << process->getProcessName() << " to queue" << endl;
        cout << processes.size() << endl;

        for (auto p : processes) {
            cout << p->getProcessName() << endl;
        }
    }

    void start() {
        shared_ptr<process> currentProcess = nullptr;
        while (true) {
            if (processes.empty()) {
                this_thread::sleep_for(chrono::milliseconds(10));
                continue;
            }
            currentProcess = processes.front();
            processes.pop_front();
            cout << "FCFS Scheduler: Starting process " << currentProcess->getProcessName() << endl;
            cpuManager->startProcess(currentProcess);
        }
    }
};