#pragma once
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "timeStamp.h"
#include <unordered_map>
#include <limits.h>
using ull = unsigned long long;

using namespace std;
struct MemoryBlock {
    bool isFree;
    size_t startAddress;
    size_t endAddress;
    string processName;  // or empty if it's free
};

struct AllocationEntry {
    bool isAllocated = false;
    string processName = "";
};

class MemoryAllocator {
private:
    vector<char> memory;
    vector<AllocationEntry> allocationMap;
    size_t frameSize;
    size_t totalFrames;
	size_t totalMemorySize;
	int numOfProcesses = 0;
    int currentAge = 0;

    unordered_map<string, int> processAges;
public:
    mutex mtx;
    MemoryAllocator(size_t totalMemorySize, size_t frameSize)
        : memory(totalMemorySize, 0), allocationMap(totalMemorySize / frameSize),
        frameSize(frameSize), totalFrames(totalMemorySize / frameSize), totalMemorySize(totalMemorySize),
		numOfProcesses(0)
    {}
    
    pair<void*, string> allocate(size_t size, string strategy, string processName) {
		pair <void*, string> result;
		result.first = nullptr;
		result.second = "";
        if (strategy == "FirstFit") {
            void* temp = allocateFirstFit(size, processName);
            string replacedProncessName;
            
            if (temp == nullptr) {
                // Swap out the oldest process if allocation failed
                replacedProncessName = swapOutOldestProcess();
                temp = allocateFirstFit(size, processName);  // Retry allocation after swapping
            }
            if (temp != nullptr) {
                numOfProcesses++;
                processAges[processName] = currentAge++;  // Assign age to the newly allocated process
            }
			result.first = temp;
			result.second = replacedProncessName;
            return result;
        }
        else if (strategy == "BestFit") {
            //numOfProcesses++;
            //return allocateBestFit(size);
        }
        else {
            return result;
        }
    }

    void printProcessAges() {
		for (const auto& entry : processAges) {
			cout << entry.first << " " << entry.second << endl;
		}
    }

    void* allocateFirstFit(size_t size, string processName) {
        size_t framesNeeded = (size + frameSize - 1) / frameSize;

        for (size_t i = 0; i + framesNeeded <= allocationMap.size(); i++) {
            bool found = true;
            for (size_t j = 0; j < framesNeeded; j++) {
                if (allocationMap[i + j].isAllocated) {
                    found = false;
                    break;
                }
            }
            if (found) {
                // Mark frames as allocated and assign process name
                for (size_t j = 0; j < framesNeeded; j++) {
                    allocationMap[i + j].isAllocated = true;
                    allocationMap[i + j].processName = processName;
                }
                return &memory[i * frameSize];
            }
        }

        return nullptr;  // No space available
    }
	

    void* allocateBestFit(size_t size) {
        int framesNeeded = (size + frameSize - 1) / frameSize;
        size_t bestStart = allocationMap.size();
        size_t bestSize = allocationMap.size() + 1;

        for (size_t i = 0; i <= allocationMap.size() - framesNeeded;) {
            size_t start = i;
            size_t blockSize = 0;

            while (i < allocationMap.size() && !allocationMap[i].isAllocated && blockSize < framesNeeded) {
                blockSize++;
                i++;
            }

            if (blockSize >= framesNeeded && blockSize < bestSize) {
                bestStart = start;
                bestSize = blockSize;
            }

            i = start + blockSize + 1;
        }

        if (bestStart < allocationMap.size()) {
            for (size_t j = 0; j < framesNeeded; j++) {
                allocationMap[bestStart + j].isAllocated = true;
            }
            cout << "Allocated " << framesNeeded * frameSize << " bytes at frame " << bestStart << " using BestFit strategy" << endl;
            return &memory[bestStart * frameSize];
        }

        return nullptr;
    }

    string swapOutOldestProcess() {
        if (processAges.empty()) return "";

        // Find the oldest process
        string oldestProcess;
        int oldestAge = INT_MAX;
        for (const auto& entry : processAges) {
            if (entry.second < oldestAge) {
                oldestAge = entry.second;
                oldestProcess = entry.first;
            }
        }

        // Deallocate the oldest process
        for (size_t i = 0; i < allocationMap.size(); i++) {
            if (allocationMap[i].isAllocated && allocationMap[i].processName == oldestProcess) {
                allocationMap[i].isAllocated = false;
                allocationMap[i].processName = "";
            }
        }

        // Remove the process from the age tracker
        processAges.erase(oldestProcess);
        numOfProcesses--;
		//cout << "Swapped out process " << oldestProcess << " to make room for new process." << endl;
		return oldestProcess;
    }


