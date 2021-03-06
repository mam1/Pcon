 #include <propeller.h>
#include <stdio.h>
#include <string.h>
#include "simpletools.h"
#include "Pcon.h"
#include "char_fsm.h"
#include "channel.h"
#include "schedule.h"

/*********************** externals **************************/
 extern int          cmd_state,char_state;
 extern char         input_buffer[_INPUT_BUFFER],*input_buffer_ptr;
 extern char         c_name[_CHANNEL_NAME_SIZE][_NUMBER_OF_CHANNELS];
 extern uint32_t     bbb[];
 /* rtc control block */ 
 extern struct {
    unsigned stack[_STACK_SIZE_RTC];
    volatile RTC_CB rtc;
 } rtc_cb;
 /* control block & stack for dio cog */
 extern struct {
    unsigned stack[_STACK_SIZE_DIO];
    volatile DIO_CB dio;
 } dio_cb;
/*********************** globals **************************/
 char                prompt_buffer[_PROMPT_BUFFER];
 struct {
    int                 channel; 
    int                 day; 
    int                 hour; 
    int                 minute; 
    int                 key;
    } edit;
 uint8_t             editing;
/***************** global code to text conversion ********************/
extern char *day_names_long[7];     
extern char *day_names_short[7];
extern char *onoff[2];
extern char *con_mode[3];
extern char *sch_mode[2];    
/*********************** functions **************************/

/***************************************/
/*****  command  parser fsm start ******/
/***************************************/
/* state prompts */
 char    *s_prompt[] ={
 /*  0 */    "enter command",
 /*  1 */    "channel maintenance: <save><load><display> or chanel number to edit",
 /*  2 */    "editing channel:",
 /*  3 */    "EMPTY",
 /*  4 */    "save",
 /*  5 */    "load",
 /*  6 */    "done",
 /*  7 */    "state",
 /*  8 */    "schedule",
 /*  9 */    "channel",
 /* 10 */    "name"};

/* key word list */
char    *keyword[_CMD_TOKENS] = {
 /*  0 */    "INT",
 /*  1 */    "STR",
 /*  2 */    "OTHER",
 /*  3 */    "EMPTY",
 /*  4 */    "save",
 /*  5 */    "load",
 /*  6 */    "done",
 /*  7 */    "state",
 /*  8 */    "schedule",
 /*  9 */    "channel",
 /* 10 */    "name",
 /* 11 */    "mode",
 /* 12 */    "on",
 /* 13 */    "off",
 /* 14 */    "?",
 /* 15 */    "display",
 /* 16 */    "day",
 /* 17 */    "system",
 /* 18 */    "delete",
 /* 19 */    "time",
 /* 20 */    "yes",
 /* 21 */    "no",
 /* 22 */    "add",
 /* 23 */    "change",
 /* 24 */    "dump"   
};

/* cmd processor state transition table */
int cmd_new_state[_CMD_TOKENS][_CMD_STATES] ={
/*                   0  1  2  3  4  5  6  7  8   9  10 */
/*  0      INT */   {0, 2, 2, 3, 2, 5, 7, 9, 8, 10, 10},
/*  1      STR */   {0, 1, 2, 2, 4, 5, 6, 7, 8,  9, 10},
/*  2    OTHER */   {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10},
/*  3    EMPTY */   {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10},
/*  4     save */   {0, 1, 2, 3, 4, 5, 6, 6, 8,  9,  6}, 
/*  5     load */   {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10},
/*  6     done */   {0, 0, 1, 2, 2, 2, 2, 6, 8,  7,  7},
/*  7    state */   {0, 1, 5, 3, 4, 5, 6, 7, 8,  9, 10},
/*  8 schedule */   {0, 1, 6, 3, 4, 5, 6, 7, 8,  9, 10},
/*  9  channel */   {1, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10},
/* 10     name */   {0, 1, 3, 3, 4, 5, 6, 7, 8,  9, 10},
/* 11     mode */   {0, 1, 4, 3, 4, 5, 6, 7, 8,  9, 10},
/* 12       on */   {0, 1, 2, 3, 4, 2, 6, 7, 8,  9,  7},
/* 13      off */   {0, 1, 2, 3, 4, 2, 6, 7, 8,  9,  7},
/* 14        ? */   {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10},
/* 15  display */   {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10},
/* 16      day */   {0, 1, 2, 3, 4, 5, 7, 7, 8,  9, 10},
/* 17   system */   {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10},
/* 18   delete */   {0, 1, 2, 3, 4, 5, 6, 7, 8,  9,  7},
/* 19     time */   {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10},
/* 20      yes */   {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10},
/* 21       no */   {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10},
/* 22      add */   {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10},
/* 23   change */   {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10},
/* 24     dump */   {0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10}};


