#pragma once
#include <queue>
#include "screen.h"
#include "CPUManager.h"

using namespace std;

class FCFSScheduler 
{
private:
	queue<process*> processes;
	CPUManager* cpuManager;
public:
	FCFSScheduler(CPUManager* cpuManager) {
		this->cpuManager = cpuManager;
	}

	void addProcess(process* process) {
		processes.push(process);
	}

	void start() {
		while (!processes.empty()) {
			process* currentProcess = processes.front();
			processes.pop();
			cpuManager->startProcess(currentProcess);
		}
	}
};


