/*
RE -> TermAterm     

Aterm ->  | RE     |
          NIL      $, )

Term -> FactorAfactor

Afactor -> Term     (, [, \, s 
           NIL      |, ), $

Factor -> ElemAelem  

Aelem -> *             
         NIL        (, [, \, s, |,  ), $

Elem ->  (RE)       (
		 [Set]      [
         Symbol     \, s

Set -> ^RangeRangelist       ^
       | RangeRangelist      s, \

Rangelist -> RangeRangelist  s, \  
             | NIL           ]

Range -> symbolAsymbol      s
         | \any             \

Asymbol -> -symbol          -
          | NIL         s, \, ]


Symbol -> \any     \
          symbol   s

128: RE 129:Aterm 130:Term 131:Afactor 132:Factor  133:Aelem 134: Elem 135:Symbol
136: Set 137: Rangelist 138:Range 139: Asymbol 
*/

#include <stdio.h>
#include <stdlib.h>
#include "re.h"
#include "ds.h"

//Tree (*NTs[])(char inputs[]) = {RE, Aterm, Term, Afactor, Factor, Aelem, Elem, Symbol};

char * name_list[]= {"RE", "Aterm", "Term", "Afactor", "Factor", "Aelem", "Elem", "Symbol",
                     "Set", "RL", "Range", "Asymbol"};

void fail(){
    printf("error in parsing the re, invaild syntax\n");
    exit(0);
}

void check_stack(Stack * stack){
    if(stack_empty(stack)){
        printf("has reached the end of the input\n");
        fail();
    }
}

int is_symbol(char s){
    if(s != '(' && s != '\\' && s!= '^' && s != '|' && s!= '\0' && s!= ')' && s != '*'
       && s != '[' && s != ']' && s != '-')
    {
        return 1;
    }
    else
        return 0;
}

Tree build_node(char c){
    Tree tree = malloc(sizeof(TreeNode));
    tree->firstChild = NULL;
    tree->content = c;
    return tree;
}

Tree RE(Stack * stack){
    Tree tree = malloc(sizeof(TreeNode));
    tree->content = _RE;
    tree->firstChild = Term(stack);
    tree->firstChild->sibling =Aterm(stack);
    tree->firstChild->sibling->sibling = NULL;
    return tree;
}
/*
Aterm ->  | RE     |
          NIL      $, )
 */

Tree Aterm(Stack * stack){
    Tree tree = malloc(sizeof(TreeNode));
    tree->content = _Aterm;
    check_stack(stack);
    char next_char = (char)top(stack);
    if(next_char == '|'){
        pop(stack);
        tree->firstChild = build_node('|');
        tree->firstChild->sibling = RE(stack);
        tree->firstChild->sibling->sibling = NULL;
    }
    else if(next_char == '\0' || next_char == ')'){
        tree->firstChild = NULL;
    }
    else{
        fail();
    }
    return tree;
}

Tree Term(Stack * stack){
    Tree tree = malloc(sizeof(TreeNode));
    tree->content = _Term;
    tree->firstChild = Factor(stack);
    tree->firstChild->sibling =Afactor(stack);
    tree->firstChild->sibling->sibling = NULL;
    return tree;
}

/*
Afactor -> Term     (, [, \, s 
           NIL      |, ), $
 */

Tree Afactor(Stack * stack){
    Tree tree = malloc(sizeof(TreeNode));
    tree->content = _Afactor;
    check_stack(stack);
    char next_char = (char)top(stack);
    if(next_char == '(' || next_char == '[' || next_char == '\\' || is_symbol(next_char)){
        tree->firstChild = Term(stack);
        tree->firstChild->sibling = NULL;
    }
    else if(next_char == '|' || next_char == '\0' || next_char == ')'){
        tree->firstChild = NULL;
    }
    else{
        fail();
    }
    return tree;
}

//Factor -> ElemAelem  
Tree Factor(Stack * stack){
    Tree tree = malloc(sizeof(TreeNode));
    tree->content = _Factor;
    tree->firstChild = Elem(stack);
    tree->firstChild->sibling =Aelem(stack);
    tree->firstChild->sibling->sibling = NULL;
    return tree;
}

/*
Aelem -> *             
         NIL        (, [, \, s, |,  ), $
 */

