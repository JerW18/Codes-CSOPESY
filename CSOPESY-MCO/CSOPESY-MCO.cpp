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
#include "FCFSScheduler.h"
#include "RRScheduler.h"
#include <mutex>

typedef unsigned long long ull;
using namespace std;

mutex mtx;
screenManager sm = screenManager(&mtx);
global g;
thread schedulerThread;
CPUManager* cpuManager;
RRScheduler* rrScheduler;
FCFSScheduler* fcfsScheduler;
thread processThread;

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

string trim(const string& str) {
    size_t start = str.find_first_not_of(" \t\"");
    size_t end = str.find_last_not_of(" \t\"");
    return (start == string::npos || end == string::npos) ? "" : str.substr(start, end - start + 1);
}

void readConfig(const string& filename) {
    ifstream configFile(filename);
    if (!configFile.is_open()) {
        cerr << "Error: Could not open config file. Using stored defaults.\n" << endl;
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
            }
            else if (key == "scheduler") {
                schedulerType = trim(value);
            }
            else if (key == "quantum-cycles") {
                quantumCycles = stoull(value);
            }
            else if (key == "batch-process-freq") {
                batchProcessFreq = stoull(value);
            }
            else if (key == "min-ins") {
                minInstructions = stoull(value);
            }
            else if (key == "max-ins") {
                maxInstructions = stoull(value);
				if (maxInstructions < minInstructions) {
					cerr << "Error: max-ins must be greater than or equal to min-ins. Using value min-ins + 1.\n" << endl;
					maxInstructions = minInstructions + 1;
				}
            }
            else if (key == "delays-per-exec") {
                delaysPerExec = stoull(value);
                if (delaysPerExec < 0 || delaysPerExec > (4294967296ULL)) {
					cout << "Error: delays-per-exec must be between 0 and 2^32. Using default value of 0.\n" << endl;
					delaysPerExec = 0;
				}

            }
        }
    }

    configFile.close();
}


void initialize() {
    if (!initialized) {
        cout << "'initialize' command recognized. Starting scheduler.\n" << endl;
		lock_guard<mutex> lock(mtx);
		readConfig("config.txt");


        if (schedulerType == "fcfs") {
            cpuManager = new CPUManager(numCPU, quantumCycles, delaysPerExec, schedulerType);
			fcfsScheduler = new FCFSScheduler(cpuManager);
            schedulerThread = thread(&FCFSScheduler::start, fcfsScheduler);
			schedulerThread.detach();
        }
        else if (schedulerType == "rr") {
            cpuManager = new CPUManager(numCPU, quantumCycles, delaysPerExec, schedulerType);
			rrScheduler = new RRScheduler(cpuManager);
            schedulerThread = thread(&RRScheduler::start, rrScheduler);
            schedulerThread.detach();
        }
        else {
            cerr << "Error: Unknown scheduler type specified in config file.\n" << endl;
            return;
        }

        initialized = true;  
    }
    else {
        cout << "'initialize' command has already been executed.\n" << endl;
    }
}

ull randomInsLength() {
    return rand() % (maxInstructions - minInstructions + 1) + minInstructions;
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
        cout << "Error: Scheduler not initialized. Use 'initialize' command first.\n" << endl;
    }
}

