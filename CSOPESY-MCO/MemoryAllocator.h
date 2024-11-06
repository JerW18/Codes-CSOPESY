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

public:
    MemoryAllocator(size_t totalMemorySize, size_t frameSize)
        : memory(totalMemorySize, 0), allocationMap(totalMemorySize / frameSize, false),
        frameSize(frameSize), totalFrames(totalMemorySize / frameSize) {}

    void* allocate(size_t size, string strategy) {
        if (strategy == "FirstFit") {
            return allocateFirstFit(size);
        }
        else if (strategy == "BestFit") {
            return allocateBestFit(size);
        }
        else {
            //cout << "Invalid allocation strategy" << endl;
            return nullptr;
        }
    }

    // FirstFit allocation
    void* allocateFirstFit(size_t size) {
        size_t framesNeeded = (size + frameSize - 1) / frameSize;

        for (size_t i = 0; i <= allocationMap.size() - framesNeeded; i++) {
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
                //cout << "Allocated " << framesNeeded * frameSize << " bytes at frame " << i << " using FirstFit strategy" << endl;
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
            //cout << "Allocated " << framesNeeded * frameSize << " bytes at frame " << bestStart << " using BestFit strategy" << endl;
            return &memory[bestStart * frameSize];
        }

        //cout << "No available block found with BestFit strategy" << endl;
        return nullptr;
    }

    // Deallocate memory at a specified pointer
    // Deallocate memory at a specified pointer
    void deallocate(void* ptr, size_t size) {
		size_t index = static_cast<char*>(ptr) - &memory[0];
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

