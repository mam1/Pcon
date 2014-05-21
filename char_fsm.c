#include <propeller.h>
#include <stdio.h>
#include "simpletools.h"
#include "Pcon.h"
#include "char_fsm.h"
#include "cmd_fsm.h"

/***************************** globals ******************************/
    TQ              *head, *tail;
/***************************** externals ************************/
extern char         input_buffer[_INPUT_BUFFER], *input_buffer_ptr;  
extern int          cmd_state,char_state;
/**********************support fuctions ****************************/
TQ *process_buffer(void)
{
    char        tb[_INPUT_BUFFER],*t_ptr,*start_char;        //,*end_char;
    int         i; 
    input_buffer_ptr = input_buffer;
    t_ptr = tb;    
    start_char = input_buffer_ptr;
    head = '\0';
    tail = head;    
    while(*input_buffer_ptr != '\0')
    {
    if(*input_buffer_ptr==_QUOTE)
    {
        *t_ptr++ = *input_buffer_ptr++;            
        while((*input_buffer_ptr != _QUOTE)&&(*input_buffer_ptr != '\0'))
            *t_ptr++ = *input_buffer_ptr++;            
        *t_ptr++ = _QUOTE;
            *(++input_buffer_ptr)='\0';
            if(tail == '\0')
            {             
                tail = malloc(sizeof(TQ));
                head = tail;
            }
            else
            {
                tail->next = malloc(sizeof(TQ));
                tail = tail->next;
            }
            tail->tptr = malloc(input_buffer_ptr - start_char );
            memcpy(tail->tptr,start_char,input_buffer_ptr - start_char + 1);
            tail->next='\0';
            start_char = input_buffer_ptr;
            start_char++;         
    }          
        if((*input_buffer_ptr==_SPACE)||(*input_buffer_ptr==_COLON)||(*input_buffer_ptr==_SLASH)||(*input_buffer_ptr==_COMMA)) 
        {
            *input_buffer_ptr='\0';
            if(tail == '\0')
            {             
                tail = malloc(sizeof(TQ));
                head = tail;
            }
            else
            {
                tail->next = malloc(sizeof(TQ));
                tail = tail->next;
            }
            tail->tptr = malloc(input_buffer_ptr - start_char );
            memcpy(tail->tptr,start_char,input_buffer_ptr - start_char + 1);
            tail->next='\0';
            start_char = input_buffer_ptr;
            start_char++;
        }
        *t_ptr++ = *input_buffer_ptr++;            
    }    
    for(i=0;i<_INPUT_BUFFER;i++) input_buffer[i] = '\0';
    input_buffer_ptr = input_buffer;
    return head;
}
char *pop()
{
    TQ                  *hold;
    static char         tb[128];
    char                *ptr1, *ptr2;

    if(head == '\0')
        return NULL;
    ptr1 = head->tptr;
    ptr2 = tb;
    hold = head;
    while(*ptr1 != '\0') *ptr2++ = *ptr1++;
    *ptr2 = '\0';
    head = head->next;     
    free(hold);
    return tb;
}
int char_type(char c)
{
    switch(c)
    {
        case _ESC:
            return 0;
        case _SPACE:
            return 1;
        case _COLON:
            return 2;
        case _SLASH:
            return 3;
        case _BS:
            return 4;
        case _QUOTE:
            return 5;
        case _CR:
            return 6;
    }
    return 7;    
}

/********************************************/
/** character input parser action routines **/
/********************************************/

/* 0 -  do nothing */
int char_nop(char *c)
{
    return 0;
}
/* 1 – clear all buffers, reset both state machines */
int char_esc(char *c)
{
    char    dump[_TOKEN_BUFFER];
    cmd_state = 0;          //reset state
    while(pop_cmd_q(dump)); //clear out the comand queue
    printf("\n\nfsm reset\n");
    reset_active();
    input_buffer_ptr = input_buffer;
    *input_buffer_ptr++ = '?';
    *input_buffer_ptr++ = ' ';
    *input_buffer_ptr++ = '\0';
    
    process_buffer();
    return 0;
}
/* 2 – add char to buffer */                    
int char_add_char(char *c)
{
//    printf("add char to buffer <%c>",*c);
    *input_buffer_ptr++ = *c;
//    printf(" <%s>\n",input_buffer);
    return 0;
}
/* 3 – remove char from buffer */               
int char_remove(char *c)
{   
    input_buffer_ptr--;
    return 0;
}
/* 4 – add char to buffer, add delimiter to buffer */                 
int char_add_dlim(char *c)
{
    *input_buffer_ptr++ = *c;
    return 0;
}
/* 5 -  add QUOTE to buffer, add char to buffer,  process buffer */              
int char_add_quote_char_process(char *c)
{
    *input_buffer_ptr++ = _QUOTE;
    *input_buffer_ptr++ = *c;
    return 0;
}
/* 6 - add char to buffer,  process buffer */
int char_add_process(char *c)
{
    *input_buffer_ptr++ = ' '; 
    *input_buffer_ptr++ = '\0'; 
    process_buffer();
    return 0;
} 

