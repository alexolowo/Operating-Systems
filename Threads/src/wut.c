#include "wut.h"
#include "tests/test.h"
#include <stdbool.h>
#include <assert.h> // assert
#include <bits/pthreadtypes.h>
#include <errno.h> // errno
#include <stddef.h> // NULL
#include <stdio.h> // perror
#include <stdlib.h> // reallocarray
#include <sys/mman.h> // mmap, munmap
#include <sys/signal.h> // SIGSTKSZ
#include <sys/queue.h> // TAILQ_*
#include <sys/ucontext.h>
#include <ucontext.h> // getcontext, makecontext, setcontext, swapcontext
#include <valgrind/valgrind.h> // VALGRIND_STACK_REGISTER



static int runningThreadID = -1;
int assignedID;
int prevAssignedID;
int capacity = 20;
static ucontext_t tempContext;
static int BLOCKED = 10;
static int READY = 0;
static int TERMINATED = 128;


struct list_entry {
    int id;
    TAILQ_ENTRY (list_entry) pointers;
};
TAILQ_HEAD(list_head, list_entry); 
static struct list_head readyQueue;
const int THREAD_NONE = -1;
typedef struct TCB {
    int id;
    int status;
    char* stackBaseAddress;
    ucontext_t *context;
    int waitingThreadId; // keep track of what it's waiting on?
    bool contextSet;
    // TBD
} TCB;
TCB *ThreadControlBlocks;  // global array of thread control blocks.
// static array of TCBs
    // no rebalancing
    // 
