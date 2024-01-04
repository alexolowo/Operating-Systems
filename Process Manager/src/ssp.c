#include <asm-generic/errno-base.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// initialize a global ssp_id and global longest string length
int ssp_id_count = -1;
int longest=3;

// create a struct to hold information
struct processInfo {
    char * name;
    pid_t processInfoPid;
    int status;
};

void printSpaces(unsigned long longestString, unsigned long nameLen){
    int length = (longestString - nameLen) & 0xFFFFFFFF;
    for(int i=0; i<length; i++){
        printf(" ");
    }
}
// checking function
int check(int num, char* message){
    if (num<0){
        perror(message);
        return -1;
    } else {
        return 1;
    }
}
char * string;
// declaring struct array to hold running process information
struct processInfo procArray[4096];


// using a singal handler function for the subreaper
void sigchld_handler(int signum) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // Check if the terminated process is in procArray
        int ssp_id = -1;
        for (int i = 0; i <= ssp_id_count; i++) {
            if (procArray[i].processInfoPid == pid) {
                ssp_id = i;
                break;
            }
        }

        if (ssp_id >= 0) {
            // Process found in procArray, update its status
            if (WIFEXITED(status)) {
                procArray[ssp_id].status = WEXITSTATUS(status);
                //printf("WEXITSTATUS %d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                // int exit_signal = WTERMSIG(status);
                // if (exit_signal == SIGPIPE) {
                //     procArray[ssp_id].status = 128 + exit_signal;
                //     //printf("WTERMSIG %d\n", 128 + exit_signal);

                // } else {
                //     procArray[ssp_id].status = exit_signal;
                //     //printf("ELSE %d\n", exit_signal);

                // }
                procArray[ssp_id].status = 128 + WTERMSIG(status);
            }
        } else {
            // Orphan process, record it as "<unknown>"
            ssp_id_count++;
            procArray[ssp_id_count].name = "<unknown>";
            procArray[ssp_id_count].processInfoPid = pid;

            if (WIFEXITED(status)) {
                procArray[ssp_id_count].status = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                /*
                The commented sections are unnecessary. Code convolution
                */
                // int exit_signal = WTERMSIG(status);
                // if (exit_signal == SIGPIPE) {
                //     procArray[ssp_id_count].status = 128 + exit_signal;
                // } else {
                //     procArray[ssp_id_count].status = exit_signal;
                // }
                procArray[ssp_id].status = 128 + WTERMSIG(status);

            }
        }
    }
}

void ssp_init() {
    prctl(PR_SET_CHILD_SUBREAPER, 1);

    // Register the SIGCHLD signal handler
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);
    // Initialize the array of structs.
    // procArray = (struct processInfo *)malloc(sizeof(struct processInfo));
    // if (procArray == NULL){
    //     perror("memory Allocation");
    //     exit(errno);
    // }

}

int ssp_create(char *const *argv, int fd0, int fd1, int fd2) {
    // string = argv[0];
    pid_t pid = fork();
    if (!check(pid, "Forking")){
        exit(errno);
    }
    ssp_id_count++;
    
    // the parent process
    if (pid > 0){
    
        procArray[ssp_id_count].name = strdup(argv[0]);
        procArray[ssp_id_count].processInfoPid = pid;
        procArray[ssp_id_count].status = -1;
        //printf("PID in Create %d,,,%d\n", procArray[ssp_id_count].processInfoPid,ssp_id_count);
        return ssp_id_count;
        
    } else { // in the child process
        if (!(
            check (dup2(fd0, 0), "DUP0") &&
            check (dup2(fd1, 1), "DUP1") &&
            check (dup2(fd2, 2), "DUP2")
            )){
                exit(errno);
        }

        // close all file descriptors except 0, 1 and 2
        struct dirent *cd;
        DIR *dir = fdopendir(open("/proc/self/fd", O_DIRECTORY));
        if (dir==NULL){
            perror("failed to open the current process");
            closedir(dir);
            exit(errno);
        }

        // checking the other open file descriptors and closing everything > 2
        while ((cd = readdir(dir))!= NULL){
            if (cd->d_type == DT_LNK) {
                int fd = atoi(cd->d_name);
                if (fd>2){
                    close(fd);
                }
            }

        }
        closedir(dir);
        if (!check(execvp(argv[0], argv), "ExecVP")){
            exit(errno);
        }
       
    }
    
    return -1;
}

int ssp_get_status(int ssp_id) {
    
    int status;
    pid_t pid = waitpid(procArray[ssp_id].processInfoPid, &status, WNOHANG);
    //printf("Inside ssp_get_status - PID: %d, PID from procArray: %d, Status: %d\n", pid, procArray[ssp_id].processInfoPid, status);

    if (pid > 0) {
        if (WIFEXITED(status)) {
            //printf("WEXITED %d:\n" , WEXITSTATUS(status));
            procArray[ssp_id].status = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            int exit_signal = WTERMSIG(status);
            procArray[ssp_id].status = 128 + exit_signal;
            //printf("WTERMSIG %d:\n" , WTERMSIG(status));

        }
        return procArray[ssp_id].status;
    }
    //printf("REACHED END: %d\n", status);
    return procArray[ssp_id].status;
    

    // if (pid == -1) {
    //     if (errno == ECHILD) {
    //         if (procArray[ssp_id].status == -1) {
    //             return -1;
    //         }
    //         return procArray[ssp_id].status;
    //     } else {
    //         perror("SSP Get Status");
    //         exit(errno);
    //     }
    // }    
}

void ssp_send_signal(int ssp_id, int signum) {
    if (kill(procArray[ssp_id].processInfoPid, signum)==0){
        //check(kill(procArray[ssp_id].processInfoPid, signum), "Send Signal");
        procArray[ssp_id].status = 128+signum;
    } else {
        perror("SEND SIGNAL");
    }
}

void ssp_wait() {
    int returned;
    pid_t pid;
    //printf("Inside Wait\n");
    for (int i = 0; i<ssp_id_count+1; i++){
        //("PID in Wait %d\n",procArray[i].processInfoPid);
        if (procArray[i].processInfoPid > 0) {
            pid = waitpid(procArray[i].processInfoPid, &returned, 0);
        }
        //printf("STATUS: %d\n", returned);
       // printf("WAITPID: %d\n", pid);
        
        // if(pid == -1){
        //     if(errno == ECHILD){ // the process that called waitpid() has no child
        //         //printf("ECHILD\n");
        //         procArray[i].status = returned;
        //         continue;
        //     }
        //     perror("SSP Wait");
        //     exit(errno);
        //     return;
        if(pid > 0){
            // re-assign the status based of the process
            procArray[i].status = WEXITSTATUS(returned);
        
            if (WIFSIGNALED(returned)) {       // check if the process was terminated because of a signal
                // add the signal that terminated the process + 128
                procArray[i].status = 128+WTERMSIG(returned);
            }
            // if (procArray[i].name == NULL) {
            //         printf("entered name is NULL \n");
            //         procArray[i].name = "<unknown>";
            // }
        }
    }
}

void ssp_print() {
    for(int i =0; i<ssp_id_count+1; i++){
        if ((int)strlen(procArray[i].name)>longest){
            longest = (int)strlen(procArray[i].name);
        }
    }   
    
    printf("%7s %-*s %s\n","PID",longest,"CMD","STATUS");
   for(int i=0;i<=ssp_id_count;i++){
        printf("%7d %-*s %d\n",procArray[i].processInfoPid,longest,procArray[i].name, procArray[i].status);
    }
    //printf("ssp_id: %d\n", ssp_id_count);
}