/* 7 – add delimiter to buffer, add quote to buffer */
int char_delim_add(char *c)
{
    *input_buffer_ptr++ = _QUOTE; 
    return 0;
} 

/* 8  - add char to buffer,  process buffer */
int char_eof_process(char *c)
{
    *input_buffer_ptr++ = '\0'; 
    process_buffer();
    return 0;
} 

/***************************************/
/**  character input parser fsm start **/
/***************************************/

/* fsm support functions */
int char_type(char);
TQ *process_buffer(void);

/* fsm fuctions */
int char_nop(char *);                    // 0
int char_esc(char *);                    // 1
int char_add_char(char *);               // 2
int char_remove(char *);                 // 3
int char_add_dlim(char *);               // 4
int char_add_quote_char_process(char *); // 5
int char_add_process(char *);            // 6
int char_delim_add(char *);              // 7
int char_eof_process(char *);            // 8


/* character processor action table - initialized with fsm fuctions */
typedef int (*ACTION_PTR)(char *); 
ACTION_PTR char_action[_CHAR_TOKENS][_CHAR_STATES] = {     
/* ESC   */ {char_esc,      char_esc,                    char_esc,         char_esc         },
/* SPACE */ {char_nop,      char_add_char,               char_add_char,    char_nop         },
/* COLON */ {char_nop,      char_add_char,               char_add_char,    char_nop         },
/* SLASH */ {char_nop,      char_add_char,               char_add_char,    char_nop         },
/* BSPACE*/ {char_nop,      char_remove,                 char_remove,      char_remove      },
/* QUOTE */ {char_add_char, char_add_dlim,               char_delim_add,   char_delim_add },
/* CR    */ {char_add_process,      char_add_quote_char_process, char_add_process, char_add_process },
/* other */ {char_add_char, char_add_char,               char_add_char,    char_add_char    }};

/* character processor state transition table */
int char_new_state[_CHAR_TOKENS][_CHAR_STATES] ={
/* ESC   */ {0, 0, 0, 0},
/* SPACE */ {0, 1, 3, 3},
/* COLON */ {0, 1, 3, 3},
/* SLASH */ {0, 1, 3, 3},
/* BSPACE*/ {0, 1, 2, 3},
/* QUOTE */ {1, 3, 1, 1},
/* CR    */ {0, 0, 0, 0},
/* other */ {2, 1, 2, 2}};

/*****************************************************/
/****  character input parser state machine end  *****/
/*****************************************************/

void char_fsm(int c_type,int *state,char *c)
{
//    printf("\nchar_fsm called: token type <%i>, state <%i>, char <%c>\n",
//            c_type, *state, *c);
    char_action[c_type][*state](c); 
    *state = char_new_state[c_type][*state];

    return;
}

/* test command queue return: 0 empty queue, -1 toke available */ 
int test_cmd_q()
{
    if(head == '\0')
        return 0;
    return -1;
}

/* pop token into buffer, return: 0 empty queue, -1 data placed in buffer  */ 
int pop_cmd_q(char *buf)
{
    TQ                  *hold;
    char                *ptr1, *ptr2;

    if(head == '\0')
    {
        *buf = '\0';    //set buffer to empty
        return 0;
    }
    ptr1 = head->tptr;
    ptr2 = buf;
    hold = head;
    while(*ptr1 != '\0') *ptr2++ = *ptr1++;
    *ptr2 = '\0';
    head = head->next;
    free(hold->tptr);     
    free(hold);

    return -1;
}

