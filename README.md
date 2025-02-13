# MultiThreaded Elevator System

The **MultiThreaded Elevator System** is a **POSIX-compliant C program** that efficiently controls multiple elevators in a **K-floor building** to fulfill passenger requests with **minimal turns and movements**. The system utilizes **shared memory, message queues, and multi-threading** to communicate with helper and solver processes.


## 📑 Table of Contents
- [🎯 Features](#-features)
- [⚙️ Setup & Compilation](#️-setup--compilation)
  - [Prerequisites](#prerequisites)
  - [Compilation and Execution](#compilation-and-execution)
- [🚀 Comparing the Results](#-comparing-the-results)
- [🔗 Inter-Process Communication Overview](#-inter-process-communication-overview)
- [📊 Efficiency Metrics](#-efficiency-metrics)
- [🔄 Algorithm Logic](#-algorithm-logic)


---

## 🎯 Features
- 🏢 **Multi-elevator simulation**: Supports up to **100 elevators** operating concurrently.
- 🔄 **Efficient passenger handling**: Optimizes **pickup and drop-off** strategies.
- ⚡ **Multi-threading & IPC**: Uses **POSIX shared memory and message queues**.
- 🔐 **Authorization Mechanism**: Uses **solver processes** to obtain authorization strings for movement.
- 📊 **Performance Metrics**: Tracks and saves **total turns & movements** in *"results.txt"* file.

---

## ⚙️ Setup & Compilation

### Prerequisites
Ensure your system has:
- **Ubuntu 22.04+** (or any Linux distribution supporting POSIX)
- **GCC Compiler** (`gcc` installed)
- **POSIX IPC Libraries** (`#include <sys/ipc.h>`)


### Compilation and Execution
To compile the program for Brute-Force Approach:
```bash
gcc helper-brute.c -lpthread -o helper
gcc solution_brute.c -lpthread -o solution
```
**OR**

To compile the program for Multi_Threaded Approach:
```bash
gcc helper-mt.c -lpthread -o helper
gcc solution_mt.c -lpthread -o solution
```
Then run a particular test case (example 2):
```bash
./helper 2
```

---

## 🚀 Comparing the Results

The results of each test case is updated in the *results_mt.txt* and *results_brute.txt* in the following format:
```csharp
Test Case No:       2
Time Taken:         8.34 s
Number of Turns:    232
Elevator Movement:  1413
```

---

## 🔗 Inter-Process Communication Overview

1. **Shared Memory (shm)**
   - Stores **elevator positions, movement instructions, and requests**.
   - Used for **synchronization between processes**.

2. **Message Queues (msg)**
   - **Solver processes** provide **authorization strings** for non-empty elevators.
   - **Helper process** validates movements and updates the **elevator state**.

3. **Synchronization Mechanisms**
   - **Mutex Locks & Semaphores** ensure data integrity.
   - **Signal handling** manages inter-process communication.

---

## 📊 Efficiency Metrics
Your implementation is evaluated based on:
1. **Total turns** required to process all requests.
2. **Total elevator movements** (each up/down counts as one move).
3. **Correctness of passenger handling** (no request mishandling).

---

## 🔄 Algorithm Logic
1. **Initialize elevators** at the ground floor.
2. **Process new passenger requests** each turn.
3. **Decide elevator movements** (up, down, stay).
4. **Communicate with solver processes** to obtain authorization strings.
5. **Update shared memory & notify helper** for state validation.
6. **Repeat until all requests are completed**.

---
