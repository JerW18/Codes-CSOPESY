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
    CPUManager* cpuManager;
    mutex mtx;
public:

    deque<shared_ptr<process>> processes;
    RRScheduler(CPUManager* cpuManager) {
        this->cpuManager = cpuManager;
    }

    void addProcess(shared_ptr<process> process) {
        lock_guard<mutex> lock(mtx);
        processes.push_back(process);
    }

    vector<shared_ptr<process>> getReadyQueue() {
        lock_guard<mutex> lock(mtx);
        return vector<shared_ptr<process>>(processes.begin(), processes.end());
    }

    void start() {
        shared_ptr<process> currentProcess = nullptr;
        while (true) {
            {
				vector<shared_ptr<process>> toAdd = cpuManager->isAnyoneAvailable();
                for (auto p : toAdd) {
					addProcess(p);
                }
				
            }
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
