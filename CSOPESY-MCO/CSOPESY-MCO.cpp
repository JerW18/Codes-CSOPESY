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

using namespace std;

screenManager sm = screenManager();
global g;

bool inScreen = false;
bool initialized = false; 
thread schedulerThread;

void initialize10Processes() {
    for (int i = 0; i < 10; i++) {
        std::string processName = "process_" + std::to_string(i);
        sm.addProcess(processName, 100);
    }
}

// Variables to hold config values
int numCPU = 1;
string schedulerType = "fcfs";
int quantumCycles = 0;
int batchProcessFreq = 0;
int minInstructions = 0;
int maxInstructions = 0;
int delaysPerExec = 0;
bool makeProcess = false;

// Helper function to trim quotes and whitespace from a string
string trim(const string& str) {
    size_t start = str.find_first_not_of(" \t\"");
    size_t end = str.find_last_not_of(" \t\"");
    return (start == string::npos || end == string::npos) ? "" : str.substr(start, end - start + 1);
}

// Function to read configuration values
void readConfig(const string& filename) {
    ifstream configFile(filename);
    if (!configFile.is_open()) {
        cerr << "Error: Could not open config file." << endl;
        return;
    }

    string line;
    while (getline(configFile, line)) {
        stringstream ss(line);
        string key, value;

        // Read the key
        if (ss >> key) {
            // Read the rest of the line as the value (without trimming)
            ss >> ws; // Ignore leading whitespace
            getline(ss, value);

            // Assign the value to the appropriate variable based on the key
            if (key == "num-cpu") {
                numCPU = stoi(value);
            }
            else if (key == "scheduler") {
                schedulerType = trim(value);
            }
            else if (key == "quantum-cycles") {
                quantumCycles = stoi(value);
            }
            else if (key == "batch-process-freq") {
                batchProcessFreq = stoi(value);
            }
            else if (key == "min-ins") {
                minInstructions = stoi(value);
            }
            else if (key == "max-ins") {
                maxInstructions = stoi(value);
            }
            else if (key == "delays-per-exec") {
                delaysPerExec = stoi(value);
            }
        }
    }

    configFile.close();
}
CPUManager* cpuManager;
RRScheduler* rrScheduler;
FCFSScheduler* fcfsScheduler;

void initialize() {
    if (!initialized) {
        cout << "'initialize' command recognized. Initializing processes and starting scheduler." << endl << endl;
        
        readConfig("config.txt");

        cpuManager = new CPUManager(numCPU, quantumCycles, schedulerType);

        if (schedulerType == "fcfs") {
			// Initialize CPUManager and FCFSScheduler
            //CPUManager* cpuManager = new CPUManager(numCPU, quantumCycles, schedulerType);
            //FCFSScheduler* fcfsScheduler = new FCFSScheduler(cpuManager);
			fcfsScheduler = new FCFSScheduler(cpuManager);
            schedulerThread = thread(&FCFSScheduler::start, fcfsScheduler);
        }
        else if (schedulerType == "rr") {
			// Initialize CPUManager and RoundRobinScheduler
            //CPUManager* cpuManager = new CPUManager(numCPU, quantumCycles, schedulerType);
            //RRScheduler* rrScheduler = new RRScheduler(cpuManager);  
			rrScheduler = new RRScheduler(cpuManager);
            schedulerThread = thread(&RRScheduler::start, rrScheduler);
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

int randomInsLength() {
    return rand() % (maxInstructions - minInstructions + 1) + minInstructions;
}

thread processThread;
void schedStartThread();
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

void schedStartThread() {
    int i = sm.getProcessCount() + 1;
    int numIns = 0;
    while (makeProcess) {
		numIns = randomInsLength();
        string processName = "process_" + std::to_string(i);
        sm.addProcess(processName, numIns);
        if (schedulerType == "fcfs") {
            fcfsScheduler->addProcess(sm.processes.back());
        }
        else if (schedulerType == "rr") {
            rrScheduler->addProcess(sm.processes.back());
        }
        i = sm.getProcessCount() + 1;
        this_thread::sleep_for(chrono::milliseconds(batchProcessFreq));
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



void screens(const string& option, const string& name) {
    if (option == "-r") {
        for (auto screen : sm.processes) {
            if (screen->getProcessName() == name) {
                int id = screen->getId();
                cout << "Reattaching to screen session: " << name << endl;
                inScreen = true;
                sm.reattatchProcess(name, id);
                return;
            }
        }
        cout << "Screen not found. Try a different name or use screen -s <name> to start a new screen." << endl;
    }
    else if (option == "-s") {
        for (auto screen : sm.processes) {
            if (screen->getProcessName() == name) {
                cout << "Screen already exists. Try a different name or use screen -r <name> to reattach it." << endl;
                return;
            }
        }
        cout << "Starting new terminal session: " << name << endl;
        sm.addProcessManually(name, randomInsLength()); //ig it should also add it to the queue?
        inScreen = true;
    }
    else if (option == "-ls") {
        cout << "----------------------------------" << endl;
        cout << "Running Processes:" << endl;

        for (auto screen : sm.processes) {
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

        for (auto screen : sm.processes) {
            if (screen->isFinished()) {
                cout << screen->getProcessName() << " ("
                    << screen->getDateOfBirth() << ") Core: "
                    << screen->getCoreAssigned() << " Finished "
                    << screen->getTotalInstructions() << " / "
                    << screen->getTotalInstructions() << endl;
            }
        }

        cout << "----------------------------------" << endl;
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
    if (schedulerThread.joinable()) {
        schedulerThread.join();  
    }
    exit(0);
}


map<string, void (*)()> commands = {
    {"ini", initialize},
	{"sst", schedStart},
	{"ssp", schedStop},
	{"scheduler-start", schedStart},
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

    // Only start command loop, initialization happens after 'initialize' command
    thread testThread(test);
    testThread.join();

    return 0;
}
