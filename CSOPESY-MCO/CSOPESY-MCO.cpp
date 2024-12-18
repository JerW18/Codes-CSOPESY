#ifndef MAIN_H
#define MAIN_H
#include "screen.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <map>
#include <sstream>
#include <vector>
#include "CSOPESY-MCO.h"
#include "timeStamp.h"
#include "global.h"
#include "CPUManager.h"
#include "Scheduler.h"
#include <mutex>
//#include "MemoryAllocator.h"

typedef unsigned long long ull;
using namespace std;

volatile bool running = true;
volatile ull cycleCount = 0;
volatile ull activeCycleCount = 0;
string memType;
mutex cycleMutex;

thread cycleThread;

mutex mtx;
mutex testMtx;
screenManager* sm;
global g;
thread schedulerThread;
CPUManager* cpuManager;
Scheduler* processScheduler;
thread processThread;
MemoryAllocator* memoryAllocator;

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

deque<shared_ptr<process>> processes;
deque<shared_ptr<process>> backingStore;

void incrementCycleCount() {
    cycleCount += 1;
    this_thread::sleep_for(chrono::milliseconds(100));
}
void resetCycleCount() {
    cycleCount = 0;
}
ull getCycleCount() {
    return cycleCount;
}

void incrementActiveCycleCount() {
    activeCycleCount += 1;
    this_thread::sleep_for(chrono::milliseconds(100));
}
void resetActiveCycleCount() {
    activeCycleCount = 0;
}
ull getActiveCycleCount() {
    return activeCycleCount;
}

