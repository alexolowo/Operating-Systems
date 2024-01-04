# Lab README: User-Space Threads with libwut

## Overview

This lab involves the development of a user-space threading library named `libwut`. The library implements cooperative threads that continue execution until they either exit or explicitly yield. Utilizing system calls and concepts covered in the lectures, `libwut` provides functions for thread creation, yielding, cancellation, joining, and exiting.

## Task

### Threading Library - libwut

#### Functions:

1. **`wut_init()`**
   - Called once before any other library call.
   - Initializes the threading library, setting up the main thread as thread 0.

2. **`wut_create(void (*run)(void))`**
   - Creates a new thread, configuring it to execute the specified function.
   - Returns a unique thread ID.

3. **`wut_id()`**
   - Returns the ID of the currently executing thread.

4. **`wut_yield()`**
   - Yields to the next thread in the ready queue.
   - Returns 0 on successful yield, -1 on error (e.g., no available threads to switch to).

5. **`wut_cancel(int id)`**
   - Cancels the specified thread, removing it from the ready queue.
   - Frees associated memory.
   - Returns 0 on successful cancellation, -1 on error.

6. **`wut_join(int id)`**
   - Causes the calling thread to block until the specified thread terminates.
   - Frees associated memory.
   - Returns the status of the waited-on thread on success, -1 on error.

7. **`wut_exit(int status)`**
   - Exits the current thread, setting its status.
   - Removes the thread from the ready queue.

#### Error Handling:

- Proper error-checking is implemented to gracefully handle errors.
- For fatal errors, the library exits with the `errno` of the first encountered fatal error.
- The implementation is designed to avoid generating errors.

## Testing

### Manual Testing:

1. **Execution:**
   - Execute test programs manually by locating the files in `tests/*.c`.
   - Observe interactions between the test programs and the `libwut` library.

2. **Test Suite:**
   - Run the provided test suite:

     ```bash
     meson test --print-errorlogs -C build
     ```

## Building

1. **Directory:**
   - Ensure you are in the `wut` directory.

2. **Compilation:**
   - Run the following commands for setup and compilation:

     ```bash
     meson setup build
     meson compile -C build
     ```

3. **Recompilation:**
   - After making changes, recompile using:

     ```bash
     meson compile -C build
     ```

   - The setup command needs to be executed only once.