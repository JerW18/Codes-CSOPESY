#pragma once

#ifndef MANAGER_H
#define MANAGER_H

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
    thread workerThread;
    ull quantumCycles;
    ull delaysPerExec;
    string schedulerType;
    mutex mtx;
    MemoryAllocator *memoryAllocator;
	mutex* mainMtxAddress;
    bool isStarted = false;
	string memType;
	list <string>* runningProcesses;
	deque<shared_ptr<process>>* processes;

    void run() {
		int totalInstructionsExecuted = 0;
		while (!isStarted) {
			if (cycleCount % 10 == 0) {
                isStarted = true;
			}
			//cout << this->cpu_Id << " Waiting for start signal " << cycleCount  << endl;
        }
        if (isStarted) {
            while (true) {

                unique_lock<mutex> lock(mtx);
                if (!available && currentProcess != nullptr) {
                    int instructionsExecuted = 0;
                    if (currentProcess != nullptr && !currentProcess->hasMemoryAssigned()) {
						//lock_guard<mutex> lock(*mainMtxAddress);
                        cout << "You shouldnt be here" << endl;
                        currentProcess->assignCore(-1); // Unassign core
                        available = true;
                        currentProcess = nullptr;
                        continue;
                    }

                    if (schedulerType == "rr") {
                        while (currentProcess != nullptr && instructionsExecuted < quantumCycles &&
                            currentProcess->getInstructionIndex() < currentProcess->getTotalInstructions()) {

                            this_thread::sleep_for(chrono::milliseconds(100 * delaysPerExec));
                            currentProcess->incrementInstructionIndex();
                            instructionsExecuted++;
                            totalInstructionsExecuted++;
                            this_thread::sleep_for(chrono::milliseconds(100)); //100*qc count
                            logMemoryState(totalInstructionsExecuted);
                        }
                        if (currentProcess != nullptr && currentProcess->getInstructionIndex() >= currentProcess->getTotalInstructions()) {
                            if (currentProcess->getMemoryAddress() != nullptr) {
                                //cout << "here" << endl;
                                memoryAllocator->deallocate(currentProcess->getMemoryAddress(), currentProcess->getMemoryRequired(), memType, currentProcess->getProcessName());
                                currentProcess->assignMemoryAddress(nullptr);
                                currentProcess->setMemoryAssigned(false);
                                runningProcesses->remove(currentProcess->getProcessName());
                                memoryAllocator->removeRunningProcessId(currentProcess->getId());
                            }
                            currentProcess = nullptr;
                            this->available = true;
                            
                        }

                        else {
                            /*if (currentProcess->getMemoryAddress() != nullptr && memType == "Flat Memory") {
                                memoryAllocator->deallocate(currentProcess->getMemoryAddress(), currentProcess->getMemoryRequired(), memType, currentProcess->getProcessName());
                                currentProcess->assignMemoryAddress(nullptr);
                                currentProcess->setMemoryAssigned(false);
                            }*/
							//cout << currentProcess->getProcessName() << " is done running." << endl;
							//processes.push_back(currentProcess);
                            if (currentProcess != nullptr) {
								currentProcess->assignCore(-1);
                                runningProcesses->remove(currentProcess->getProcessName());
                                memoryAllocator->removeRunningProcessId(currentProcess->getId());
                                cout << "Process " << currentProcess->getProcessName() << " is done running." << endl;
								processes->push_back(currentProcess);
                                cout << "Added " << currentProcess->getProcessName() << " to the queue" << endl;
								currentProcess = nullptr;
                            }
                            available = true;
                        }
                        lock.unlock();

                    }
                    else if (schedulerType == "fcfs") {

                        while (currentProcess->getInstructionIndex() < currentProcess->getTotalInstructions()) {
                            this_thread::sleep_for(chrono::milliseconds(100 * delaysPerExec)); 
                            currentProcess->incrementInstructionIndex();
                            this_thread::sleep_for(chrono::milliseconds(100)); //100 * total isntructions
                        }

                        if (currentProcess != nullptr) {
                            currentProcess->assignCore(-1);
                            runningProcesses->remove(currentProcess->getProcessName());
                            memoryAllocator->removeRunningProcessId(currentProcess->getId());
                            memoryAllocator->deallocate(currentProcess->getMemoryAddress(), currentProcess->getMemoryRequired(), memType, currentProcess->getProcessName());
                            cout << "Process " << currentProcess->getProcessName() << " is done running." << endl;
                            currentProcess = nullptr;
                        }
                        available = true;
                        lock.unlock();
                    }
                }
                else {
                    this_thread::sleep_for(chrono::milliseconds(100));
                }
            }
        }
    }
    string getCurrentTimestamp() {
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
        size_t lastEndAddress=-1;
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

        int numProcessesInMemory = memoryAllocator->getNumOfProcesses();  
        int totalExternalFragmentation = memoryAllocator->getExternalFragmentation();  
        auto memoryState = memoryAllocator->getMemoryState(); 

        outFile << "Timestamp: " << timestamp << "\n";
        outFile << "Number of processes in memory: " << numProcessesInMemory << "\n";
        outFile << "Total external fragmentation in KB: " << totalExternalFragmentation << "\n\n";

        outFile << "----end---- = " << memoryAllocator->getTotalMemorySize() << "\n\n"; 
        
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
        size_t lastEndAddress=-1;
        bool changedProcess = false;

        std::string timestamp = getCurrentTimestamp();

        int numProcessesInMemory = memoryAllocator->getNumOfProcesses();
        int totalExternalFragmentation = memoryAllocator->getExternalFragmentation();
        auto memoryState = memoryAllocator->getMemoryState();

        cout << "Timestamp: " << timestamp << "\n";
        cout << "Number of processes in memory: " << numProcessesInMemory << "\n";
        cout << "Total external fragmentation in KB: " << totalExternalFragmentation << "\n\n";

        cout << "----end---- = " << memoryAllocator->getTotalMemorySize() << "\n\n\n";

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
	volatile ull& cycleCount;
    atomic<bool> available;
    /*CPUWorker(int id, ull quantumCycles, ull delaysPerExec, string schedulerType, MemoryAllocator* allocator, 
		mutex* mainMtxAddress, volatile ull& cycleCount, string memType)
        : cpu_Id(id), available(true), currentProcess(nullptr), quantumCycles(quantumCycles), delaysPerExec(delaysPerExec),
        schedulerType(schedulerType), cycleCount(cycleCount), memType(memType) {
        this->mainMtxAddress = mainMtxAddress;
		this->memoryAllocator = allocator;
        workerThread = thread(&CPUWorker::run, this);
        workerThread.detach();
    }*/

    CPUWorker(int id, ull quantumCycles, ull delaysPerExec, string schedulerType, MemoryAllocator* allocator,
        mutex* mainMtxAddress, volatile ull& cycleCount, string memType, list<string>* runningProcesses, deque<shared_ptr<process>>* processes)
        : cpu_Id(id), available(true), currentProcess(nullptr), quantumCycles(quantumCycles), delaysPerExec(delaysPerExec),
        schedulerType(schedulerType), cycleCount(cycleCount), memType(memType) {
		this->processes = processes;
		this->runningProcesses = runningProcesses;
        this->mainMtxAddress = mainMtxAddress;
        this->memoryAllocator = allocator;
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
    MemoryAllocator* allocator;
    mutex* mainMtxAddress;
	string memType;
	volatile ull& cycleCount;
    deque<shared_ptr<process>>* processes;
    deque<shared_ptr<process>>* backingStore;
	ull delaysPerExec;

public:

    list<string> runningProcesses;
    CPUManager(int numCpus, ull quantumCycles, ull delaysPerExec, string schedulerType, MemoryAllocator* allocator,
                mutex* mainMtxAddress, volatile ull& cycleCount, string memType, deque<shared_ptr<process>>* processes, deque<shared_ptr<process>>* backingStore)
        : numCpus(numCpus), cycleCount(cycleCount), memType(memType){
		this->delaysPerExec = delaysPerExec;
        this->mainMtxAddress = mainMtxAddress;
		this->allocator = allocator;
		this->processes = processes;
		this->backingStore = backingStore;
        for (int i = 0; i < numCpus; i++) {
            cpuWorkers.push_back(new CPUWorker(i, quantumCycles, delaysPerExec, schedulerType, allocator, mainMtxAddress, *&cycleCount, memType, addressof(runningProcesses), processes));
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


    int assignMemory(shared_ptr<process> proc) {
        if (!proc) {
            return -10;
        }
        lock_guard<mutex> lock(mtx);
        ull pos;
        string temp;
        if (!proc->hasMemoryAssigned()) {
            pair <void*, string> allocatedMemory = allocator->allocate(proc->getMemoryRequired(),memType, proc->getProcessName());

            if (allocatedMemory.first) {
                proc->assignMemory(allocatedMemory.first, proc->getMemoryRequired());
                proc->setMemoryAssigned(true);
				if (allocatedMemory.second == "") {//process did not kick anything out
					return -1;
				}
                pos = allocatedMemory.second.find('_');
				temp = allocatedMemory.second.substr(2);
                return stoi(temp);
            }
			else if (allocatedMemory.first == nullptr && allocatedMemory.second != "") {
				//cout << "Process " << proc->getProcessName() << " could not be allocated enough memory. Kicking out process " << allocatedMemory.second << " from memory." << endl;
				pos = allocatedMemory.second.find('_');
				temp = allocatedMemory.second.substr(2);
                return stoi(temp);
            }
            else {
                //memory allocation failed
				//cout << "No memory available for process " << proc->getProcessName() << endl;   
                return -100;
            }
		}
		else {
            //process has already has mem assigned
			return -2;
		}
    }

    atomic <bool> xd = true;
    int startProcess(shared_ptr<process> proc) {
        int response = -11;

        xd = true;
        if (!proc) {
            return response;
        }

        for (int i = 0; i < numCpus; i++) {
            if (cpuWorkers[i]->isAvailable() && cpuWorkers[i]->getCurrentProcess() == nullptr) {
                if (!proc->hasMemoryAssigned()) {
                    response = assignMemory(proc);

                    if (response == -100) {
                        {
                            lock_guard<mutex> lock(mtx);
                            processes->push_front(proc);
                        }
                        return response;
                    }
                    else if (response > -1) {
                        // a process was kicked out of memory.
                        //lock_guard<mutex> lock(mtx);
                        for (int i = 0; i < delaysPerExec+2; i++) {
                            this_thread::sleep_for(chrono::milliseconds(100));
                        }
						this_thread::sleep_for(chrono::milliseconds(100)); //100*numins of last process
                        handlePreemptedProcess(response);
                        if (xd) {
							handlePreemptedProcess(response);
                        }
                        
                    }


                    for (auto it = backingStore->begin(); it != backingStore->end(); it++) {
                        if ((*it)->getId() == proc->getId()) {
                            backingStore->erase(it);
                            // delete the file

                            std::filesystem::remove("backingstore\\process_" + to_string(proc->getId()) + ".txt");
                            break;
                        }
                    }
                }
                
                if (response != -100 && proc->hasMemoryAssigned()) {
                    // Step 2: Assign the process to the CPU core
					//cout << "Process " << proc->getProcessName() << " assigned to core " << i << endl;
                    lock_guard<mutex> lock(mtx);
                    proc->assignCore(i);
                    cpuWorkers[i]->assignScreen(proc);
					allocator->addRunningProcessId(proc->getId());
					runningProcesses.push_back(proc->getProcessName());
                    return -10; // Process successfully started
                }
            }
        }
		this_thread::sleep_for(chrono::milliseconds(100));

        // Step 3: No cores available, requeue the process
        {
			//cout << proc->getProcessName() << " here2";
            /*lock_guard<mutex> lock(mtx);
            processes->push_front(proc);*/
        }
        return response;
    }
	void printRunningProcesses() {
		cout << "CPUManager: Running processes:" << endl;
		for (auto p : runningProcesses) {
			cout << p << endl;
		}
		cout << endl;
	}
    // Helper to handle preempted process
    void handlePreemptedProcess(int preemptedProcessId) {
        //do {
            for (auto& p : *processes) {
                if (p == nullptr)
                    continue;
                //cout << p->getId() << "|" << preemptedProcessId << " ";
                if (p->getId() == preemptedProcessId && p->hasMemoryAssigned()) {

                    //cout << "Process " << p->getProcessName() << " was preempted and deallocated." << endl;
                    p->setMemoryAssigned(false);
                    allocator->deallocate(p->getMemoryAddress(), p->getMemoryRequired(), memType, p->getProcessName());
                    p->assignMemoryAddress(nullptr);
                    xd = false;
                    if (!p->isFinished()) {
                        backingStore->push_back(p);
                        // Create a new file for the process, store process id, current instruction index, and total instructions
                        // create backing store directory if doesnt exist
                        std::filesystem::create_directories("backingstore");
                        ofstream processFile("backingstore\\process_" + to_string(p->getId()) + ".txt");
                        processFile << "Process ID: " << p->getId() << endl;
                        processFile << "Current Instruction Index: " << p->getInstructionIndex() << endl;
                        processFile << "Total Instructions: " << p->getTotalInstructions() << endl;
                    }
                    break;
                }
            }
        //} while (xd);
        if (xd) {
            
			cout << "Process " << preemptedProcessId << " was not found in the queue, checking cpu." << endl;
            for (auto cpu : cpuWorkers) {
				if (cpu->available)
					continue;

				shared_ptr<process> currentProcess = cpu->getCurrentProcess();

                if (currentProcess->getId() == preemptedProcessId && currentProcess->hasMemoryAssigned()) {
					cout << "it didnt fucking work." << endl;
                }
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

#endif