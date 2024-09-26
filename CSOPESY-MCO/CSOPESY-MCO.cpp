// CSOPESY-MCO.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <map>
#include <sstream>
#include <vector>
#include "CSOPESY-MCO.h"
#include "command.h"
#include "timeStamp.h"
#include "screen.h"

using namespace std;
screenManager sm = screenManager();
bool inScreen = false;
void printHeader() {
    cout << "      ___           ___           ___                         ___           ___                   \n"
        "     /  /\\         /  /\\         /  /\\          ___          /  /\\         /  /\\          __      \n"
        "    /  /::\\       /  /::\\       /  /::\\        /  /\\        /  /::\\       /  /::\\        |  |\\    \n"
        "   /  /:/\\:\\     /__/:/\\:\\     /  /:/\\:\\      /  /::\\      /  /:/\\:\\     /__/:/\\:\\       |  |:|   \n"
        "  /  /:/  \\:\\   _\\_ \\:\\ \\:\\   /  /:/  \\:\\    /  /:/\\:\\    /  /::\\ \\:\\   _\\_ \\:\\ \\:\\      |  |:|   \n"
        " /__/:/ \\  \\:\\ /__/\\ \\:\\ \\:\\ /__/:/ \\__\\:\\  /  /::\\ \\:\\  /__/:/\\:\\ \\:\\ /__/\\ \\:\\ \\:\\     |__|:|__ \n"
        " \\  \\:\\  \\__\\/ \\  \\:\\ \\:\\_\\/ \\  \\:\\ /  /:/ /__/:/\\:\\_\\:\\ \\  \\:\\ \\:\\_\\/ \\  \\:\\ \\:\\_\\/     /  /::::\\\n"
        "  \\  \\:\\        \\  \\:\\_\\:\\    \\  \\:\\  /:/  \\__\\/  \\:\\/:/  \\  \\:\\ \\:\\    \\  \\:\\_\\:\\      /  /:/~~~~\n"
        "   \\  \\:\\        \\  \\:\\/:/     \\  \\:\\/:/        \\  \\::/    \\  \\:\\_\\/     \\  \\:\\/:/     /__/:/     \n"
        "    \\  \\:\\        \\  \\::/       \\  \\::/          \\__\\/      \\  \\:\\        \\  \\::/      \\__\\/      \n"
        "     \\__\\/         \\__\\/         \\__\\/                       \\__\\/         \\__\\/                  \n" << endl;
    cout << "\033[32mHello! Welcome to CSOPESY commandline!\033[0m" << endl;
    cout << "\033[33mType 'exit' to quit, 'clear' to clear the screen!\033[0m" << endl;
}

void initialize() {
    cout << "'initialize' command recognized. Doing something." << endl;
}

void screens(const string& option, const string& name) {
    if (option == "-r") {
        //find the screen with the name
        for(auto screen : sm.screens) {
            if(screen.getProcessName() == name) {
                int id = screen.getId();
                cout << "Reattaching to screen session: " << name << endl;
                inScreen = true;
                sm.reattatchScreen(name, id);
                //system("screen -r " + name);
                return;
            }
        }
        cout << "Screen not found. Try a different name or use screen -s <name> to start a new screen." << endl;
        
    }
    else if (option == "-s") {
        for(auto screen : sm.screens) {
            if(screen.getProcessName() == name) {
                cout << "Screen already exists. Try a different name or use screen -r <name> to reattatch it." << endl;
                return;
            }
        }
        cout << "Starting new terminal session: " << name << endl;
        #ifdef _WIN32
            string command = "start cmd /k title " + name;
        #else
            string command = "screen -S " + name; // Unix-based system
        #endif
            //system(command.c_str());
        //check if screen already exists
        
		sm.addScreen(name, 999999);
        inScreen = true;
    }
    else if (option == "-ls") {
        cout << "Available Screens:" << endl;
        for(auto screen : sm.screens) {
            cout << screen.getProcessName() << endl;
        }
    }
    else {
        cout << "Invalid screen option: " << option << endl;
    }
}


void schedulerTest() {
    cout << "'scheduler-test' command recognized. Doing something." << endl;
}

void schedulerStop() {
    cout << "'scheduler-stop' command recognized. Doing something." << endl;
}

void reportUtil() {
    cout << "'report-util' command recognized. Doing something." << endl;
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
    printHeader();
}

void exit() {
    exit(0);
}

map<string, void (*)()> commands = {
    {"initialize", initialize},
    {"scheduler-test", schedulerTest},
    {"scheduler-stop", schedulerStop},
    {"report-util", reportUtil},
    {"clear", clearScreen},
    {"exit", exit}
};

// Helper function to split input by spaces
vector<string> splitInput(const string& input) {
    vector<string> result;
    istringstream iss(input);
    for (string s; iss >> s;) {
        result.push_back(s);
    }
    return result;
}


int main() {

    printHeader();
    string input;

    while (true) {
        inScreen = sm.inScreen;
        if(inScreen) {
            continue;
        }
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
        else if (commands.find(command) != commands.end()) {
            //printf("Not in screen\n");
            commands[command]();
        }
        else {
            cout << "Invalid command '" << command << "'" << endl;
        }

    }

    return 0;
}