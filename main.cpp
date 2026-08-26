#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <thread>
#include <signal.h>
#include <sys/wait.h>
#include "src/app/processes/ProcessManagement.hpp"
#include "src/app/processes/Task.hpp"

using namespace std;  
namespace fs = filesystem;            

// Global ProcessManagement pointer for signal handling
ProcessManagement* globalProcessManagement = nullptr;

// Signal handler for cleanup
void signalHandler(int signum) {
    if (globalProcessManagement) {
        cout << "\nReceived signal " << signum << ". Cleaning up..." << endl;
        delete globalProcessManagement;
    }
    exit(signum);
}

ExecutionVersion getUserChoice() {
    cout << "\n=== Encryption/Decryption Execution Versions ===\n";
    cout << "1. V1 - Sequential (Baseline)\n";
    cout << "2. V2 - Multiprocessing\n";
    cout << "3. V3 - Multiprocessing with Nested Multithreading\n";
    cout << "\nEnter your choice (1-3): ";
    
    string choice;
    getline(cin, choice);
    
    if (choice == "1") return ExecutionVersion::V1_SEQUENTIAL;
    if (choice == "2") return ExecutionVersion::V2_MULTIPROCESSING;
    if (choice == "3") return ExecutionVersion::V3_MULTIPROCESSING_NESTED;
    
    cout << "Invalid choice. Defaulting to V1 - Sequential.\n";
    return ExecutionVersion::V1_SEQUENTIAL;
}

int main(int argc, char* argv[]) {
    cout << "=== File Encryption/Decryption System ===\n";
    
    // Get user inputs
    string directory;
    cout << "Enter the directory path: ";
    getline(cin, directory);

    string action;
    cout << "Enter the action (encryption/decryption): ";
    getline(cin, action);

    ExecutionVersion version = getUserChoice();

    // Set up signal handlers for proper cleanup
    signal(SIGINT, signalHandler);   // Ctrl+C
    signal(SIGTERM, signalHandler);  // Termination signal

    auto start = chrono::high_resolution_clock::now();
    
    // Create ProcessManagement instance
    ProcessManagement* processManagement = new ProcessManagement(version);
    globalProcessManagement = processManagement;
    
    try {
        if (!fs::exists(directory) || !fs::is_directory(directory)) {
            cout << "Error: Invalid directory path\n";
            delete processManagement;
            return 1;
        }

        cout << "\nProcessing files...\n";
        int fileCount = 0;
        
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                string path = entry.path().string();
                
                IO io(path);
                fstream stream = move(io.getFileStream());

                if (stream.is_open()) {
                    Action actionType = (action == "ENCRYPT" ? Action::ENCRYPT : Action::DECRYPT);
                    auto task = make_unique<Task>(move(stream), actionType, path);
                    
                    if (processManagement->submitToQueue(move(task))) {
                        fileCount++;
                    } else {
                        cout << "Failed to submit task for: " << path << '\n';
                    }
                } else {
                    cout << "Unable to open file: " << path << '\n';
                }
            }
        }
        
        cout << "Submitted " << fileCount << " files for processing.\n";
        
        // Wait for all tasks to complete
        cout << "Waiting for completion...\n";
        processManagement->waitForCompletion();
        
    } catch (const fs::filesystem_error& e) {
        cout << "Filesystem error: " << e.what() << '\n';
        delete processManagement;
        return 1;
    } catch (const exception& e) {
        cout << "Error: " << e.what() << '\n';
        delete processManagement;
        return 1;
    }

    auto end = chrono::high_resolution_clock::now(); 
    chrono::duration<double> elapsed = end - start;

    cout << "\n=== Results ===\n";
    cout << "Version: " << processManagement->getVersionName() << '\n';
    cout << "Total execution time: " << elapsed.count() << " seconds\n";
    cout << "Action: " << action << '\n';
    cout << "Directory: " << directory << '\n';

    // Clean up
    delete processManagement;
    globalProcessManagement = nullptr;

    return 0;
}