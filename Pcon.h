#ifndef PCON_H_
#define PCON_H_

#define _major_version 0
#define _minor_version 8

/* propeller io pin assignments */
#define _DataIO          4              // DIO -DIN & DATA_RLY
#define _Clock           5              // DIO SCLK_IN & SCLK_RLY
#define _HC165           6              // DIO LOAD_IN Pin
#define _HC595           7              // DIO LAT_RLY Pin
#define _toggle_VGA     15              // toggle the port B header to VGA
#define _VGA_start      16              // VGA uses pins 16-
#define _data_pin       26              // keyboard */
#define _clk_pin        27              // keyboard */
#define _rtcClockPin    28              //i2c - DS3231 clock
#define _rtcDataPin     29              //i2c - DS3231 data

/* cog parameters */
#define _STACK_SIZE_RTC     64
#define _STACK_SIZE_DIO     128

/* character parser fsm */
#define _CHAR_TOKENS     8
#define _CHAR_STATES     4

/* command parser fsm */
#define _CMD_TOKENS     22
#define _CMD_STATES     14 

/* channel parameters */
#define _NUMBER_OF_CHANNELS 8
#define _CHANNEL_NAME_SIZE  15
#define _CHANNEL_NAME_BUFFER_SIZE  30
#define _CHANNEL_FILE_NAME_SIZE 30

#define _DAYS_PER_WEEK      7

/* sd card file parameters */
//#define _FILE_NAME_SIZE         20
#define _F_PREFIX               "d007"
#define _F_SYSTEM_SUFIX         ".sys"
#define _F_SCHEDULE_SUFIX       ".sch"
#define _F_CHANNEL_SUFIX        ".ch"

/* schedule parameters */
#define _DAYS_PER_WEEK          7
#define _NUMBER_OF_SCHEDULES    _NUMBER_OF_CHANNELS * _DAYS_PER_WEEK
#define _MAX_SCHEDULE_RECS  10
#define _SCHEDULE_FILE_NAME_SIZE    30
#define _STATE_MASK         B32(10000000,00000000,00000000,00000000)                 
#define _KEY_MASK           B32(01111111,11111111,11111111,11111111) 
#define _SB volatile uint32_t[_NUMBER_OF_CHANNELS][_MAX_SCHEDULE_RECS+1] //schedule buffer


/* buffers */
#define _INPUT_BUFFER       128
#define _TIME_STRING_BUFFER 40
#define _TOKEN_BUFFER       128
#define _VCMD_BUFFER        128
#define _PROMPT_BUFFER      128

/* key codes */
#define _ESC        27
#define _SPACE      32
#define _COLON      58
#define _SLASH      47
#define _BS         8
#define _QUOTE      34
#define _CR         13
#define _NO_CHAR    255


/*******************/
/* data structures */
/*******************/

/* action routines for the cmd fsm */
typedef int(*CMD_ACTION_PTR)(int, int *,char *); 

/* channel control record (ccr) */
typedef struct  
{
    int                 c_mode; //Control mode: 0-manual, 1-time, 2-time & sensor
    int                 s_mode; //Schedule mode: 0-by dow, 1-by week
    int                 state;  //Channel State: 0-off, 1-on
    char                name[_CHANNEL_NAME_SIZE];
}CCR;

/* time/date buffer */
typedef struct{
    uint8_t     sec;
    uint8_t     min;
    uint8_t     hour;
    uint8_t     day;
    uint8_t     month;
    uint8_t     year;
    uint8_t     dow;
}TD_BUF;


/* rtc cog control block */
typedef volatile struct 
{
    int                 tdb_lock;   //lock ID for time date buffer
    int                 abort;      //!= 0 cog requests a system abort,value = error code
    TD_BUF              td_buffer;  //time, date & dow stored as uint8_t 
}RTC_CB;

/*  data structure for each node in the schedule array */
typedef struct{
    int    cnt;
    uint32_t    r[_MAX_SCHEDULE_RECS];
}SCH;

/* dio cog control block */
typedef volatile struct 
{
    int        tdb_lock;   //lock ID for time date buffer
    int        cca_lock;   //lock ID for the channel control array
    int        sch_lock;   //lock ID for schedule records
    int        abort;      //!= 0 cog requests a system abort,value = error code 
    TD_BUF     *td_ptr;    //pointer to the time date buffer
    CCR        cca[_NUMBER_OF_CHANNELS];                 //channel control array
//    SCH        sch[_NUMBER_OF_CHANNELS][_DAYS_PER_WEEK]; //schedule for each channel for current day
    uint32_t   sch[_NUMBER_OF_CHANNELS][_MAX_SCHEDULE_RECS+1]; //schedule buffer
}DIO_CB;
#endif


