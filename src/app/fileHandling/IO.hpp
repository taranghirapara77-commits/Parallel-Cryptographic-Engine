/*
    - An <fileName>.hpp file is a C++ header file used to declare classes, functions, and templates.
    - it helps separate declarations from definitions, making code modular and easier to manage.
*/

// used to prevent multiple inclusions of the same file.
#ifndef IO_HPP 
#define IO_HPP

#include <iostream>
#include <string>
#include <fstream>

using namespace std;

class IO{
    private: 
        fstream fileStream;
    
    public:
        IO(const string &filePath);
        ~IO();

        fstream getFileStream();
};

#endif