Tree Aelem(Stack * stack){
    Tree tree = malloc(sizeof(TreeNode));
    tree->content = (void*)_Aelem;
    check_stack(stack);
    char next_char = (char)top(stack);
    if(next_char == '*'){
        pop(stack);
        tree->firstChild = build_node('*');
        tree->firstChild->sibling = NULL;
    }
    else if(next_char == '(' || next_char == '\\' || next_char == '[' 
           || next_char == '|' || next_char == '\0' || next_char == ')' || is_symbol(next_char)){
        tree->firstChild = NULL;
    }
    else{
        fail();
    }
    return tree;
}
/*
Elem ->  (RE)       (
		 [Set]      [
         Symbol     \, s
 */
Tree Elem(Stack * stack){
    Tree tree = malloc(sizeof(TreeNode));
    tree->content = (void*)_Elem;
    check_stack(stack);
    char next_char = (char)top(stack);
    if(next_char == '(' ){
        tree->firstChild = build_node('(');
        pop(stack);
        tree->firstChild->sibling = RE(stack);
        check_stack(stack);
        next_char = (char)pop(stack);
        if(next_char == ')'){
            tree->firstChild->sibling->sibling = build_node(')');
            tree->firstChild->sibling->sibling->sibling = NULL;
        }
        else{
            fail();
        }
    }
    else if(next_char == '['){
        tree->firstChild = build_node('[');
        pop(stack);
        tree->firstChild->sibling = Set(stack);
        check_stack(stack);
        next_char = pop(stack);
        if(next_char == ']'){
            tree->firstChild->sibling->sibling = build_node(']');
            tree->firstChild->sibling->sibling->sibling = NULL;
        }
        else{
            fail();
        }
    }
    else if(next_char == '\\' || is_symbol(next_char) ){
        tree->firstChild = Symbol(stack);
        tree->firstChild->sibling = NULL;
    }
    else{
        fail();
    }
    return tree;
}
/*
Set -> ^RangeRangelist       ^
       | RangeRangelist      s, \
 */

Tree Set(Stack * stack){
    Tree tree = malloc(sizeof(TreeNode));
    tree->content = (void *)_Set;
    check_stack(stack);
    char next_char = (char)top(stack);
    if(next_char == '^'){
        tree->firstChild = build_node('^');
        pop(stack);
        check_stack(stack);
        next_char = (char)top(stack);
        tree->firstChild->sibling = Range(stack);
        tree->firstChild->sibling->sibling = Rangelist(stack);
        tree->firstChild->sibling->sibling->sibling =NULL;
    }
    else if(is_symbol(next_char)|| next_char == '\\'){
        tree->firstChild = Range(stack);
        tree->firstChild->sibling = Rangelist(stack);
        tree->firstChild->sibling->sibling = NULL;
    }
    else{
        fail();
    }
    return tree;
}
/*
Rangelist -> RangeRangelist  s, \  
             | NIL           ]
*/
Tree Rangelist(Stack * stack){
    Tree tree = malloc(sizeof(TreeNode));
    tree->content = (void *)_RL;
    check_stack(stack);
    char next_char = (char)top(stack);
    if(next_char == '\\' || is_symbol(next_char)){
        tree->firstChild = Range(stack);
        tree->firstChild->sibling = Rangelist(stack);
        tree->firstChild->sibling->sibling =NULL;
    }
    else if(next_char == ']'){
        tree->firstChild = NULL;
    }
    else{
        fail();
    }
    return tree;
}

/*
Range -> symbolAsymbol      s
         | \any             \
 */

Tree Range(Stack * stack){
    Tree tree = malloc(sizeof(TreeNode));
    tree->content = (void *)_Range;
    check_stack(stack);
    char next_char = (char)pop(stack);
    if(is_symbol(next_char)){
        tree->firstChild = build_node(next_char);
        tree->firstChild->sibling = Asymbol(stack);
        tree->firstChild->sibling->sibling =NULL;
    }
    else if(next_char == '\\'){
        tree->firstChild = build_node('\\');
        check_stack(stack);
        next_char = (char)pop(stack);
        tree->firstChild->sibling = build_node(next_char);
        tree->firstChild->sibling->sibling = NULL;
    }
    else{
        fail();
    }
    return tree;
}

/*
Asymbol -> -symbol          -
          | NIL         s, \, ]
 */
