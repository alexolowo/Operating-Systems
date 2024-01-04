#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_NAME_LENGTH 256 // Adjust this as needed

void removeLeading(char *str, char *str1)
{
    int idx = 0, j, k = 0;
 
    // Iterate String until last
    // leading space character
    while (str[idx] == ' ' || str[idx] == '\t' || str[idx] == '\n')
    {
        idx++;
    }
 
    // Run a for loop from index until the original
    // string ends and copy the content of str to str1
    for (j = idx; str[j] != '\0'; j++)
    {
        str1[k] = str[j];
        k++;
    }
 
    // Insert a string terminating character
    // at the end of new string
    str1[k] = '\0';
}

void spacesBefore(char* procName, char* result){
    int spacesToAdd = 5 - strlen(procName);
    for (int i = 0; i < spacesToAdd; i++) {
        strcat(result, " ");
    }
    strcat(result, procName);
}

void listFilesInProc(){
    DIR *dir = opendir("/proc");
    if (dir == NULL){
        perror("Error opening /proc");
        closedir(dir);
        return;
    }

    struct dirent *entity;
    puts("  PID CMD");
    while ((entity = readdir(dir))) {
        if (isdigit((unsigned char)*entity->d_name)) {
            char source[MAX_NAME_LENGTH];
            snprintf(source, sizeof(source), "/proc/%s/status", entity->d_name);

            // Buffer allocation for read function
            char buff[1024] = ""; // Initialize the buffer

            int fd = open(source, O_RDONLY);
            if (fd == -1) {
                perror("Error opening file");
                continue;
            }

            ssize_t bytesRead = read(fd, buff, sizeof(buff) - 1);
            if (bytesRead == -1) {
                perror("Error reading file");
                close(fd);
                continue;
            }
            buff[bytesRead] = '\0'; // Null-terminate the buffer

            const char* searchPrefix = "Name:";
            char* line = strtok(buff, "\n");

            char output[MAX_NAME_LENGTH];
            while (line != NULL) {
                if (strncmp(line, searchPrefix, strlen(searchPrefix)) == 0) {
                    char* nameStart = line + strlen(searchPrefix);
                    removeLeading(nameStart, output);
                    //puts(output);
                }
                line = strtok(NULL, "\n");
            }

            char pid_part[MAX_NAME_LENGTH] = "";
            spacesBefore(entity->d_name, pid_part);
            strcat(pid_part," ");
            strcat(pid_part,output);
            puts(pid_part);

            if (close(fd) <0){
                perror("Error closing file");
                closedir(dir);
                exit(0);
            }
            // break;
        }
    }
    
    closedir(dir);
}

int main() {
    listFilesInProc();
    return 0;
}
