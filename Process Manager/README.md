# SSP Library - README

## Overview

This project involves creating a C library called `libssp` that acts as a process manager and subreaper. The library provides an API to initialize, create, monitor, and manage processes. It uses concepts learned during lectures, incorporating new system calls to enhance functionality. The library is designed to be incorporated into other programs.

## Task

### Library API

The `libssp` library provides the following API:

1. **`void ssp_init()`**
   - This function is called once before any other library calls. It initializes or sets up necessary components.

2. **`int ssp_create(char *const *argv, int fd0, int fd1, int fd2)`**
   - Creates a new process that eventually calls `execvp(argv[0], argv)`.
   - Sets file descriptors 0, 1, and 2 to match `fd0`, `fd1`, and `fd2` using `dup2`.
   - Closes all other file descriptors except 0, 1, and 2.
   - Records process details (PID, name, and status) and returns a unique `ssp_id`.
   - The process name is a copy of `argv[0]` since the memory may be reused by the library user.

3. **`void ssp_send_signal(int ssp_id, int signum)`**
   - Sends a signal `signum` to the process identified by `ssp_id`.
   - If the process is no longer running, the function does nothing.

4. **`int ssp_get_status(int ssp_id)`**
   - Returns the current status of the process identified by `ssp_id` without blocking.

5. **`void ssp_wait()`**
   - Blocks until all processes created through `ssp_create` terminate.
   - Ensures that all process statuses are between 0 and 255 after completion.

6. **`void ssp_print()`**
   - Outputs the PID, name, and current status of every process created through `ssp_create`.
   - The output includes a header with PID (right-justified, width 7), CMD (left-justified, width of the longest process name), and STATUS.
   - Process status is set based on exit status or signal termination. `-1` indicates an active process.

### Error Handling

- Proper error-checking is implemented.
- Expected errors are handled without additional output or process termination.
- For fatal errors, the library exits with the `errno` of the first fatal error encountered.

### Subreaper

- Optionally, the library can become a subreaper by adding `prctl(PR_SET_CHILD_SUBREAPER, 1)` in `ssp_init`.
- A subreaper adopts orphan processes created by child processes.
- Terminated orphan processes are recorded with their PID, status, and a name "<unknown>".
- These unknown processes are displayed in `ssp_print` after directly managed processes.

## Building

1. **Navigate:**
   - Ensure you are in the `ssp` directory.

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

1. **Manual Test Execution:**
   - Execute test programs manually. Find the test files in `tests/*.c` and corresponding executables in `build/tests/*`.

2. **Test Suite:**
   - Run the provided test suite:

     ```bash
     meson test --print-errorlogs -C build
     ```

