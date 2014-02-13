/**************************************************/
/*  char_fsm.h version 0.0                        */
/*  header file for char_fsm.c                    */
/*                                                */
/**************************************************/
#ifndef CHAR_FSM_H_
#define CHAR_FSM_H_
/* data structures */
typedef struct node
{
    struct node     *next;
    char            *tptr;
}TQ;

/* fuctions */
void char_fsm(int ,int *,char *);
void cmd_fsm(char *,int *);
int char_type(char);
int pop_cmd_q(char *);
int test_cmd_q();
TQ *process_buffer(void);






#endif