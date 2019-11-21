#ifndef FAST_STACK_H
#define FAST_STACK_H

#include <stdlib.h>

#define pop(s) ((s->top == -1)?0:s->stack[s->top--])
#define push(s, elem)\
            if(s->top >= s->maxsize - 1){\
                printf("fatal error, stack overflows\n");\
                exit(0);\
            }\
            s->stack[++s->top] = elem

#define empty(s) (s->top == -1)

typedef struct Stack
{
    int * stack;
    int maxsize;
    int top;
}Stack;

Stack * stack(int size){
    Stack * s = malloc(sizeof(Stack));
    s->top = -1;
    s->maxsize = size;
    s->stack = malloc(sizeof(int)* size);
    return s;
}


#endif