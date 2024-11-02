#pragma once
#include "screen.h"
#include <thread>
#include <iostream>
#include <atomic>
#include "timeStamp.h"
#include <fstream>
#include <memory>

using namespace std;

class CPUWorker {
private:
    int cpu_Id;
    shared_ptr<process> currentProcess;
    atomic<bool> available;
    thread workerThread;
    ull quantumCycles;
    ull delaysPerExec;
    string schedulerType;
    mutex mtx;

    void run() {
        int instructionsExecuted = 0;
        while (true) {
            if (!available && currentProcess != nullptr) {
                instructionsExecuted = 0;
                if (schedulerType == "rr") {
                    

                    while (instructionsExecuted < quantumCycles &&
                        currentProcess->getInstructionIndex() < currentProcess->getTotalInstructions()) {
                        lock_guard<mutex> lock(mtx);

                        this_thread::sleep_for(chrono::milliseconds(100 * delaysPerExec));
                        currentProcess->incrementInstructionIndex();
                        instructionsExecuted++;
                        this_thread::sleep_for(chrono::milliseconds(100));
                    }

                    if (currentProcess->getInstructionIndex() >= currentProcess->getTotalInstructions()) {
                        available = true;
                        currentProcess = nullptr;
                    }
                    else {
                        lock_guard<mutex> lock(mtx);

                        available = true;
                        currentProcess->assignCore(-1);
                    }

                }
                else if (schedulerType == "fcfs") {
                    

                    while (currentProcess->getInstructionIndex() < currentProcess->getTotalInstructions()) {
                        lock_guard<mutex> lock(mtx);

                        this_thread::sleep_for(chrono::milliseconds(100 * delaysPerExec));
                        currentProcess->incrementInstructionIndex();
                        this_thread::sleep_for(chrono::milliseconds(100));
                    }
                    available = true;
                    currentProcess = nullptr;
                }
            }
        }
    }

public:
    CPUWorker(int id, ull quantumCycles, ull delaysPerExec, string schedulerType)
        : cpu_Id(id), available(true), currentProcess(nullptr), quantumCycles(quantumCycles), delaysPerExec(delaysPerExec), schedulerType(schedulerType) {
        workerThread = thread(&CPUWorker::run, this);
		workerThread.detach();
    }

    ~CPUWorker() {
        if (workerThread.joinable()) {
            workerThread.join();
        }
    }

    void assignScreen(shared_ptr<process> proc) {
        currentProcess = proc;
        available = false;
    }

    bool isAvailable() {
        return available;
    }

	shared_ptr<process> getCurrentProcess() {
		return currentProcess;
	}

	void setCurrentProcess(shared_ptr<process> proc) {
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


public:
    CPUManager(int numCpus, ull quantumCycles, ull delaysPerExec, string schedulerType) : numCpus(numCpus) {
        for (int i = 0; i < numCpus; i++) {
            cpuWorkers.push_back(new CPUWorker(i, quantumCycles, delaysPerExec, schedulerType));
        }
    }

	

	vector<shared_ptr<process>> isAnyoneAvailable() {
		vector<shared_ptr<process>> availableProcesses;
		for (int i = 0; i < numCpus; i++) {
			if (cpuWorkers[i]->isAvailable() && cpuWorkers[i]->getCurrentProcess()) {
				availableProcesses.push_back(cpuWorkers[i]->getCurrentProcess());
				cpuWorkers[i]->setCurrentProcessNull();
			}
		}
		return availableProcesses;
	}


    void startProcess(shared_ptr<process> proc) {
        if (!proc) {
            return;
        }
        for (int i = 0; i < numCpus; i++) {
            if (cpuWorkers[i]->isAvailable() && proc->getCoreAssigned() == -1 && cpuWorkers[i]->getCurrentProcess() == nullptr) {
                proc->assignCore(i);
                cpuWorkers[i]->assignScreen(proc);
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