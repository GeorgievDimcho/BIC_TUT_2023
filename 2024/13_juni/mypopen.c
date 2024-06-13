
#include "assignment1_2024_popen.h"
  // Mutex for process ID list
FILE *popen(const char *command, const char *type)
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