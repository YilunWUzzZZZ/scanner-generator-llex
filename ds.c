#include "ds.h"
#include <stdlib.h>
#include <stdio.h>

Transition Epsilon = {0,0};



Stack * create_stack(){
    Stack * stack = malloc(sizeof(Stack));
    stack->top = NULL;
    return stack;
}

void destroy_stack(Stack * stack){
    while(stack->top){
        pop(stack);
    }
    free(stack);
}

void * pop(Stack * stack){
    if(!(stack->top)){
        printf("can't pop an empty stack\n");
        return -1;
    }
    StackNode * top = stack->top;
    void * rval = top->content;
    stack->top = top->next;
    free(top);
    return rval;
}

void push(Stack * stack, void * elem){
    StackNode * new_node = malloc(sizeof(StackNode));
    new_node->next = stack->top;
    new_node->content = elem;
    stack->top = new_node;
}

void * top(Stack * stack){
    return stack->top->content;
}

int stack_empty(Stack * stack){
    
    return (stack->top == NULL);
}

FA_State  * new_state(){
    FA_State * s = malloc(sizeof(FA_State));
    s->first_in = NULL;
    s->first_out = NULL;
    s->next_state = NULL;
    s->index = -1;
    s->type = -1;
    return s;
}
int is_epsilon(Transition * t){
    return t->low == 0 && t->high == 0;
}

List * new_list(){
    List * l = (List *)malloc(sizeof(List));
    l->next = NULL;
    return l;
}

void list_append(List * head, void * content){
    List * new_elem = (List *)malloc(sizeof(List));
    new_elem->next = head->next;
    new_elem->content = content;
    head->next = new_elem;
}

void * list_remove(List * head, void * content){
    List * walker = head->next;
    List * pre = head;
    while(walker){
        if(walker->content == content){
            pre->next = walker->next;
            free(walker);
            return content;
        }
        pre = walker;
        walker = walker->next;
    }
    printf("target not in the list\n");
    return NULL;
}

Queue * create_queue(){
    QueueNode * dummy_node = malloc(sizeof(QueueNode));
    dummy_node->next = NULL;
    Queue * q = malloc(sizeof(Queue));
    q->head = dummy_node;
    q->tail = dummy_node;
    return q;
}

void queue_append(Queue * q, void * content){
    QueueNode * qnode = malloc(sizeof(qnode));
    qnode->next = NULL;
    qnode->content = content;
    q->tail->next = qnode;
    q->tail = qnode;
}

void * queue_remove(Queue * q){
    if(!(q->head->next)){
        return NULL;
    }
    QueueNode * removed = q->head->next;
    q->head->next = removed->next;
    void * rv = removed->content;
    free(removed);
    //move tail if empty
    if(!(q->head->next)){
        q->tail = q->head;
    }
    return rv;
}

void * queue_top(Queue * q){
    if(q->head->next){
        return q->head->next->content;
    }
    else
    {
        return NULL;
    }
    
}



void add_edge(FA_State * s1, FA_State * s2, Transition t){
    FA_Edge * edge = malloc(sizeof(FA_Edge));
    In_Edges * s2_in_edge = malloc(sizeof(In_Edges));
    edge->state = s2;
    (edge->trans).low = t.low;
    (edge->trans).high = t.high;
    edge->next_edge = s1->first_out;
    s1->first_out = edge;
    s2_in_edge->state = s1;
    s2_in_edge->in_edge = edge;
    s2_in_edge->next = s2->first_in;
    s2->first_in = s2_in_edge;
}

void add_state(FA_Graph * G, FA_State * s){
    G->size += 1;
    s->next_state = G->states;
    G->states = s;
}

FA_Graph * alter(FA_Graph * G1, FA_Graph * G2){
    FA_State * new_start = new_state();
    FA_State * new_accept = new_state();
    add_state(G1, new_start);
    add_state(G1, new_accept);
    add_edge(new_start, G1->start, Epsilon);
    add_edge(new_start, G2->start, Epsilon);
    add_edge(G1->accept, new_accept, Epsilon);
    add_edge(G2->accept, new_accept, Epsilon);
    //find the end of G1's state list
    FA_State * last = G1->states;
    while(last->next_state){
        last = last->next_state;
    }
    last->next_state = G2->states;
    G1->size += G2->size;
    G1->start = new_start;
    G1->accept = new_accept;
    free(G2);
    return G1;
}

FA_Graph * concat(FA_Graph * G1, FA_Graph * G2){
    add_edge(G1->accept, G2->start, Epsilon);
    //find the end of G1's state list
    FA_State * last = G1->states;
    while(last->next_state){
        last = last->next_state;
    }
    last->next_state = G2->states;
    G1->size += G2->size;
    G1->accept = G2->accept;
    free(G2);
    return G1;
}
FA_Graph * closure(FA_Graph * G){
    FA_State * new_start = new_state();
    FA_State * new_accept = new_state();
    add_state(G, new_start);
    add_state(G, new_accept);
    add_edge(new_start, G->start, Epsilon);
    add_edge(new_start, new_accept, Epsilon);
    add_edge(G->accept, new_accept, Epsilon);
    add_edge(G->accept, G->start, Epsilon);
    G->start = new_start;
    G->accept = new_accept;
    return G;
}

FA_Graph * new_graph(){
    FA_Graph * G = malloc(sizeof(FA_Graph));
    G->size = 0;
    G->states = NULL;
    G->accept = NULL;
    G->start = NULL;
    return G;
}

FA_Graph * create_FA(Transition t){
    FA_Graph * G = new_graph();
    FA_State * start = new_state();
    FA_State * accept = new_state();
    add_state(G, start);
    add_state(G, accept);
    add_edge(start, accept, t);
    G->start = start;
    G->accept = accept;
    return G;
}

Transition * set_transition(Transition * t, char c){
    if(!t){
        printf("NULL trans\n");
    }
    if(c < 64){
        t->low |= 1LL << (int)c;
    }
    else{
        t->high |= 1LL << (c - 64);
    }
    return t;
}

int64_t has_transition(Transition * t, char c){
    if(c < 64){
        return (t->low & 1LL << (int)c);
    }
    else{
        return (t->high & 1LL << (c - 64) );
    }
}

FA_State * transiton(FA_State * s, char c){
    FA_Edge * e = s->first_out;
    while(e){
        if(has_transition(&e->trans, c )){
            return e->state;
        }
        e = e->next_edge;
    }
    return NULL;
}