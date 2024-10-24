#pragma once
#include "screen.h"
#include <thread>
#include <iostream>
#include <atomic>
#include "timeStamp.h"
#include <fstream>
#include <semaphore>
#include <memory>

using namespace std;

class CPUWorker {
private:
    int cpu_Id;
    shared_ptr<process> currentProcess;
    atomic<bool> available;
    thread workerThread;
    int quantumCycles;
    int delaysPerExec;
    string schedulerType;
    counting_semaphore<>& clockSemaphore;

    void run() {
        while (true) {
            if (!available && currentProcess != nullptr) {
                int instructionsExecuted = 0;

                if (schedulerType == "rr") {
                    while (instructionsExecuted < quantumCycles &&
                        currentProcess->getInstructionIndex() < currentProcess->getTotalInstructions()) {
                        // Busy-wait for `delaysPerExec` cycles before executing the instruction
                        for (int i = 0; i < delaysPerExec; i++) {
                            this_thread::sleep_for(chrono::milliseconds(100));  // Simulate one CPU cycle of delay
                        }
                        currentProcess->incrementInstructionIndex();
                        instructionsExecuted++;
                        this_thread::sleep_for(chrono::milliseconds(100));  // Simulate work
                    }

                    if (currentProcess->getInstructionIndex() >= currentProcess->getTotalInstructions()) {
                        available = true;
                        currentProcess = nullptr;
                        clockSemaphore.release();  // Release semaphore when task is finished
                    }
                    else {
                        available = true;  // Mark worker available after quantum slice (RR logic)
                        clockSemaphore.release();  // Release semaphore even if quantum finished
                    }

                }
                else if (schedulerType == "fcfs") {
                    while (currentProcess->getInstructionIndex() < currentProcess->getTotalInstructions()) {
                        // Busy-wait for `delaysPerExec` cycles before executing the instruction
                        for (int i = 0; i < delaysPerExec; i++) {
                            this_thread::sleep_for(chrono::milliseconds(100));  // Simulate one CPU cycle of delay
                        }
                        currentProcess->incrementInstructionIndex();
                        this_thread::sleep_for(chrono::milliseconds(100));  // Simulate work
                    }
                    available = true;  // Mark the CPU as available
                    currentProcess = nullptr;  // Clear the process
                    clockSemaphore.release();  // Release semaphore after task completion
                }
            }

            this_thread::sleep_for(chrono::milliseconds(100));  // Sleep briefly to avoid busy-waiting
        }
    }

public:
    CPUWorker(int id, counting_semaphore<>& semaphore, int quantumCycles, int delaysPerExec, string schedulerType)
        : cpu_Id(id), available(true), currentProcess(nullptr), quantumCycles(quantumCycles), delaysPerExec(delaysPerExec), schedulerType(schedulerType), clockSemaphore(semaphore) {
        workerThread = thread(&CPUWorker::run, this);
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
};


class CPUManager {
private:
    vector<CPUWorker*> cpuWorkers;
    int numCpus;
    counting_semaphore<> clockSemaphore;

public:
    CPUManager(int numCpus, int quantumCycles, int delaysPerExec, string schedulerType) : numCpus(numCpus), clockSemaphore(numCpus) {
        for (int i = 0; i < numCpus; i++) {
            cpuWorkers.push_back(new CPUWorker(i, clockSemaphore, quantumCycles, delaysPerExec, schedulerType));
        }
    }


    void startProcess(shared_ptr<process> proc) {
        clockSemaphore.acquire();

        while (true) {
            for (int i = 0; i < numCpus; i++) {
                if (cpuWorkers[i]->isAvailable()) {
                    proc->assignCore(i);
                    cpuWorkers[i]->assignScreen(proc); 
                    return;
                }
            }
            this_thread::sleep_for(chrono::milliseconds(50));
        }
    }

    ~CPUManager() {
        for (int i = 0; i < numCpus; i++) {
            delete cpuWorkers[i];
        }
    }
};