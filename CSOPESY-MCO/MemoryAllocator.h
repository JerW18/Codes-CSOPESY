#pragma once

#ifndef MEMORY_H
#define MEMORY_H

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <algorithm>

class MemoryAllocator {
private:
    std::string memType;
    int maxOverallMem;
    int memPerFrame; // For paging

    // Common Data
    std::vector<std::string> memoryBlocks;

    // Paging-specific data
    std::unordered_map<int, std::vector<int>> pageTable; // Process to frames mapping
    std::vector<bool> frameBitmap;

public:
    // Constructor for flat memory
    MemoryAllocator(int maxMem)
        : maxOverallMem(maxMem), memPerFrame(0), memType("Flat Memory") {
        memoryBlocks.resize(maxMem, ""); // Initialize flat memory
    }

    // Constructor for paging memory
    MemoryAllocator(int maxMem, int frameSize)
        : maxOverallMem(maxMem), memPerFrame(frameSize), memType("Paging") {
        int numFrames = maxMem / frameSize;
        frameBitmap.resize(numFrames, false); // Initialize bitmap
    }

    // Allocate memory
    std::pair<void*, std::string> allocate(
        int requestedSize, const std::string& memType, const std::string& processName, const std::string& schedulerType) {
        if (memType == "Flat Memory") {
            // Flat Memory Allocation
            for (int i = 0; i <= maxOverallMem - requestedSize; i++) {
                bool spaceAvailable = true;
                for (int j = 0; j < requestedSize; j++) {
                    if (!memoryBlocks[i + j].empty()) {
                        spaceAvailable = false;
                        break;
                    }
                }

                if (spaceAvailable) {
                    for (int j = 0; j < requestedSize; j++) {
                        memoryBlocks[i + j] = processName;
                    }
                    return {nullptr, "Flat Memory allocation successful"};
                }
            }
            return {nullptr, "Flat Memory allocation failed"};
        } else if (memType == "Paging") {
            // Paging Memory Allocation
            int numFramesNeeded = (requestedSize + memPerFrame - 1) / memPerFrame; // Ceiling division
            std::vector<int> allocatedFrames;

            for (int i = 0; i < frameBitmap.size() && numFramesNeeded > 0; i++) {
                if (!frameBitmap[i]) {
                    frameBitmap[i] = true;
                    allocatedFrames.push_back(i);
                    numFramesNeeded--;
                }
            }

            if (numFramesNeeded == 0) {
                pageTable[std::stoi(processName)] = allocatedFrames; // Assign frames to process
                return {nullptr, "Paging allocation successful"};
            }

            // Rollback in case of failure
            for (int frame : allocatedFrames) {
                frameBitmap[frame] = false;
            }
            return {nullptr, "Paging allocation failed"};
        }

        return {nullptr, "Invalid memory type"};
    }

    // Deallocate memory
    void deallocate(const std::string& processName, const std::string& memType) {
        if (memType == "Flat Memory") {
            // Flat Memory Deallocation
            for (auto& block : memoryBlocks) {
                if (block == processName) {
                    block = "";
                }
            }
        } else if (memType == "Paging") {
            // Paging Memory Deallocation
            int processId = std::stoi(processName);
            if (pageTable.find(processId) != pageTable.end()) {
                for (int frame : pageTable[processId]) {
                    frameBitmap[frame] = false;
                }
                pageTable.erase(processId);
            }
        }
    }

    // Print Memory State
    void printMemoryState() {
        if (memType == "Flat Memory") {
            std::cout << "Flat Memory State:\n";
            for (int i = 0; i < memoryBlocks.size(); i++) {
                if (memoryBlocks[i].empty()) {
                    std::cout << "[ ]";
                } else {
                    std::cout << "[" << memoryBlocks[i] << "]";
                }
                if ((i + 1) % 10 == 0) std::cout << "\n"; // Format in rows of 10
            }
        } else if (memType == "Paging") {
            std::cout << "Paging Memory State:\n";
            for (const auto& [process, frames] : pageTable) {
                std::cout << "Process " << process << ": ";
                for (int frame : frames) {
                    std::cout << frame << " ";
                }
                std::cout << "\n";
            }
        }
    }
};







