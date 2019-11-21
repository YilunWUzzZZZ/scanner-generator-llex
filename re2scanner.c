#include "re.h"
#include "ds.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Transition AlphaBeta = {.low=0, .high=0x7fffffe07fffffe};
Transition Digits = {.low = 0x3ff000000000000, .high = 0};
Transition Any    = {.low = 0xffffffffffffffff, .high= 0xffffffffffffffff};
extern Transition Epsilon;

struct state_set
{
    int64_t * set;
    FA_State * state;
};

void escape_seq(char c, Transition * t){
        if(c == 'd'){
            t->high |= Digits.high;
            t->low |= Digits.low;
        }
        else if(c == 's'){
            t->high |= AlphaBeta.high;
            t->low |= AlphaBeta.low;
        }
        else if(c == 'a'){
            t->high |= Any.high;
            t->low |= Any.low;
        }
        else if(c == 'n'){
            t->low |= 1LL << '\n';
        }
        else if(c == 't'){
            t->low |= 1LL << '\t';
        }
        else if(c == 'w'){
            t->low |= 1LL << ' ';
        }
        else if(c == 'b'){
            ;
        }
        else if(c == '\\' || c == '|' || c == '*' || c == '(' || c == ')' || c == '^'
               || c == '[' || c == ']')
        {
            set_transition(t, c);
        }
        else{
            printf("unknown escape sequence, parsing failed\n");
            exit(0);
        }
}

void range(char c1, char c2, Transition * t){
    if(c1 > c2){
        printf("invalid range in [ ]\n");
        exit(0);
    }
    if(c1 >= 64){
        for(int i = c1; i<=c2; i++){
            t->high |= 1LL << (i-64);
        }
    }
    else if( c2 < 64){
        for(int i = c1; i<=c2; i++){
            t->low |= 1LL << i;
        }
    }
    else{
        for(int i = c1; i< 64; i++){
            t->low |= 1LL << i;
        }
        for(int i = 0; i <= c2-64; i++){
            t->high |= 1LL << i;
        }
    }
}

void print_trans(Transition t){
    for(int i=63; i>=0; i--){
        if(t.high & 1LL << i){
            printf("%c", '1');
        }
        else{
            printf("%c", '0');
        }
    }
    printf(" ");
    for(int i=63; i>=0; i--){
        if(t.low & 1LL << i){
            printf("%c", '1');
        }
        else{
            printf("%c", '0');
        }
    }
    printf("\n");
}

FA_Edge * transiton_2(FA_State * s, char c){
    FA_Edge * e = s->first_out;
    while(e){
        if(has_transition(&e->trans, c )){
            return e;
        }
        e = e->next_edge;
    }
    return NULL;
}

//128: RE 129:Aterm 130:Term 131:Afactor 132:Factor  133:Aelem 134: Elem 135:Symbol
FA_Graph * AST_to_NFA(Tree AST){
    //RE
    if((int)AST->content == _RE){ 
        FA_Graph * G1 = AST_to_NFA(AST->firstChild);
        FA_Graph * G2 = AST_to_NFA(AST->firstChild->sibling->sibling);
        return alter(G1, G2);
    }
    //Term
    else if((int)AST->content == _Term){
        FA_Graph * G1 = AST_to_NFA(AST->firstChild);
        FA_Graph * G2 = AST_to_NFA(AST->firstChild->sibling);
        return concat(G1, G2);
    }
    //Factor
    else if((int)AST->content == _Factor){
        FA_Graph * G = AST_to_NFA(AST->firstChild);
        return closure(G);
    }

    //Set
    else if((int)AST->content == _Set){
        char complement = 0;
        Tree child = AST->firstChild;
        if((int)child->content == '^'){
            complement = 1;
            child = child->sibling;
        }
        Transition t ={0,0};
        while(child){
            if((int)child->content < 128){
                set_transition(&t, (char)child->content);
            }
            else if((char)child->firstChild->content == '\\'){
                escape_seq((char)child->firstChild->sibling->content, &t);
            }
            else{
                char c1 = (char)child->firstChild->content;
                char c2 = (char)child->firstChild->sibling->sibling->content;
                range(c1, c2, &t);
            }
            child = child->sibling;
        }
        if(complement){
            t.high = ~t.high;
            t.low = ~t.low;
        }
        return create_FA(t);
    }

    else if((int)AST->content == _Range){
        Transition t ={0,0};
        if((char)AST->firstChild->content == '\\'){
            escape_seq((char)AST->firstChild->sibling->content, &t);
        }
        else{
            char c1 = (char)AST->firstChild->content;
            char c2 = (char)AST->firstChild->sibling->sibling->content;
            range(c1, c2, &t);
        }
        return create_FA(t);
    }

    //Symbol
    else if((int)AST->content == _Symbol){
        char c = (char)(AST->firstChild->sibling->content);
        Transition t = {0,0};
        escape_seq(c, &t);
        return create_FA(t);
    }
    //character
    else{
        char c = (char)(AST->content);
        Transition t ={0,0};
        set_transition(&t, c);
        return create_FA(t);
    }

}

