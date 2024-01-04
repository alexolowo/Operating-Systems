# Process Utility - README

## Overview

This program is a small utility written in C that provides information about the currently running processes on a Linux machine. It replicates the functionality of popular utilities like `ps` and `htop`. The implementation involves reading the `/proc` directory and its contents to extract process details, displaying the PID and name of each active process.

## Functionality

1. **Output Format:**
   - The program prints the PID and name of every running process.
   - A header is displayed with PID right-justified (width of 5 characters), followed by CMD.

2. **Data Retrieval:**
   - The program reads the `/proc` directory, arranging directories in ascending PID order.
   - For each process, it reads the `/proc/<pid>/status` file to obtain the process name.

3. **Error Handling:**
   - Proper error-checking is implemented.
   - Directories and file descriptors are closed appropriately.
   - Information about nonexistent processes is not printed.

## Building

1. **Navigate:**
   - Move to the `tps` directory before proceeding.

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

## Testing

1. **Run Program:**
   - Execute your program with `build/tps` and compare the output to `ps -eo pid:5,ucmd`.

2. **Test Suite:**
   - Run the provided test suite:

     ```bash
     meson test --print-errorlogs -C build
     ```

3. **Test Failure:**
   - If a test fails, check the log file for details on the failure.

4. **Specific Test:**
   - To run a specific test, use:

     ```bash
     tests/ps_compare.py build/tps
     ```

5. **Explore Tests:**
   - Discover other tests in the `tests` directory.

## Tips

1. **Function Knowledge:**
   - Familiarize yourself with C functions such as `opendir`, `readdir`, `closedir`, `open`, `read`, `close`, `perror`, and `exit`.

2. **Header Files:**
   - Necessary header files are provided in the skeleton code.

3. **Experience Gain:**
   - While it might be possible to complete the lab without using all the specified functions, this lab is designed to provide valuable experience with these functions, which will be useful in future labs.

