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
    CPUManager* cpuManager;
    mutex mtx;
public:
    deque<shared_ptr<process>> processes;
    FCFSScheduler(CPUManager* cpuManager) {
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
                this_thread::sleep_for(chrono::milliseconds(50));
                continue;
            }
            {
                lock_guard<mutex> lock(mtx);
                currentProcess = processes.front();
                processes.pop_front();
            }
            cpuManager->startProcess(currentProcess);
			//cout << "Process " << currentProcess->getId() << " started" << endl;
            if (currentProcess->getCoreAssigned() == -1) {
                lock_guard<mutex> lock(mtx);
                processes.push_front(currentProcess);
            }
        }
    }
    void getSize() {
        cout << processes.size() << endl;
    }
};