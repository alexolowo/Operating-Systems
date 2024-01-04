# Concurrent Hash Table Implementation - Lab README

## Overview

This lab focuses on creating a concurrent hash table implementation, building upon a provided serial implementation. There are three versions of the hash table: a base serial implementation, and two concurrent versions labeled v1 and v2. The primary goal is to implement two locking strategies for v1 and v2, compare their performance, and ensure correctness in concurrent usage.


## Task

### Understanding Serial Hash Table Implementation

- Thoroughly read and comprehend the base serial hash table implementation present in `src/hash-table-base.c`. Review related header files (`src/hash-table-common.h` and `src/hash-table-base.h`) for additional context.

### Creating Hash Table v1

1. **Implementation:**
   - Develop the v1 version of the hash table in `src/hash-table-v1.c`.
   - Introduce thread safety to `hash_table_v1_add_entry` using a single mutex.
   - Initialize and destroy the mutex in `hash_table_v1_create` and `hash_table_v1_destroy` respectively.

2. **Testing:**
   - Execute the provided tester using:

     ```bash
     build/pht-tester -t 8 -s 50000
     ```

   - Ensure both correctness and performance are maintained.

3. **Questions:**
   - Respond to questions 1 and 2 on Crowdmark.

### Creating Hash Table v2

1. **Implementation:**
   - Implement the v2 version of the hash table in `src/hash-table-v2.c`.
   - Introduce thread safety to `hash_table_v2_add_entry` using multiple mutexes.
   - Initialize and destroy the mutex in `hash_table_v2_create` and `hash_table_v2_destroy` respectively.

2. **Testing:**
   - Run the provided tester again using:

     ```bash
     build/pht-tester -t 8 -s 50000
     ```

   - Ensure correctness and assess the performance of v2.


### Additional APIs

- Note the use of `pthread_mutex_t` for synchronization. Ensure to include the necessary header (`#include <pthread.h>`) in your implementation.

## Building

1. **Directory:**
   - Confirm that you are in the `pht` directory.

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

   - The setup command only needs execution once.

## Running the Tester

- Following the build, execute the tester with different configurations:

  ```bash
  build/pht-tester -t 8 -s 50000
  ```

- Analyze the output to ensure correct and efficient concurrent behavior.

## Errors

- Check for errors in any `pthread_mutex_*` functions used.
- Exit with the appropriate error code if any error occurs.
- Ensure the destruction of any locks created.

## Tips

- Consider increasing the number of cores allocated to your virtual machine or run the code on a machine with more cores for effective testing of concurrency and parallelism.