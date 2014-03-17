/**
 * This is the main pc program file.
 */
#include <stdio.h>
#include <propeller.h>
#include <unistd.h>
#include "Pcon.h"
#include "channel.h"
#include "schedule.h"
#include "char_fsm.h"


/*************************** drivers  ********************************/
extern _Driver _FullDuplexSerialDriver;
extern _Driver _FileDriver;
_Driver *_driverlist[] = {
  &_FullDuplexSerialDriver,
  &_FileDriver,
  NULL
};
/***************************** external ******************************/
// extern char    *day_names[];
 extern uint32_t       bbb[_SCHEDULE_BUFFER];
/***************************** globals ******************************/
    int                 char_state, cmd_state; //current state
    char                input_buffer[_INPUT_BUFFER], *input_buffer_ptr;
    // char                *file_set_prefix[_SCHEDULE_NAME_SIZE];
/***************** global code to text conversion ********************/
char *day_names_long[7] = {
     "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"};
char *day_names_short[7] = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
char *onoff[2] = {"off"," on"};
char *con_mode[3] = {"manual","  time","time & sensor"};
char *sch_mode[2] = {"day","week"};
/**************** real time clock cog stuff **************************/
/* beginning and ending addresses of the code for the rtc cog */
  extern unsigned int _load_start_rtc_cog[];
  extern unsigned int _load_stop_rtc_cog[]; 
/* allocate control block & stack for rtc cog */
struct {
    unsigned stack[_STACK_SIZE_RTC];
    volatile RTC_CB rtc;
} rtc_cb;
/* start rtc cog */
int start_rtc(volatile void *parptr)
{ 
    int size = (_load_stop_rtc_cog - _load_start_rtc_cog)*4;//code size in bytes
    printf("rtc cog code size %i bytes\n",size);
    unsigned int code[size];  //allocate enough HUB to hold the COG code
    memcpy(code, _load_start_rtc_cog, size); //assume xmmc
    return cognew(code, parptr);
}
//**************** digital IO board cog stuff ************************/
/* begining and ending address of the code for the dio cog */
 extern unsigned int _load_start_dio_cog[];
 extern unsigned int _load_stop_dio_cog[];
/* allocate control block & stack for dio cog */
struct {
    unsigned stack[_STACK_SIZE_DIO];
    volatile DIO_CB dio;
} dio_cb;
/* start dio cog */

int start_dio(volatile void *parptr)
{ 
    int size = (_load_stop_dio_cog - _load_start_dio_cog)*4;
    printf("dio cog code size %i bytes\n",size);
    unsigned int code[size];
    memcpy(code, _load_start_dio_cog, size); //assume xmmc
    return cognew(code, parptr);
}

