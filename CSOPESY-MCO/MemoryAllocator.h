#pragma once
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "timeStamp.h"
#include <unordered_map>
#include <limits.h>
#include <queue>
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

struct PageTableEntry {
    size_t frameNumber;   
    bool isValid;
};

class PageTable {
private:
    vector<PageTableEntry> table;
    queue<size_t> freePageList;  // Queue to track free page numbers

public:
    PageTable(size_t totalPages) {
        table.resize(totalPages); // Preallocate page table
        for (size_t i = 0; i < totalPages; i++) {
            freePageList.push(i);  // Initialize all pages as free
        }
    }

    void addMapping(size_t pageNumber, size_t frameNumber) {
        table[pageNumber].frameNumber = frameNumber;
        table[pageNumber].isValid = true;
    }

    size_t getFrame(size_t pageNumber) {
        if (pageNumber >= table.size() || !table[pageNumber].isValid) {
            return SIZE_MAX; // Invalid page
        }
        return table[pageNumber].frameNumber;
    }

    void removeMapping(size_t pageNumber) {
        if (pageNumber < table.size() && table[pageNumber].isValid) {
            table[pageNumber].isValid = false;
            freePageList.push(pageNumber);  // Reclaim the page number
        }
    }

    size_t getNextFreePage() {
        if (freePageList.empty()) {
            return SIZE_MAX; // No free pages available
        }
        size_t freePage = freePageList.front();
        freePageList.pop();
        return freePage;
    }

    size_t size() {
        return table.size();
    }
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

	PageTable pageTable;
    queue<size_t> freeFrameList;

    unordered_map<string, int> processAges;
    unordered_map<string, vector<size_t>> processPageMapping;
public:
    mutex mtx;
    MemoryAllocator(size_t totalMemorySize, size_t frameSize)
        : memory(totalMemorySize, 0), allocationMap(totalMemorySize / frameSize),
        frameSize(frameSize), totalFrames(totalMemorySize / frameSize),
        totalMemorySize(totalMemorySize), pageTable(totalMemorySize / frameSize),
        numOfProcesses(0), currentAge(0) {
        // Initialize the free frame queue
        for (size_t i = 0; i < totalFrames; ++i) {
            freeFrameList.push(i);  // Add all frames to the free list
        }
    }
    
