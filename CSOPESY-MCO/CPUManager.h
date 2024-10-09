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
		while (!available) {
			if (currentProcess != nullptr) {
				if (currentProcess->getInstructionIndex() < currentProcess->getTotalInstructions()) {
					currentProcess->incrementInstructionIndex();
				}
				else {
					available = true;
					currentProcess = nullptr;
				}
			}
		}
	}

	void assignScreen(process* process) {
		currentProcess = process;
	}

	void start() {
		available = false;
		thread t(&CPUWorker::run, this);
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
	}

	void startProcess(process* process) {
		for (int i = 0; i < numCpus; i++) {
			if (cpuWorkers[i].isAvailable()) {
				process->assignCore(i);
				cpuWorkers[i].assignScreen(process);
				cpuWorkers[i].start();
				break;
			}
		}
	}
	
	~CPUManager() {
		delete[] cpuWorkers;
	}

};