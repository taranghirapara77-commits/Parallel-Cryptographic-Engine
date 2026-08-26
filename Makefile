# Compiler and flags
CXX = /opt/homebrew/bin/g++-15
CXXFLAGS = -std=c++17 -g -Wall -pthread \
           -I. \
           -Isrc/app/encryptDecrypt \
           -Isrc/app/fileHandling \
           -Isrc/app/processes

# Executable names
MAIN_TARGET = encrypt_decrypt
CRYPTION_TARGET = cryption

# Source files for each executable
MAIN_SRC = main.cpp \
           src/app/processes/ProcessManagement.cpp \
           src/app/fileHandling/IO.cpp \
           src/app/fileHandling/ReadEnv.cpp \
           src/app/encryptDecrypt/Cryption.cpp

CRYPTION_SRC = src/app/encryptDecrypt/CryptionMain.cpp \
               src/app/encryptDecrypt/Cryption.cpp \
               src/app/fileHandling/IO.cpp \
               src/app/fileHandling/ReadEnv.cpp

# Object files
MAIN_OBJ = $(MAIN_SRC:.cpp=.o)
CRYPTION_OBJ = $(CRYPTION_SRC:.cpp=.o)

# Default target
all: $(MAIN_TARGET) $(CRYPTION_TARGET)

# Build main executable
$(MAIN_TARGET): $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ -pthread

# Build cryption executable
$(CRYPTION_TARGET): $(CRYPTION_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@ -pthread

# Compile .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(MAIN_OBJ) $(CRYPTION_OBJ) $(MAIN_TARGET) $(CRYPTION_TARGET)

# Declare targets that aren't files
.PHONY: clean all