#include "wut.h"

#include <stdio.h>

/* Do not modify this function, you should call this to check for any value
   you want to inspect from the solution. */
void check(int value, const char* message) {
    printf("Check: %d (%s)\n", value, message);
}
void testFunction(void){
        printf("I ran successfully!\n");
        return;
    }
int main() {
    /*
    You may write any of your own testing code here.

    You can execute it using `build/test/wut`. However, without your own
    implementation, it shouldn't do much. We'll run this code with the solution
    so you can clarify anything you want, or write a tricky test case.

    Place at least one call to `check` with a value (probably a return from a
    library call) that you'd like to see the output of. For example, here's
    how to convert `tests/main-thread-is-0.c` into this format:
    
    wut_init();
    check(wut_id(), "wut_id of the main thread is should be 0");

    */

    wut_init();
    int shared_memory[5];    
    shared_memory[0] = wut_id();
    shared_memory[1] = wut_create(testFunction);
    shared_memory[2] = wut_create(testFunction);
    check(shared_memory[0], "The main thread\n");
    check(shared_memory[0], "The 1st thread\n");
    check(shared_memory[0], "The 2nd thread\n");
    return 0;
}
