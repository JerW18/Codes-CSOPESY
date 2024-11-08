#pragma once
#include "screen.h"
#include <thread>
#include <iostream>
#include <atomic>
#include "timeStamp.h"
#include <fstream>
#include <memory>
#include "MemoryAllocator.h"
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#include <filesystem>

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
    MemoryAllocator &memoryAllocator;

    void run() {
		int totalInstructionsExecuted = 0;
        while (true) {
            
            lock_guard<mutex> lock(mtx);
            if (!available && currentProcess != nullptr) {
                int instructionsExecuted = 0;
                if (schedulerType == "rr") {

                    while (instructionsExecuted < quantumCycles &&
                        currentProcess->getInstructionIndex() < currentProcess->getTotalInstructions()) {

                        this_thread::sleep_for(chrono::milliseconds(100 * delaysPerExec));
                        currentProcess->incrementInstructionIndex();
                        instructionsExecuted++;
                        totalInstructionsExecuted++;
                        this_thread::sleep_for(chrono::milliseconds(100));
						logMemoryState(totalInstructionsExecuted);
                    }
                    if (currentProcess->getInstructionIndex() >= currentProcess->getTotalInstructions()) {
                        if (currentProcess->getMemoryAddress() != nullptr) {
                            memoryAllocator.deallocate(currentProcess->getMemoryAddress(), currentProcess->getMemoryRequired());
                            currentProcess->assignMemoryAddress(nullptr);
                            currentProcess->setMemoryAssigned(false);
                        }
                        this->available = true;
                        currentProcess = nullptr;
                        
                    }

                    else {
                        /*if (currentProcess->getMemoryAddress() != nullptr) {
                            memoryAllocator.deallocate(currentProcess->getMemoryAddress(), currentProcess->getMemoryRequired());
							currentProcess->assignMemoryAddress(nullptr);
							currentProcess->setMemoryAssigned(false);
                        }*/
                        currentProcess->assignCore(-1);
                        available = true;
                    }

                }
                else if (schedulerType == "fcfs") {
                    
                    while (currentProcess->getInstructionIndex() < currentProcess->getTotalInstructions()) {
                        this_thread::sleep_for(chrono::milliseconds(100 * delaysPerExec));
                        currentProcess->incrementInstructionIndex();
                        this_thread::sleep_for(chrono::milliseconds(100));
                    }
                   
                    available = true;
                    memoryAllocator.deallocate(currentProcess->getMemoryAddress(), currentProcess->getMemoryRequired());
                    currentProcess = nullptr;
                    
                }
            }
            else { 
                this_thread::sleep_for(chrono::milliseconds(100));
            }
        }
    }
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        std::tm localTime;

        localtime_s(&localTime, &time);

        std::stringstream timestamp;
        timestamp << std::put_time(&localTime, "%m/%d/%Y %I:%M:%S %p");
        return timestamp.str();
    }

    void logMemoryState(int quantumCycle) {

        string lastPrintedProcessName = "";
        size_t lastEndAddress;
		bool changedProcess = false;

        std::string timestamp = getCurrentTimestamp();
        std::stringstream filename;
        filename << "logs\\memory_stamp_" << std::setw(2) << std::setfill('0') << quantumCycle << ".txt";

        std::filesystem::create_directories("logs");

        std::ofstream outFile(filename.str(), std::ofstream::out);
        if (!outFile) {
            std::cerr << "Failed to open file for logging memory state." << std::endl;
            return;
        }

        int numProcessesInMemory = memoryAllocator.getNumOfProcesses();  
        int totalExternalFragmentation = memoryAllocator.getExternalFragmentation();  
        auto memoryState = memoryAllocator.getMemoryState(); 

        outFile << "Timestamp: " << timestamp << "\n";
        outFile << "Number of processes in memory: " << numProcessesInMemory << "\n";
        outFile << "Total external fragmentation in KB: " << totalExternalFragmentation << "\n\n";

        outFile << "----end---- = " << memoryAllocator.getTotalMemorySize() << "\n\n"; 
        
        for (auto it = memoryState.rbegin(); it != memoryState.rend(); ++it) {
            if (!it->isFree) {
                if (it->processName != lastPrintedProcessName) {
                    if (changedProcess) {
                        outFile << lastEndAddress << "\n\n";
                    }
                    outFile << it->endAddress << "\n";
                    outFile << it->processName << "\n";
					
                    lastPrintedProcessName = it->processName;
                    changedProcess = true;
                }
                lastEndAddress = it->startAddress;
            }
        }

        if (changedProcess && !memoryState.empty()) {
            outFile << memoryState.front().startAddress << "\n\n";
        }
        outFile << "----start---- = 0\n";

        outFile.close();
    }

    void printMemoryState(int quantumCycle) {

        string lastPrintedProcessName = "";
        size_t lastEndAddress;
        bool changedProcess = false;

        std::string timestamp = getCurrentTimestamp();

        int numProcessesInMemory = memoryAllocator.getNumOfProcesses();
        int totalExternalFragmentation = memoryAllocator.getExternalFragmentation();
        auto memoryState = memoryAllocator.getMemoryState();

        cout << "Timestamp: " << timestamp << "\n";
        cout << "Number of processes in memory: " << numProcessesInMemory << "\n";
        cout << "Total external fragmentation in KB: " << totalExternalFragmentation << "\n\n";

        cout << "----end---- = " << memoryAllocator.getTotalMemorySize() << "\n\n\n";

        for (auto it = memoryState.rbegin(); it != memoryState.rend(); ++it) {
            if (!it->isFree) {
                if (it->processName != lastPrintedProcessName) {
                    if (changedProcess) {
                        cout << lastEndAddress << "\n\n\n";
                    }
                    cout << it->endAddress << "\n";
                    cout << it->processName << "\n";

                    lastPrintedProcessName = it->processName;
                    changedProcess = true;
                }
                lastEndAddress = it->startAddress;
            }
        }

        if (changedProcess && !memoryState.empty()) {
            cout << memoryState.front().startAddress << "\n\n\n";  
        }
        cout << "----start---- = 0\n";
    }

