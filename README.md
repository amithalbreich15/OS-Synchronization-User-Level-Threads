This project is focused on implementing a user-level threading library. The library will be responsible for creating and managing threads in a program. Here's an outline of what the project entails:

### **Theoretical Questions:**
The first part of the assignment asks theoretical questions about user-level threads, system calls, interrupts, signals, and related concepts. You need to demonstrate your understanding of key mechanisms that affect threads and processes, such as `sigsetjmp` and `siglongjmp`, interrupts triggered by commands like `kill`, the distinction between 'real' and 'virtual' time, and the trade-offs between using user-level threads versus kernel-level threads.

### **Coding Assignment:**
The second part requires implementing a **static library** that manages user-level threads. Your task is to build this library with the following features:

1. **Thread Creation & Management**:
   - Each thread has a unique ID, and the library supports a maximum number of threads.
   - The library must use a **Round-Robin scheduling algorithm** to manage thread execution.
   
2. **Thread States**:
   - Threads can be in one of three states: **RUNNING**, **BLOCKED**, or **READY**.
   - Transitions between these states occur due to library function calls or elapsed time.
   - The main thread (ID 0) behaves slightly differently, as it uses the same stack as the program’s initial state.

3. **Scheduler**:
   - The **Round-Robin scheduler** allocates each thread a predefined quantum (time slice) to run. Threads can be preempted if their quantum expires, they enter a blocked state, or they terminate.
   - The scheduler must handle thread state transitions, and when threads are preempted or blocked, it should ensure that the next thread in the queue starts executing immediately.

4. **Time Management**:
   - A **virtual timer** measures running time in the process, distinct from the real time of the system.
   - The library should efficiently handle thread scheduling and blocking with accurate timing.

5. **Library Functions**:
   - You will need to implement various functions to control thread behavior: create threads, block/unblock threads, terminate threads, and more.
   - A **queue** or **list** will be needed to manage ready threads, with careful attention to the order in which threads are scheduled.

6. **Signal Handling**:
   - Signal handling (e.g., using `sigsetjmp` and `siglongjmp`) is crucial for saving and restoring thread execution contexts, especially when switching between threads.

7. **Error Handling**:
   - Errors in system calls or the library functions must be handled gracefully, with appropriate error messages printed to `stderr`.

### **Submission Requirements**:
- A **README file** that includes answers to the theoretical questions and any other relevant details.
- The source code files for the threading library, along with the appropriate Makefile to generate the static library.
- The static library should be named `libuthreads.a`.
- Testing should be done thoroughly with assertions, but no debug printouts should be included in the final submission.

The goal of this project is to get hands-on experience with thread management, scheduling, and system-level programming.
