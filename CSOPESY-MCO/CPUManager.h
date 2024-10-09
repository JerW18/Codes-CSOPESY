#pragma once
#include "screen.h"
#include <thread>
#include <iostream>
#include <atomic> 

using namespace std;

class CPUWorker {
private:
    int cpu_Id;
    process* currentProcess;
    atomic<bool> available; 
    thread workerThread;

    void run() {
        while (true) {
            if (!available && currentProcess != nullptr) {
                while (currentProcess->getInstructionIndex() < currentProcess->getTotalInstructions()) {
                    currentProcess->incrementInstructionIndex();
                    this_thread::sleep_for(chrono::milliseconds(100));  
                }
                available = true;
                currentProcess = nullptr;
            }
            
            this_thread::sleep_for(chrono::milliseconds(10));
        }
    }

public:
    CPUWorker(int id) : cpu_Id(id), available(true), currentProcess(nullptr) {
        workerThread = thread(&CPUWorker::run, this);  
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
};


class CPUManager {
private:
    vector<CPUWorker*> cpuWorkers;
    int numCpus;

public:
    CPUManager(int numCpus) : numCpus(numCpus) {
        for (int i = 0; i < numCpus; i++) {
            cpuWorkers.push_back(new CPUWorker(i));
        }
    }

    void startProcess(process* proc) {
        while (true) {
            for (int i = 0; i < numCpus; i++) {
                if (cpuWorkers[i]->isAvailable()) {
                    proc->assignCore(i);  
                    cpuWorkers[i]->assignScreen(proc); 
                    return;
                }
            }
            // If no workers are available, wait briefly and retry
            this_thread::sleep_for(chrono::milliseconds(50));
        }
    }

    ~CPUManager() {
        for (int i = 0; i < numCpus; i++) {
            delete cpuWorkers[i];
        }
    }
};

