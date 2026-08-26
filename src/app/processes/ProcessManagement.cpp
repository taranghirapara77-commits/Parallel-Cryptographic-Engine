#include <iostream>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <thread>
#include <atomic>
#include <semaphore.h>
#include <signal.h>
#include "ProcessManagement.hpp"
#include "../encryptDecrypt/Cryption.hpp"

using namespace std;

ProcessManagement::ProcessManagement(ExecutionVersion ver) : version(ver) {
    if (version == ExecutionVersion::V2_MULTIPROCESSING || 
        version == ExecutionVersion::V3_MULTIPROCESSING_NESTED) {
        initializeMultiprocessing();
    }
    // V1_SEQUENTIAL - no initialization needed
}

ProcessManagement::~ProcessManagement() {
    if (version == ExecutionVersion::V2_MULTIPROCESSING || 
        version == ExecutionVersion::V3_MULTIPROCESSING_NESTED) {
        cleanupMultiprocessing();
    }
}

bool ProcessManagement::submitToQueue(unique_ptr<Task> task) {
    if (version == ExecutionVersion::V1_SEQUENTIAL) {
        sequentialQueue.push(move(task));
        // Execute immediately for sequential
        while (!sequentialQueue.empty()) {
            auto curTask = move(sequentialQueue.front());
            sequentialQueue.pop();
            executeCryption(curTask->toString());
        }
        return true;
    }
    else if (version == ExecutionVersion::V2_MULTIPROCESSING || 
             version == ExecutionVersion::V3_MULTIPROCESSING_NESTED) {
        cleanupChildProcesses();
        sem_wait(emptySlotsSemaphore);
        
        {
            unique_lock<mutex> lock(taskMutex);
            if (sharedMem->size.load() >= 1000) return false;
            
            strcpy(sharedMem->tasks[sharedMem->rear], task->toString().c_str());
            sharedMem->rear = (sharedMem->rear + 1) % 1000;
            sharedMem->size.fetch_add(1);
        }
        
        sem_post(itemsSemaphore);
        
        int pid = fork();
        if (pid < 0) return false;
        else if (pid == 0) {
            // Child process
            sem_wait(itemsSemaphore);
            char taskStr[256];
            {
                unique_lock<mutex> lock(taskMutex);
                strcpy(taskStr, sharedMem->tasks[sharedMem->front]);
                sharedMem->front = (sharedMem->front + 1) % 1000;
                sharedMem->size.fetch_sub(1);
            }
            sem_post(emptySlotsSemaphore);
            
            // Use nested multithreading for V3
            if (isMultiprocessingNested()) {
                executeCryptionParallel(taskStr, 4);
            } else {
                executeCryption(taskStr);
            }
            exit(0);
        } else {
            // Parent process
            unique_lock<mutex> lock(processMutex);
            childProcesses.push_back(pid);
        }
        return true;
    }
    return false;
}

void ProcessManagement::waitForCompletion() {
    if (version == ExecutionVersion::V1_SEQUENTIAL) {
        // Already completed in submitToQueue
    }
    else if (version == ExecutionVersion::V2_MULTIPROCESSING || 
             version == ExecutionVersion::V3_MULTIPROCESSING_NESTED) {
        waitForChildProcesses();
    }
}

void ProcessManagement::initializeMultiprocessing() {
    itemsSemaphore = sem_open("/items_semaphore", O_CREAT, 0666, 0);
    emptySlotsSemaphore = sem_open("/empty_slots_semaphore", O_CREAT, 0666, 1000);
    
    if (itemsSemaphore == SEM_FAILED || emptySlotsSemaphore == SEM_FAILED) {
        throw runtime_error("Failed to create semaphores");
    }
    
    shmFd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shmFd == -1) {
        throw runtime_error("Failed to create shared memory");
    }
    
    ftruncate(shmFd, sizeof(SharedMemory));
    sharedMem = static_cast<SharedMemory*>(mmap(nullptr, sizeof(SharedMemory), 
                                                PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0));
    
    sharedMem->front = 0;
    sharedMem->rear = 0;
    sharedMem->size.store(0);
}

void ProcessManagement::cleanupMultiprocessing() {
    waitForChildProcesses();
    
    if (sharedMem) {
        munmap(sharedMem, sizeof(SharedMemory));
        shm_unlink(SHM_NAME);
    }
    
    if (itemsSemaphore != SEM_FAILED) {
        sem_close(itemsSemaphore);
        sem_unlink("/items_semaphore");
    }
    
    if (emptySlotsSemaphore != SEM_FAILED) {
        sem_close(emptySlotsSemaphore);
        sem_unlink("/empty_slots_semaphore");
    }
}

void ProcessManagement::cleanupChildProcesses() {
    unique_lock<mutex> lock(processMutex);
    
    for (auto it = childProcesses.begin(); it != childProcesses.end();) {
        int status;
        pid_t result = waitpid(*it, &status, WNOHANG);
        
        if (result == *it || (result == -1 && errno == ECHILD)) {
            it = childProcesses.erase(it);
        } else {
            ++it;
        }
    }
}

void ProcessManagement::waitForChildProcesses() {
    unique_lock<mutex> lock(processMutex);
    
    for (pid_t childPid : childProcesses) {
        int status;
        waitpid(childPid, &status, 0);
    }
    
    childProcesses.clear();
}

bool ProcessManagement::isMultiprocessingNested() const {
    return version == ExecutionVersion::V3_MULTIPROCESSING_NESTED;
}

string ProcessManagement::getVersionName() const {
    if (version == ExecutionVersion::V1_SEQUENTIAL) {
        return "V1 - Sequential";
    }
    else if (version == ExecutionVersion::V2_MULTIPROCESSING) {
        return "V2 - Multiprocessing";
    }
    else if (version == ExecutionVersion::V3_MULTIPROCESSING_NESTED) {
        return "V3 - Multiprocessing with Nested Multithreading";
    }
    else {
        return "Unknown Version";
    }
}