/*cmd processor functions */
int c_0(int,int *,char *); /* do nothing */
int c_1(int,int *,char *); /* display all valid commands for the current state */  
int c_2(int,int *,char *); /* prompt */
int c_3(int,int *,char *); /* prompt for channel number */
int c_4(int,int *,char *); /* set active channel */
int c_5(int,int *,char *); /* display info for all channels */
int c_6(int,int *,char *); /* save info for all channels */
int c_7(int,int *,char *); /* load info for all channels */
int c_8(int,int *,char *); /* set channel name */
int c_9(int,int *,char *); /* nop */
int c_10(int,int *,char *); /* set channel control mode */
int c_11(int,int *,char *); /* set channel state on */
int c_12(int,int *,char *); /* unknown command */
int c_13(int,int *,char *); /* invalid command */
int c_14(int,int *,char *); /* save schedule buffer */
int c_15(int,int *,char *); /* prompt for channel control mode */
int c_16(int,int *,char *); /* prompt for channel state */
int c_17(int,int *,char *); /* display data for a single channel */
int c_18(int,int *,char *); /* display formatted schedules */
int c_19(int,int *,char *); /* set channel state to off */
int c_20(int,int *,char *); /* set active day */
int c_21(int,int *,char *); /* set active hour */
int c_22(int,int *,char *); /* set active minute */
int c_23(int,int *,char *); /* display time and date */
int c_24(int,int *,char *); /* set schedule record state to on */
int c_25(int,int *,char *); /* set schedule record state to off */
int c_26(int,int *,char *); /* delete schedule record */

int c_27(int,int *,char *); /* load schedule buffer from SD card */
int c_28(int,int *,char *); /* save schedule buffer to SD card */

int c_29(int,int *,char *); /* dump schedule buffer */
int c_30(int,int *,char *); /* display edit buffer schedule for active day and channel */
int c_31(int,int *,char *); /* display system configuration infromation */


/* cmd processor action table - initialized with fsm functions */

CMD_ACTION_PTR cmd_action[_CMD_TOKENS][_CMD_STATES] = {
/*            STATE    0     1     2     3    4     5      6    7     8     9     10 */ 
/*  0      INT */   {c_13,  c_4,  c_0,  c_0, c_10, c_13, c_20, c_21,  c_0, c_22, c_13}, 
/*  1      STR */   {c_13,  c_0,  c_0,  c_8,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0, c_13},
/*  2    OTHER */   {c_12, c_12, c_12, c_12, c_12, c_12, c_12, c_12, c_12, c_12, c_12},
/*  3    EMPTY */   { c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0},
/*  4     save */   {c_13,  c_6,  c_6,  c_0,  c_0,  c_0, c_14, c_14,  c_0,  c_0, c_14},
/*  5     load */   {c_13,  c_7,  c_7,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0, c_13},
/*  6     done */   {c_13,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_7, c_13},
/*  7    state */   {c_13, c_13, c_16,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0, c_13},
/*  8 schedule */   {c_18,  c_0,  c_9,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0, c_13},
/*  9  channel */   { c_3, c_13,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0, c_13},
/* 10     name */   {c_13, c_13,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0, c_13},
/* 11     mode */   {c_13, c_13, c_15, c_10,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0, c_13},
/* 12       on */   {c_13,  c_0,  c_0,  c_0,  c_0, c_11,  c_0,  c_0,  c_0,  c_0, c_24},
/* 13      off */   {c_13,  c_0,  c_0,  c_0,  c_0, c_19,  c_0,  c_0,  c_0,  c_0, c_25},
/* 14        ? */   { c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1,  c_1},
/* 15  display */   {c_18,  c_5, c_17, c_17, c_17, c_17, c_31, c_30,  c_0,  c_0, c_30},
/* 16      day */   {c_13,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0, c_13},
/* 17   system */   {c_31,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0, c_13},
/* 18   delete */   {c_13,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0, c_26},
/* 19     time */   {c_23, c_23, c_23, c_23, c_23, c_23, c_23, c_23, c_23, c_23, c_23},
/* 20      yes */   {c_13,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0, c_13},
/* 21       no */   {c_13,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0, c_13},
/* 22      add */   {c_13,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0, c_13},
/* 23   change */   {c_13,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0,  c_0, c_13},
/* 24    dump  */   {c_29, c_29, c_29, c_29, c_29, c_29,  c_29, c_29, c_29, c_29, c_29}};


