#pragma once
#include <queue>
#include "screen.h"
#include "CPUManager.h"
#include <mutex>
#include <memory>
#include <condition_variable> 
#include "MemoryAllocator.h"

using namespace std;

class Scheduler
{
private:
    CPUManager* cpuManager;
    mutex mtx;
    condition_variable cv;
    MemoryAllocator& allocator;
    volatile ull& cycleCount;
public:
    deque<shared_ptr<process>> processes;
	/*Scheduler(CPUManager* cpuManager, MemoryAllocator& allocator) : allocator(allocator)
    {
        this->cpuManager = cpuManager;
    }*/

	// Scheduler(CPUManager* cpuManager, MemoryAllocator& allocator, volatile ull* cycleCount) : allocator(allocator), cycleCount(cycleCount)
    // {
    //     this->cpuManager = cpuManager;
    // }
    Scheduler(CPUManager* cpuManager, MemoryAllocator& allocator, volatile ull& cycleCount) : allocator(allocator), cycleCount(cycleCount)
    {
        this->cpuManager = cpuManager;
    }

    void addProcess(shared_ptr<process> process) {
        {
            lock_guard<mutex> lock(mtx);
            processes.push_back(process);
        }
        cv.notify_all();
    }
    deque<shared_ptr<process>> getReadyQueue() {
        lock_guard<mutex> lock(mtx);
        return processes;
    }

    void starFCFS() {
        shared_ptr<process> currentProcess = nullptr;
		int response = 0;
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
            response = cpuManager->startProcess(currentProcess);
			if (response == 1) {
				lock_guard<mutex> lock(mtx);
				processes.push_back(currentProcess);
			}
            if (response != 1 && currentProcess->getCoreAssigned() == -1) {
                lock_guard<mutex> lock(mtx);
                processes.push_front(currentProcess);
            }
			response = 3;
        }
    }

    void startRR() {
        shared_ptr<process> currentProcess = nullptr;
        int response = 0;
        int prevCycleCount = cycleCount;
        while (true) {
            vector<shared_ptr<process>> toAdd = cpuManager->isAnyoneAvailable();
            for (auto& p : toAdd) {
                addProcess(p);
            }
            if (prevCycleCount != cycleCount) {
                /*if (processes.empty() && prevCycleCount != cycleCount) {
                    this_thread::sleep_for(chrono::milliseconds(100));
                    continue;
                }
                else {
                    lock_guard<mutex> lock(mtx);
                    if (prevCycleCount != cycleCount) {
                        currentProcess = processes.front();
                        processes.pop_front();
                    }
                    else {
                        continue;
                    }
                }*/

                {
                    unique_lock<mutex> lock(mtx);
                    if (cv.wait_for(lock, chrono::milliseconds(10), [this] { return !processes.empty(); })) {
                        currentProcess = processes.front();
                        processes.pop_front();
                    }
                    else {
                        continue;
                    }
                }
                response = cpuManager->startProcess(currentProcess);

                if (response == 1) {
                    lock_guard<mutex> lock(mtx);
                    processes.push_back(currentProcess);
                }

                if (response != 1 && currentProcess != nullptr && currentProcess->getCoreAssigned() == -1) {
                    lock_guard<mutex> lock(mtx);
                    processes.push_front(currentProcess);
                    cv.notify_all();
                }
                response = 3;
                prevCycleCount = cycleCount;
            }
        }
    }
	void printQueue() {
		for (int i = 0; i < processes.size(); i++) {
			cout << i << " is " << processes[i]->getProcessName() << endl;
		}
	}
    ull getSize() {
        return processes.size();
    }
};