FA_Graph * REs_to_NFA(char * res[],  int size, FA_State ** accepts){
    FA_Graph * NFAs[size];
    FA_Graph * NFA = new_graph();
    NFA->start = new_state();
    NFA->start->index = 0;
    NFA->size = 1;
    for(int i = 0; i < size; i++){
        Tree AST = re_to_AST(res[i]);
        //visualize_AST(AST, 0);
        FA_Graph * G = AST_to_NFA(AST);
        NFAs[i] = G;
        G->accept->index = i;
        G->accept->type = i;
        add_edge(NFA->start, G->start, Epsilon);
        //printf("gs:%d\n", G->size);
        NFA->size +=  G->size;
    }
    FA_State * * states = malloc(NFA->size * sizeof(FA_State *));
    states[0] = NFA->start;
    int cnt = 1;
    for(int i = 0; i < size; i++){
        FA_State * s = NFAs[i]->states;
        while(s){
            states[cnt] = s;
            if(s->index != -1){
               accepts[i] = s; 
            }
            s->index = cnt;
            s  = s->next_state;
            cnt++;
        }
        free(NFAs[i]);
    }
    NFA->states = (FA_State *) states;
    return NFA;
}

int64_t * *  cpt_e_closure(FA_Graph * NFA, int * bvn){
    
    int bit_vec_num = NFA->size / 64 + 1;
    *bvn = bit_vec_num;
    int64_t  ** e_closures = (int64_t  **)malloc(NFA->size * sizeof(int64_t*));
    for(int i=0; i<NFA->size; i++){
        e_closures[i] = (int64_t  *)malloc(sizeof(int64_t)*bit_vec_num);
        for(int j=0; j<bit_vec_num; j++){
            e_closures[i][j]= 0;
        }
    }
    char in_stack[NFA->size];
    Stack * s = create_stack();
    for(int i=0; i < NFA->size; i++){
        //memset(e_closures[i], 0, sizeof(int64_t)*bit_vec_num );
        e_closures[i][i/64] = 1LL << (i%64);
        push(s, i);
        in_stack[i] = 1;
    }

    FA_State * * states = (FA_State * *)NFA->states;
    while(!stack_empty(s)){
        int changed = 0;
        int i = (int)pop(s);
        //printf("i: %d\n", i);
        FA_State * node = states[i];
        in_stack[i] = 0;
        FA_Edge * out_edge = node->first_out;
        while(out_edge){
            if(is_epsilon(&(out_edge->trans))){
                for(int k=0; k<bit_vec_num; k++){
                    int64_t pre = e_closures[i][k];
                    e_closures[i][k] |= e_closures[out_edge->state->index][k];
                    if(pre != e_closures[i][k]){
                        changed = 1;
                    }
                }
            }
            out_edge = out_edge->next_edge;
        }
        if(changed){
            In_Edges * in_e = node->first_in;
            while(in_e){
                int neibor_index = in_e->state->index;
                if(!in_stack[neibor_index] && is_epsilon(&(in_e->in_edge->trans))){
                    push(s, neibor_index);
                    in_stack[neibor_index] = 1;
                }
                in_e = in_e->next;
            }
        }
    }
    return e_closures;
}

