#include <iostream>
#include <string>
#include <iomanip>
#include <vector>
#include <thread>
#include <algorithm>
#include <fstream>
#include "Cryption.hpp"
#include "../processes/Task.hpp"
#include "../fileHandling/ReadEnv.cpp"

char encrypt(char ch, int eKey){
    return (ch + eKey)%256;
}

char decrypt(char ch, int dKey){
    return (ch - dKey + 256)%256;
}

int executeCryption(const string& taskData){
    Task task = Task::fromString(taskData);

    ReadEnv env;
    string envKey = env.getEnv();
    int edKey = stoi(envKey);

    if(task.action == Action::ENCRYPT){
        char ch;

        // Reads, encrypt and overwrites each byte in-place one by one.
        while(task.stream.get(ch)){
            ch = encrypt(ch,edKey);
            task.stream.seekp(-1, ios::cur);
            task.stream.put(ch);
        }
        task.stream.close();
    }
    else if(task.action == Action::DECRYPT){
        char ch;

        // Reads, decrypt and overwrites each byte in-place one by one.
        while(task.stream.get(ch)){
            ch = decrypt(ch,edKey);
            task.stream.seekp(-1, ios::cur);
            task.stream.put(ch);
        }
        task.stream.close();
    }
    else{
        throw runtime_error("Invalid action type");
    }

    return 0;
}

// Parallel version of encryption/decryption
int executeCryptionParallel(const string& taskData, int numThreads) {
    Task task = Task::fromString(taskData);

    ReadEnv env;
    string envKey = env.getEnv();
    int edKey = stoi(envKey);

    // Get file size
    task.stream.seekg(0, ios::end);
    long fileSize = task.stream.tellg();
    task.stream.seekg(0, ios::beg);

    // Read entire file into memory
    vector<char> fileData(fileSize);
    task.stream.read(fileData.data(), fileSize);
    task.stream.close();

    // Divide file into chunks
    long chunkSize = fileSize / numThreads;
    vector<thread> threads;
    bool isEncrypt = (task.action == Action::ENCRYPT);

    // Process chunks in parallel
    for (int i = 0; i < numThreads; ++i) {
        long startPos = i * chunkSize;
        long endPos = (i == numThreads - 1) ? fileSize : (i + 1) * chunkSize;
        
        threads.emplace_back(processChunk, ref(fileData), startPos, endPos, edKey, isEncrypt);
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Write back to file
    fstream outFile(task.filePath, ios::in | ios::out | ios::binary);
    if (outFile.is_open()) {
        outFile.write(fileData.data(), fileSize);
        outFile.close();
    }

    return 0;
}

void processChunk(vector<char>& fileData, long startPos, long endPos, int key, bool isEncrypt) {
    for (long pos = startPos; pos < endPos; ++pos) {
        if (isEncrypt) {
            fileData[pos] = encrypt(fileData[pos], key);
        } else {
            fileData[pos] = decrypt(fileData[pos], key);
        }
    }
}