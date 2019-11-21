#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dfa_table.h"
#define BUFSIZE  4096
#define Forward(ptr) (ptr = (ptr + 1)%BUFSIZE)
#define BackTrack(ptr, step) (ptr = (ptr-step+BUFSIZE)%BUFSIZE)
#define FillBuf(buf, fp) \
            read_cnt = fread(buf, 1, BUFSIZE/2, fp);\
            if(read_cnt < BUFSIZE/2)\
                buf[read_cnt] = EOF
        
#define fail() printf("failed\n"); exit(0)


extern short start;
extern short table[35][28];
extern short mapping[128];
extern char * category[];
extern short state_type[35];

int read_cnt=0;

void scan(char * path, char * outpath){
    char buffer[BUFSIZE];
    char * buffer1 = buffer;
    char * buffer2 = &buffer[BUFSIZE/2];
    short buf_ptr = 0;
    char updated_1 = 0, updated_2 = 0;
    FILE * fp = fopen(path, "r");
    FILE * fp_out = fopen(outpath, "w");
    FillBuf(buffer1, fp);
    updated_1 = 1;

    char lexeme[BUFSIZE];
    int lexeme_ptr = 0;
    char c;
    short state = start;
    int step = 0;
    short last_accept = -1;

    while( !((c = buffer[buf_ptr]) == EOF && lexeme_ptr == 0) ){
        int temp_s = state;
        if( c != EOF && (state = table[state][mapping[c]]) != -1)
        {
            printf("state: %d, input: %c, to state: %d\n", temp_s, c, state);
            lexeme[lexeme_ptr++] = c;
            step++;
            Forward(buf_ptr);
            if(buf_ptr == BUFSIZE/2 && updated_2 == 0)
            {
                FillBuf(buffer2, fp);
                updated_2 = 1;
                updated_1 = 0;
            }
            else if(buf_ptr == 0 && updated_1 == 0)
            {
                FillBuf(buffer1, fp);
                updated_1 = 1;
                updated_2 = 0;
            }
            if(state_type[state] != -1)
            {
                last_accept = state_type[state];
                step = 0;
            }
        }
       
        else //BackTrack
        {   
            if(c == EOF){
                printf("reached eof\n");
            }
            else{
                printf("no transiton on %c\n", c );
            }
            BackTrack(buf_ptr, step);
            lexeme_ptr -= step;
            printf("%c, %d, %d\n", c, lexeme_ptr ,step);
            step = 0;
            if(last_accept == -1 ){
                fail();
            }
            else{
                lexeme[lexeme_ptr]= '\0';
                lexeme_ptr = 0;
                state = start;
                if( strcmp(category[last_accept], "whitespace" ))
                {
                    fprintf(fp_out, "( %s , %s )", category[last_accept], lexeme);
                }
                last_accept = -1;
            }
        }
    }
    printf("success\n");

}

int main(){
    scan("test_text.c", "scan_out");
    return 0;
}
                