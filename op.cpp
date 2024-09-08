#include <iostream>
#include <string>
#include <cstdlib>
#include <map>


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

void initialize(){
    cout << "initialize command recognized. Doing something." << endl;
}

void screen(){
    cout << "screen command recognized. Doing something." << endl;
}

void schedulerTest(){
    cout << "scheduler-test command recognized. Doing something." << endl;
}

void schedulerStop(){
    cout << "scheduler-stop command recognized. Doing something." << endl;
}

void reportUtil(){
    cout << "report-util command recognized. Doing something." << endl;
}

void clearScreen(){
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
    printHeader();
}

void exit(){
    exit(0);
}

map<string, void (*)()> commands = {
    {"initialize", initialize},
    {"screen", screen},
    {"scheduler-test", schedulerTest},
    {"scheduler-stop", schedulerStop},
    {"report-util", reportUtil},
    {"clear", clearScreen},
    {"exit", exit}
};

int main (){

    printHeader();
    string command;

    while (true) {
        cout << "Enter command: ";
        cin >> command;
        if (commands.find(command) == commands.end()) {
            cout << "Invalid command." << endl;
        }
        else {
            commands[command]();
        }
    }
    return 0;

}