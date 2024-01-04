#include "test.h"

#include "wut.h"

static int x = 0;

void run(void) {
    ++x;
}

void test(void) {
    wut_init();
    shared_memory[0] = wut_id();
    int id1 = wut_create(run);
    shared_memory[1] = id1;
    int id2 = wut_create(run);
    shared_memory[2] = id2;
    shared_memory[3] = wut_yield();
    shared_memory[4] = x;
}

void check(void) {
    printf("Shared Memory 0: %d\n", shared_memory[0]);
    printf("Shared Memory 1: %d\n", shared_memory[1]);
    printf("Shared Memory 2: %d\n", shared_memory[2]);
    printf("Shared Memory 3: %d\n", shared_memory[3]);
    printf("Shared Memory 4: %d\n", shared_memory[4]);
    
    expect(
        shared_memory[0], 0, "wut_id of the main thread is wrong"
    );
    expect(
        shared_memory[1], 1, "wut_id of the first thread is wrong"
    );
    expect(
        shared_memory[2], 2, "wut_id of the second thread is wrong"
    );
    expect(
        shared_memory[3], 0, "wut_yield should be successful"
    );
    expect(
        shared_memory[4], 2, "two threads should write to x"
    );
}
