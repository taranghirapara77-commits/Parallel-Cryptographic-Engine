#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <sstream>
#include "IO.hpp"

using namespace std;

class ReadEnv{
    public:
        string getEnv(){
            // get the filestream of .env file
            string envPath = (filesystem::current_path().string() + "/.env");
            IO io(envPath);
            fstream stream = io.getFileStream();

            // upload all data/stream into a buffer
            stringstream buffer;
            buffer << stream.rdbuf();

            // get that data from buffer
            string content = buffer.str();

            return content;
        }
};
