#include <condition_variable> // Add this for condition variable

class RRScheduler {
private:
    CPUManager* cpuManager;
    mutex mtx;
    condition_variable cv; // Condition variable to notify when a new process is added
    deque<shared_ptr<process>> processes;

public:
    RRScheduler(CPUManager* cpuManager) : cpuManager(cpuManager) {}

    void addProcess(shared_ptr<process> process) {
        {
            lock_guard<mutex> lock(mtx);
            processes.push_back(process);
        }
        cv.notify_all();
    }

    vector<shared_ptr<process>> getReadyQueue() {
        lock_guard<mutex> lock(mtx);
        return vector<shared_ptr<process>>(processes.begin(), processes.end());
    }

    void start() {
        shared_ptr<process> currentProcess = nullptr;
        while (true) {

            vector<shared_ptr<process>> toAdd = cpuManager->isAnyoneAvailable();
            for (auto& p : toAdd) {
                addProcess(p);
				//cout << "Process added by CPUManager" << endl;
            }

            /*{
                unique_lock<mutex> lock(mtx);
                cv.wait(lock, [this] { return !processes.empty(); });

                currentProcess = processes.front();
                processes.pop_front();
            }*/
                
            {
                /*if (processes.empty()) {
                    this_thread::sleep_for(chrono::milliseconds(50));
                    continue;
                }
                {
                    lock_guard<mutex> lock(mtx);
                    currentProcess = processes.front();
                    processes.pop_front();
                }*/
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
        lock_guard<mutex> lock(mtx);
        cout << "Queue size: " << processes.size() << endl;
    }
};