/******************************* support functions *********************/
int ckeck_abort(void)
{
        if(rtc_cb.rtc.abort != 0)
        {
            printf("\n\n*** rtc cog issued an abort\n ACK missing - message number %i\n",rtc_cb.rtc.abort);
            return 1;
        }

        if(dio_cb.dio.abort != 0)
        {
            printf("\n\n*** dio cog issued an abort\n message number %i\n",dio_cb.dio.abort);
            return 1;
        }

    return 0;
}
int sd_setup(void)
{
    if(init_channel_data())    //create a channel file if one is not present
    {
        printf("**** init_channel_data aborted application ***\n");
        return 1;
    }
    if(load_channel_data())    //load data from channel file into cca
    {
        printf("**** load_channel_data aborted application ****\n");
        return 1;
    }
    
    if(init_sch(bbb))   //create a schedule file if it is not present
    {
        printf("**** init_sch aborted application ****\n");
        return 1;
    }
    
    if(read_sch(bbb))    
    {
        printf("**** read_sch aborted application ****\n");
        return 1;
    }
    
    return 0;
}
/********************************************************************/
/************************** start main  *****************************/
/********************************************************************/
 int main(void)
{
    static int          cog,hold_day;
    static char         tbuf[_TOKEN_BUFFER];
    char                c;   
/************************ initializations ****************************/
    sleep(1);   //wait for the serial terminal to start
/* display system info on serial terminal */
    printf("\n*** Pcon  %i.%i ***\n\n",_major_version,_minor_version);
    #if _DRIVEN == _DIOB
        printf("system is configured to drive a Parallax Digital IO Board\n");
    #else
        printf("system is configured to drive 5 IO pins\n");
    #endif
/* build file set prefix */
    // strcat(file_set_prefix,_F_PREFIX);
    // strcat(file_set_prefix,_FILE_SET_ID);
    // printf("file set prefix <%s>\n",file_set_prefix);
/* check out the SD card */
    if(sd_setup())
    {
        printf("**** sd_setup aborted application ****\n");
        return 1;
    }    
/* start monitoring the real time clock DS3231 */
    rtc_cb.rtc.tdb_lock = locknew();
    lockclr(rtc_cb.rtc.tdb_lock);
    cog = start_rtc(&rtc_cb.rtc);
    if(cog == -1)
    {
        printf("** error attempting to start rtc cog\n  cognew returned %i\n\n",cog);
        return 1;
    }     
    printf(" DS3231 monitored by code running on cog %i\n",cog);
/* setup  the dio control block */
    dio_cb.dio.tdb_lock = rtc_cb.rtc.tdb_lock;
    dio_cb.dio.cca_lock = locknew();
    lockclr(dio_cb.dio.cca_lock);
    dio_cb.dio.sch_lock = locknew();
    lockclr(dio_cb.dio.sch_lock);
    dio_cb.dio.update_ptr = &(rtc_cb.rtc.update);
    dio_cb.dio.td_ptr = &(rtc_cb.rtc.td_buffer);
    *dio_cb.dio.update_ptr = 0;

/* start the dio cog  */
    cog = start_dio(&dio_cb.dio);
    if(cog == -1)
    {
        printf("** error attempting to start dio cog\n  cognew returned %i\n\n",cog);
        return 1;
    }
    #if _DRIVEN == _DIOB
            printf(" DIO Board controlled by code running on cog %i\n",cog);
    #else
         printf(" relays controlled by code running on cog %i\n",cog);

    #endif      
/* set up unbuffered nonblocking io */
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    stdin->_flag &= ~_IOCOOKED;
    stdin->_flag |= _IONONBLOCK;
/* initialize fsm(s) parameters */
    input_buffer_ptr = input_buffer;//setup so actions routines can mess the buffer
    cmd_state = 0;                  //set initial command parser state
    char_state = 0;                 //set initial caracter parser state
    hold_day = -1;                  //force schedule load first time through the main loop
    *input_buffer = ' ';            //load a a blank into the buffer to force first prompt
    process_buffer();
    printf("initialization complete\n");

/************************************************************/
/********************** main processing loop ****************/
/************************************************************/
    while(1)
    {
        /* check for problems */
        if(ckeck_abort())
            return 1;
        /* reload schedule buffer if the day has changed */    
        if(rtc_cb.rtc.td_buffer.dow != hold_day)
        {
            /*
            hold_day = rtc_cb.rtc.td_buffer.dow;
            if(load_schedule_data(dio_cb.dio.sch,hold_day-1))
            {
                printf("**** load_schedule_data abborted application ****\n");
                return 1;
            }
            printf("schedules for %s loaded\n\n",day_names_long[rtc_cb.rtc.td_buffer.dow-1]);
            // dump_sch_recs(dio_cb.dio.sch,0,2);
            rtc_cb.rtc.update = 1;  //force update
            */
        }

        /* check the token stack */
        while(pop_cmd_q(tbuf))
        {
            cmd_fsm(tbuf,&cmd_state);   //cycle cmd fsm until queue is empty
        }
        /* grab a character from the keyboard if one is present  */
        c = fgetc(stdin);     //nonblocking read
        /*  process input char */
        if(c!=_NO_CHAR)       
        {
            fputc(c, stdout);       // echo char 
            if(c==_CR) fputc(_CR, stdout);   //second CR after uer input
            char_fsm(char_type(c),&char_state,&c);  //cycle fsm
        }
    };
    printf("\nnormal termination\n\n");
    return 0;
}
