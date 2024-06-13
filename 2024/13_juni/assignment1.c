#include "assignment1_2024_popen.h"

int main()
{
    //exec(ls) -> 400 bis 500
    // 550 bis 650
    // Creating processes for both `ls` and `wc`
    // `popen` returns a FILE* that represents the process' stdout (`r`) or stdin (`w`)
    FILE *ls = popen("ls", "r"); // Execute "ls" command with read access
    //printf("File pointer: %d\n", *ls);
    FILE *wc = popen("wc", "w"); // Execute "wc" command with write access

    // Consuming the output of `ls` and feeding it to `wc`
    char buf[1024];
    while (fgets(buf, sizeof(buf), ls) != NULL) // Read each line from ls output
        //printf("buf:%s", buf);
        fputs(buf, wc); // Write each line to wc

    // Once we're done, we close the streams
    pclose(ls); // Close ls process
    pclose(wc); // Close wc process

    return 0; // Return success
}