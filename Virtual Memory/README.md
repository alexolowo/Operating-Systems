# VMS Library - README

## Overview

This project involves the implementation of a virtual memory system (VMS) library that manages page tables and simulates the fork behavior of processes. The Sv39 multi-level page table design is used, and two fork strategies are implemented: one that creates a new page table and copies all page tables, and another that creates a new page table and utilizes a copy-on-write optimization. The provided `vms` executable in the `build` directory serves as a testing environment.

## Implementation

1. **Source Code:**
   - The main code modification takes place in the `src/vms.c` file.

2. **Library Functions:**
   - Two library functions are implemented in `vms.h`:
     - `vms_fork_copy()`: Creates a new L2 page table and simulates a fork operation without copy-on-write.
     - `vms_fork_copy_on_write()`: Creates a new L2 page table and simulates a fork operation with copy-on-write optimization.

3. **Page Fault Handling:**
   - The `page_fault_handler` function in `src/vms.c` is implemented to handle page faults.
   - After a page fault, the faulting PTE is modified to make it valid.
   - If the MMU generates another page fault, the process exits with the `EFAULT` error code.

4. **Custom Bit in PTE:**
   - A bit in the PTE is utilized for a specific purpose, accessible through `vms_pte_custom` with clear and set operations.

5. **Tracking References:**
   - The implementation keeps track of the number of references to some pages.
   - Defined values in `pages.h` are used, and it is assumed that the system will not create more pages, representing unchanging physical memory.

## Error Handling

- Proper error-checking is implemented to handle errors gracefully.
- For fatal errors, the library exits with the `errno` of the first fatal error encountered.
- The implementation is designed to avoid generating errors.

## Building

1. **Navigate:**
   - Ensure you are in the `vms` directory.

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
   - Execute test programs manually. Locate the test files in `tests/*.c` and the corresponding executables in `build/tests/*`.
   - Understand and observe the interactions between the test programs and the library.

2. **Test Suite:**
   - Run the provided test suite:

     ```bash
     meson test --print-errorlogs -C build
     ```