    pair<void*, string> allocate(size_t size, string strategy, string processName) {
		pair <void*, string> result;
		result.first = nullptr;
		result.second = "";
        if (strategy == "Flat Memory") {
            void* temp = allocateFirstFit(size, processName);
            string replacedProncessName;
            
            if (temp == nullptr) {
                // Swap out the oldest process if allocation failed
                replacedProncessName = swapOutOldestProcess(strategy);
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
        else {
            // Paging allocation logic
            void* temp = allocatePaging(size, processName);
            string replacedProcessName;

            if (temp == nullptr) {
                // Swap out the oldest process if allocation failed
                replacedProcessName = swapOutOldestProcess(strategy);
                //cout << replacedProcessName << " was swapped out to make room for new process." << endl;
                temp = allocatePaging(size, processName);  // Retry allocation after swapping
            }

            if (temp != nullptr) {
                numOfProcesses++;
                processAges[processName] = currentAge++;  // Assign age to the newly allocated process
            }

            result.first = temp;
            result.second = replacedProcessName;
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

    void* allocatePaging(size_t size, string processName) {
        size_t pagesNeeded = (size + frameSize - 1) / frameSize;  // Calculate the number of pages needed
        vector<size_t> allocatedFrames;

        // Allocate frames from the free frame list
        for (size_t i = 0; i < pagesNeeded; ++i) {
            if (freeFrameList.empty()) {

                // Clean up partially allocated frames
                for (size_t frameIndex : allocatedFrames) {
                    freeFrameList.push(frameIndex);
                }

                return nullptr;  // Allocation failed
            }

            size_t frameIndex = freeFrameList.front();  // Get the next available frame
            freeFrameList.pop();
            allocatedFrames.push_back(frameIndex);

            if (frameIndex < allocationMap.size()) {
                allocationMap[frameIndex].isAllocated = true;
                allocationMap[frameIndex].processName = processName;
            }

			//cout << "Allocated frame " << frameIndex << " for process " << processName << endl;
        }

        // Map logical pages to physical frames and track the process
        for (size_t i = 0; i < allocatedFrames.size(); i++) {
            size_t pageNumber = pageTable.getNextFreePage();  // Get the next free page number
           //  cout << "Page number: " << pageNumber << "Number of Processes " << this->numOfProcesses << endl;
            if (pageNumber == SIZE_MAX) {
                //cout << "Page table is full, allocation fails" << endl;

                // Clean up partially allocated resources
                for (size_t frameIndex : allocatedFrames) {
                    freeFrameList.push(frameIndex);
                }
                return nullptr;
            }

            pageTable.addMapping(pageNumber, allocatedFrames[i]);  // Map page to frame
            
        }

        // Track the pages allocated to the process
        processPageMapping[processName].insert(processPageMapping[processName].end(), allocatedFrames.begin(), allocatedFrames.end());

        // Return a pointer to the allocated memory (start of first frame)
        return &memory[allocatedFrames[0] * frameSize];
    }

    void printFreeFrameList() {
		cout << "Free Frame List: ";
		queue<size_t> temp = freeFrameList;
		while (!temp.empty()) {
			cout << temp.front() << " ";
			temp.pop();
		}
		cout << endl;
    }

    string swapOutOldestProcess(string strategy) {
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

        // Deallocate all frames for the oldest process
        for (size_t i = 0; i < allocationMap.size(); i++) {
            if (allocationMap[i].isAllocated && allocationMap[i].processName == oldestProcess) {
                allocationMap[i].isAllocated = false;
                allocationMap[i].processName = "";
            }
        }

        // Handle paging strategy
        if (strategy == "Paging") {
            if (processPageMapping.find(oldestProcess) != processPageMapping.end()) {
                auto& allocatedPages = processPageMapping[oldestProcess];
                for (size_t pageNumber : allocatedPages) {
                    size_t frameNumber = pageTable.getFrame(pageNumber);
                    if (frameNumber != SIZE_MAX) {
                        // Free the frame and update allocation map
                        pageTable.removeMapping(pageNumber);
                        allocationMap[frameNumber].isAllocated = false;
                        allocationMap[frameNumber].processName = "";
                        freeFrameList.push(frameNumber);
                    }
                }
                processPageMapping.erase(oldestProcess);
            }
        }

        // Remove from processAges
        processAges.erase(oldestProcess);

        numOfProcesses--;
        return oldestProcess;
    }




    void deallocate(void* ptr, size_t size, string strategy, string processName) {
        lock_guard<mutex> lock(mtx);
		if (strategy == "Flat Memory") {
            
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
        else if (strategy == "Paging") {

            // Ensure the process exists in the page mapping
            if (processPageMapping.find(processName) == processPageMapping.end()) {
				//cout << "Error: Process '" << processName << "' not found in the page mapping." << endl;
                return;
            }

            auto& allocatedPages = processPageMapping[processName];  // Get all pages for the process

            // Iterate through all pages allocated to this process
            for (size_t pageNumber : allocatedPages) {
                size_t frameNumber = pageTable.getFrame(pageNumber);  // Get the frame mapped to this page

                if (frameNumber == SIZE_MAX) {
                    // Skip if the page was already deallocated (should not happen unless corrupted)
                    continue;
                }

                // Invalidate the page mapping in the page table
                pageTable.removeMapping(pageNumber);

                // Mark the frame as free and return it to the free list
                if (frameNumber < allocationMap.size()) {
                    allocationMap[frameNumber].isAllocated = false;
                    allocationMap[frameNumber].processName = "";
                }
                freeFrameList.push(frameNumber);  // Return frame to the free list
            }

            // Remove the process from the process page mapping
            processPageMapping.erase(processName);

            // Update process count
            numOfProcesses--;

            // Remove the process from the age tracker, if applicable
            for (auto it = processAges.begin(); it != processAges.end();) {
                if (it->first == processName) {
                    it = processAges.erase(it);
                }
                else {
                    ++it;
                }
            }

            //cout << "Process '" << processName << "' deallocated successfully." << endl;
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

