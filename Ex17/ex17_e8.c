#include <stdio.h>

#define MAX_SIZE 100

struct Stack{
  int top;
  int elements[MAX_SIZE];
};

struct Stack stack;

void 
push(int n)
{
  stack.elements[stack.top] = n;
  stack.top++;
}

int 
pop()
{
 stack.top--;
 int n = stack.elements[stack.top];
 return n;
}

int
main(int argc, char **argv){
  stack.top = 0;
  push(1);
  push(2);
  push(5);

  int a = pop();
  int b = pop();

  printf("a = %d\nb = %d\n", a, b);

  return 0;
}

