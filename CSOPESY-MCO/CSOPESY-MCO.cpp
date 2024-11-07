#include <windows.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <map>
#include <sstream>
#include <vector>
#include "CSOPESY-MCO.h"
#include "timeStamp.h"
#include "screen.h"
#include "global.h"
#include "CPUManager.h"
#include "Scheduler.h"
#include <mutex>
#include "MemoryAllocator.h"

typedef unsigned long long ull;
using namespace std;

mutex mtx;
screenManager* sm;
global g;
thread schedulerThread;
CPUManager* cpuManager;
//Sheduler* rrScheduler;
//Sheduler* fcfsScheduler;
Scheduler* processScheduler;
thread processThread;
unique_ptr<MemoryAllocator> memoryAllocator;

bool inScreen = false;
bool initialized = false; 
bool makeProcess = false;

int numCPU = 4;
string schedulerType = "rr";
ull quantumCycles = 5;
ull batchProcessFreq = 1;
ull minInstructions = 1000;
ull maxInstructions = 2000;
ull delaysPerExec = 0;

ull maxOverallMem = 16384;
ull memPerFrame = 16;
ull maxMemPerProcess = 4096;
ull minMemPerProcess = 4096;
ull memPerProc = 4096;

ull totalFrames = maxOverallMem / memPerFrame;
bool useFlat = maxOverallMem == memPerFrame;
/*
mem-per-frame 
The size of memory in KB per frame. This is also the memory size per page. 
The total number of frames is equal to max-overall-mem / mem-per-frame. 
If max-overall-mem = mem-per-frame, then the emulator will use a flat memory allocator.

min-mem-per-proc / max-mem-per-proc
Memory required for each process 
Let P be the number of pages required by a process 
and M is the rolled value between min-mem-per-proc and max-mem-proc. 
P can be computed as M/ mem-per-frame.

*/


string trim(const string& str) {
    size_t start = str.find_first_not_of(" \t\"");
    size_t end = str.find_last_not_of(" \t\"");
    return (start == string::npos || end == string::npos) ? "" : str.substr(start, end - start + 1);
}

void readConfig(const string& filename) {
    ifstream configFile(filename);
    if (!configFile.is_open()) {
        cout << "Error: Could not open config file. Using stored defaults.\n" << endl;
        return;
    }

    string line;
    while (getline(configFile, line)) {
        stringstream ss(line);
        string key, value;

        if (ss >> key) {
            ss >> ws;
            getline(ss, value);

            if (key == "num-cpu") {
                numCPU = stoi(value);
                if (stoi(value) < 1 || stoi(value) > 128) {
                    cout << "Error: num-cpu must be within the range of 1 to 128. Default value of 4 will be set\n" << endl;
                    numCPU = 4;
                }
            }
            else if (key == "scheduler") {
                // schedulerType should only be fcfs or rr
                schedulerType = trim(value);

                if (schedulerType != "fcfs" && schedulerType != "rr") {
                    cout << "Error: Unknown scheduler type specified in config file. Using default value of 'fcfs'.\n" << endl;
                    schedulerType = "fcfs";
                }
            }
            else if (key == "quantum-cycles") {
                quantumCycles = stoull(value);
                if (quantumCycles < 1 || quantumCycles > 4294967296ULL) {
                    cout << "Error: quantum-cycles must be between 1 and 2^32. Using default value of 5.\n" << endl;
                    quantumCycles = 5;
                }

            }
            else if (key == "batch-process-freq") {
                batchProcessFreq = stoull(value);
                if (batchProcessFreq < 1 || batchProcessFreq > 4294967296ULL) {
                    cout << "Error: batch-process-freq must be between 1 and 2^32. Using default value of 1.\n" << endl;
                    batchProcessFreq = 1;

                }
            }
            else if (key == "min-ins") {
                minInstructions = stoull(value);
                if (minInstructions < 1 || minInstructions > 4294967296ULL) {
                    cout << "Error: min-ins must be between 1 and 2^32. Using default value of 1000.\n" << endl;
                    minInstructions = 1000;
                }
            }
            else if (key == "max-ins") {
                maxInstructions = stoull(value);
                if (maxInstructions < minInstructions) {
                    cout << "Error: max-ins must be greater than or equal to min-ins. Using value min-ins + 1.\n" << endl;
                    maxInstructions = minInstructions + 1;
                }
                if (maxInstructions < 1 || maxInstructions > 4294967296ULL) {
                    cout << "Error: max-ins must be between 1 and 2^32. Using default value of 2000.\n" << endl;
                    maxInstructions = 2000;
                }
            }
            else if (key == "delays-per-exec") {
                delaysPerExec = stoull(value);
                if (delaysPerExec < 0 || delaysPerExec >(4294967296ULL)) {
                    cout << "Error: delays-per-exec must be between 0 and 2^32. Using default value of 0.\n" << endl;
                    delaysPerExec = 0;
                }
            }
            else if (key == "max-overaall-mem") {
				maxOverallMem = stoull(value);
				if (maxOverallMem < 2 || maxOverallMem > 4294967296ULL) {
					cout << "Error: max-overall-mem must be between 1 and 2^32. Using default value of 16384.\n" << endl;
					maxOverallMem = 16384;
				}
            }
            else if (key == "mem-per-frame") {
				memPerFrame = stoull(value);
				if (memPerFrame < 2 || memPerFrame > 4294967296ULL) {
					cout << "Error: mem-per-frame must be between 1 and 2^32. Using default value of 16.\n" << endl;
					memPerFrame = 16;
				}
			}
			else if (key == "max-mem-per-proc") {
				maxMemPerProcess = stoull(value);
                if (maxMemPerProcess < 2 || maxMemPerProcess > 4294967296ULL) {
                    cout << "Error: max-mem-per-proc must be between 1 and 2^32. Using default value of 4096.\n" << endl;
                    maxMemPerProcess = 4096;
                }
            }
            else if (key == "min-mem-per-proc") {
				minMemPerProcess = stoull(value);
                if (minMemPerProcess < 2 || minMemPerProcess > 4294967296ULL) {
					cout << "Error: min-mem-per-proc must be between 1 and 2^32. Using default value of 4096.\n" << endl;
					minMemPerProcess = 4096;
                }
            }
            else if (key == "mem-per-proc") {
                memPerProc = stoull(value);
                if (memPerProc < 2 || memPerProc > 4294967296ULL) {
                    cout << "Error: mem-per-proc must be between 1 and 2^32. Using default value of 4096.\n" << endl;
                    memPerProc = 4096;
                }
            }
            totalFrames = maxOverallMem / memPerFrame;
            useFlat = maxOverallMem == memPerFrame;
        }
    }
    configFile.close();
}