/*
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include "timeStamp.h"
#include <unordered_map>
#include <limits.h>
#include <queue>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <atomic>
#include <algorithm>
#include <functional>
#include <iomanip>

#include "screen.h"


using ull = unsigned long long;

using namespace std;
struct MemoryBlock {
    bool isFree;
    size_t startAddress;
    size_t endAddress;
    string processName;
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
    queue<size_t> freePageList;

    

public:
    PageTable(size_t totalPages) {
        table.resize(totalPages); 
        for (size_t i = 0; i < totalPages; i++) {
            freePageList.push(i); 
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
	int pageOut = 0;
	int pageIn = 0;

    unordered_map<string, int> processAges;
    unordered_map<string, vector<size_t>> processPageMapping;
    deque<shared_ptr<process>>* processes;

    size_t totalAllocatedMemory = 0;
public:
    mutex mtx;

    //list<int> runningProcessesId;
	vector<int> runningProcessesId;

    MemoryAllocator(size_t totalMemorySize, size_t frameSize, deque<shared_ptr<process>>* processes)
        : memory(totalMemorySize, 0), allocationMap(totalMemorySize / frameSize),
        frameSize(frameSize), totalFrames(totalMemorySize / frameSize),
        totalMemorySize(totalMemorySize), pageTable(totalMemorySize / frameSize),
        numOfProcesses(0), currentAge(0) {
        this->processes = processes;
        for (size_t i = 0; i < totalFrames; ++i) {
            freeFrameList.push(i);  
        }
    }

    MemoryAllocator(size_t totalMemorySize, deque<shared_ptr<process>>* processes)
        : memory(totalMemorySize, 0), allocationMap(totalMemorySize),
        frameSize(1), totalFrames(totalMemorySize),
        totalMemorySize(totalMemorySize), pageTable(totalMemorySize),
        numOfProcesses(0), currentAge(0) {
        this->processes = processes;
        for (size_t i = 0; i < totalFrames; ++i) {
            freeFrameList.push(i);
        }
    }

	void setRunningProcessesId(vector<int> runningProcessesId) {
		this->runningProcessesId = runningProcessesId;
	}

	void addRunningProcessId(int processId) {
		runningProcessesId.push_back(processId);
	}

	/*void removeRunningProcessId(int processId) {
		runningProcessesId.erase(remove(runningProcessesId.begin(), runningProcessesId.end(), processId), runningProcessesId.end());
	}*/
    void removeRunningProcessId(int processId) {
        mutex mute_x;
		lock_guard<mutex> lock(mute_x);
        runningProcessesId.erase(remove(runningProcessesId.begin(), runningProcessesId.end(), processId), runningProcessesId.end());
    }

	void printRunningProcessesId() {
		cout << "Running Processes: ";
		for (int processId : runningProcessesId) {
			cout << processId << " ";
		}
		cout << endl;
	}

    void printProcessAges() {
        for (const auto& entry : processAges) {
            cout << entry.first << " " << entry.second << endl;
        }
    }
    
    pair<void*, string> allocate(size_t size, string strategy, string processName, string scheduler) {
		lock_guard<mutex> lock(mtx);
		pair <void*, string> result;
		result.first = nullptr;
		result.second = "";

        void* temp;
        string replacedProncessName="";
		//first try to allocate memory
		//cout << "Allocating " << size << " bytes for process '" << processName << "' using " << strategy << " strategy." << endl;
        if (strategy == "Flat Memory") {
			temp = allocateFirstFit(size, processName);
        }
        else
			temp = allocatePaging(size, processName);

		//try again if allocation failed
		if (temp == nullptr && scheduler!="fcfs") {
			replacedProncessName = swapOutOldestProcess(strategy);
            //returns empty if nothing was dealloc'd
            if (replacedProncessName != "") {
                if (strategy == "Flat Memory") {
                    temp = allocateFirstFit(size, processName);
                }
                else
                    temp = allocatePaging(size, processName);
            }
            else {
                //cout << "Nada";
				return result;
            }
		}

        if (temp != nullptr) {
            numOfProcesses++;
			processAges[processName] = currentAge++;
        }

		result.first = temp; //null, "p_x" where x is the process name
		result.second = replacedProncessName;
		return result;
    }

    void* allocateFirstFit(size_t size, string processName) {
        size_t framesNeeded = (size + frameSize - 1) / frameSize;

        for (size_t i = 0; i + framesNeeded <= allocationMap.size(); i++) {
            bool found = true;
            for (size_t j = 0; j < framesNeeded; j++) {
                if (allocationMap[i + j].isAllocated) {  // Check if the frame is already allocated
                    found = false;
                    break;
                }
            }
            if (found) {
                // Mark the frames as allocated and assign the process name
                for (size_t j = 0; j < framesNeeded; j++) {
                    allocationMap[i + j].isAllocated = true;
                    allocationMap[i + j].processName = processName;  // Store the process name
                }

                return &memory[i * frameSize];  // Return the starting address of the allocated memory block
            }
        }

        return nullptr;
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
			pageIn++;
            
        }

        // Track the pages allocated to the process
        processPageMapping[processName].insert(processPageMapping[processName].end(), allocatedFrames.begin(), allocatedFrames.end());

        // Return a pointer to the allocated memory (start of first frame)
        return &memory[allocatedFrames[0] * frameSize];
    }

    string swapOutOldestProcess(string strategy) {
		mutex mtx1;
		lock_guard<mutex> lock(mtx1);
        if (processAges.empty()) return "";

        // Find the oldest process that isnt running
        string oldestProcess;
        int oldestAge = INT_MAX;
        int processId = -1;
        for (const auto& entry : processAges) {
            if (entry.second < oldestAge) {
                oldestAge = entry.second;
                oldestProcess = entry.first;
            }
            processId = stoi(oldestProcess.substr(2));
        }
        if (!runningProcessesId.empty()) {
			this_thread::sleep_for(chrono::milliseconds(100));
            if(find(runningProcessesId.begin(), runningProcessesId.end(), processId) != runningProcessesId.end()) 
                return "";
        }
            
		if (processAges.empty()) return "";

        if (strategy == "Flat Memory") {
            if (processPageMapping.find(oldestProcess) != processPageMapping.end()) {
                auto& allocatedPages = processPageMapping[oldestProcess];
                for (size_t frameNumber : allocatedPages) {
					cout << "Frame Number " << frameNumber << " is deallocated" << endl;
                    allocationMap[frameNumber].isAllocated = false;
                    allocationMap[frameNumber].processName = "";
                    freeFrameList.push(frameNumber);
                    
                }

                processPageMapping.erase(oldestProcess);
            }
        } else if (strategy == "Paging") {
            // Deallocate all frames for the oldest process
            for (size_t i = 0; i < allocationMap.size(); i++) {
                if (allocationMap[i].isAllocated && allocationMap[i].processName == oldestProcess) {
                    allocationMap[i].isAllocated = false;
                    allocationMap[i].processName = "";
                }
            }

            if (processPageMapping.find(oldestProcess) != processPageMapping.end()) {
                auto& allocatedPages = processPageMapping[oldestProcess];
                for (size_t pageNumber : allocatedPages) {
                    size_t frameNumber = pageTable.getFrame(pageNumber);
                    if (frameNumber != SIZE_MAX) {
                        pageTable.removeMapping(pageNumber);
                        allocationMap[frameNumber].isAllocated = false;
                        allocationMap[frameNumber].processName = "";
                        freeFrameList.push(frameNumber);
                    }
                }
                processPageMapping.erase(oldestProcess);
            }

            numOfProcesses--;

        }


        processAges.erase(oldestProcess);
        
        return oldestProcess;
    }

    void deallocate(void* ptr, size_t size, string strategy, string processName) {
        mutex mutex3;
        lock_guard<mutex> lock(mutex3);
		    
		if (strategy == "Flat Memory") {

            void* temp = &memory[0];
            //size_t index = static_cast<char*>(ptr) - temp;
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

            
            /*void* temp = &memory[0];
            size_t index = static_cast<char*>(ptr) - static_cast<char*>(temp);

            if (index % frameSize != 0 || index / frameSize >= allocationMap.size()) {
                return;
            }

            size_t frames = size;
            size_t validFrames = 0;

            for (size_t i = 0; i < frames; i++) {
                if (index + i >= allocationMap.size()) {
                    break;
                }
                

                if (allocationMap[index + i].isAllocated) {
                    allocationMap[index + i].isAllocated = false;
                    allocationMap[index + i].processName = "";
                    validFrames++;
                    // cout << index + i << " " << allocationMap[index + i].isAllocated << endl;
                }
                freeFrameList.push(index + i);
            }

            // Update global memory tracking
            if (validFrames > 0) {
                totalAllocatedMemory -= validFrames;
            }

            // Decrement process count and remove from age tracker
            numOfProcesses--;
            if (processAges.find(processName) != processAges.end()) {
                processAges.erase(processName);
            }

            //cout << "Deallocated " << validFrames << " frames for process '" << processName << "'." << endl;*/
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
				pageOut++;

                // Mark the frame as free and return it to the free list
                if (frameNumber < allocationMap.size()) {
                    allocationMap[frameNumber].isAllocated = false;
                    allocationMap[frameNumber].processName = "";
                }
                freeFrameList.push(frameNumber);  
            }

            processPageMapping.erase(processName);
            numOfProcesses--;

            // Remove the process from the age tracker, if applicable
            /*for (auto it = processAges.begin(); it != processAges.end();) {
                if (it->first == processName) {
                    it = processAges.erase(it);
                }
                else {
                    ++it;
                }
            }*/

            if (processAges.find(processName) != processAges.end()) {
                processAges.erase(processName);
            }

            //cout << "Process " << processName << " deallocated successfully." << endl;
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

	ull getUsedMemorySize() {
		ull usedMemorySize = 0;
		for (size_t i = 0; i < allocationMap.size(); i++) {
			if (allocationMap[i].isAllocated) {
				usedMemorySize += frameSize;
			}
		}
		return usedMemorySize;
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

    void printFreeFrameList() {
        cout << "Free Frame List: ";
        queue<size_t> temp = freeFrameList;
        while (!temp.empty()) {
            cout << temp.front() << " ";
            temp.pop();
        }
        cout << endl;
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
            cout << allocationMap[i].isAllocated << "|" << allocationMap[i].processName << " . ";
        }
        cout << endl;
    }
    void printOtherStuff() {
        cout << "Total memory size: " << totalMemorySize << ", Frame size: " << frameSize << endl;
        cout << "Memory vector size should be: " << memory.size() << endl;
        cout << "Allocation map size should be: " << allocationMap.size() << endl;
    }

	int getPageIn() {
		return pageIn;
	}

	int getPageOut() {
		return pageOut;
	}
};

*/
#endif