FA_Graph * NFA_to_DFA(FA_Graph * NFA, int64_t * * e_closures,  int ac_num, FA_State ** accepts, int bit_vec_num){
    FA_Graph * DFA = new_graph();
    FA_State * * states = (FA_State * *)NFA->states;
    List * added = malloc(sizeof(List));
    added->next = NULL;
    Stack * s = create_stack();
    int global_index = 0;
    DFA->start = new_state();
    DFA->start->index = global_index++;
    add_state(DFA, DFA->start);
    struct state_set start = {e_closures[0], DFA->start};
    push(s, &start);
    added->content = &start;
    while(!stack_empty(s)){
        struct state_set * state_set_stru = (struct state_set *)pop(s);
        int64_t * state_set = state_set_stru->set;
        FA_State * dfa_state = state_set_stru->state;

        for(int c=0; c<128; c++){
            int64_t * set = (int64_t *)malloc(sizeof(int64_t) * bit_vec_num);
            memset(set, 0, sizeof(int64_t) * bit_vec_num);
            char has_trans_on_c = 0;
            for(int i=0; i<bit_vec_num; i++){
                for(int j=0; j<64; j++){
                    if(state_set[i] & (1LL << j)){
                        int index = i*64 + j;
                        FA_State * node = states[index];
                        FA_State * end = transiton(node, (char)c);
                        if(end){
                    
                            for(int i=0; i<bit_vec_num; i++){
                                    set[i] |= e_closures[end->index][i];
                                    has_trans_on_c = 1;
                            }
                        }
                    }
                }
            }
            if(!has_trans_on_c){
                continue;
            }
            List * cur = added;
            char is_new = 1;
            FA_State * cur_dfa_state;
            while(cur){
                struct state_set * stru = (struct state_set *)(cur->content);
                int64_t * vec =  stru->set;
                char same = 1;

                for(int i=0; i< bit_vec_num; i++){
                    if(vec[i] != set[i]){
                        same = 0;
                        break;
                    }
                }
                if(same){
                    is_new = 0;
                    cur_dfa_state = stru->state;
                    free(set);
                    break;
                }

                cur = cur->next;
            }
            if(is_new){
                struct state_set * new_set = (struct state_set *)malloc(sizeof(struct state_set));
                cur_dfa_state = new_state();
                new_set->set = set;
                new_set->state = cur_dfa_state;
                push(s, new_set);
                list_append(added, new_set);
                cur_dfa_state->index = global_index++;
                add_state(DFA, cur_dfa_state);
                Transition t = {0,0};
                set_transition(&t, (char)c);
                // printf("after set from init:\n");
                // print_trans(t);
                add_edge(dfa_state, cur_dfa_state, t);

                //check if the set contains accept state
                int min = ac_num;
                for(int i=0; i<ac_num; i++){
                    int ac_i = accepts[i]->index;
                    if(set[ac_i/64] & (1LL << (ac_i%64)) ){
                        min = i;
                        break;
                    }
                }
                if(min < ac_num){
                    cur_dfa_state->type = min;
                }
            }
            else{
                FA_Edge * e = dfa_state->first_out;

                char no_edge_before = 1;

                while(e){
                    if(e->state == cur_dfa_state){
                        set_transition(&(e->trans), (char)c);
                        // printf("after added %c:\n", c);
                        // print_trans(e->trans);
                        no_edge_before = 0;
                        break;
                    }
                    e = e->next_edge;
                }
                if(no_edge_before){
                    Transition t = {0,0};
                    set_transition(&t, (char)c);
                    add_edge(dfa_state, cur_dfa_state, t);
                }
            }
        }//end 1st for
    }//end while
    FA_State * * states_array = (FA_State * *)malloc(sizeof(FA_State *) * DFA->size);
    FA_State * state = DFA->states;
    while(state){
        states_array[state->index] = state;
        state = state->next_state;
    }
    DFA->states = (FA_State *) states_array;
    return DFA;
}