void displayConfig() {
	cout << "num-cpu: " << numCPU << endl;
	cout << "scheduler: " << schedulerType << endl;
	cout << "quantum-cycles: " << quantumCycles << endl;
	cout << "batch-process-freq: " << batchProcessFreq << endl;
	cout << "min-ins: " << minInstructions << endl;
	cout << "max-ins: " << maxInstructions << endl;
	cout << "delays-per-exec: " << delaysPerExec << endl;
	cout << "max-overall-mem: " << maxOverallMem << endl;
	cout << "mem-per-frame: " << memPerFrame << endl;
	cout << "max-mem-per-proc: " << maxMemPerProcess << endl;
	cout << "min-mem-per-proc: " << minMemPerProcess << endl;
	cout << "mem-per-proc: " << memPerProc << endl;
	cout << "total-frames: " << totalFrames << endl; 
    cout << "use-flat: " << (useFlat ? "yes" : "no") << "\n" << endl;

}


void initialize() {
    if (!initialized) {
        cout << "'initialize' command recognized. Starting scheduler.\n" << endl;
		lock_guard<mutex> lock(mtx);
		readConfig("config.txt");
        memoryAllocator = make_unique<MemoryAllocator>(maxOverallMem, memPerFrame);
        cpuManager = new CPUManager(numCPU, quantumCycles, delaysPerExec, schedulerType, *memoryAllocator);
        processScheduler = new Scheduler(cpuManager, *memoryAllocator);

        // Initialize screenManager with MemoryAllocator
        sm = new screenManager(&mtx, *memoryAllocator);


        schedulerThread = (schedulerType == "fcfs")
            ? thread(&Scheduler::starFCFS, processScheduler)
            : (schedulerType == "rr")
            ? thread(&Scheduler::startRR, processScheduler)
            : thread();
		schedulerThread.detach();
        initialized = true;

        /*if (schedulerType == "fcfs") {
            schedulerThread = thread(&Scheduler::starFCFS, processScheduler);
			schedulerThread.detach();
        }
        else if (schedulerType == "rr") {
            schedulerThread = thread(&Scheduler::startRR, processScheduler);
            schedulerThread.detach();
        }
        else {
            cout << "Error: Unknown scheduler type specified in config file.\n" << endl;
            return;
        }*/

    }
    else {
        cout << "'initialize' command has already been executed.\n" << endl;
    }
}

ull randomInsLength() {
    return rand() % (maxInstructions - minInstructions + 1) + minInstructions;
}
ull randomMemLength() {
	return rand() % (maxMemPerProcess - minMemPerProcess + 1) + minMemPerProcess;
}


void schedStartThread();
void schedStop();
void schedStart() {
    if (initialized && !makeProcess) {
        cout << "Starting scheduler.\n" << endl;
        makeProcess = true;
        processThread = thread(schedStartThread);
    }
    else {
        cout << "Error: Scheduler not initialized or running already. Use 'initialize' command first.\n" << endl;
    }
}

