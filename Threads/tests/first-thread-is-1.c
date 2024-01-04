#include "test.h"

#include "wut.h"
#include <stdio.h>

void run(void) {
    return;
}

void test(void) {
    wut_init();
    shared_memory[0] = wut_id();
    shared_memory[1] = wut_create(run);
}

void check(void) {
    //int testVar = wut_create(run);
    //printf("Shared Mem 1: %d\n", testVar);
    expect(
        shared_memory[0], 0, "wut_id of the main thread is wrong"
    );
    expect(
        shared_memory[1], 1, "wut_id of the first thread is wrong"
    );
}
