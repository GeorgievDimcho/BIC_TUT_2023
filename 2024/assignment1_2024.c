#if defined(LIBC_SCCS) && !defined(lint)                    // Check if LIBC_SCCS is defined and lint is not defined
static char sccsid[] = "@(#)popen.c	8.3 (Berkeley) 5/3/95"; // Static character array for version information
#endif                                                      /* LIBC_SCCS and not lint */

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
static pthread_mutex_t pidlist_mutex = PTHREAD_MUTEX_INITIALIZER;   // Mutex for process ID list

// Function to execute a command with a specified type of I/O operation
FILE *popen(command, type)
    const char *command,
    *type; // Command and type of I/O operation
{
    printf("popen is called! \n");     // Print a message indicating popen is called
    struct pid *cur;                   // Pointer to current process ID structure
    FILE *iop;                         // File pointer
    int pdes[2], pid, twoway, cloexec; // Pipe descriptors, process ID, and flags
    char *argv[4];                     // Array of command arguments
    struct pid *p;                     // Pointer to process ID structure

    cloexec = strchr(type, 'e') != NULL; // Check if 'e' is present in type string

    // Creating a pipe for inter-process communication
    pipe(pdes);

    if ((cur = malloc(sizeof(struct pid))) == NULL)
    {                         // Allocate memory for a process ID structure
        (void)close(pdes[0]); // Close pipe descriptors
        (void)close(pdes[1]);
        return (NULL); // Return NULL if memory allocation fails
    }

    argv[0] = "sh";            // Shell command
    argv[1] = "-c";            // Argument indicating a command to be executed by the shell
    argv[2] = (char *)command; // Command to be executed
    argv[3] = NULL;            // Null-terminated array of arguments

    // Creating a child process to execute the command
    switch (pid = fork())
    {
    case -1:                  /* Error. */
        (void)close(pdes[0]); // Close pipe descriptors
        (void)close(pdes[1]);
        free(cur);     // Free allocated memory
        return (NULL); // Return NULL if fork fails
    case 0:            /* Child. */
        if (*type == 'r')
        {
            // Redirecting standard input/output for reading
            if (!cloexec)
                (void)close(pdes[0]); // Close pipe descriptor if cloexec flag is not set

            if (pdes[1] != STDOUT_FILENO)
            {
                (void)dup2(pdes[1], STDOUT_FILENO); // Duplicating pipe write descriptor to stdout
                if (!cloexec)
                    (void)close(pdes[1]); // Close pipe write descriptor if cloexec flag is not set
            }
            if (cloexec)
                (void)fcntl(pdes[1], F_SETFD, 0); // Set close-on-exec flag for pipe write descriptor
        }
        else
        {
            // Redirecting standard input/output for writing
            if (pdes[0] != STDIN_FILENO)
            {
                (void)dup2(pdes[0], STDIN_FILENO); // Duplicating pipe read descriptor to stdin
                if (!cloexec)
                    (void)close(pdes[0]); // Close pipe read descriptor if cloexec flag is not set
            }
            else if (cloexec)
                (void)fcntl(pdes[0], F_SETFD, 0); // Set close-on-exec flag for pipe read descriptor
            if (!cloexec)
                (void)close(pdes[1]); // Close pipe write descriptor if cloexec flag is not set
        }
        // Closing file descriptors of other processes
        SLIST_FOREACH(p, &pidlist, next)
        (void)close(fileno(p->fp)); // Close file descriptor of each process in the list
        // Executing the command
        execve(_PATH_BSHELL, argv, environ); // Execute the shell command
        exit(127);                           // Exit with error if execve fails
    }

    // Parent process; setting up file descriptors
    if (*type == 'r')
    {
        iop = fdopen(pdes[0], type); // Open file descriptor for reading
        (void)close(pdes[1]);        // Close write end of the pipe
    }
    else
    {
        iop = fdopen(pdes[1], type); // Open file descriptor for writing
        (void)close(pdes[0]);        // Close read end of the pipe
    }

    // Linking into the list of file descriptors
    cur->fp = iop;                          // Set file pointer in process ID structure
    cur->pid = pid;                         // Set process ID in process ID structure
    SLIST_INSERT_HEAD(&pidlist, cur, next); // Insert process ID structure into the list

    return (iop); // Return file pointer
}

/*
 * pclose --
 *  Pclose returns -1 if stream is not associated with a `popened' command,
 *  if already `pclosed', or waitpid returns an error.
 */
int pclose(iop)
FILE *iop; // File pointer to close
{
    struct pid *cur, *last = NULL; // Pointers to process ID structures
    int pstat;                     // Process status
    pid_t pid;                     // Process ID

    // Finding the appropriate file pointer and removing it from the list
    SLIST_FOREACH(cur, &pidlist, next)
    {
        if (cur->fp == iop)
            break;
        last = cur;
    }
    if (cur == NULL)
    {
        return (-1); // Return -1 if file pointer is not found
    }
    if (last == NULL)
    {
        SLIST_REMOVE_HEAD(&pidlist, next); // Remove head of the list if it's the only element
    }
    (void)fclose(iop); // Close
    // Closing the file pointer

    // Waiting for the child process to terminate
    do {
        pid = wait4(cur->pid, &pstat, 0, (struct rusage *)0); // Wait for child process to terminate
    } while (pid == -1 && errno == 4); // Repeat if interrupted by a signal

    free(cur); // Free memory allocated for process ID structure

    return (pid == -1 ? -1 : pstat); // Return -1 if waitpid fails, otherwise return the process status
}

int main()
{
    //exec(ls) -> 400 bis 500
    // 550 bis 650
    // Creating processes for both `ls` and `wc`
    // `popen` returns a FILE* that represents the process' stdout (`r`) or stdin (`w`)
    FILE *ls = popen("ls", "r"); // Execute "ls" command with read access
    printf("File pointer: %d\n", *ls);
    FILE *wc = popen("wc", "w"); // Execute "wc" command with write access

    // Consuming the output of `ls` and feeding it to `wc`
    char buf[1024];
    while (fgets(buf, sizeof(buf), ls) != NULL) // Read each line from ls output
        printf("buf:%s", buf);
        fputs(buf, wc); // Write each line to wc

    // Once we're done, we close the streams
    pclose(ls); // Close ls process
    pclose(wc); // Close wc process

    return 0; // Return success
}

