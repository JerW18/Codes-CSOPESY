#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>    
#include <iomanip>  
#include "timeStamp.h"

using namespace std;
timeStamp ts;

struct process
{
    int pid;
    string type;
    string processName;
    int gpuMemUsage;
};

void printSeparator1() {
    cout << "+-----------------------------------------------------------------------------------------+" << endl;
}
void printSeparator2() {
    cout << "+----------------------------------------+-------------------------+----------------------+" << endl;
}
void printSeparator3() {
    cout << "|========================================+=========================+======================|" << endl;
}

void printHeader() {
    cout << ts.getTimeStampNVIDIA_SMI() << endl;
    printSeparator1();
    cout << "| NVIDIA-SMI 560.81                Driver Version: 560.81           CUDA Version: 12.6    |" << endl;
    printSeparator2();
}

void printGPUSummary() {
    cout << "| GPU  Name                 Driver-Model | Bus-Id           Disp.A | Volatile Uncorr. ECC | " << endl;
    cout << "| Fan  Temp  Perf          Pwr:Usage/Cap |            Memory-Usage | GPU-Util  Compute M. |" << endl;
    cout << "|                                        |                         |               MIG M. |" << endl;
    printSeparator3();
    cout << "|   0  NVIDIA GeForce RTX 2080 Ti   WDDM |    00000000:00:1E.0 Off |                  N/A |" << endl;
    cout << "| N/A   67C    P8            66W /  300W |     1556MiB /  11264MiB |      1%      Default |" << endl;
    cout << "|                                        |                         |                  N/A |" << endl;
    printSeparator2();
    cout << "\n";
    printSeparator1();
}

class nvidiaSMI
{
    vector<process> processes;

public:
    void printProcesses() {
        printHeader();
        printGPUSummary();
        cout << "|  GPU   GI   CI      PID      Type   Process name                             GPU Memory |" << endl;
        cout << "|        ID   ID                                                               Usage      |" << endl;
        printSeparator3();
        string temp;
        for (const auto& p : processes) {
            temp = p.processName;
            //if name too long, truncate to ...<name>
            if (p.processName.length() > 34) {
                temp = "..." + p.processName.substr(p.processName.length() - 32, 32);
			}
            else {
                while (temp.length() <= 34) {
                    temp += " ";  
                }
            }

            cout << "|    " << setw(1) << 0 
                << "   " << setw(3) << "N/A"  // GI
                << "  " << setw(3) << "N/A"  // CI
                << "    " << setw(4) << p.pid  
                << "  " << setw(8) << p.type  
                << "   " << setw(9) << temp;
            cout << "    " << right << setw(6) << p.gpuMemUsage << "MiB    |" << endl;
        }
        printSeparator1();
    }

    void generateProcesses(int count) {
        if (processes.size() > 0) {
            processes.clear();
        }
        srand(time(0));
        for (int i = 0; i < count; i++) {
            process p;
            p.pid = i + 1000;  
            p.type = rand() % 2 == 0 ? "CUDA" : "Graphics";
            p.processName = "\\";
            p.gpuMemUsage = (rand() % 500) + 100;  

            // add random characters to process name
            for (int i = 0; i < rand()%40+20; i++) {
               p.processName += (char)(rand() % 26 + 65);
            }
            p.processName += "\\process" + to_string(i) + ".exe";

            processes.push_back(p);
        }
    }

    void clearProcesses() {
        processes.clear();
    }
};