/***************start fsm support functions ********************/
//char *valid_cmds(void);
char *build_prompt(char *, int);


/**************** command fsm action routines ******************/
/* do nothing except prompt if needed */
int c_0(int tt, int *n, char *s)
{
    if(test_cmd_q()== 0)
    {
        *prompt_buffer = '\0';
        printf("%s\n\n>> ",build_prompt(prompt_buffer,tt));
    }
    return 0;
}
/* display all valid commands for the current state */
int c_1(int tt, int *n, char *s)
{
    int     i;

    printf("state %i, ", cmd_state);
    if((cmd_state > 1) && (cmd_state < 6))
        printf("editing channel %i, ",edit.channel);
    else if(cmd_state > 6)
        printf("editing channel %i schedule, ",edit.channel);
    printf("valid commands: <?> ");
    for(i=0; i<_CMD_TOKENS;i++)
    {
        if(cmd_action[i][cmd_state] == c_0)
            continue;
        if(cmd_action[i][cmd_state] == c_2)
            continue;
        if(cmd_action[i][cmd_state] == c_12)
            continue;
        if(cmd_action[i][cmd_state] == c_13)
            continue;
        if(cmd_action[i][cmd_state] == c_1)
            continue;
        printf("<%s> ",keyword[i]);        
    }
    printf("\n\n");
    c_0(tt,n,s);
    return 0;
}
/* prompt */
int c_2(int tt, int *n, char *s)
{    
    printf("%s >>",s_prompt[cmd_state]);    
    return 0;
}
/* prompt for channel number */
int c_3(int tt, int *n, char *s)
{
    char    dump[_TOKEN_BUFFER];    
    /* if queue is empty prompt for input */
    if(test_cmd_q(dump))   
        return 0;
//    printf("enter <save>,<load>,<display> or channel number to edit >>");
    c_0(tt,n,s);
    return 0;
}
/* set active channel */
int c_4(int tt, int *n, char *s)
{
    if((*n >= 0) && (*n<_NUMBER_OF_CHANNELS))
    {      
        edit.channel = *n;
        // printf("********* channel %i\n>>",channel);
        c_0(tt,n,s);
        return 0;
    }
    printf("%i is not a valid channel number it must be 0-%i\n\n",*n,_DRIVEN);
    cmd_state = 1; // back out the state transition
    c_3(tt,n,s);
    return 1;
}
/* display info for all channels */
int c_5(int tt, int *n, char *s) 
{
    disp_channel_data(-1);
    c_0(tt,n,s);
    return 0;
}
/* save channel control array from sd card */

int c_6(int tt, int *n, char *s) //save - s1
{
    if(save_channel_data()==0)
        printf("channel data saved to sd card\n");
    else
        printf("*** error saving channel data\n");
    // disp_channel_data(-1);
    // printf(">>");  
    c_0(tt,n,s);
    return 0;
}
/*  load channel control array to sd card */
int c_7(int tt, int *n, char *s) //load - s1
{
    if(load_channel_data()==0)
        printf("channel data loaded from sd card\n");
    else
        printf("*** error loading channel data\n");
    disp_channel_data(-1);
    printf(">>");         
    return 0;
}
/* set channel name */
int c_8(int tt, int *n, char *s) //save - s1
{
    set_channel_name(s);
    printf("channel name set\n\n"); 
    disp_channel_data(edit.channel);
//    printf("editing channel %i\n>>",channel);
    c_0(tt,n,s); 
    return 0;
}
/* nop */
int c_9(int tt, int *n, char *s) //save - s1
{
    c_0(tt,n,s);
    return 0;
}
/* set channel control mode */
int c_10(int tt, int *n, char *s) //save - s1
{
    set_channel_control_mode(*n); 
    printf("channel control mode set\n\n");
    disp_channel_data(edit.channel);
//    printf("editing channel %i\n>>",channel);
    c_0(tt,n,s); 
    return 0;
}
/* set channel state on */
int c_11(int tt, int *n, char *s) //save - s1
{
    printf("setting state for channel %i\n",edit.channel);
    set_channel_state(1);
    set_channel_control_mode(0);
    rtc_cb.rtc.update = 1;
    printf("channel state set to on, control mode forced to manual\n");
    printf("displaying data for channel %i\n",edit.channel);
    disp_channel_data(edit.channel);
    c_0(tt,n,s);  
    return 0;
}

