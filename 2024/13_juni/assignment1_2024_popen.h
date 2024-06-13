#ifndef assignment1_2024_popen
#define assignment1_2024_popen
#include <sys/cdefs.h> // System-specific definitions
#include <sys/param.h> // System parameters
#include <sys/queue.h> // Linked list macros
#include <sys/wait.h>  // Wait functions

#include <signal.h>  // Signal handling
#include <errno.h>   // Error handling
#include <fcntl.h>   // File control
#include <unistd.h>  // Standard symbolic constants and types
#include <stdio.h>   // Standard I/O functions
#include <stdlib.h>  // Standard library definitions
#include <string.h>  // String manipulation functions
#include <paths.h>   // Path definitions
#include <pthread.h> // POSIX threads
FILE *popen(const char *command, const char *type);
int pclose(FILE *iop);
extern char **environ; // External environment variables
// Structure used for determining the size of memory space
struct pid
{
    SLIST_ENTRY(pid)
    next;      // Singly-linked list macros
    FILE *fp;  // File pointer
    pid_t pid; // Process ID (signed integer)
};

static SLIST_HEAD(, pid) pidlist = SLIST_HEAD_INITIALIZER(pidlist); // Static list head for process IDs
static pthread_mutex_t pidlist_mutex = PTHREAD_MUTEX_INITIALIZER; 
#endif