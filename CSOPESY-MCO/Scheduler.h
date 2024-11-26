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
    MemoryAllocator* allocator;
    volatile ull& cycleCount;
    string memType;
public:
    deque<shared_ptr<process>> processes;
	deque<shared_ptr<process>> backingStore;
	/*Scheduler(CPUManager* cpuManager, MemoryAllocator& allocator) : allocator(allocator)
    {
        this->cpuManager = cpuManager;
    }*/

	// Scheduler(CPUManager* cpuManager, MemoryAllocator& allocator, volatile ull* cycleCount) : allocator(allocator), cycleCount(cycleCount)
    // {
    //     this->cpuManager = cpuManager;
    // }
	Scheduler(CPUManager* cpuManager, MemoryAllocator* allocator, volatile ull& cycleCount, string memType) : cycleCount(cycleCount), memType(memType)
    {
        this->cpuManager = cpuManager;
		this->allocator = allocator;
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
		boolean flag = true;
        while (true) {
            vector<shared_ptr<process>> toAdd = cpuManager->isAnyoneAvailable();
            for (auto& p : toAdd) {
                addProcess(p);
				//cout << "Process " << p->getProcessName() << " added to the queue" << endl;
            }
            //if (prevCycleCount != cycleCount) {

                {
                    unique_lock<mutex> lock(mtx);
                    if (cv.wait_for(lock, chrono::milliseconds(1), [this] { return !processes.empty(); })) {
                        currentProcess = processes.front();
						if (currentProcess != nullptr && currentProcess->isFinished()) {
							//memoryAllocator->deallocate(currentProcess->getMemoryAddress(), currentProcess->getMemoryRequired(), memType, currentProcess->getProcessName());
							processes.pop_front();
							continue;
						}
                        processes.pop_front();
                    }
                    else {
                        continue;
                    }
                }
				if (currentProcess == nullptr) {
					continue;
				}
                if (!currentProcess->hasMemoryAssigned()) {
                    response = cpuManager->assignMemory(currentProcess);
					//cout << "Current Process " << currentProcess->getProcessName() <<  " Response is " << response << endl;
                }
                if (response == -100) {
					//process was not assigned memory
					lock_guard<mutex> lock(mtx);
					processes.push_front(currentProcess);
					continue;
                }
                else if (response == -1){
                    //process was given memory without any preempt.
					//response = cpuManager->startProcess(currentProcess);
                }
                else if (response > -1) {
                    // a process was preempted and kicked out of the queue 
                    // find the process that was kicked out and tell it that it has no memory assigned
                    //lock_guard<mutex> lock(mtx);
                    for (auto& p : processes) {
                        if (p->getId() == response) {
                            //process found
                            lock_guard<mutex> lock(mtx);
                            p->setMemoryAssigned(false);
							allocator->deallocate(p->getMemoryAddress(),p->getMemoryRequired(), memType, p->getProcessName());
							p->assignMemoryAddress(nullptr);
                            if (!p->isFinished()) {
                                backingStore.push_back(p);
                            }
							//p->assignCore(-1);
                            flag = true;
                            break;
                        }
                        else {
							//process not found because it is still in the cpu
                            flag = false;
                        }
                    }
					if (flag == false) {
						//process not found in the deque. add current process back to the deque and try again.
						lock_guard<mutex> lock(mtx);
						processes.push_front(currentProcess);
						this_thread::sleep_for(chrono::milliseconds(1));
						cv.notify_all();
                        continue;
					}
                    
                }

                if (flag && currentProcess->hasMemoryAssigned()) {
                    //process has memory assigned and the other process was preempted and deallocated successful
                    response = cpuManager->startProcess(currentProcess);
                    if (response == -11 && currentProcess != nullptr && currentProcess->getCoreAssigned() == -1) {
						//no cpu available
                        lock_guard<mutex> lock(mtx);
                        processes.push_front(currentProcess);
                        cv.notify_all();
                    }
                }
			    
                

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
		for (int i = 0; i < processes.size(); i++) {
			cout << i << " is " << processes[i]->getProcessName() << endl;
		}
	}
    ull getSize() {
        return processes.size();
    }
};