    void deallocate(void* ptr, size_t size) {
        lock_guard<mutex> lock(mtx);
        void* temp = &memory[0];
        size_t index = static_cast<char*>(ptr) - static_cast<char*>(temp);

        if (index % frameSize != 0 || index / frameSize >= allocationMap.size()) {
            cout << "Error: Invalid memory address for deallocation." << endl;
            return;
        }

        size_t frames = (size + frameSize - 1) / frameSize;

        for (size_t i = 0; i < frames; i++) {
            if (allocationMap[(index / frameSize) + i].isAllocated) {
                allocationMap[(index / frameSize) + i].isAllocated = false;
                allocationMap[(index / frameSize) + i].processName = "";
            }
        }

        numOfProcesses--;

        // Remove the process from the age tracker if it exists
        for (auto it = processAges.begin(); it != processAges.end(); ) {
            if (it->second == index) {
                it = processAges.erase(it);
            }
            else {
                ++it;
            }
        }
    }


	vector<bool> getAllocationMap() {
        vector<bool> boolMap;
        for (const auto& entry : allocationMap) {
            boolMap.push_back(entry.isAllocated);
        }
        return boolMap;
	}

	size_t getFrameSize() {
		return frameSize;
	}

    vector<char> getMemory() {
		return memory;
    }

	ull getTotalMemorySize() {
		return memory.size();
	}

	int getNumOfProcesses() {
		return numOfProcesses;
	}

    ull getExternalFragmentation() {
		ull externalFragmentation = 0;
		bool inBlock = false;
		ull blockSize = 0;
		for (size_t i = 0; i < allocationMap.size(); i++) {
			if (allocationMap[i].isAllocated) {
				if (inBlock) {
					externalFragmentation += blockSize;
					blockSize = 0;
					inBlock = false;
				}
			}
			else {
				blockSize += frameSize;
				inBlock = true;
			}
		}
        if (inBlock) {
            externalFragmentation += blockSize;
            blockSize = 0;
            inBlock = false;
        }
		return externalFragmentation;
    }

    vector<MemoryBlock> getMemoryState() {
        vector<MemoryBlock> memoryState;
        size_t currentAddress = 0;
        bool inFreeBlock = false;
        size_t blockStart = 0;

        for (size_t i = 0; i < allocationMap.size(); ++i) {
            if (allocationMap[i].isAllocated) {
                if (inFreeBlock) {
                    memoryState.push_back({ true, blockStart, currentAddress - 1, ""});
                    inFreeBlock = false;
                }
                memoryState.push_back({ false, currentAddress, currentAddress + frameSize - 1, allocationMap[i].processName });
            }
            else {  
                if (!inFreeBlock) {
                    blockStart = currentAddress;
                    inFreeBlock = true;
                }
            }
            currentAddress += frameSize;
        }

        if (inFreeBlock) {
            memoryState.push_back({ true, blockStart, currentAddress - 1, ""});
        }

        return memoryState;
    }


    void printMemory() {
        for (size_t i = 0; i < memory.size(); i++) {
            cout << memory[i];
        }
        cout << endl;
    }
    void printMemoryLocations() {
        for (size_t i = 0; i < memory.size(); i++) {
            cout << &memory[i] << endl;
        }
        cout << endl;
    }
    void printMemoryEnds() {
        cout << "Memory starts at: " << (void*)&memory[0] << endl;
        cout << "Memory ends at: " << (void*)&memory[memory.size() - 1] << endl;
    }
    void printAllocationMap() {
        for (size_t i = 0; i < allocationMap.size(); i++) {
            cout << allocationMap[i].isAllocated;
            //cout << allocationMap[i].processName;
        }
        cout << endl;
    }
    void printOtherStuff() {
        cout << "Total memory size: " << totalMemorySize << ", Frame size: " << frameSize << endl;
        cout << "Memory vector size should be: " << memory.size() << endl;
        cout << "Allocation map size should be: " << allocationMap.size() << endl;
    }
};

