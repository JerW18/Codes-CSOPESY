#pragma once
#include <queue>
#include "screen.h"
#include "CPUManager.h"
#include <mutex>

using namespace std;

class FCFSScheduler 
{
private:
	deque<process*> processes;
	CPUManager* cpuManager;
public:
	FCFSScheduler(CPUManager* cpuManager) {
		this->cpuManager = cpuManager;
	}

	void addProcess(process* process) {
		mutex mtx;
		lock_guard<mutex> lock(mtx);
		processes.push_back(process);
		cout << "FCFS Scheduler: Added process " << process->getProcessName() << " to queue" << endl;
		cout << processes.size() << endl;

		for (auto p : processes) {
			cout << p->getProcessName() << endl;
		}
	}

	void start() {
		process* currentProcess = NULL;
		while (true) {
			if (processes.empty()) {
				//cout << "FCFS Scheduler: No processes to executee" << endl;
				this_thread::sleep_for(chrono::milliseconds(10));
				continue;
			}
			//currentProcess = processes.front();
			//processes.pop();
			//cout << "FCFS Scheduler: Starting process " << currentProcess->getProcessName() << endl;
			//cpuManager->startProcess(currentProcess);
		}
	}
};


