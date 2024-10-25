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

// declare a global mutex
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


// config vars
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
        cerr << "Error: Could not open config file. Using stored defaults." << endl;
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
					cerr << "Error: max-ins must be greater than or equal to min-ins. Using value min-ins + 1." << endl;
					maxInstructions = minInstructions + 1;
				}
            }
            else if (key == "delays-per-exec") {
                delaysPerExec = stoull(value);
                if (delaysPerExec < 0 || delaysPerExec > (4294967296ULL)) {
					cout << "Error: delays-per-exec must be between 0 and 2^32. Using default value of 0." << endl;
					delaysPerExec = 0;
				}

            }
        }
    }

    configFile.close();
}


void initialize() {
    if (!initialized) {
        cout << "'initialize' command recognized. Starting scheduler." << endl << endl;
		lock_guard<mutex> lock(mtx);
		readConfig("config.txt"); //will use stored defaults if file not found


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
            cerr << "Error: Unknown scheduler type specified in config file." << endl;
            return;
        }

        initialized = true;  
    }
    else {
        cout << "'initialize' command has already been executed." << endl;
    }
}

ull randomInsLength() {
    return rand() % (maxInstructions - minInstructions + 1) + minInstructions;
}


void schedStartThread();
void schedStop();
void schedStart() {
    if (initialized && !makeProcess) {
        cout << "Starting scheduler." << endl;
        makeProcess = true;
        processThread = thread(schedStartThread);
    }
    else {
        cout << "Error: Scheduler not initialized. Use 'initialize' command first." << endl;
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
				cout << "Scheduler stopped." << endl;
			}
		}
		else {
			cout << "Scheduler is already stopped." << endl;
		}
	}
	else {
		cout << "Error: Scheduler not initialized. Use 'initialize' command first." << endl;
	}
}

void report() {
    //export current screen -ls to a file
    std::unique_lock<std::mutex> lock(mtx);
    ofstream reportFile("report.txt");
    if (!reportFile.is_open()) {
        cerr << "Error: Could not open report file." << endl;
        return;
    }

    reportFile << "----------------------------------" << endl;
    reportFile << "Running Processes:" << endl;

    for (auto screen : sm.processes) {
        if (!screen->isFinished()) {
            reportFile << screen->getProcessName() << " ("
                << screen->getDateOfBirth() << ") Core: "
                << screen->getCoreAssigned() << " "
                << screen->getInstructionIndex() << " / "
                << screen->getTotalInstructions() << endl;
        }
    }

    reportFile << endl;
    reportFile << "Finished Processes:" << endl;

    for (auto screen : sm.processes) {
        if (screen->isFinished()) {
            reportFile << screen->getProcessName() << " ("
                << screen->getDateOfBirth() << ") Core: "
                << screen->getCoreAssigned() << " Finished "
                << screen->getTotalInstructions() << " / "
                << screen->getTotalInstructions() << endl;
        }
    }

    reportFile << "----------------------------------" << endl;
    reportFile.close();
	lock.unlock();
    cout << "Report generated." << endl;
}




void screens(const string& option, const string& name) {
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
        cout << "Screen not found. Try a different name or use screen -s <name> to start a new screen." << endl;
		lock.unlock();
    }
    else if (option == "-s") {
        std::unique_lock<std::mutex> lock(mtx);
        for (auto screen : sm.processes) {
            if (screen->getProcessName() == name) {
                cout << "Screen already exists. Try a different name or use screen -r <name> to reattach it." << endl;
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
            cout << "Error: Scheduler not initialized or unknown scheduler type." << endl;
        }

        inScreen = true;
		lock.unlock();

    }
    else if (option == "-ls") {
        std::unique_lock<std::mutex> lock(mtx);
        cout << "----------------------------------" << endl;
        cout << "Running Processes:" << endl;

        for (auto &screen : sm.processes) {
            if (!screen->isFinished()) {
                cout << screen->getProcessName() << " ("
                    << screen->getDateOfBirth() << ") Core: "
                    << screen->getCoreAssigned() << " "
                    << screen->getInstructionIndex() << " / "
                    << screen->getTotalInstructions() << endl;
            }
        }

        cout << endl;
        cout << "Finished Processes:" << endl;

        for (auto &screen : sm.processes) {
            if (screen->isFinished()) {
                cout << screen->getProcessName() << " ("
                    << screen->getDateOfBirth() << ") Core: "
                    << screen->getCoreAssigned() << " Finished "
                    << screen->getTotalInstructions() << " / "
                    << screen->getTotalInstructions() << endl;
            }
        }

        cout << "----------------------------------" << endl;
		lock.unlock();
    }
    else {
        cout << "Invalid screen option: " << option << endl;
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
            cout << "Invalid command." << endl;
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
            cout << "Invalid command '" << command << "'" << endl;
        }
    }
}

int main() {
    g.printHeader();

    thread testThread(test);
    testThread.join();

    return 0;
}
