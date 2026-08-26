# 🔐 Encrypt-Decrypt
---
> A **high-performance** C++ system for encrypting and decrypting files, built with robust support for both **multiprocessing & multithreading**. Leveraging **shared memory and semaphores**, it seamlessly handles thousands of files in parallel — making it ideal for benchmarking and stress-testing concurrent file operations.
---

## 🚀 Features

* 🔄 **Encrypts and decrypts entire directories**
* ⚙️ **Supports Sequential, Multiprocessing, and Hybrid (MultiProcessing + MultiThreading) modes**
* 🧠 **Shared Memory** for efficient IPC + **Semaphores** for efficient process synchronization
* 📁 Handles **10,000+ files** with ease (auto-generated test cases)
* 🔑 Uses `.env` file to securely read the secret key
* 🧪 Python script to generate massive test datasets
* 📦 Build-ready with a single `Makefile`

---

## 📂 Project Structure

```
Encrypt-Decrypt/
├── main.cpp                    # Main entry point
├── Makefile                    # Build automation
├── makeDirs.py                 # Generates test files
├── src/
│   └── app/
│       ├── encryptDecrypt/
│       │   ├── Cryption.cpp   # Core encryption/decryption logic
│       │   ├── Cryption.hpp
│       │   └── CryptionMain.cpp
│       ├── fileHandling/
│       │   ├── IO.cpp         # File stream management
│       │   ├── IO.hpp
│       │   └── ReadEnv.cpp    # Reads key from .env file
│       └── processes/
│           ├── ProcessManagement.cpp  # Thread/Process manager
│           ├── ProcessManagement.hpp
│           └── Task.hpp       # Represents encryption/decryption task
```

---

## ⚙️ How It Works

```mermaid
%%{init: {'theme': 'base', 'themeVariables': {'background': '#000000'}}}%%
flowchart TD
    classDef common fill:#e0f7fa,stroke:#006064,stroke-width:2px;
    classDef v1 fill:#fff3e0,stroke:#ef6c00,stroke-width:2px;
    classDef v2 fill:#f3e5f5,stroke:#8e24aa,stroke-width:2px;
    classDef v3 fill:#e8f5e9,stroke:#2e7d32,stroke-width:2px;
    classDef child fill:#fce4ec,stroke:#ad1457,stroke-width:1.5px;
    classDef thread fill:#ede7f6,stroke:#4527a0,stroke-width:1.5px;
    classDef End fill:#e0e0e0,stroke:#424242,stroke-width:2px;

    Start(["Start"]):::common --> Inputs["Get Inputs (Path, Action, Version)"]:::common
    Inputs --> CheckDir{"📁 Dir Exists?"}:::common
    CheckDir -- No --> End(["End"]):::End
    CheckDir -- Yes --> Setup["⚙️ Setup (.env, Stream)"]:::common
    Setup --> Version{"🔀 Version?"}:::common

    %% V1
    Version -- V1 --> V1Start["V1: Sequential"]:::v1
    V1Start --> Queue1["Queue Tasks"]:::v1 --> Process1["🛠️ Process One-by-One"]:::v1 --> End

    %% V2
    Version -- V2 --> V2Start["V2: Multiprocessing"]:::v2
    V2Start --> Shared2["Shared Memory + Semaphores"]:::v2 --> Queue2["Queue Tasks"]:::v2
    Queue2 --> Forks2["Fork Child Processes"]:::v2
    Forks2 --> C2a["Child 1"]:::child
    Forks2 --> C2b["Child 2"]:::child
    C2a --> Wait2["⏳ Wait All Exit"]:::v2 --> End
    C2b --> Wait2

    %% V3
    Version -- V3 --> V3Start["V3: MultiProc + Threads"]:::v3
    V3Start --> Shared3["Shared Memory + Semaphores"]:::v3 --> Queue3["Queue Tasks"]:::v3
    Queue3 --> Forks3["Fork Child Processes"]:::v3
    Forks3 --> C3a["Child 1"]:::child --> T3a1["Thread 1"]:::thread & T3a2["Thread 2"]:::thread --> Join3a["Join"]:::child
    Forks3 --> C3b["Child 2"]:::child --> T3b1["Thread 1"]:::thread & T3b2["Thread 2"]:::thread --> Join3b["Join"]:::child
    Join3a --> Wait3["⏳ Wait All Exit"]:::v3 --> End
    Join3b --> Wait3
```

---

## 📸 Performance Comparison

Benchmarked on a MacBook Air (M2, 8-core). Each mode was tested using 10,000 files of different sizes.

| Version | Method                      | Files  | Chars/File | Encryption (s) | Decryption (s) |
| ------- | --------------------------- | ------ | ---------- | -------------- | -------------- |
| V1      | Sequential (Baseline)       | 10,000 | 1,000      | 2.17448        | 2.09383        |
| V2      | Multiprocessing             | 10,000 | 1,000      | 1.44115        | 1.51795        |
| V3      | Multiprocessing + Threading | 10,000 | 1,000      | **0.264162**   | **0.258924**   |
| V1      | Sequential (Baseline)       | 10,000 | 10,000     | 22.3493        | 22.3638        |
| V2      | Multiprocessing             | 10,000 | 10,000     | 14.2879        | 16.8432        |
| V3      | Multiprocessing + Threading | 10,000 | 10,000     | **1.90524**    | **1.62334**    |
| V1      | Sequential (Baseline)       | 10,000 | 100,000    | ∞              | ∞              |
| V2      | Multiprocessing             | 10,000 | 100,000    | 131.125        | 132.205        |
| V3      | Multiprocessing + Threading | 10,000 | 100,000    | **1.72663**    | **2.30900**    |

> ✅ **Version 3 (Hybrid Multiprocessing + Threads)** outperforms all others, especially on large file sets.

![ResultComparision](https://github.com/Harmit485/Encrypt-Decrypt/blob/main/Results/EncDecCompare.png)

---

## 🔧 Build Instructions

### Prerequisites

* `g++` with **C++17** support
* `Python 3` for test data generation

### Build the Project

```bash
make
```

### Generate Test Files

```bash
python3 makeDirs.py
```

---

## ▶️ Usage

```bash
./encrypt_decrypt
```

And follow the prompts:

```
Enter the directory path : test
Enter the action (encryption/decryption) : ENCRYPT/DECRYPT
Enter the version (V1/V2/V3) : 1/2/3
```

---

## 🔍 Internals Breakdown

| Component           | Purpose                                                         |
| ------------------- | --------------------------------------------------------------- |
| `Cryption`          | Byte-wise encryption/decryption logic                           |
| `ProcessManagement` | Manages process/thread spawning, synchronization via semaphores |
| `Task`              | Represents a unit of work for encryption/decryption             |
| `IO`                | Robust input/output file stream management                      |
| `ReadEnv`           | Parses `.env` to securely fetch the secret encryption key       |

---

## Example

Original `test1.txt`:

```
HelloWorld123
```

Encrypted with `KEY=5`:

```
MjqqtBtwqi678
```

---

## 💡 Fun Fact

The hybrid version (V3) can handle **10,000 files with 100,000 characters each** in **\~2 seconds**, thanks to intelligent use of **shared memory**, **child processes**, and **nested threads**.

---