bool firstProcess = true;
void schedStartThread() {
    ull i = sm.getProcessCount();
    ull numIns = 0;

    while (makeProcess) {
        std::unique_lock<std::mutex> lock(mtx);
        i = sm.getProcessCount();
        if (firstProcess) {
            this_thread::sleep_for(chrono::milliseconds(batchProcessFreq * 50));
            firstProcess = false;
        }
        numIns = randomInsLength();

		
		string processName = "p_" + to_string(i);
        sm.addProcess(processName, numIns);
        if (schedulerType == "fcfs") {
            fcfsScheduler->addProcess(sm.processes.back());
        }
        else if (schedulerType == "rr") {
            rrScheduler->addProcess(sm.processes.back());
        }
        lock.unlock();
        if(!firstProcess)
            this_thread::sleep_for(chrono::milliseconds(batchProcessFreq * 50));
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

    std::unique_lock<std::mutex> lock(mtx);
    ofstream reportFile("report.txt");
    if (!reportFile.is_open()) {
        cerr << "Error: Could not open report file." << endl;
        return;
    }

    reportFile << "CPU Utilization: " << (numCPU - cpuManager->getCoresAvailable()) / numCPU * 100 << "%" << endl;
    reportFile << "Cores Used: " << numCPU - cpuManager->getCoresAvailable() << endl;
    reportFile << "Cores Available: " << cpuManager->getCoresAvailable() << endl;
    reportFile << "----------------------------------" << endl;
    reportFile << "Running Processes:" << endl;

    // List running processes that have been assigned a core
    for (auto& screen : sm.processes) {
        if (!screen->isFinished() && screen->getCoreAssigned() != -1) {
            reportFile << screen->getProcessName() << " ("
                << screen->getDateOfBirth() << ") Core: "
                << screen->getCoreAssigned() << " Running "
                << screen->getInstructionIndex() << " / "
                << screen->getTotalInstructions() << endl;
        }
    }

    reportFile << endl;
    reportFile << "Ready Processes (!! NOT IN QUEUE ORDER !!):" << endl;

    // List ready processes that are not currently assigned to a core
    for (auto& screen : sm.processes) {
        if (!screen->isFinished() && screen->getCoreAssigned() == -1) {
            reportFile << screen->getProcessName() << " ("
                << screen->getDateOfBirth() << ") Ready "
                << screen->getInstructionIndex() << " / "
                << screen->getTotalInstructions() << endl;
        }
    }

    reportFile << endl;
    reportFile << "Finished Processes:" << endl;

    // List finished processes
    for (auto& screen : sm.processes) {
        if (screen->isFinished()) {
            reportFile << screen->getProcessName() << " ("
                << screen->getDateOfBirth() << ") Finished "
                << screen->getTotalInstructions() << " / "
                << screen->getTotalInstructions() << endl;
        }
    }

    reportFile << "----------------------------------" << endl;
    reportFile.close();
    lock.unlock();

    cout << "Report generated.\n" << endl;
}




void screens(const string& option, const string& name) {
    if (!initialized) {
        cout << "Error: Scheduler not initialized. Use 'initialize' command first.\n" << endl;
        return;
    }

    if (option == "-r") {
        std::unique_lock<std::mutex> lock(mtx);
        for (auto screen : sm.processes) {
            if (screen->getProcessName() == name) {
                ull id = screen->getId();
                cout << "Reattaching to screen session: " << name << endl;
                inScreen = true;
                sm.reattatchProcess(name, id);
                return;
            }
        }
        cout << "Screen not found. Try a different name or use screen -s <name> to start a new screen.\n" << endl;
		lock.unlock();
    }
    else if (option == "-s") {
        std::unique_lock<std::mutex> lock(mtx);
        for (auto screen : sm.processes) {
            if (screen->getProcessName() == name) {
                cout << "Screen already exists. Try a different name or use screen -r <name> to reattach it.\n" << endl;
                return;
            }
        }
        cout << "Starting new terminal session: " << name << endl;

        ull instructions = randomInsLength();
        sm.addProcessManually(name, instructions);

        shared_ptr<process> newProcess = sm.processes.back();

        if (schedulerType == "fcfs" && fcfsScheduler != nullptr) {
            fcfsScheduler->addProcess(newProcess);
        }
        else if (schedulerType == "rr" && rrScheduler != nullptr) {
            rrScheduler->addProcess(newProcess);
        }
        else {
            cout << "Error: Scheduler not initialized or unknown scheduler type.\n" << endl;
        }

        inScreen = true;
		lock.unlock();

    }
    else if (option == "-ls") {
        std::unique_lock<std::mutex> lock(mtx);
		cout << "CPU Utilization: " << (numCPU - cpuManager->getCoresAvailable()) / numCPU * 100 << "%" << endl;
		cout << "Cores Used: " << numCPU - cpuManager->getCoresAvailable() << endl;
		cout << "Cores Available: " << cpuManager->getCoresAvailable() << endl;
        cout << "----------------------------------" << endl;
        cout << "Running Processes:" << endl;

        for (auto& screen : sm.processes) {
            if (!screen->isFinished() && screen->getCoreAssigned() != -1) {
                cout << screen->getProcessName() << " ("
                    << screen->getDateOfBirth() << ") Core: "
                    << screen->getCoreAssigned() << " Running "
                    << screen->getInstructionIndex() << " / "
                    << screen->getTotalInstructions() << endl;
            }
        }

        cout << endl;
        cout << "Ready Processes (!! NOT IN QUEUE ORDER !!):" << endl;

        for (auto& screen : sm.processes) {
            if (!screen->isFinished() && screen->getCoreAssigned() == -1) {
                cout << screen->getProcessName() << " ("
                    << screen->getDateOfBirth() << ") Ready "
                    << screen->getInstructionIndex() << " / "
                    << screen->getTotalInstructions() << endl;
            }
        }

        cout << endl;
        cout << "Finished Processes:" << endl;

        for (auto& screen : sm.processes) {
            if (screen->isFinished()) {
                cout << screen->getProcessName() << " ("
                    << screen->getDateOfBirth() << ") Finished "
                    << screen->getTotalInstructions() << " / "
                    << screen->getTotalInstructions() << endl;
            }
        }

        cout << "----------------------------------" << endl;
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
