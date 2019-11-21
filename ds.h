#ifndef DS_H
#define DS_H
#include <stdint.h>

typedef struct StackNode StackNode;
typedef struct Stack Stack;
typedef struct TreeNode TreeNode;
typedef TreeNode * Tree;
typedef struct FA_State FA_State;
typedef struct FA_Edge  FA_Edge;
typedef struct FA_Graph FA_Graph;
typedef struct Transition Transition;
typedef struct In_Edges In_Edges;
typedef struct List List;
typedef struct TableElement TableElement;
typedef struct HashTable HashTable;
typedef struct Queue Queue;
typedef struct QueueNode QueueNode;

struct QueueNode{
    struct QueueNode * next;
    void * content;
};

struct Queue{
    struct QueueNode * head;
    struct QueueNode * tail;
};


struct TableElement{
    void * content;
    TableElement * next;
};

struct HashTable{
    int table_size;
    TableElement * table;
};


struct List{
    void * content;
    struct List * next;
    
};

struct Stack{
    StackNode * top;
};

struct StackNode{
    StackNode * next;
    void * content;
};

struct TreeNode{
    TreeNode * sibling;
    TreeNode * firstChild;
    void * content;
};

struct Transition{
    int64_t low;
    int64_t high;
};

struct FA_State
{
    int index;
    int type;
    struct FA_State * next_state;
    struct FA_Edge * first_out;
    struct In_Edges * first_in;
};


struct FA_Edge{
    struct FA_State * state;
    struct Transition trans;
    struct FA_Edge * next_edge;
};

struct In_Edges{
    struct FA_State * state;
    struct FA_Edge * in_edge;
    struct In_Edges * next;
};

struct FA_Graph
{
    int size;
    struct FA_State * states;
    struct FA_State * start;
    struct FA_State * accept;
};
//HashTable
HashTable * create_HashTable(int size);
void add_element(HashTable * ht,  void * value);
void delete_element(HashTable * ht, void * value);
int exist_element(HashTable * ht, void * value);



//Stack

Stack * create_stack();
void destroy_stack(Stack *);
void * pop(Stack *);
void push(Stack *, void * );
int stack_empty(Stack *);
void * top(Stack *);

//
void list_append(List * head, void * content);
void * list_remove(List * head, void * content);
List * new_list();

//
Queue * create_queue();
void queue_append(Queue *, void * );
void * queue_remove(Queue*);
void * queue_top(Queue*);

//Graph
void add_edge(FA_State *, FA_State *, Transition);
void add_state(FA_Graph *, FA_State *);
int is_epsilon(Transition *);
FA_Graph * new_graph();
FA_State * new_state();
FA_Graph * alter(FA_Graph *, FA_Graph *);
FA_Graph * concat(FA_Graph *, FA_Graph *);
FA_Graph * closure(FA_Graph *);
FA_Graph * create_FA(Transition);
Transition * set_transition(Transition * t, char c);
int64_t has_transition(Transition * t, char c);
FA_State * transiton(FA_State * s, char c);

#endif