FA_Graph * minimize_DFA(FA_Graph * DFA, int cat_num){
    int dfa_size = DFA->size;
    FA_State * * states = (FA_State * *)DFA->states;
    int node_group_num[dfa_size];
    List * accepts[cat_num];
    for(int i=0; i<cat_num; i++){
        accepts[i] = new_list();
    }
    List * others = new_list();
    for(int i=0; i<dfa_size; i++){
        if(states[i]->type != -1){
            node_group_num[i] = states[i]->type;
            list_append(accepts[states[i]->type], (void*)i);
        }
        else{
            node_group_num[i] = cat_num;
            list_append(others, (void*)i);
        }
    }
    int global_group_num = cat_num+1;
    char changed = 1;
    Stack * working = create_stack();
    Stack * splited = create_stack();
    for(int i=0; i<cat_num; i++){
        push(splited, accepts[i]);
    }
    push(splited, others);
    while(changed){
        Stack * temp = working;
        working = splited;
        splited = temp;
        changed=0;
        while(!stack_empty(working)){
            List * group = (List *)pop(working);
            FA_State * test_node = states[(int)group->next->content];
            
            for(short c=0; c<128; c++){
                FA_State * tar_state = transiton(test_node, (char)c);
                char is_splited = 0;
                int new_group_num = global_group_num;
                List * new_group_list = new_list();
                if(tar_state){
                    int tar_group_num = node_group_num[tar_state->index];
                
                    List * walker = group->next->next;
                    while(walker){

                        FA_State * cur_node = states[(int)walker->content];
                        FA_Edge * e = cur_node->first_out;
                    
                        FA_State * connected = transiton(cur_node, (char)c);
                        if((connected && node_group_num[connected->index] != tar_group_num)
                          || !connected)
                        {//if another end of the edge is not connected to same group
                        //  or has no transiton add the node to the new group
                            node_group_num[cur_node->index] = new_group_num;
                            list_append(new_group_list, cur_node->index);
                            list_remove(group, (void *)cur_node->index);
                            is_splited = 1;
                            
                        }
                        walker = walker->next;
                    }
                }
                else{
                    List * walker = group->next;
                    while(walker){
                        FA_State * cur_node = states[(int)walker->content];
                        if(transiton(cur_node, (char)c)){
                            node_group_num[cur_node->index] = new_group_num;
                            list_append(new_group_list, (void *)cur_node->index);
                            list_remove(group, (void *)cur_node->index);
                            is_splited = 1;
                        }
                        walker = walker->next;
                    }
                }
            
                if(is_splited){
                    global_group_num ++;
                    changed = 1;
                    push(splited, new_group_list);
                    break;
                }
                else{
                    free(new_group_list);
                }
            }//end for
            push(splited, group);
        }//end  second while
        // printf("ok\n");
    }//end first while;
    // printf("here\n");
    FA_State ** min_states = malloc(sizeof(FA_State*) * global_group_num);
    while(!stack_empty(splited)){
       int repr =  ((List *)pop(splited))->next->content;
       int group_index = node_group_num[repr];
       min_states[group_index] = states[repr];
    }
    for(int i=0 ;i<global_group_num; i++){
        FA_Edge * e =  min_states[i]->first_out;
        while(e){
            e->state = min_states[node_group_num[e->state->index]];
            e = e->next_edge;
        }
    }
    for(int i=0 ;i<global_group_num; i++){
        int old_index = min_states[i]->index;
        min_states[i]->index = node_group_num[old_index];
    }
    FA_Graph * min_DFA = new_graph();
    min_DFA->size = global_group_num;
    min_DFA->start = min_states[node_group_num[0]];
    min_DFA->states = (FA_State *)min_states;
    return min_DFA;
}

short * *DFA_to_table(FA_Graph * DFA, short * parent, int * colunms, short * types){
    short table[DFA->size][128];
    FA_State ** states = (FA_State **)DFA->states;
    for(int i=0; i<128; i++){
        parent[i] = -1;
    }

    for(int i=0; i<DFA->size; i++){
        types[i] = states[i]->type;
        for(int c=0; c<128; c++){
            FA_State * end;
            if(end = transiton(states[i], (char)c)){
                table[i][c] = end->index;
            }
            else{
                table[i][c] = -1;
            }
        }
    }
    int compressed_col_num = 128;
    for(int i=0; i<128; i++){
        if(parent[i] != -1){
            continue;
        }
        for(int j=i+1; j<128; j++){
            if(parent[j] != -1){
                continue;
            }
            char same = 1;
            for(int k=0; k<DFA->size; k++){
                if(table[k][i] != table[k][j]){
                    same = 0;
                    break;
                }
            }
            if(same){
                compressed_col_num --;
                parent[j] = i;
            }
        }
    }

    int mapping[128];
    short ** compressed_table = malloc(sizeof(short *) * DFA->size);
    for(int i=0; i<DFA->size;i++){
        compressed_table[i] = malloc(sizeof(short) * compressed_col_num);
    }
    int cnt = 0;
    for(int i=0; i<128; i++){
        if(parent[i] == -1){

            for(int j=0; j<DFA->size; j++){
                compressed_table[j][cnt] = table[j][i];
            }
            mapping[i] = cnt++;
        }
    }
    if(compressed_col_num == cnt){
        printf("size matched, compressed col size = %d\n", cnt);
    }
    for(int i=0; i<128; i++){
        if(parent[i] != -1){
            parent[i] = mapping[parent[i]];
        }
        else{
            parent[i] = mapping[i];
        }
    }
    *colunms = compressed_col_num;
    return compressed_table;
}

