#pragma once
#include "screen.h"

using namespace std;	

class CPUWorker
{
public:
	CPUWorker(int cpuID) {
		this->cpuID = cpuID;
	}
	void setScreen(screen* screen) {
		currentScreen = screen;
	}
	void setAvailable(bool available) {
		this->available = available;
	}
	bool isAvailable() {
		return available;
	}
	bool isRunning() {
		return running;
	}

private:
	int cpuID;
	bool running = false;
	bool available = true;
	screen* currentScreen;
};

class CPUManager
{
private:

public:
	

	void startCPUs() {
		for (size_t i = 0; i < 4; i++) // TODO: Change 4 to config.txt variable next time
		{
			CPUWorker* cpu = new CPUWorker(i);
			thread t(&CPUManager::runCPU, this, cpu);
			t.detach();
		}
	}
};
