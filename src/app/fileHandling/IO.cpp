// #include < > is for system headers; 
#include <iostream> 
#include <fstream>

// #include " " is for user-defined or local headers.
#include "IO.hpp" 

using namespace std;

IO::IO(const string& filePath){
    // Open the file for read + write in binary mode.
    fileStream.open(filePath, ios::in | ios::out | ios::binary); 

    if(!fileStream.is_open()){
        cout << "Unable to open the file : " << filePath << '\n';
    }
}

fstream IO::getFileStream(){
    return move(fileStream);
}

IO::~IO(){
    if(fileStream.is_open()){
        fileStream.close();
    }
}