#include "assignment1_2024_popen.h"
/*
 * pclose --
 *  Pclose returns -1 if stream is not associated with a `popened' command,
 *  if already `pclosed', or waitpid returns an error.
 */
int pclose(FILE *iop)
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