public:
    CPUWorker(int id, ull quantumCycles, ull delaysPerExec, string schedulerType, MemoryAllocator& allocator)
        : cpu_Id(id), available(true), currentProcess(nullptr), quantumCycles(quantumCycles), delaysPerExec(delaysPerExec), schedulerType(schedulerType), memoryAllocator(allocator) {
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
    mutex mtx;
    MemoryAllocator& allocator;

public:
    CPUManager(int numCpus, ull quantumCycles, ull delaysPerExec, string schedulerType, MemoryAllocator& allocator) : numCpus(numCpus), allocator(allocator) {
        for (int i = 0; i < numCpus; i++) {
            cpuWorkers.push_back(new CPUWorker(i, quantumCycles, delaysPerExec, schedulerType, allocator));
        }
    }

	

	vector<shared_ptr<process>> isAnyoneAvailable() {
		lock_guard<mutex> lock(mtx);
		vector<shared_ptr<process>> availableProcesses;
		for (int i = 0; i < numCpus; i++) {
			if (cpuWorkers[i]->isAvailable() && cpuWorkers[i]->getCurrentProcess()) {
				availableProcesses.push_back(cpuWorkers[i]->getCurrentProcess());
				cpuWorkers[i]->setCurrentProcessNull();
			}
		}
		return availableProcesses;
	}


    int startProcess(shared_ptr<process> proc) {
        if (!proc) {
            return -10;
        }
        lock_guard<mutex> lock(mtx);
        if (!proc->hasMemoryAssigned()) {
            void* allocatedMemory = allocator.allocate(proc->getMemoryRequired(), "FirstFit", proc->getProcessName());
            if (allocatedMemory) {
                proc->assignMemory(allocatedMemory, proc->getMemoryRequired());
				proc->setMemoryAssigned(true);
            }
            else {
                return 1;
            }
        }
        

        for (int i = 0; i < numCpus; i++) {
            if (cpuWorkers[i]->isAvailable() && cpuWorkers[i]->getCurrentProcess() == nullptr) {
                proc->assignCore(i);
                cpuWorkers[i]->assignScreen(proc);
                return 0;
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