Tree Asymbol(Stack * stack){
    Tree tree = malloc(sizeof(TreeNode));
    tree->content = (void *)_Asymbol;
    check_stack(stack);
    char next_char = (char)top(stack);
    if(next_char == '-'){
        pop(stack);
        tree->firstChild = build_node(next_char);
        check_stack(stack);
        next_char = (char) pop(stack);
        if(is_symbol(next_char)){
        tree->firstChild->sibling = build_node(next_char);
        tree->firstChild->sibling->sibling =NULL;
        }
        else{
            fail();
        }
    }
    else if(next_char == '\\' || next_char == ']' || is_symbol(next_char)){
        tree->firstChild = NULL;
    }
    else{
        fail();
    }
    return tree;
}


/*
Symbol -> \any    \
          symbol     s
*/

Tree Symbol(Stack * stack){
    Tree tree = malloc(sizeof(TreeNode));
    tree->content = _Symbol;
    check_stack(stack);
    char next_char = (char)pop(stack);
    if(next_char == '\\'){
        tree->firstChild = build_node(next_char);
        check_stack(stack);
        next_char = (char)pop(stack);
        tree->firstChild->sibling = build_node(next_char);
        tree->firstChild->sibling->sibling = NULL;
    }
    else if(is_symbol(next_char)){
        tree->firstChild = build_node(next_char);
        tree->firstChild->sibling = NULL;
    }
    else{
        fail();
    }
    return tree;
}

void visualize_AST(Tree tree, int indent){
    for(int i=0; i< indent; i++){
        printf(" ");
    }
    int index = (int)tree->content;
    if(index >= 128  )
        printf("%s\n", name_list[index - 128]);
    else
        printf("%c\n", (char)index);
    Tree child = tree->firstChild;
    indent += 4;
    while(child){
        visualize_AST(child, indent);
        child = child->sibling;
    }
}

Tree build_AST(char re[]){
    Stack * t_stack = create_stack();
    Stack * stack = create_stack();
    int pos = 0;
    while(re[pos]){
        push(t_stack, re[pos++]);
    }
    push(t_stack, re[pos]);
    while(!stack_empty(t_stack)){
        char poped = (char)pop(t_stack);
        //printf("%c", poped);
        push(stack, poped);
    }
    destroy_stack(t_stack);
    Tree AST = RE(stack);
    if(!stack_empty(stack) && (char)pop(stack) == '\0'){
        return AST;
    }
    else{
        fail();
        return NULL;
    }
}


//128: RE 129:Aterm 130:Term 131:Afactor 132:Factor  133:Aelem 134: Elem 135:Symbol
Tree reduce_AST(Tree tree){
    int type = (int)tree->content;
    if(type < 128){
        return tree;
    }
    else if(!(tree->firstChild)){
        free(tree);
        return NULL;
    }
   
    else if(type == _Aterm || type == _Asymbol){
        Tree first_child = tree->firstChild;
        free(tree);
        first_child->sibling = reduce_AST(first_child->sibling);
        tree = first_child;
    }

    else if(type == _Elem){
        Tree temp = tree->firstChild;
        if( (int)(temp->content) == '(' ||(int)(temp->content) == '[' ){
            tree->firstChild = temp->sibling;
            free(temp);
            free(tree->firstChild->sibling);
            tree->firstChild = reduce_AST(tree->firstChild);
            tree->firstChild->sibling = NULL;
        }
        else{
            tree->firstChild = reduce_AST(temp);
        }
        
    }

    else if(type == _RL){
        Tree first_child = reduce_AST(tree->firstChild);
        first_child->sibling = reduce_AST(tree->firstChild->sibling);
        free(tree);
        tree = first_child;
    }

    else{
        Tree other_children = tree->firstChild->sibling;
        tree->firstChild = reduce_AST(tree->firstChild);
        Tree pre = tree->firstChild;
        while(other_children){
            Tree next = other_children->sibling;
            pre->sibling = reduce_AST(other_children);
            pre = pre->sibling;
            other_children = next;
        }
    }

    if( tree->firstChild && tree->firstChild->sibling == NULL){
        Tree temp = tree;
        tree = tree->firstChild;
        free(temp);
    }
    return tree;
}

Tree re_to_AST(char re[]){
    return reduce_AST(build_AST(re));
}

// int main(int argc, char * argv[]){
//     FILE * fp = fopen("test_re","r");
//     char buf[30];
//     fscanf(fp, "%s", buf);
//     Tree AST = build_AST(buf);
//     printf("1\n");
//     AST = reduce_AST(AST);
//     printf("2\n");
//     visualize_AST(AST, 0);
//     fclose(fp);
//     return 0;
// }