//TCB* runningThread = NULL;
static void die(const char* message) {
    int err = errno;
    perror(message);
    exit(err);
}
static char* new_stack(void) {
    char* stack = mmap(
        NULL,
        SIGSTKSZ,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_ANONYMOUS | MAP_PRIVATE,
        -1,
        0
    );
    if (stack == MAP_FAILED) {
        die("mmap stack failed");
    }
    VALGRIND_STACK_REGISTER(stack, stack + SIGSTKSZ);
    return stack;
}
void checkErr(int value, char* message){
    if(value<0){
        perror(message);
    }
}
static void delete_stack(char* stack) {
    if (munmap(stack, SIGSTKSZ) == -1) {
        die("munmap stack failed");
    }
}
void wut_init() {
    // initialize a queue
    TAILQ_INIT(&readyQueue);
    assert(TAILQ_EMPTY(&readyQueue));
    // initialize the TCB array
    ThreadControlBlocks = (TCB*)malloc(capacity*sizeof(TCB));
    // printf("TCB arr size: %lu\n", sizeof(ThreadControlBlocks));
    if (ThreadControlBlocks == NULL) {
        perror("Failed to allocate memory for ThreadControlBlocks");
        exit(EXIT_FAILURE);
    }
    
    // the main thread
    TCB* mainThread = (TCB*)malloc(sizeof(TCB));
    // initialize all TCB Id's to -1
    for(int i =0; i<capacity; i++){
        ThreadControlBlocks[i].id=-1;
    }
    mainThread->waitingThreadId=THREAD_NONE;
    mainThread->id=0;
    mainThread->stackBaseAddress=NULL;
    mainThread->status=0;
    getcontext(&tempContext);
    mainThread->context = &tempContext;
    //assert(getcontext(mainThread->context)==0);
    //printf("After Init Assert\n");
    printf("(WUT INIT) Address of Temp Context: %p\n", (void*)&tempContext);
    ThreadControlBlocks[0].waitingThreadId=mainThread->waitingThreadId;
    ThreadControlBlocks[0].id=mainThread->id;
    ThreadControlBlocks[0].stackBaseAddress=mainThread->stackBaseAddress;
    ThreadControlBlocks[0].status=mainThread->status;
    ThreadControlBlocks[0].context=mainThread->context;
    ThreadControlBlocks[0].contextSet=false;
    
    //runningThread = &ThreadControlBlocks[0];
    runningThreadID = 0;
    
   free(mainThread);
    //free(tempContext);
}
int wut_id() {
    printf("(WUT_ID) ID: %d\n", runningThreadID);
    return (runningThreadID>capacity) ? -1 : runningThreadID ;
}
void printTCB_Arr(){
    for(int i =0; i<capacity; i++){
        printf("entry: %d, %d\n",i, ThreadControlBlocks[i].id);
    }
}
void thread_start(void (*run)(void)) {
    run(); // Call the thread's actual run function.
    wut_exit(0); // Call wut_exit with a status of 0 when done.
}
int wut_create(void (*run)(void)) {
    assignedID = -1;
    for(int i = 1; i < capacity; i++) {
        if(ThreadControlBlocks[i].id == -1) {
            assignedID = i;
            break;
        }
        printf("For Loop Iteration Exit\n");
    }
    if (assignedID == -1) {
        // No available ID found.
        return -1;
    }
    // Allocate a TCB object and context
    TCB *newThread = &ThreadControlBlocks[assignedID];
    newThread->context = (ucontext_t*)malloc(sizeof(ucontext_t));
    if (newThread->context == NULL) {
        return -1; // Could not allocate memory for context.
    }
    // Allocate a stack for the new thread
    char* newThreadsStack = new_stack();
    if (newThreadsStack == NULL) {
        free(newThread->context);
        return -1; // Could not allocate memory for stack.
    }
    // Initialize the new thread context
    getcontext(newThread->context);
    newThread->context->uc_stack.ss_sp = newThreadsStack;
    newThread->context->uc_stack.ss_size = SIGSTKSZ;
    newThread->context->uc_link = NULL; // Set uc_link to what makes sense for your implementation
    // Set its user context using ucontext_t (using makecontext)
    // makecontext(newThread->context, run, 0);
    makecontext(newThread->context, (void (*)(void))thread_start, 1, run);
    // Assign TCB fields
    newThread->stackBaseAddress = newThreadsStack;
    newThread->id = assignedID;
    newThread->status = 0;
    newThread->waitingThreadId = THREAD_NONE;
    newThread->contextSet = false;
    printf("(WUT CREATE) Address of Temp Context: %p\n", (void*)newThread->context);
    printf("Tempcontext id: %d\n", assignedID);
    // Add it to a ready queue in FIFO order.
    struct list_entry *newQueueEntry = (struct list_entry*)malloc(sizeof(struct list_entry));
    newQueueEntry->id = newThread->id;
    TAILQ_INSERT_TAIL(&readyQueue, newQueueEntry, pointers);
    
    printf("Created Thread id: %d\n", newThread->id);
    // No need to free newThread here, it's part of the ThreadControlBlocks array.
    // Freeing newThread would actually cause undefined behavior since it's not dynamically allocated.
    return assignedID;
}
int wut_cancel(int id) {
    for (int i=0; i<capacity; i++) {
        if (ThreadControlBlocks[i].id==wut_id() && ThreadControlBlocks[i].id==id) {
            printf("Cannot Cancel Self\n");
            return -1;
        }
    }

    // remove from readyQ
    struct list_entry *e=NULL;
    TAILQ_FOREACH(e, &readyQueue, pointers){
        if (e->id==id) {
            TAILQ_REMOVE(&readyQueue, e, pointers);
            break;
        }
    }
    // free stack & ucontext
    for (int i=0; i<capacity; i++) {
        if (ThreadControlBlocks[i].id==e->id) {
            delete_stack(ThreadControlBlocks[i].stackBaseAddress);
            free(ThreadControlBlocks[i].context);
            // set status to 128
            ThreadControlBlocks[i].status=TERMINATED;
            for (int j =0; j<capacity; j++) {
                if (j!=i && ThreadControlBlocks[j].waitingThreadId==e->id) {
                    // wake up the thread that's waiting on me, if any.
                    ThreadControlBlocks[j].waitingThreadId = THREAD_NONE;
                }
            }
            return 0;
        }
    }
    return -1;
    // return 0 if successful, -1 if wut_id() == id or id == 0
}
int wut_join(int id) {
    // if thread[id]->state == DEAD or smth
        // return status.
    // if id in readyQ, then 
        // set curThread to waiting and remove from readyQ.
    // else
        // return -1
    // if threads[id] already has thread waiting on it,
        // return -1
    // else set threads[id]->waitingThread = wut_id()
    
    // wut_yield()
    // at this point threads[id] has terminated/exited.
    // store thread[id]->status locally
    // could free resources of threads[id] now, remove from ThreadArray
    // if thread with ID == id has finished/terminated
        // add the calling thread to the readyQ
        // free id memory, stack and ucontext
    
    // the waited on thread should have its TCB removed (removed TCB ID is now available)
    // return the status of the waited on thread
    // Error checking
    if (id <= 0 || id >= capacity || id == wut_id()) {
        printf("Invalid ID or trying to wait on self\n");
        // Invalid ID or trying to wait on self.
        return -1;
    }
    // Error checking for a thread already being joined
    for (int i = 0; i < capacity; i++) {
        printf("ThreadControlBlocks ID: %d\n", ThreadControlBlocks[i].id);
        printf("ThreadControlBlocks WaitigThreadID: %d\n", ThreadControlBlocks[i].waitingThreadId);
        printf("Current Thread ID: %d\n", wut_id());
        printf("Joining Thread ID: %d\n", id);
        if (ThreadControlBlocks[i].id == id  && ThreadControlBlocks[i].waitingThreadId!=wut_id() && ThreadControlBlocks[i].waitingThreadId!=THREAD_NONE){
                printf("Some thread is already waiting on the thread with this ID\n");
                // Some thread is already waiting on the thread with this ID.
                return -1;
        }
    }
    // Check if the thread to join is already terminated
    for (int i = 0; i < capacity; i++)
    {
        if (ThreadControlBlocks[i].id==id && ThreadControlBlocks[i].status == TERMINATED) {
            int retVal = ThreadControlBlocks[i].status; // Store return value before cleanup
            // Free resources for the terminated thread
            delete_stack(ThreadControlBlocks[i].stackBaseAddress);
            free(ThreadControlBlocks[i].context);
            ThreadControlBlocks[i].id = -1; // Mark TCB as available
            printf("Thread to join is already terminated: %d\n", retVal);
            return retVal;
        }
    }
    // Otherwise, block the current thread
    int currentId = wut_id();
    for (int i = 0; i < capacity; i++)
    {
        if (ThreadControlBlocks[i].id==currentId)
        {
            ThreadControlBlocks[i].status = BLOCKED; // Define BLOCKED if not defined
            ThreadControlBlocks[i].waitingThreadId = id; // Mark which thread we're waiting on
        }    
    }
    struct list_entry *queueEntry = (struct list_entry*)malloc(sizeof(struct list_entry));
    TAILQ_FOREACH(queueEntry, &readyQueue, pointers){
        if (queueEntry->id==id)
        {
            TAILQ_REMOVE(&readyQueue, queueEntry, pointers);
            TAILQ_INSERT_TAIL(&readyQueue, queueEntry, pointers);
            break;
        }
    }
    // Yield execution and effectively block the current thread.
    // This will cause the current thread to stop running.
    if(wut_yield()==-1){
        printf("Join Faled To Yield\n");
        return -1;
    }
    // When this thread is resumed, collect the return value.
    int retVal = ThreadControlBlocks[id].status;
    // Clean up the resources of the thread that has terminated.
    delete_stack(ThreadControlBlocks[id].stackBaseAddress);
    free(ThreadControlBlocks[id].context);
    ThreadControlBlocks[id].id = -1; // Mark TCB as available again
    ThreadControlBlocks[id].waitingThreadId = THREAD_NONE; // Reset the waitingThreadId
    // Now, re-add the current thread to the ready queue
    // newQueueEntry.id = currentId;
    // TAILQ_INSERT_TAIL(&readyQueue, &newQueueEntry, pointers);
    for (int i = 0; i < capacity; i++)
    {
        if (ThreadControlBlocks[i].id==currentId)
        {
            ThreadControlBlocks[i].status = READY; // Define READY if not defined
        }
        
    }
    printf("Reached end of join\n");
    return retVal; // Return the exit status of the waited-on thread.
}

