#pragma once
#include <queue>
#include "screen.h"
#include "CPUManager.h"
#include <mutex>
#include <memory>
#include <condition_variable> 

using namespace std;

class Scheduler
{
private:
    CPUManager* cpuManager;
    mutex mtx;
    condition_variable cv;

public:
    deque<shared_ptr<process>> processes;
    Scheduler(CPUManager* cpuManager) {
        this->cpuManager = cpuManager;
    }

    void addProcess(shared_ptr<process> process) {
        {
            lock_guard<mutex> lock(mtx);
            processes.push_back(process);
        }
        cv.notify_all();
    }
    deque<shared_ptr<process>> getReadyQueue() {
        lock_guard<mutex> lock(mtx);
        return processes;
    }

    void starFCFS() {
        shared_ptr<process> currentProcess = nullptr;
        while (true) {
            if (processes.empty()) {
                this_thread::sleep_for(chrono::milliseconds(100));
                continue;
            }
            {
                lock_guard<mutex> lock(mtx);
                currentProcess = processes.front();
                processes.pop_front();
            }
            cpuManager->startProcess(currentProcess);
            if (currentProcess->getCoreAssigned() == -1) {
                lock_guard<mutex> lock(mtx);
                processes.push_front(currentProcess);
            }
        }
    }

    void startRR() {
        shared_ptr<process> currentProcess = nullptr;
        while (true) {

            vector<shared_ptr<process>> toAdd = cpuManager->isAnyoneAvailable();
            for (auto& p : toAdd) {
                addProcess(p);
            }

            {
                unique_lock<mutex> lock(mtx);
                if (cv.wait_for(lock, chrono::milliseconds(100), [this] { return !processes.empty(); })) {
                    currentProcess = processes.front();
                    processes.pop_front();
                }
                else {
                    continue;
                }
            }

            cpuManager->startProcess(currentProcess);

            if (currentProcess != nullptr && currentProcess->getCoreAssigned() == -1) {
                lock_guard<mutex> lock(mtx);
                processes.push_front(currentProcess);
                cv.notify_all();
            }
        }
    }

    void getSize() {
        cout << processes.size() << endl;
    }
};