void runCycleCount() {
    //cout << "Running cycle count." << endl;
    while (running) {
        unique_lock<mutex> lock(cycleMutex);
        incrementCycleCount();
        if (processes.size() > 0) {
            incrementActiveCycleCount();
        }
        //cout << "Cycle count outside: " << cycleCount << endl;
        lock.unlock();
    }
}

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
            else if (key == "max-overall-mem") {
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
    cout << "use-flat: " << (useFlat ? "yes" : "no") << "\n";
	cout << "mem-type: " << memType << "\n" << endl;
}
namespace fs = std::filesystem;
void clearLogFiles() {
    string logDirectory = "logs";

    try {
        if (!fs::exists(logDirectory) || !fs::is_directory(logDirectory)) {
            cerr << "Log directory does not exist.\n";
            return;
        }

        for (const auto& entry : fs::directory_iterator(logDirectory)) {
            if (entry.is_regular_file()) {
                const auto& filePath = entry.path();
                if (filePath.filename().string().rfind("memory_stamp_", 0) == 0 && filePath.extension() == ".txt") {
                    fs::remove(filePath);
                }
            }
        }
    }
    catch (const filesystem::filesystem_error& e) {
        cerr << "Filesystem error: " << e.what() << '\n';
    }
}
void clearBackingStoreFiles() {
	string logDirectory = "backingstore";

	try {
		if (!fs::exists(logDirectory) || !fs::is_directory(logDirectory)) {
			cerr << "Backing store directory does not exist.\n";
			return;
		}

		for (const auto& entry : fs::directory_iterator(logDirectory)) {
			if (entry.is_regular_file()) {
				const auto& filePath = entry.path();
				if (filePath.filename().string().rfind("process_", 0) == 0 && filePath.extension() == ".txt") {
					fs::remove(filePath);
				}
			}
		}
	}
	catch (const filesystem::filesystem_error& e) {
		cerr << "Filesystem error: " << e.what() << '\n';
	}
}
void initialize() {
    if (!initialized) {
        cout << "'initialize' command recognized. Starting scheduler.\n" << endl;
        
		lock_guard<mutex> lock(mtx);
        clearBackingStoreFiles();
		clearLogFiles();
        
		readConfig("config.txt");
		//start cycle thread

        if (maxOverallMem == memPerFrame) {
            memType = "Flat Memory";
			memoryAllocator = new MemoryAllocator(maxOverallMem, addressof(processes));
        }
        else {
            memType = "Paging";
            memoryAllocator = new MemoryAllocator(maxOverallMem, memPerFrame, addressof(processes));
        }

		//memType = "Flat Memory";
        cycleThread = thread(runCycleCount);
        cycleThread.detach();

        //memoryAllocator = new MemoryAllocator(maxOverallMem, memPerFrame);
        cpuManager = new CPUManager(numCPU, quantumCycles, delaysPerExec, schedulerType, memoryAllocator, addressof(testMtx), *&cycleCount, memType, addressof(processes), addressof(backingStore));
        
        processScheduler = new Scheduler(cpuManager, memoryAllocator, *&cycleCount, memType, addressof(processes), addressof(backingStore));

	

        sm = new screenManager(&mtx, memType);


        schedulerThread = (schedulerType == "fcfs")
            ? thread(&Scheduler::starFCFS, processScheduler)
            : (schedulerType == "rr")
            ? thread(&Scheduler::startRR, processScheduler)
            : thread();
		schedulerThread.detach();
        initialized = true;

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
ull previousCycle;
void schedStartThread() {
	previousCycle = cycleCount;
    while (makeProcess) {
        if (previousCycle != cycleCount) {
            unique_lock<mutex> lock(mtx);
            ull i = sm->getProcessCount();
            ull numIns = randomInsLength();
            //ull memoryReq = memPerProc; //randomMemLength();
			ull memoryReq = randomMemLength();

            string processName = "p_" + to_string(i);

            sm->addProcess(processName, numIns, memoryReq);
            processScheduler->addProcess(sm->processes.back());

            lock.unlock();
            //this_thread::sleep_for(chrono::milliseconds(batchProcessFreq * 100));
			//cout << "Process " << processName << " added to scheduler at time "<< cycleCount << endl;
			while (cycleCount - previousCycle < batchProcessFreq) {
				this_thread::sleep_for(chrono::milliseconds(100));
			}
            previousCycle = cycleCount;
        }
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

    reportFile << "Cycle count: " << getCycleCount() << endl;
    if (processScheduler->getSize() == 0)
    {
        reportFile << "Size of queue is: 0" << endl;
    }
    else
    {
        reportFile << "Size of queue is: " << processScheduler->getSize() + 1 << endl;
    }
    reportFile << "CPU Utilization: " << ((float)(numCPU - cpuManager->getCoresAvailable()) / numCPU) * 100 << "%" << endl;
    reportFile << "Cores Used: " << numCPU - cpuManager->getCoresAvailable() << endl;
    reportFile << "Cores Available: " << cpuManager->getCoresAvailable() << endl;
    reportFile << "----------------------------------" << endl;

    reportFile << "Running Processes:" << endl;
    for (auto& screen : sm->processes) {
        if (!screen->isFinished() && screen->getCoreAssigned() != -1) {
            reportFile << screen->getProcessName() << " ("
                << screen->getDateOfBirth() << ") Core: "
                << screen->getCoreAssigned() << " Running "
                << screen->getInstructionIndex() << " / "
                << screen->getTotalInstructions() << " Memory: "
                << screen->getMemoryRequired() << " / " <<
                //(screen->getMemoryAddress() == nullptr ? "Not allocated" : "Allocated") <<
                (screen->hasMemoryAssigned() ? "Allocated" : "Not Allocated") << endl;
        }
    }

    reportFile << endl;

    reportFile << "Ready Processes (Not in Queue Order):" << endl;
    for (auto& screen : sm->processes) {
        if (!screen->isFinished() && screen->getCoreAssigned() == -1) {
            reportFile << screen->getProcessName() << " ("
                << screen->getDateOfBirth() << ") Core: None"
                << " Ready "
                << screen->getInstructionIndex() << " / "
                << screen->getTotalInstructions() << " Memory: "
                << screen->getMemoryRequired() << " / " <<
                //(screen->getMemoryAddress() == nullptr ? "Not allocated" : "Allocated") <<
                (screen->hasMemoryAssigned() ? "Allocated" : "Not Allocated") << endl;
        }
    }

    reportFile << endl;
    reportFile << "Finished Processes:" << endl;

    for (auto& screen : sm->processes) {
        if (screen->isFinished()) {
            reportFile << screen->getProcessName() << " ("
                << screen->getDateOfBirth() << ") Finished "
                << screen->getTotalInstructions() << " / "
                << screen->getTotalInstructions() << " Memory used: "
                << screen->getMemoryRequired() <<
                endl;
        }
    }

    reportFile << "----------------------------------" << endl;
   
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
		ull memoryReq = randomMemLength();
        sm->addProcessManually(name, instructions, memoryReq);

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
		unique_lock<mutex> lock(testMtx);
		cout << "Cycle count: " << getCycleCount() << endl;
        if (processScheduler->getSize() == 0)
        {
            cout << "Size of queue is: 0" << endl;
        }
        else
        {
            cout << "Size of queue is: " << processScheduler->getSize() + 1 << endl;
        }
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
                    //(screen->getMemoryAddress() == nullptr ? "Not allocated" : "Allocated") <<
                    (screen->hasMemoryAssigned() ? "Allocated" : "Not Allocated") << endl;
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
                    //(screen->getMemoryAddress() == nullptr ? "Not allocated" : "Allocated") <<
					(screen->hasMemoryAssigned() ? "Allocated" : "Not Allocated") << endl;
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

        /*timeStamp t;
        std::string timestamp = t.getTimeStamp();

        string lastPrintedProcessName = "";
        bool changedProcess = false;

        int numProcessesInMemory = memoryAllocator->getNumOfProcesses();
        int totalExternalFragmentation = memoryAllocator->getExternalFragmentation();
        auto memoryState = memoryAllocator->getMemoryState();

        cout << "Timestamp: " << timestamp << "\n";
        cout << "Number of processes in memory: " << numProcessesInMemory << "\n";
        cout << "Total external fragmentation in KB: " << totalExternalFragmentation << "\n\n";

        cout << "----end---- = " << memoryAllocator->getTotalMemorySize() << "\n\n";

        int lastEndAddress = memoryAllocator->getTotalMemorySize(); // Start from the total memory size

        for (auto it = memoryState.rbegin(); it != memoryState.rend(); ++it) {
            if (!it->isFree) {
                if (it->processName != lastPrintedProcessName) {
                    // If there is a gap, print the gap's start address
                    if (lastEndAddress != it->endAddress) {
                        cout << lastEndAddress << "\n\n";
                    }
                    cout << it->endAddress << "\n"; // Print current process's end address
                    cout << it->processName << "\n"; // Print current process name
                    lastPrintedProcessName = it->processName;
                    changedProcess = true;
                }
                lastEndAddress = it->startAddress; // Update the last end address
            }
        }

        // Handle the case where the last process doesn't connect to memory address 0
        if (changedProcess && !memoryState.empty()) {
            if (lastEndAddress != 0) {
                cout << lastEndAddress << "\n\n";
            }
        }

        cout << "----start---- = 0\n";*/

        //memoryAllocator->printProcessAges();
        //processScheduler->printQueue();
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
    running = false;

	if (cycleThread.joinable()) {
		cycleThread.join();
	}

    if (processThread.joinable()) {
        processThread.join();
    }

    if (schedulerThread.joinable()) {
        schedulerThread.join();
    }

    cout << "Program exited successfully." << endl;
    exit(0);
}


void rrq() {
	for (auto& p : processes) {
		cout << p->getProcessName() << endl;
	}
	
    //memoryAllocator->printFreeFrameList();
	//memoryAllocator->printAllocationMap();
    cpuManager->printRunningProcesses();
    memoryAllocator->printRunningProcessesId();
}

void vmstat() {
    if (!initialized) {
        cout << "Error: Scheduler not initialized. Use 'initialize' command first.\n" << endl;
        return;
    }

    unique_lock<mutex> lock(mtx);

    size_t totalMemory = memoryAllocator->getTotalMemorySize();
    size_t usedMemory = memoryAllocator->getUsedMemorySize();
    size_t freeMemory = totalMemory - usedMemory;

    ull totalCPUTicks = cycleCount;
    ull idleCPUTicks = cycleCount - activeCycleCount;
    ull activeCPUTicks = activeCycleCount;

    size_t numPagedIn = memoryAllocator->getPageIn();
    size_t numPagedOut = memoryAllocator->getPageOut();

    // Display the statistics
    cout << "----------------------------------" << endl;
    cout << "Virtual Memory Statistics (vmstat)" << endl;
    cout << "----------------------------------" << endl;

    cout << "Total Memory:         " << totalMemory << " KB" << endl;
    cout << "Used Memory:          " << usedMemory << " KB" << endl;
    cout << "Free Memory:          " << freeMemory << " KB" << endl << endl;

    cout << "Idle CPU Ticks:       " << idleCPUTicks << endl;
    cout << "Active CPU Ticks:     " << activeCPUTicks << endl;
    cout << "Total CPU Ticks:      " << totalCPUTicks << endl << endl;

    cout << "Pages Paged In:       " << numPagedIn << endl;
    cout << "Pages Paged Out:      " << numPagedOut << endl;

    cout << "----------------------------------\n" << endl;
}

void processsmi() {
    if (!initialized) {
        cout << "Error: Scheduler not initialized. Use 'initialize' command first.\n" << endl;
        return;
    }
    lock_guard<mutex> lock(mtx);
    auto memoryState = memoryAllocator->getMemoryState();

    cout << "-----------------------------------------\n";
    cout << "CPU Utilization: " << ((float)(numCPU - cpuManager->getCoresAvailable()) / numCPU) * 100 << "%" << endl;
    cout << "Memory Usage: " << memoryAllocator->getUsedMemorySize() << "/" << memoryAllocator->getTotalMemorySize() << endl;
    cout << "Memory Utilization: " << ((float)memoryAllocator->getUsedMemorySize() / memoryAllocator->getTotalMemorySize()) * 100 << "%" << endl;
    cout << "-----------------------------------------\n";
    cout << "Running processes and their memory usage:\n";
    cout << "-----------------------------------------\n\n";

    for (const auto& proc : sm->processes) {
        if (proc->hasMemoryAssigned()) {
            cout << proc->getProcessName() << "\t" << proc->getMemoryRequired() << "\n";
        }
    }

    if (processes.empty()) {
        cout << "No running processes.\n";
    }

    cout << "\n-----------------------------------------\n\n";
}

map<string, void (*)()> commands = {
    {"rrq", rrq},
    {"ini", initialize},
    {"sst", schedStart},
	{"ssp", schedStop},
    {"display-config", displayConfig},
    {"report-util", report},
	{"scheduler-test", schedStart},
    {"scheduler-stop", schedStop},
    {"initialize", initialize},
    {"clear", clearScreen},
    {"exit", exitProgram},
    {"vmstat", vmstat},
    {"process-smi", processsmi}
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

	delete sm;
	delete cpuManager;
	delete processScheduler;
    delete memoryAllocator;


    return 0;
}

#endif