#pragma once
#include "screen.h"
#include <thread>
#include <iostream>

using namespace std;	

class CPUWorker
{
private:
	int id; 
	int cpu_Id;
	process* currentProcess;
	bool available;
	thread t;

public:
	CPUWorker() {
		cpu_Id = id++;
		available = true;
		currentProcess = nullptr;
	}


	void run() {
		while (true) {
			if (currentProcess != nullptr) {
				if (currentProcess->getInstructionIndex() < currentProcess->getTotalInstructions()) {
					currentProcess->incrementInstructionIndex();
				}
				else {
					available = true;
				}
			}
		}
	}

	void assignScreen(process* process) {
		currentProcess = process;
		available = false;
	}

	void start() {
		t = thread(&CPUWorker::run, this);
		t.detach();
	}

	bool isAvailable() {
		return available;
	}
};

class CPUManager
{
private:
	CPUWorker* cpuWorkers;
	int numCpus;
public:
	CPUManager(int numCpus) {
		cpuWorkers = new CPUWorker[numCpus];
		this->numCpus = numCpus;
		for (int i = 0; i < numCpus; i++)
		{
			cpuWorkers[i].start();
		}
	}

	void startProcess(process* process) {
		for (int i = 0; i < numCpus; i++) {
			if (cpuWorkers[i].isAvailable()) {
				process->assignCore(i);
				cpuWorkers[i].assignScreen(process);
				// cpuWorkers[i].start();
				break;
			}
		}
	}
	
	~CPUManager() {
		delete[] cpuWorkers;
	}

};