/* unrecognized input */
int c_12(int tt, int *n, char *s) //save - s1
{
    printf("unrecognized command\n>>");
    c_0(tt,n,s);  
    return 0;
}


/* invalid input */
int c_13(int tt, int *n, char *s) //save - s1
{
    printf("command is not valid in state %i, enter <?> for a list of valid commands\n>>",cmd_state); 
    c_0(tt,n,s);  
    return 0;
}

/* save edit schedule buffer */
int c_14(int tt, int *n, char *s) 
{
    write_sch(bbb);
    printf("schedule buffer save to SD card\n");
    c_0(tt,n,s); 
    return 0;
}

/* prompt for channel mode */
int c_15(int tt, int *n, char *s)
{
    char    dump[_TOKEN_BUFFER];    
    /* if queue is empty prompt for input */
    if(test_cmd_q(dump))   
        return 0;
    printf("enter channel control mode: 0-manual, 1-time, 2-time & sensor >>");
    return 0;
}

/* prompt for channel state */
int c_16(int tt, int *n, char *s)
{
    char    dump[_TOKEN_BUFFER];    
    /* if queue is empty prompt for input */
    if(test_cmd_q(dump))   
        return 0;
    printf("enter channel state: {off}|{on} >>");
    return 0;
}

/* display info for single channel */
int c_17(int tt, int *n, char *s) 
{
    printf("request to display data for channel %i\n",edit.channel);
    disp_channel_data(edit.channel);
    printf(">>"); 
    return 0;
}


/*display formatted scheduls */

int c_18(int tt, int *n, char *s)
{
    disp_all_schedules(bbb);

    c_0(tt,n,s);
    return 0;
}

/* set channel state off */
int c_19(int tt, int *n, char *s) //save - s1
{
    set_channel_state(0);
    set_channel_control_mode(0);
    rtc_cb.rtc.update = 1;
    printf("channel state set to off, control mode forced to manual\n");
    disp_channel_data(edit.channel);
    c_0(tt,n,s);  
    return 0;
}

/* set active day and load schedule buffer*/
int c_20(int tt, int *n, char *s) 
{
    if((*n < 1) || (*n > 7))
    {
        printf("invalid day number, ");
        printf("%i is not a valid day number it must be 1-7\n\n",*n);
        cmd_state = 1; // back out the state transition
        c_3(tt,n,s);
        return 0;
    }
    edit.day = *n;
    // dspl_sch(bbb,edit.day,edit.channel);
    c_0(tt,n,s);  
    return 0;
}

/* set active hour */
int c_21(int tt, int *n, char *s) 
{
    if((*n < 0) || (*n > 23))
    {
        printf("invalid hour, ");
        printf("%i is not a valid hour it must be 0-23\n\n",*n);
        cmd_state -= 1; // back out the state transition
        c_3(tt,n,s);
        return 0;
    }
    edit.hour = *n;
    // printf("active hour set:  \n  ");
    c_0(tt,n,s);  
    return 0;
}

/* set active minute */
int c_22(int tt, int *n, char *s) 
{
    if((*n < 0) || (*n > 59))
    {
        printf("invalid minute, ");
        printf("%i is not a valid minute it must be 0-59\n\n",*n);
        cmd_state -= 2; // back out the state transition
        c_3(tt,n,s);
        return 0;
    }
    edit.minute = *n;
    // printf("active minute set:  \n  ");
    edit.key = (edit.hour*60)+edit.minute;
//    dump_sch_recs(dio_cb.dio.sch, channel, day);
    c_0(tt,n,s);  
    return 0;
}

/* display time and date */
int c_23(int tt, int *n, char *s) 
{

    printf("%s, %i:%02i:%02i  %i/%i/%i\n\n",
        day_names_long[rtc_cb.rtc.td_buffer.dow-1],
        rtc_cb.rtc.td_buffer.hour,
        rtc_cb.rtc.td_buffer.min,
        rtc_cb.rtc.td_buffer.sec,

        rtc_cb.rtc.td_buffer.month,
        rtc_cb.rtc.td_buffer.day,
        rtc_cb.rtc.td_buffer.year+2000);
    c_0(tt,n,s);  
    return 0;
}

/* set schedule record state to on */
int c_24(int tt, int *n, char *s) 
{
    // printf("set to on\n");
    // printf("add_sch_rec parameters before call: edit_schedule address <%x>, active channel <%i>, key <%i>, state <1>\n",edit_schedule,channel,key);
    add_sch_rec(get_schedule(bbb,edit.day,edit.channel),edit.key,1);
    rtc_cb.rtc.update = 1;
    c_0(tt,n,s); 
    return 0;
}

