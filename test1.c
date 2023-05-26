#include <stdio.h>

int main(void) {

    typedef struct node {
        char *names;
        struct node * next; 
    } node_t;

    struct node_t{
        char *names;
        struct node * next;
    };
    /*struct * node1 
    {
        
    };*/
    
    

    const char *c = "hello";
    const char **cp = &c;
    const char ***cpp = &cp;
  
     return 0;
}
