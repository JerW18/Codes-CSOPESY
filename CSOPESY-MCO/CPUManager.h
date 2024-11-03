#pragma once
#include "screen.h"
#include <thread>
#include <iostream>
#include <atomic>
#include "timeStamp.h"
#include <fstream>
#include <memory>
#include "TS.h"


using namespace std;

class CPUWorker {
private:
    int cpu_Id;
    process* currentProcess;
    atomic<bool> available;
    thread workerThread;
    ull quantumCycles;
    ull delaysPerExec;
    string schedulerType;
	TS <process* >* schedulerQueue;
    mutex mtx;

    void run() {
        int instructionsExecuted = 0;
        while (true) {
            
            lock_guard<mutex> lock(mtx);
            if (!available && currentProcess != nullptr) {
                instructionsExecuted = 0;
                if (schedulerType == "rr") {

                    while (instructionsExecuted < quantumCycles &&
                        currentProcess->getInstructionIndex() < currentProcess->getTotalInstructions()) {
                        //lock_guard<mutex> lock(mtx);

                        this_thread::sleep_for(chrono::milliseconds(100 * delaysPerExec));
                        currentProcess->incrementInstructionIndex();
                        instructionsExecuted++;
                        this_thread::sleep_for(chrono::milliseconds(100));
                    }

                    if (currentProcess->getInstructionIndex() >= currentProcess->getTotalInstructions()) {
						//lock_guard<mutex> lock(mtx);
                        available = true;
                        currentProcess = nullptr;
                    }
                    else {
                        //lock_guard<mutex> lock(mtx);
                        currentProcess->assignCore(-1);
						schedulerQueue->push(currentProcess);
                        available = true;
                    }

                }
                else if (schedulerType == "fcfs") {
                    

                    while (currentProcess->getInstructionIndex() < currentProcess->getTotalInstructions()) {
                        //lock_guard<mutex> lock(mtx);

                        this_thread::sleep_for(chrono::milliseconds(100 * delaysPerExec));
                        currentProcess->incrementInstructionIndex();
                        this_thread::sleep_for(chrono::milliseconds(100));
                    }
                    available = true;
                    currentProcess = nullptr;
                }
            }
			this_thread::sleep_for(chrono::milliseconds(1));
        }
    }

public:
    CPUWorker(int id, ull quantumCycles, ull delaysPerExec, string schedulerType, TS<process* >*schedulerQueue)
        : cpu_Id(id), available(true), currentProcess(nullptr), quantumCycles(quantumCycles), delaysPerExec(delaysPerExec), schedulerType(schedulerType) {
        workerThread = thread(&CPUWorker::run, this);
        this->schedulerQueue = schedulerQueue;
		workerThread.detach();
    }

    ~CPUWorker() {
        if (workerThread.joinable()) {
            workerThread.join();
        }
    }

    void assignScreen(process* proc) {
        currentProcess = proc;
        available = false;
    }

    bool isAvailable() {
        return available;
    }

	process* getCurrentProcess() {
		return currentProcess;
	}

	void setCurrentProcess(process* proc) {
		currentProcess = proc;
	}

	void setCurrentProcessNull() {
		currentProcess = nullptr;
	}
};


class CPUManager {
private:
    vector<CPUWorker*> cpuWorkers;
    int numCpus;
    mutex mtx;


public:
    CPUManager(int numCpus, ull quantumCycles, ull delaysPerExec, string schedulerType, TS<process *> *schedulerQueue) : numCpus(numCpus) {
        for (int i = 0; i < numCpus; i++) {
            cpuWorkers.push_back(new CPUWorker(i, quantumCycles, delaysPerExec, schedulerType, schedulerQueue));
        }
    }

	

	vector<process *> isAnyoneAvailable() {
		vector<process* > availableProcesses;
		for (int i = 0; i < numCpus; i++) {
			if (cpuWorkers[i]->isAvailable() && cpuWorkers[i]->getCurrentProcess() && cpuWorkers[i]->getCurrentProcess() != nullptr) {
				availableProcesses.push_back(cpuWorkers[i]->getCurrentProcess());
				cpuWorkers[i]->setCurrentProcessNull();
			}
		}
		return availableProcesses;
	}


    void startProcess(process* proc) {
        if (!proc) {
            return;
        }
		lock_guard<mutex> lock(mtx);
        for (int i = 0; i < numCpus; i++) {
            if (cpuWorkers[i]->isAvailable() && cpuWorkers[i]->getCurrentProcess() == nullptr) {
                proc->assignCore(i); // Assign the process to this idle core
                cpuWorkers[i]->assignScreen(proc);
                //cout << "Process " << proc->getId() << " assigned to idle core " << i << endl;
                return;
            }
        }
    }


	int getCoresAvailable() {
		int coresAvailable = 0;
		for (int i = 0; i < numCpus; i++) {
			if (cpuWorkers[i]->isAvailable()) {
				coresAvailable++;
			}
		}
		return coresAvailable;
	}

    ~CPUManager() {
        for (int i = 0; i < numCpus; i++) {
            delete cpuWorkers[i];
        }
    }
};