/* set schedule record state to off */
int c_25(int tt, int *n, char *s) 
{
    // printf("set to off\n");
    add_sch_rec(get_schedule(bbb,edit.day,edit.channel),edit.key,0);
    rtc_cb.rtc.update = 1;
    c_0(tt,n,s); 
    return 0;}

/* delete schedule record */
int c_26(int tt, int *n, char *s) 
{
    del_sch_rec(get_schedule(bbb,edit.day,edit.channel),edit.key);
     rtc_cb.rtc.update = 1;
    c_0(tt,n,s); 
    return 0;
}

/* load schedule buffer from SD card */
int c_27(int tt, int *n, char *s) 
{
    read_sch(bbb);
    printf("schedule buffer loaded from the sd card\n");
    c_0(tt,n,s);
    return 0;
}

/* save edit schedule buffer to SD card */
int c_28(int tt, int *n, char *s) 
{
    write_sch(bbb);
    printf("schedule buffer save to SD card\n");
    c_0(tt,n,s); 
    return 0;
}

/* save edit schedule buffer before load*/
int c_29(int tt, int *n, char *s) 
{
    dump_schs(bbb);
    c_0(tt,n,s); 
    return 0;
}
/* display the schedule for active day and channel */
int c_30(int tt, int *n, char *s) 
{
    dump_sch(get_schedule(bbb,edit.day-1,edit.channel));
    c_0(tt,n,s); 
    return 0;
}
/* display the schedules for all days and all channels */
int c_31(int tt, int *n, char *s) 
{
    

    disp_sys();
    
    c_0(tt,n,s); 
    return 0;

}


/*****************************************************/
/*********  command parser state machine end  ********/
/*****************************************************/


int is_valid_int(const char *str)
{   
   if (*str == '-')     //negative numbers
      ++str;   
   if (!*str)           //empty string or just "-"
      return 0;   
   while (*str)         //check for non-digit chars in the rest of the string
   {
      if (!isdigit(*str))
         return 0;
      else
         ++str;
   }
   return -1;
}

int cmd_type(char *c)
{
    int     i;
    /*test for an empty command que */
    if(*c==NULL)
    {
//        printf("\nNULL buffer\n");
        return 3;
    }
    /*test empty buffer */
    if(*c==' ')
    {
//        printf("\nempty buffer\n");
        return 3;
    }
    /* test for a keyword */
    for(i=3;i<_CMD_TOKENS;i++)
    {   
        if(strlen(c) == strlen(keyword[i]))
            if(strncmp(c,keyword[i],strlen(c))==0)
                return i;
    }
    /* test for a quoted string*/
    if(*c==_QUOTE)
        return 1;

    /* test for a integer */
    if(is_valid_int(c))
    {
        return 0;
    }
    
    /* unrecognized token */
    return 2;            
}

