#include <iostream>
#include <string>
#include <cstdlib>


using namespace std;

void printHeader() {
    cout << "   _____  _____  ____  _____  ______  _______     __" << endl;
cout << "  / ____|/ ____|/ __ \\|  __ \\|  ____|/ ____\\ \\   / /" << endl;
cout << " | |    | (___ | |  | | |__) | |__  | (___  \\ \\_/ / " << endl;
cout << " | |     \\___ \\| |  | |  ___/|  __|  \\___ \\  \\   /  " << endl;
cout << " | |____ ____) | |__| | |    | |____ ____) |  | |   " << endl;
cout << "  \\_____|_____/ \\____/|_|    |______|_____/   |_|  " << endl;
    cout << "Type 'exit' to quit and 'clear' to clear the screen." << endl;
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

int main (){

    printHeader();
    string command;

    while (true) {
        cout << "Enter command: ";
        cin >> command;
        if (command == "exit"){
            exit();
        } else if (command == "clear"){
            clearScreen();
        } else if (command == "initialize"){
            initialize();
        } else if (command == "screen"){
            screen();
        } else if (command == "scheduler-test"){
            schedulerTest();
        } else if (command == "scheduler-stop"){
            schedulerStop();
        } else if (command == "report-util"){
            reportUtil();
        }
         else {
            cout << "Invalid command." << endl;
        }
    }
    return 0;

}