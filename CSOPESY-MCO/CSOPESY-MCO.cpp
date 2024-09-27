// CSOPESY-MCO.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <cstdlib>
#include <map>
#include <sstream>
#include <vector>
#include "CSOPESY-MCO.h"

using namespace std;

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

void screen(const string& option, const string& name) {
    if (option == "-r") {
        string command = "screen -r " + name;
        cout << "Reattaching to screen session: " << name << endl;
        system(command.c_str());
    }
    else if (option == "-s") {
        cout << "Starting new terminal session: " << name << endl;

        #ifdef _WIN32
            string command = "start cmd /k title " + name;
        #else
            string command = "screen -S " + name; // Unix-based system
        #endif
            system(command.c_str());
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
        cout << "Enter command: ";
        getline(cin, input);

        vector<string> tokens = splitInput(input);

        if (tokens.empty()) {
            cout << "Invalid command." << endl;
            continue;
        }

        string command = tokens[0];

        if (command == "screen" && tokens.size() >= 3) {
            screen(tokens[1], tokens[2]);
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

    return 0;
}