#ifndef PCON_H_
#define PCON_H_

#define _major_version 1
#define _minor_version 0

#define _FILE_SET_ID            "000"


/* configuration options */
#define _DIOB       1       //configure to drive Parallax Digital IO Board
#define _212GH      2       //configure to drive 5 AYQ212GH relays

#define _DRIVEN     _DIOB

/* propeller io pin assignments */
#if _DRIVEN == _DIOB
    /* if configured to drive the Parallax Digital IO board */
    #define _DataIO          4              // DIO -DIN & DATA_RLY
    #define _Clock           5              // DIO SCLK_IN & SCLK_RLY
    #define _HC165           6              // DIO LOAD_IN Pin
    #define _HC595           7              // DIO LAT_RLY Pin
#else
    /* if configure to drie AYQ212GH relays */
    #define _RLY1           3              // relay 1
    #define _RLY2           4              // relay 2
    #define _RLY3           5              // relay 3
    #define _RLY4           6              // relay 4
    #define _RLY5           7              // relay 5
#endif
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
#define _CMD_TOKENS     38
#define _CMD_STATES     26 

/* channel parameters */
#if _DRIVEN == _DIOB
    #define _NUMBER_OF_CHANNELS 8 
#else
    #define _NUMBER_OF_CHANNELS 5
#endif
#define _CHANNEL_NAME_SIZE          20
#define _CHANNEL_NAME_BUFFER_SIZE   30
#define _CHANNEL_FILE_NAME_SIZE     30

/* sd card file parameters */
#if _DRIVEN == _DIOB
    #define _F_PREFIX           "dio"
#else
    #define _F_PREFIX           "rly"
#endif

#define _F_SCHEDULE_SUFIX       ".sch"
#define _F_CHANNEL_SUFIX        ".ch"

/* schedule parameters */
#define _DAYS_PER_WEEK          7
#define _MAX_SCHEDULE_RECS      10
#define _SCHEDULE_NAME_SIZE     40
#define _STATE_MASK             B32(10000000,00000000,00000000,00000000)                 
#define _KEY_MASK               B32(01111111,11111111,11111111,11111111) 
#define _BYTES_PER_CHANNEL      (_MAX_SCHEDULE_RECS + 1) * 4
#define _BYTES_PER_DAY          _BYTES_PER_CHANNEL * _NUMBER_OF_CHANNELS

/* buffers */
#define _INPUT_BUFFER           128
#define _TIME_STRING_BUFFER     40
#define _TOKEN_BUFFER           128
#define _VCMD_BUFFER            128
#define _PROMPT_BUFFER          128
#define _SCHEDULE_BUFFER        (_BYTES_PER_DAY * _DAYS_PER_WEEK)/4

/* key codes */
#define _ESC        27
#define _SPACE      32
#define _COLON      58
#define _SLASH      47
#define _BS         8
#define _QUOTE      34
#define _CR         13
#define _NO_CHAR    255

/****************** functions *****************/
void disp_sys(void);

/*******************/
/* data structures */
/*******************/

/* action routines for the cmd fsm */
typedef int(*CMD_ACTION_PTR)(int, int *,char *); 

/* channel control record (ccr) */
typedef struct  
{
    int                 c_mode; //Control mode: 0-manual, 1-time, 2-time & sensor
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
    int                 abt;        //!= 0 cog requests a system abort,value = error code
    int                 update;     //trigger update flag, 1=wait, 0=update 
    TD_BUF              td_buffer;  //time, date & dow stored as uint8_t 
}RTC_CB;
 
/*  data structures for indexing the schedule array */
typedef struct
 {
    uint32_t            rec[_MAX_SCHEDULE_RECS+1];
 } SCH;

typedef struct
 {
    SCH                 sch[_NUMBER_OF_CHANNELS];
 } DAY;


/* dio cog control block */
typedef volatile struct 
{
    int        tdb_lock;    //lock ID for time date buffer
    int        cca_lock;    //lock ID for the channel control array
    int        sch_lock;    //lock ID for schedule records
    int        abt;         //!= 0 cog requests a system abort,value = error code
    int        *update_ptr; //pointer to trigger update flag, 1=wait, 0=update 
    TD_BUF     *td_ptr;     //pointer to the time date buffer
    uint32_t   *sch_ptr;    //schedule buffer pointer
        CCR        cca[_NUMBER_OF_CHANNELS];                 //channel control array

}DIO_CB;
#endif


