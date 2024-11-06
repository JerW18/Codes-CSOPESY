#pragma once
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "timeStamp.h"

using namespace std;

class MemoryAllocator {
private:
    vector<char> memory;
    vector<bool> allocationMap;
    size_t frameSize;
    size_t totalFrames;
	size_t totalMemorySize;
public:
    MemoryAllocator(size_t totalMemorySize, size_t frameSize)
        : memory(totalMemorySize, '.'), allocationMap(totalMemorySize / frameSize, false),
        frameSize(frameSize), totalFrames(totalMemorySize / frameSize), totalMemorySize(totalMemorySize) {}

    void* allocate(size_t size, string strategy) {
        if (strategy == "FirstFit") {
			void* temp = allocateFirstFit(size);
			//cout << "B " << temp << endl;
            return temp;
        }
        else if (strategy == "BestFit") {
            return allocateBestFit(size);
        }
        else {
            //cout << "Invalid allocation strategy" << endl;
            return nullptr;
        }
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
		cout << "Memory starts at: " << (void*) & memory[0] << endl;
		cout << "Memory ends at: " << (void*) & memory[memory.size() - 1] << endl;
    }
	void printAllocationMap() {
		for (size_t i = 0; i < allocationMap.size(); i++) {
			cout << allocationMap[i];
		}
		cout << endl;
	}
    void printOtherStuff() {
        cout << "Total memory size: " << totalMemorySize << ", Frame size: " << frameSize << endl;
        cout << "Memory vector size should be: " << memory.size() << endl;
		cout << "Allocation map size should be: " << allocationMap.size() << endl;
    }

    // FirstFit allocation
    void* allocateFirstFit(size_t size) {
        size_t framesNeeded = (size + frameSize - 1) / frameSize;

        for (size_t i = 0; i+framesNeeded <= allocationMap.size(); i++) {
            bool found = true;
            for (size_t j = 0; j < framesNeeded; j++) {
                if (allocationMap[i + j]) {
                    found = false;
                    break;
                }
            }
            if (found) {
                for (size_t j = 0; j < framesNeeded; j++) {
                    allocationMap[i + j] = true;
                }
                
                return &memory[i * frameSize];
            }
        }
        //cout << "No available block found with FirstFit strategy" << endl;
        return nullptr;
    }

    // BestFit allocation
    void* allocateBestFit(size_t size) {
        int framesNeeded = (size + frameSize - 1) / frameSize;
        size_t bestStart = allocationMap.size();
        size_t bestSize = allocationMap.size() + 1;

        for (size_t i = 0; i <= allocationMap.size() - framesNeeded;) {
            size_t start = i;
            size_t blockSize = 0;

            while (i < allocationMap.size() && !allocationMap[i] && blockSize < framesNeeded) {
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
                allocationMap[bestStart + j] = true;
            }
            cout << "Allocated " << framesNeeded * frameSize << " bytes at frame " << bestStart << " using BestFit strategy" << endl;
            return &memory[bestStart * frameSize];
        }

        return nullptr;
    }

    // Deallocate memory at a specified pointer
    void deallocate(void* ptr, size_t size) {
		
		void* temp = &memory[0];
		cout << temp << endl;
		size_t index = static_cast<char*>(ptr) - temp;
        if (index % frameSize != 0 || index / frameSize >= allocationMap.size()) {
            cout << "Error: Invalid memory address for deallocation." << endl;
            return;
        }
		size_t frames = (size + frameSize - 1) / frameSize;

        for (size_t i = 0; i < frames; i++) {

			if (allocationMap[(index / frameSize) + i]) {
				allocationMap[(index / frameSize) + i] = false;
			}
        }

    }


	// Get memory allocation map
	vector<bool> getAllocationMap() {
		return allocationMap;
	}

    // get frame size
	size_t getFrameSize() {
		return frameSize;
	}

   // get memory
    vector<char> getMemory() {
		return memory;
    }
	
};

