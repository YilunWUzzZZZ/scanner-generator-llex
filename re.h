#ifndef RE_H
#define RE_H
#define _RE 128
#define _Aterm 129
#define _Term 130
#define _Afactor 131
#define _Factor 132
#define _Aelem 133
#define _Elem  134
#define _Symbol 135
#define _Set 136
#define _RL 137
#define _Range 138
#define _Asymbol 139
#include "ds.h"

Tree RE(Stack *);
Tree Aterm(Stack *);
Tree Term(Stack *);
Tree Afactor(Stack *);
Tree Factor(Stack *);
Tree Aelem(Stack *);
Tree Elem(Stack *);
Tree Symbol(Stack *);
Tree Set(Stack *);
Tree Rangelist(Stack *);
Tree Range(Stack *);
Tree Asymbol(Stack *);
Tree re_to_AST(char re[]);
void fail();
void visualize_AST(Tree tree, int indent);
//char next_char(char inputs[]);

/* 
128: RE 129:Aterm 130:Term 131:Afactor 132:Factor  133:Aelem 134: Elem 135:Symbol
136: Set 137: Rangelist 138:Range 139: Asymbol 
*/

#endif