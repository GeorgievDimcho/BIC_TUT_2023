#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node {
    char *names;
    struct node * next; 
} node_t;

void print_list(node_t * head) {
    node_t * current = head;
    node_t ** abc = &current;
     printf("current addr : %d\n",*current);
    printf("----------");
    printf("current addr : %d\n",current);
    printf("----------");
    printf("current addr : %d\n",&current); // warum?
    printf("abc addr : %d\n",*abc);

    int x = 2;
    int * px = &x;
    //wert *px
    printf("%d",*px); // wert von px
    printf("%d",px); //die adresse von px
    printf("%d",&px); // die adresse von der adresse von px
    char ** menu;
    // char * = string; char ** = adresse (wo man verschiedene längen von strings speichern kann) | char * menu[]

    while (current != NULL) {
        printf("%s\n", current->names);
        printf("%d\n", current->names);
        current = current->next;
    }
}
/*
int pop(node_t ** head) {
    int retval = -1;
    node_t * next_node = NULL;

    if (*head == NULL) {
        return -1;
    }

    next_node = (*head)->next;
    retval = (*head)->names;
    free(*head);
    *head = next_node;

    return retval;
}*/

int remove_by_value(node_t ** head, int val) {
    /* TODO: fill in your code here */
}

int main() {

    node_t * test_list = malloc(sizeof(node_t*));
    test_list->names = malloc(sizeof(char)*6);
    strcpy(test_list->names, "Name1");
    test_list->next = malloc(sizeof(node_t*));
    test_list->next->names = malloc(sizeof(char)*6); // malloc(größe) , int = 4 byte, char = 1 byte, wie viele zeichen
     
    strcpy(test_list->next->names, "Name2");
    realloc(test_list->next->names,sizeof(char)*4); //erste param - was ich resizen will, zweite param - neue größe
    // malloc - neue address, realloc - selbe address
    strcpy(test_list->next->names, "Nam");
    test_list->next->next = NULL;
    //printf("print 4th element: %s",test_list->next->next->next->names);
    //remove_by_value(&test_list, 3);

    // am ende von progamm speicher leeren
    print_list(test_list); // zue
    free(test_list->next->names); 
     free(test_list->next); 
    free(test_list->names);
    free(test_list);

    // test_list als letzen leer
}