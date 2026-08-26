#ifndef PROCESS_MANAGEMENT_HPP
#define PROCESS_MANAGEMENT_HPP

#include <queue>
#include <atomic>
#include <mutex>
#include <semaphore.h>
#include <memory>
#include <thread>
#include <vector>
#include <condition_variable>
#include <sys/wait.h>
#include "Task.hpp"

using namespace std;

// Enum for different execution versions
enum class ExecutionVersion {
    V1_SEQUENTIAL,
    V2_MULTIPROCESSING,
    V3_MULTIPROCESSING_NESTED
};

class ProcessManagement {
private:
    ExecutionVersion version;
    
    // ==== V1: Sequential ==== //
    queue<unique_ptr<Task>> sequentialQueue;
    
    // ==== V2/V3: Multiprocessing ==== //
    sem_t* itemsSemaphore;
    sem_t* emptySlotsSemaphore;
    struct SharedMemory {
        atomic<int> size;
        char tasks[1000][256];
        int front;
        int rear;
    };
    SharedMemory* sharedMem;
    int shmFd;
    const char* SHM_NAME = "/encrypt_queue";
    vector<pid_t> childProcesses;
    mutex processMutex;
    mutex taskMutex;
    
    // Helper methods
    void initializeMultiprocessing();
    void cleanupMultiprocessing();
    void cleanupChildProcesses();
    void waitForChildProcesses();
    bool isMultiprocessingNested() const;
    
public:
    ProcessManagement(ExecutionVersion ver);
    ~ProcessManagement();
    
    // Main interface
    bool submitToQueue(unique_ptr<Task> task);
    void waitForCompletion();
    
    // Utility methods
    string getVersionName() const;
    ExecutionVersion getVersion() const { return version; }
    
    // Disable copy constructor and assignment
    ProcessManagement(const ProcessManagement&) = delete;
    ProcessManagement& operator=(const ProcessManagement&) = delete;
};

#endif