void cmd_fsm(char *token,int *state)
{
    static int         tt, num,*n_ptr;         
    static char        *s_ptr;

    tt = cmd_type(token);
//    printf("cmd_fsm called: token <%s>, token type <%i>, state <%i>\n",token,tt, *state);
    if((tt==1)||(tt==2))
    {
        n_ptr = NULL;
        s_ptr = token;
    }
    else if(tt==0)
    {
        sscanf(token,"%u",&num);
        n_ptr = &num;
        s_ptr = NULL;
    }
//    else if(tt==6)
//    {
//        tt=3;
//        n_ptr = NULL;
//        s_ptr = token;        
//    }
    else
    {
        num = tt;
        n_ptr = &num;
        s_ptr = NULL;
    }
//    printf("call cmd_action[%i][%i](<%i>,<%i>,<%s>)\n",tt,*state,tt,*n_ptr,s_ptr);    
    if(cmd_action[tt][*state](tt,n_ptr,s_ptr)==0) 
        *state = cmd_new_state[tt][*state];
    else
    {
        printf("*** error returned from action routine\n");
    }
    return;
}
/* build prompt */
char *build_prompt(char *b,int tt)
{
    char    temp[_PROMPT_BUFFER], *hold_b;
    int     ns;

    hold_b = b;
    ns = cmd_new_state[tt][cmd_state];  //set prompt for new state
    switch(ns)
    {
        case 0:
            strcat(b,"enter a command");
            break;
        case 1:
            strcat(b,"editing channels\nenter <#> to edit channel or <display> <load> <save> channel information");
            break;
        case 2:
            sprintf(temp,"%i",edit.channel);
            strcat(b,"editing channel ");
            strcat(b,temp);
            break;        
        case 3:
            sprintf(temp,"%i",edit.channel);
            strcat(b,"editing channel ");
            strcat(b,temp);
            strcat(b," name - enter name in quotes");
            break;
        case 4:
            sprintf(temp,"%i",edit.channel);
            strcat(b,"editing channel ");
            strcat(b,temp);
            strcat(b," control mode - 0-manual, 1-time, 2-time & sensor");
            strcat(b,temp);
            break;
        case 5:
            sprintf(temp,"%i",edit.channel);
            strcat(b,"editing state for channel ");
            strcat(b,temp);
            strcat(b," state - enter: <on>|<off>");
            break;
        case 6:
            strcat(b,"editing schedules for channel ");
            sprintf(temp,"%i",edit.channel);
            strcat(b,temp);
            strcat(b,"\nenter day #, Sun=1 ...  Sat=7");
            break;
        case 7:
            strcat(b,"editing ");
            strcat(b,day_names_long[edit.day-1]);
            strcat(b," schedules for channel ");
            sprintf(temp,"%i",edit.channel);
            strcat(b,temp);
            printf("%s\n",b);
            b = hold_b;
            *b = '\0';
            dspl_sch(bbb,edit.day,edit.channel);
            strcat(b,"\nenter time <HH:MM>");
            break;
       case 8:
            strcat(b,"save schedule buffer?\n>>");
            // sprintf(temp,"%i",edit.channel);
            // strcat(b,"editing schedule for edit.channel ");
            // strcat(b,temp);
            // sprintf(temp,", %s ",day_names[day-1]);
            // strcat(b,temp);
            // dump_sch_recs(dio_cb.dio.sch, edit.channel, day);
            // strcat(b," - enter time HH:MM to edit schedule record");
            break;
        case 9:
            strcat(b,"editing ");
            strcat(b,day_names_long[edit.day-1]);
            strcat(b," schedules for edit.channel ");
            sprintf(temp,"%i",edit.channel);
            strcat(b,temp);
            strcat(b," hour ");
            sprintf(temp,"%i",edit.hour);
            strcat(b,temp);
            strcat(b," enter minute");
            break;
        case 10:
            strcat(b,"editing ");
            strcat(b,day_names_long[edit.day-1]);
            strcat(b," schedules for edit.channel ");
            sprintf(temp,"%i",edit.channel);
            strcat(b,temp);
            printf("%s\n",b);
            b = hold_b;
            *b = '\0';
            dspl_sch(bbb,edit.day,edit.channel);
            strcat(b,"enter action for ");
            sprintf(temp,"%i",edit.hour);
            strcat(b,temp);
            strcat(b,":");
            sprintf(temp,"%i",edit.minute);
            strcat(b,temp);
            strcat(b," <on>|<off>|<delete>");
            // sprintf(temp,", %s %i:%i",day_names[day-1],hour,minute);
            // strcat(b,temp);
            // dump_sch_recs(dio_cb.dio.sch, edit.channel, day);
            break;
        case 11:
            strcat(b,"editing ");
            strcat(b,day_names_long[edit.day-1]);
            strcat(b," schedule for channel ");
            sprintf(temp,"%i",edit.channel);
            strcat(b,temp);
            strcat(b," ");
            sprintf(temp,"%i",edit.hour);
            strcat(b,temp);
            strcat(b,":");
            sprintf(temp,"%i",edit.minute);
            strcat(b,temp);
            strcat(b," enter state <on>|<off>");
            break;
        case 12:
            sprintf(temp,"%i",edit.channel);
            strcat(b,"editing schedule for channel ");
            strcat(b,temp);
            sprintf(temp," all days %i:",edit.hour);
            strcat(b,temp);
            strcat(b," - enter minute");
            break;
        case 13:
            sprintf(temp,"%i",edit.channel);
            strcat(b,"editing schedule for channel ");
            strcat(b,temp);
            sprintf(temp," all days %i:%i",edit.hour,edit.minute);
            strcat(b,temp);
            break;
        default:
            printf("\n***\n****\nthe world has ended - new state <%i>\n***\n***\n",ns);
    }
    return b;
}
void reset_active(void)
{
    edit.day = rtc_cb.rtc.td_buffer.dow;
    edit.hour = 0;
    edit.minute = 0;
    return;
}
    