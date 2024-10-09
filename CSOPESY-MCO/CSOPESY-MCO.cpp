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

using namespace std;

screenManager sm = screenManager();
global g;

bool inScreen = false;
bool initialized = false; 
CPUManager* cpuManager = nullptr;  
FCFSScheduler* scheduler = nullptr;
thread schedulerThread;

// Function to initialize 10 processes (called inside initialize)
void initialize10Processes() {
    for (int i = 0; i < 10; i++) {
        std::string processName = "process_" + std::to_string(i);
        sm.addProcess(processName, 100);
    }
}

void initialize() {
    if (!initialized) {
        cout << "'initialize' command recognized. Initializing processes and starting scheduler." << endl;

        initialize10Processes();

        cpuManager = new CPUManager(4); 
        scheduler = new FCFSScheduler(cpuManager);

        for (size_t i = 0; i < 10; i++) {
            scheduler->addProcess(&sm.processes[i]);
        }

        // Start the scheduler in a new thread
        schedulerThread = thread(&FCFSScheduler::start, scheduler);

        initialized = true;  
    }
    else {
        cout << "'initialize' command has already been executed." << endl;
    }
}

void screens(const string& option, const string& name) {
    if (option == "-r") {
        for (auto screen : sm.processes) {
            if (screen.getProcessName() == name) {
                int id = screen.getId();
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
            if (screen.getProcessName() == name) {
                cout << "Screen already exists. Try a different name or use screen -r <name> to reattach it." << endl;
                return;
            }
        }
        cout << "Starting new terminal session: " << name << endl;
        sm.addProcess(name, 999999);
        inScreen = true;
    }
    else if (option == "-ls") {
        cout << "----------------------------------" << endl;
        cout << "Running Processes:" << endl;

        for (auto screen : sm.processes) {
            if (!screen.isFinished()) {
                cout << screen.getProcessName() << " ("
                    << screen.getDateOfBirth() << ") Core: "
                    << screen.getCoreAssigned() << " "
                    << screen.getInstructionIndex() << " / "
                    << screen.getTotalInstructions() << endl;
            }
        }

        cout << endl;
        cout << "Finished Processes:" << endl;

        for (auto screen : sm.processes) {
            if (screen.isFinished()) {
                cout << screen.getProcessName() << " ("
                    << screen.getDateOfBirth() << ") Core: "
                    << screen.getCoreAssigned() << " Finished "
                    << screen.getTotalInstructions() << " / "
                    << screen.getTotalInstructions() << endl;
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
