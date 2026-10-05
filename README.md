# Student Record Management System
### B.Tech Artificial Intelligence and Data Science — DSA Capstone Project

A full-stack, dual-mode academic project demonstrating **genuine Data Structures and Algorithms in C** integrated seamlessly with a modern web dashboard.

---

## 🏗️ Architecture Overview

This project uses a **Dual-Mode Hybrid Architecture**:

```
 [ Web Browser ] <─── JSON / HTTP ───> [ Flask Python Bridge ] <─── CLI / stdio ───> [ C DSA Engine (dsa_core) ]
                                                                                             │
                                                                                 [ Binary File Storage (students.dat) ]
```

- **C DSA Core (`c_src/`)**: Handcrafted C99 implementation using dynamic memory allocation (`malloc`/`free`), raw pointers, structs, and no third-party libraries.
- **Python Flask Bridge (`app.py`)**: Lightweight backend executing the compiled C engine via subprocess and exposing standard RESTful JSON APIs.
- **Modern Web Dashboard (`templates/`, `static/`)**: Clean single-page application with interactive visualizers for Linked Lists, Undo Stacks, Request Queues, Department BSTs, and Course Prerequisite DAGs.
- **College Viva Mode**: Direct interactive command-line terminal interface (`dsa_core.exe --cli`) allowing complete demonstration without a browser.

---

## 📊 Core Data Structures Implemented

| Data Structure / Algorithm | File Location | Academic Purpose | Complexity |
|---|---|---|---|
| **Singly Linked List** | `c_src/linked_list.c` | Primary dynamic storage for student records; maintains sorted roll number order | Search: $O(n)$, Insert/Delete: $O(1)$/$O(n)$ |
| **Stack ADT (LIFO)** | `c_src/stack.c` | Undo desk: records snapshots of updated/deleted records and rolls them back | Push: $O(1)$, Pop: $O(1)$ |
| **Queue ADT (FIFO)** | `c_src/queue.c` | Service desk: processes certificate / ID-card requests in First-Come, First-Served order | Enqueue: $O(1)$, Dequeue: $O(1)$ |
| **Hash Table** | `c_src/hash_table.c` | Fast roll number lookup using separate chaining with 17 prime buckets | Average: $O(1)$, Worst: $O(n)$ |
| **Binary Search Tree** | `c_src/bst.c` | Department-wise grouping; in-order traversal yields alphabetical department rosters | Insert: $O(h)$, In-order: $O(n)$ |
| **Directed Graph & DAG** | `c_src/graph.c` | Academic course prerequisite network using adjacency lists | Space: $O(V + E)$ |
| **Topological Sort** | `c_src/graph.c` | Kahn's Algorithm (in-degree array + queue) to compute valid semester course progression | Time: $O(V + E)$ |
| **Merge Sort** | `c_src/sorting_searching.c` | Stable sorting of student records by CGPA (merit list) and by Name | Time: $O(n \log n)$ |
| **Binary Search** | `c_src/sorting_searching.c` | Rapid search by roll number on sorted student array | Time: $O(\log n)$ |
| **Linear Search** | `c_src/sorting_searching.c` | Case-insensitive substring matching for student names | Time: $O(n)$ |
| **File Handling** | `c_src/file_io.c` | Persistent storage using C binary I/O (`fopen`, `fread`, `fwrite`, `fclose`) | Synced automatically |

---

## 🚀 How to Run Locally

### 1. Launch the Web Application
Double-click `run_web.bat` or run:
```cmd
python app.py
```
Then open your browser at: **`http://127.0.0.1:5000`**

### 2. Launch the Terminal Viva / CLI Mode (Pure C)
Double-click `run_cli.bat` or run:
```cmd
.\dsa_core.exe --cli
```
This presents the 15-option interactive console menu directly in your terminal.

### 3. Recompiling the C Engine
If you modify any C files, simply run:
```cmd
build.bat
```
*(On Linux/macOS, run `make`)*

---

## 🎓 College Viva Preparation Guide

### Q1: How does your web application connect to your C code?
> *"Our project uses a subprocess IPC bridge. When an action occurs on the web dashboard (e.g. adding a student), Flask receives the HTTP request and invokes our compiled C binary `dsa_core` with structured command arguments (`--api add ...`). The C binary dynamically allocates memory, updates the linked list, saves to `students.dat`, and outputs a JSON response back to Flask."*

### Q2: How did you implement the Undo functionality?
> *"We implemented a Stack ADT using a dynamic singly linked list. When a student record is updated or deleted, the previous state is pushed onto the stack (`pushUndo`). When the user clicks Undo, `popUndo()` retrieves the top node in LIFO order and restores the previous student record in both the linked list and storage."*

### Q3: Why use both a Linked List and a Hash Table for students?
> *"The Singly Linked List allows dynamic, unbounded in-memory storage without pre-allocating large fixed arrays. The Hash Table with separate chaining provides $O(1)$ average time lookup by Roll Number, avoiding $O(n)$ linear traversal for frequent queries."*

### Q4: How is the Course Prerequisite graph evaluated?
> *"The curriculum prerequisites form a Directed Acyclic Graph (DAG). We implement Kahn's Algorithm for Topological Sorting: we compute the in-degree of every course vertex, enqueue courses with an in-degree of 0 (no prerequisites), and iteratively remove them while decrementing adjacent dependencies. If the number of processed vertices equals $V$, there are no circular dependency deadlocks."*