bool firstProcess = true;
void schedStartThread() {
    while (makeProcess) {
        unique_lock<mutex> lock(mtx);
        ull i = sm->getProcessCount();
        ull numIns = randomInsLength();
		ull memoryReq = memPerProc; //randomMemLength();

        string processName = "p_" + to_string(i);

        // Allocate memory and add process with FirstFit strategy
        //sm->addProcess(processName, numIns, memoryReq, "FirstFit");
		sm->addProcess(processName, numIns, memoryReq);
        processScheduler->addProcess(sm->processes.back());

        lock.unlock();
        this_thread::sleep_for(chrono::milliseconds(batchProcessFreq * 100));
    }
}



void schedStop() {
	if (initialized) {
        if(makeProcess) {
			cout << "Stopping scheduler." << endl;
			makeProcess = false;
			if (processThread.joinable()) {
                processThread.join();
				cout << "Scheduler stopped.\n" << endl;
			}
		}
		else {
			cout << "Scheduler is already stopped.\n" << endl;
		}
	}
	else {
		cout << "Error: Scheduler not initialized. Use 'initialize' command first.\n" << endl;
	}
}

void report() {
    if (!initialized) {
        cout << "Error: Scheduler not initialized. Use 'initialize' command first.\n" << endl;
        return;
    }

    unique_lock<mutex> lock(mtx);
    ofstream reportFile("report.txt");
    if (!reportFile.is_open()) {
        cout << "Error: Could not open report file." << endl;
        return;
    }

    reportFile << "CPU Utilization: " << ((float)(numCPU - cpuManager->getCoresAvailable()) / numCPU) * 100 << "%" << endl;
   
    reportFile.close();

    cout << "Report generated.\n" << endl;
}


