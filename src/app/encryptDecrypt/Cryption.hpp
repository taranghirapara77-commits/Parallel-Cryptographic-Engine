#ifndef CRYPTION_HPP
#define CRYPTION_HPP

#include <string>
#include <vector>
#include <thread>
#include <fstream>

using namespace std;

int executeCryption(const string& taskData);
char encrypt(char ch, int eKey);
char decrypt(char ch, int dKey);

// Parallel encryption functions
int executeCryptionParallel(const string& taskData, int numThreads = 4);
void processChunk(vector<char>& fileData, long startPos, long endPos, int key, bool isEncrypt);

#endif