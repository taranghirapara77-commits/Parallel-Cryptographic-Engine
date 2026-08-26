#ifndef TASK_HPP
#define TASK_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "../fileHandling/IO.hpp"

using namespace std;

enum class Action{
    ENCRYPT,
    DECRYPT
};

struct Task{
    string filePath;
    fstream stream;
    Action action;
                                                 // member initializer list
    Task(fstream &&str, Action act, string path) : stream(move(str)), action(act), filePath(path){}
    // fstream&& is an rvalue reference used to move the stream efficiently — a temporary or movable fstream object.

    
    string toString(){
        ostringstream oss;
        oss << filePath << "," << (action == Action::ENCRYPT ? "ENCRYPT" : "DECRYPT");

        return oss.str();
    }

    static Task fromString(const string& taskData){
        istringstream iss(taskData);
        
        string path;
        string actStr;

        if(getline(iss, path, ',') && getline(iss, actStr)){
            Action act = (actStr == "ENCRYPT" ? Action::ENCRYPT : Action::DECRYPT);

            IO io(path);
            fstream fStr = move(io.getFileStream());

            if(fStr.is_open()){
                return Task(move(fStr), act, path);
            }
            else{
                throw runtime_error("Failed to open file" + path);
            }
        }
        else{
            throw runtime_error("Invalid Task data format");
        }
    }
};

#endif