void screens(const string& option, const string& name) {
    if (!initialized) {
        cout << "Error: Scheduler not initialized. Use 'initialize' command first.\n" << endl;
        return;
    }

    if (option == "-r") {
        std::unique_lock<std::mutex> lock(mtx);
        for (auto screen : sm->processes) {
            if (screen->getProcessName() == name) {
                ull id = screen->getId();
                cout << "Reattaching to screen session: " << name << endl;
                inScreen = true;
                sm->reattachProcess(name, id);
                return;
            }
        }
        cout << "Screen not found. Try a different name or use screen -s <name> to start a new screen.\n" << endl;
		lock.unlock();
    }
    else if (option == "-s") {
        std::unique_lock<std::mutex> lock(mtx);
        for (auto screen : sm->processes) {
            if (screen->getProcessName() == name) {
                cout << "Screen already exists. Try a different name or use screen -r <name> to reattach it.\n" << endl;
                return;
            }
        }
        cout << "Starting new terminal session: " << name << endl;

        ull instructions = randomInsLength();
        sm->addProcessManually(name, instructions, memPerProc, "FirstFit");

        shared_ptr<process> newProcess = sm->processes.back();
        processScheduler->addProcess(newProcess);

        /*if (schedulerType == "fcfs" && processScheduler != nullptr) {
            processScheduler->addProcess(newProcess);
        }
        else if (schedulerType == "rr" && processScheduler != nullptr) {
            processScheduler->addProcess(newProcess);
        }
        else {
            cout << "Error: Scheduler not initialized or unknown scheduler type.\n" << endl;
        }*/

        inScreen = true;
		lock.unlock();

    }
    else if (option == "-ls") {
        std::unique_lock<std::mutex> lock(mtx);

        cout << "CPU Utilization: " << ((float)(numCPU - cpuManager->getCoresAvailable()) / numCPU) * 100 << "%" << endl;
        cout << "Cores Used: " << numCPU - cpuManager->getCoresAvailable() << endl;
        cout << "Cores Available: " << cpuManager->getCoresAvailable() << endl;
        cout << "----------------------------------" << endl;

        cout << "Running Processes:" << endl;
        for (auto& screen : sm->processes) {
            if (!screen->isFinished() && screen->getCoreAssigned() != -1) {
                cout << screen->getProcessName() << " ("
                    << screen->getDateOfBirth() << ") Core: "
                    << screen->getCoreAssigned() << " Running "
                    << screen->getInstructionIndex() << " / "
                    << screen->getTotalInstructions() << " Memory: "
                    << screen->getMemoryRequired() << " / " <<
                    (screen->getMemoryAddress() == nullptr ? "Not allocated" : "Allocated") <<
                    endl;
            }
        }

        cout << endl;

        cout << "Ready Processes (Not in Queue Order):" << endl;
        for (auto& screen : sm->processes) {
            if (!screen->isFinished() && screen->getCoreAssigned() == -1) {
                cout << screen->getProcessName() << " ("
                    << screen->getDateOfBirth() << ") Core: None"
                    << " Ready "
                    << screen->getInstructionIndex() << " / "
                    << screen->getTotalInstructions() << " Memory: "
                    << screen->getMemoryRequired() << " / " <<
                    (screen->getMemoryAddress() == nullptr ? "Not allocated" : "Allocated") <<
                    endl;
            }
        }

        cout << endl;
        cout << "Finished Processes:" << endl;

        for (auto& screen : sm->processes) {
            if (screen->isFinished()) {
                cout << screen->getProcessName() << " ("
                    << screen->getDateOfBirth() << ") Finished "
                    << screen->getTotalInstructions() << " / "
                    << screen->getTotalInstructions() << " Memory used: "
                    << screen->getMemoryRequired() <<
                    endl;
            }
        }

        cout << "----------------------------------" << endl;

        // Number of processes in memory
        int processCount = 0;
        for (const auto& process : sm->processes) {
            //if (process->getMemoryAddress() != nullptr) processCount++;
            if (process->hasMemoryAssigned()) processCount++;
        }
        cout << "Number of processes in memory: " << processCount << "\n";

        // Total external fragmentation
        size_t totalFragmentation = 0;
        bool inFreeBlock = false;
        size_t currentFreeBlockSize = 0;

        for (bool allocated : memoryAllocator->getAllocationMap()) {
            if (!allocated) {
                if (!inFreeBlock) {
                    inFreeBlock = true;
                    currentFreeBlockSize = memoryAllocator->getFrameSize();
                }
                else {
                    currentFreeBlockSize += memoryAllocator->getFrameSize();
                }
            }
            else if (inFreeBlock) {
                inFreeBlock = false;
                totalFragmentation += currentFreeBlockSize;
            }
        }
        if (inFreeBlock) {
            totalFragmentation += currentFreeBlockSize;
        }
        cout << "Total external fragmentation in KB: " << totalFragmentation / 1024 << "\n";

        // ASCII memory layout with boundaries
        cout << "---end---- = " << maxOverallMem << "\n";
        size_t address = maxOverallMem;
        bool previousAllocated = false;

        for (int i = memoryAllocator->getAllocationMap().size() - 1; i >= 0; --i) {
            address -= memoryAllocator->getFrameSize();

            if (memoryAllocator->getAllocationMap()[i]) {
                // Find the process that owns this memory frame
                for (const auto& process : sm->processes) {
                    if (process->getMemoryAddress() == &(memoryAllocator->getMemory())[i * memoryAllocator->getFrameSize()]) {
                        if (!previousAllocated) {
                            cout << address + memoryAllocator->getFrameSize() << "\n"; // Upper limit of the process
                        }
                        cout << address << " " << process->getProcessName() << "\n";
                        previousAllocated = true;
                        break;
                    }
                }
            }
            else {
                if (previousAllocated) {
                    cout << address + memoryAllocator->getFrameSize() << " [Free]\n"; // Lower limit after the last process block
                }
                previousAllocated = false;
            }
        }

        //memoryAllocator->printAllocationMap();

        cout << "----------------------------------\n" << endl;
        lock.unlock();
    }
    else {
        cout << "Invalid screen option: " << option << "\n" << endl;
    }
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    g.printHeader();
}

void exitProgram() {
    cout << "Exiting program." << endl;

    makeProcess = false;

    if (processThread.joinable()) {
        processThread.join();
    }

    if (schedulerThread.joinable()) {
        schedulerThread.join();
    }

    cout << "Program exited successfully." << endl;
    exit(0);
}


map<string, void (*)()> commands = {
    {"sst", schedStart},
	{"ssp", schedStop},
    {"display-config", displayConfig},
    {"report-util", report},
	{"scheduler-test", schedStart},
    {"scheduler-stop", schedStop},
    {"initialize", initialize},
    {"clear", clearScreen},
    {"exit", exitProgram}
};


vector<string> splitInput(const string& input) {
    vector<string> result;
    istringstream iss(input);
    for (string s; iss >> s;) {
        result.push_back(s);
    }
    return result;
}


void test() {
    string input;
    while (true) {
        cout << "Enter command: ";
        getline(cin, input);
        vector<string> tokens = splitInput(input);

        if (tokens.empty()) {
            cout << "Invalid command.\n" << endl;
            continue;
        }

        string command = tokens[0];

        if (command == "screen" && tokens.size() >= 3) {
            screens(tokens[1], tokens[2]);
        }
        else if (command == "screen" && tokens.size() == 2) {
            screens(tokens[1], "");
        }
        else if (commands.find(command) != commands.end()) {
            commands[command]();
        }
        else {
            cout << "Invalid command '" << command << "'\n" << endl;
        }
    }
}

int main() {
    g.printHeader();

    thread testThread(test);
    testThread.join();

    return 0;
}
