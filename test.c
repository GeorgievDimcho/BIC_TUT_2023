#include <stdio.h>

// function declaration
void add10(int *);

int main(void) {
    
  // integer variable
  int num = 30;
  
  // print value of num
  printf("Value of num before function call: %d\n", num);
  
  // pass by reference
  add10(num);
  printf("%d\n",&num);
  // print value of num
  printf("Value of num after function call: %d\n", num);
  
  return 0;
}

// function definition
void add10(int *n) {
    printf("no *: %d",n);
  *n = *n + 10;
  printf("Inside add10(): Value %d\n", *n);
}