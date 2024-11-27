#pragma once

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>

#include "CPUManager.h"
#include "screen.h"
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
    MemoryAllocator* allocator;
    volatile ull& cycleCount;
    string memType;
    deque<shared_ptr<process>>* processes;
    deque<shared_ptr<process>>* backingStore;

public:
	/*Scheduler(CPUManager* cpuManager, MemoryAllocator& allocator) : allocator(allocator)
    {
        this->cpuManager = cpuManager;
    }*/

	// Scheduler(CPUManager* cpuManager, MemoryAllocator& allocator, volatile ull* cycleCount) : allocator(allocator), cycleCount(cycleCount)
    // {
    //     this->cpuManager = cpuManager;
    // }
	Scheduler(CPUManager* cpuManager, MemoryAllocator* allocator, volatile ull& cycleCount, string memType, deque<shared_ptr<process>>* processes, deque<shared_ptr<process>>* backingStore) : cycleCount(cycleCount), memType(memType)
    {
        this->cpuManager = cpuManager;
		this->allocator = allocator;
		this->processes = processes;
		this->backingStore = backingStore;
    }

    void addProcess(shared_ptr<process> process) {
        {
            lock_guard<mutex> lock(mtx);
            processes->push_back(process);
        }
        cv.notify_all();
    }

    /*deque<shared_ptr<process>> getReadyQueue() {
        lock_guard<mutex> lock(mtx);
        return processes;
    }*/

    void starFCFS() {
        shared_ptr<process> currentProcess = nullptr;
		int response = 0;
        while (true) {
            if (processes->empty()) {
                this_thread::sleep_for(chrono::milliseconds(100));
                continue;
            }
            {
                lock_guard<mutex> lock(mtx);
                currentProcess = processes->front();
                processes->pop_front();
            }
            response = cpuManager->startProcess(currentProcess);
			if (response == 1) {
				lock_guard<mutex> lock(mtx);
				processes->push_back(currentProcess);
			}
            if (response != 1 && currentProcess->getCoreAssigned() == -1) {
                lock_guard<mutex> lock(mtx);
                processes->push_front(currentProcess);
            }
			response = 3;
        }
    }

    void startRR() {
        shared_ptr<process> currentProcess = nullptr;
        int response = 0;
        int prevCycleCount = cycleCount;
		boolean flag = true;
        vector<shared_ptr<process>> toAdd;
        while (true) {
            toAdd = cpuManager->isAnyoneAvailable();
            for (auto& p : toAdd) {
                addProcess(p);
            }
            //if (prevCycleCount != cycleCount) {

                {
                    unique_lock<mutex> lock(mtx);
                    if (cv.wait_for(lock, chrono::milliseconds(100), [this] { return !processes->empty(); })) {
                        currentProcess = processes->front();
                        processes->pop_front();
						if (currentProcess != nullptr && currentProcess->isFinished()) {
							//memoryAllocator->deallocate(currentProcess->getMemoryAddress(), currentProcess->getMemoryRequired(), memType, currentProcess->getProcessName());
							continue;
						}
                    }
                    else {
                        continue;
                    }
                }

				if (currentProcess == nullptr) {
					continue;
				}
                //if (flag && currentProcess->hasMemoryAssigned()) {
                    //process has memory assigned and the other process was preempted and deallocated successful
                    response = cpuManager->startProcess(currentProcess);
                    if (response == -11 && currentProcess != nullptr && currentProcess->getCoreAssigned() == -1) {
                        //no cpu available
                        //cout << currentProcess->getProcessName() << " here";
                        lock_guard<mutex> lock(mtx);
                        processes->push_front(currentProcess);
                        cv.notify_all();
                    }
                  
                //}
			    
                

                /*if (  response != -10 && response < 0 && currentProcess != nullptr && currentProcess->getCoreAssigned() == -1) {
                    cout << "here";
                    lock_guard<mutex> lock(mtx);
                    processes.push_front(currentProcess);
                    cv.notify_all();
                }*/
                //response = -3;
                //prevCycleCount = cycleCount;
            //}
        }
    }
    void printQueue() {
        for (int i = 0; i < processes->size(); i++) {
            cout << i << " is " << (*processes)[i]->getProcessName() << endl;
        }
    }
    ull getSize() {
        return processes->size();
    }
};

#endif