int wut_yield() {
    // Get the next thread to run from the queue
    struct list_entry *queueEntry;
    TAILQ_FOREACH(queueEntry, &readyQueue, pointers){
        printf("Queue Entry with ID: %d\n", queueEntry->id);
    }
    struct list_entry *next_thread_to_run = TAILQ_LAST(&readyQueue, list_head);
    if (!next_thread_to_run) {
        printf("No available thread\n");
        return -1;
    }
    if (next_thread_to_run->id == runningThreadID) {
        printf("The next thread is the main thread\n");
        return -1;
    }
    // Find the TCB of the current running thread
    TCB *currTCB = NULL;
    for (int i = 0; i < capacity; i++) {
        if (ThreadControlBlocks[i].id == runningThreadID) {
            currTCB = &ThreadControlBlocks[i];
            currTCB->contextSet=false;
            break;
        }
    }
    // Find the TCB of the next thread to be run
    TCB *nextTCB = NULL;
    for (int i = 0; i < capacity; i++) {
        if (ThreadControlBlocks[i].id == next_thread_to_run->id) {
            nextTCB = &ThreadControlBlocks[i];
            break;
        }
    }
    if (!currTCB || !nextTCB) {
        printf("Invalid TCBs\n");
        return -1;
    }
    // Update the current running thread's status if needed
    currTCB->status = 0;  // Update the status as needed
    // Remove the next thread to be run from the queue
    // struct list_entry *currentVar;
    
    TAILQ_REMOVE(&readyQueue, next_thread_to_run, pointers);
    //printf("Queue is empty: %d\n", TAILQ_EMPTY(&readyQueue));
    // Add the current running thread to the queue
    struct list_entry *yieldingThreadEntry = (struct list_entry *)malloc(sizeof(struct list_entry));
    yieldingThreadEntry->id = runningThreadID;
    yieldingThreadEntry->pointers = next_thread_to_run->pointers;
    TAILQ_INSERT_TAIL(&readyQueue, yieldingThreadEntry, pointers);
    
    runningThreadID = nextTCB->id;
    
    // Set the running thread's status (if needed)
    nextTCB->status = 0;  // Update the status as needed
    //printf("Running Thread ID: %d\n", nextTCB->id);
    // Perform the context switch
    // prevAssignedID = currTCB->id;
    // getcontext(currTCB->context);
    // //check to see if setcontext of a thread was called
    //     // if it as called, skip this setcontext
    //     // if not, run this setcontext
    // if (!currTCB->contextSet) {
    //     // If not, then we can call setcontext and update the flag
    //     nextTCB->contextSet = true;
    //     //currTCB->contextSet=false;
    //     if (setcontext(nextTCB->context) == -1) {
    //         printf("setcontext Next TCB\n");
    //         return -1;
    //     }
    // }
    int ret= swapcontext(currTCB->context, nextTCB->context);
    TAILQ_REMOVE(&readyQueue, yieldingThreadEntry, pointers);
    struct list_entry *currentVar;
    TAILQ_FOREACH(currentVar, &readyQueue, pointers){
        printf("(WUT_YIELD END) Entry in Queue whose ID is: %d\n", currentVar->id);
    }
    return ret;
}

void wut_exit(int status) {
    int currID;
    // if Queue is empty, exit(0)
    if (TAILQ_EMPTY(&readyQueue)) 
        exit(0);
    // set the status using status &= 0xFF
    status &= 0xFF;
    printf("Current Thread ID: %d\n", wut_id());
    struct list_entry *currentVar;
    TAILQ_FOREACH(currentVar, &readyQueue, pointers){
        printf("(WUT_EXIT) Entry in Queue whose ID is: %d\n", currentVar->id);
    }
    for (int i=0; i<capacity; i++) {
        if(ThreadControlBlocks[i].id==wut_id()){
            ThreadControlBlocks[i].status = status;
            currID = i;
            break;
        }
    }
    // struct list_entry *nextThreadToRun = TAILQ_LAST(&readyQueue, list_head);
    // TAILQ_REMOVE(&readyQueue, nextThreadToRun, pointers);
    for (int i=0; i<capacity; i++) {
        if(ThreadControlBlocks[i].id==prevAssignedID){
            printf("Thread ID: %d\n", prevAssignedID);
            swapcontext(ThreadControlBlocks[currID].context, ThreadControlBlocks[i].context);
            break;
        }
    }
    printf("REached exit end\n");
    
   exit(0);
}