void print_table(FILE * fp, short ** table, short * mapping, char * category[], short * types, short start, int dfa_size, int col, int cat_num){
    fprintf(fp, "#ifndef DFA_TABLE_H\n#define DFA_TABLE_H\n");
    fprintf(fp, "short table[%d][%d] = {", dfa_size, col);
    for(int i=0; i<dfa_size; i++){
        fprintf(fp, "\n                      {");
        for(int j=0; j<col; j++){
            fprintf(fp, "%d, ", table[i][j]);
        }
        fprintf(fp, "},");
    }
    fprintf(fp, "\n                     };\n");
    fprintf(fp, "short mapping[128] = {");
    for(int i=0; i<128; i++){
        fprintf(fp, "%d, ", mapping[i]); 
    }
    fprintf(fp, "};\n");
    fprintf(fp, "char * category[%d] = {", cat_num);
    for(int i=0; i<cat_num; i++){
        fprintf(fp, "\"%s\", ", category[i]); 
    }
    fprintf(fp, "};\n");
    fprintf(fp, "short state_type[%d] = {", dfa_size);
    for(int i=0; i<dfa_size; i++){
        fprintf(fp, "%d, ", types[i]); 
    }
    fprintf(fp, "};\n");
    fprintf(fp, "short start = %d;\n",start);
    fprintf(fp, "#endif\n");
}


int main(int argc, char * argv[]){
    FILE * fp = fopen("re_text","r");
    Stack * re_strs = create_stack();
    Stack * cat_strs = create_stack();
    int re_cnt =0;
    char cat_buf[200];
    char re_buf[200];
    while(2 == fscanf(fp, "%s %s", re_buf, cat_buf)){
        printf("%s  %s\n", re_buf, cat_buf);
        char * re = malloc(sizeof(char) * (strlen(re_buf) + 1));
        char * cat = malloc(sizeof(char) * (strlen(cat_buf) + 1));
        strcpy(re, re_buf);
        strcpy(cat, cat_buf);
        push(re_strs, re);
        push(cat_strs, cat);
        re_cnt++;
    }
    char * res[re_cnt];
    char * cats[re_cnt];
    for(int i=re_cnt-1; i>=0; i--){
        res[i] = (char *)pop(re_strs);
        cats[i] = (char *)pop(cat_strs);
        printf("%s  %s\n", res[i], cats[i]);
    }
   
    FA_State * acs[re_cnt];
    FA_Graph * NFA = REs_to_NFA(res, re_cnt, acs);
    printf("1\n");
    int  bvn;
    int64_t ** e_cls = cpt_e_closure(NFA, &bvn);
    printf("2\n");
    FA_Graph * DFA = NFA_to_DFA(NFA, e_cls, re_cnt, acs, bvn);
    printf("DFA size: %d\n",DFA->size);
    FA_Graph * min_DFA = minimize_DFA(DFA, re_cnt);
    printf("Min DFA size: %d\n",min_DFA->size);
    FA_State ** states = (FA_State **)min_DFA->states;
    //FA_State ** states = (FA_State **)DFA->states;
    int col;
    short mapping[128];
    short type[min_DFA->size];
    short ** table = DFA_to_table(min_DFA, mapping, &col, type);
    FILE * fp_out = fopen("dfa_table.h", "w");
    print_table(fp_out, table, mapping, cats, type, min_DFA->start->index, min_DFA->size, col, re_cnt);
    fclose(fp_out);

    // for(int i=0; i<min_DFA->size; i++){
    //     if(states[i]->type != -1){
    //         printf("ac: %d, index: %d\n", states[i]->type, i);
    //     }
    // }
    // for(int i=0; i<min_DFA->size; i++){
    //     FA_Edge * e = states[i]->first_out;
    //     while(e){
    //         print_trans(e->trans);
    //         e = e->next_edge;
    //     }
    // }

   // tester
    // FILE * fp1 = fopen("test_word","r");
    // char input[100];
    // int start = min_DFA->start->index;
    // while(1 == fscanf(fp1, "%s\n", input)){
    //     int cnt=0;
    //     char c;
    //     short state = start;
    //     while( (c=input[cnt++]) ){
    //         int mapped = mapping[c];
    //         state = table[state][mapped];
    //         if(state == -1){
    //             break;
    //         }
    //     }
    //     if (state != -1 && type[state] != -1){
    //         printf("%s: %s\n", input, cats[type[state]]);
    //     }
    //     else{
    //         printf("%s: failed\n", input);
    //     }
    // }
    // fclose(fp1);

    
    return 0;
}


