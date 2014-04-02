###Pcon - multi channel programmable controller
- - - - - - - - - 
#####Code is under construction it is not stable
- - - - - - - - -
Channels can be controlled by time of day, time of day and a sensor value or manually. There are different schedules for each day of the week. The application can be configured to drive 1 to 8 channels using the preprocessor variable *_NUMBER_OF_CHANNELS* in Pcon.h.  It can be configured to drive IO pins or to drive the Parallax Digital IO Board serial interface. The the preprocessor variable *_DRIVEN* in Pcon.h controls which interface is used. 

In one implementation I am driving AQY212GH PhotoMOS relays connected to 5 propeller IO pins.  The PhotoMOS relays are rated at 60 V AC/DC 1.1 A. These relays come in a 4-pin DIP package.  They work great to control 24 V zone valves. The second instance is driving a Parallax Digital IO board which can control 8, 120 VAC 8 A loads and accept input from 8 sensors. 

The code was developed on a Parallax C3 making use of flash memory and the SD card.  A DS3231 real time clock module is connected to the C3's i2c bus (pins 28,29) to provide a time reference. The DS3231 module, the AQY212GH relays and terminals for the external connections are mounted on an additional board connected to the C3.
####Language:
C - Propgcc
####Hardware:
* Parallax - C3 micro controller 
* Parallax - Digital IO Board (DIOB), Sharp solid state relays part# S202S02F
* adafruit - ChronoDot real time clock module, based on a DS3231.
* Newark   - AQY212GH PhotoMOS relays

####Command processor functions:
* name channels  
* manually control channel state  
* set channel control mode (manual, time, time & sensor)
* load/save channel control information to SD card  
* create and maintain schedules for each channel
* load/save schedules to SD card
* display current time and date
* display schedules

Because the command processor is implemented by a state machine there is a lot of flexibility in they way tokens can be entered.  Entering a '?' will display the current state of the command fsm and a list of commands and tokens (INT for a integer and STR for a quoted string) that are valid in that state. Tokens can be entered individually or strung together. If the fsm requires additional information a prompt will be displayed, however the main loop will not wait for input.

####Schedules:
A schedule is a list of times and corresponding states.  A channel that is controlled by time will be a list of times and states.  For example, a schedule of:

* 1:00  on
* 13:00 off

will result in the channel turning on at 1:00AM and off at 1:00PM.  If the current time is between 13:00 and 24:00 or between 0:0 and 13:00 the channel will be off.  Between 1:00 and 13:00 it will be on.  

* 1:00  on
* 13:00 off
* 18:00 on

will result in the channel turning on at 1:00, off at 13:00 and on at 18:00.  If the current time is between 1:00 and 13:00 the channel will be on, between 13:00 and 18:00 it will be off, between 18:00 and 0:0 it will be on and between 0:0 and 13:00 it will also be on. 

The maximum number of records for a schedule is configured by setting the preprocessor variable *_MAX_SCHEDULE_RECS* .

Schedules are stored on the SD card. 

A schedule record is 32 bits.  A schedule is implemented as an array of 32 bit unsigned integers. The schedule key is the number of minutes past midnight (0 - 1440).  The first element in the array contains the number of records to follow.  The following bits are parsed as follows:

* bit 32 - state
* bit 31-17 -  key
* bit 16-1 sensor value

####Architecture:
A schedule is a vector of 32 bit unsigned integers. The length is configured by the preprocessor variable *_MAX_SCHEDULE_RECS* .  These are fixed length vectors. I have an alternate implementation using linked lists but his approach is much simpler and has less chance of memory leaks. Since I am already using xmmc size is not a big deal.

The first 32 bits of the vector contain the number of active records in the schedule. The following 32 bit "records" are interpreted as:

* bit 31 (high bit) .... state (0=off, 1=on)
* bit 30-16 ............ key (number of minutes past midnight, 0-1440)
* bit 15-0 ............. sensor value (0-65536)

The schedules for each (day,channel) tuple are combined into one contiguous vector of unsigned integers. The schedule for a particular tuple is accessed via pointer offsets. 

The control part of the application uses 2 cogs, "rtc.cogc" and "dio.cogc".  The rtc cog talks to the DS3231, converts BCD to decimal and updates a time/date buffer in hub memory.  The rtc cog contains i2c bit banging code because the library code is too large to run from a cog and because the DS3231 requires clock stretching if the code is running in a cog. 

The dio cog reads the time from the buffer in hub memory.  Once a minute the dio cog creates a control byte.  Each bit controls the state of the corresponding channel. The bits are set based on the current time, the schedule for the channel and the control information for the channel. The dio cog can be forced to update by use of a flag in hub memory. The control byte is sent to either the Parallax Digital IO Board or the PhotoMos relays depending on the application configuration.

The schedule and control information are stored on a SD card and loaded into hub memory at initialization or on command.  

The complex part of the application is the command processor.  XMMC is required because of the code size.  It uses a finite state machine (fsm) to parse the input character stream into tokens and a second fsm to process the tokens.  This type of command processor is probably inappropriate for a micro controller, however no one is paying me anymore so I can do what I want. 

The command processor loops checking to see if a character has been typed. Input buffering has been disabled so the read is non blocking.

**If a character is present**, unless it is an ESC, it is passed to the first fsm char_fsm). An ESC will clear all buffers and reset both state machines.  char_fsm pareses the input stream into tokens and pushes them on to FIFO stack.  A CR will cause char_fsm to pass the stack of tokes to the command processor fsm (cms_fsm). When cmd_fsm finds a full token stack it pops tokens off the stack until it is empty.

**If a character is not found** the code checks to see if the the cogs have sent any messages.  

####SD Files:
Schedules and persistent channel information (name, control mode, state) are stored in files on a SD card. The file names are generated in the following format: 
>   s<tag>d<day #>c<chanel #>.SCH
    s<tag>  .

The tag is a user supplied 3 digit number, it is currently implemented as a preprocessor variable. 



####Propeller Pins:

    0 
    1
    2

        3 - 
        4 - dio, DIN
        4 - dio, DATA_RLY
        5 - dio, SCLK_IN
        5 - dio, SCLK_RLY
        6 - dio, LOAD_IN
        7 - dio, LAT_RLY

        3 - 
        4 - dio, DIN
        4 - dio, DATA_RLY
        5 - dio, SCLK_IN
        5 - dio, SCLK_RLY
        6 - dio, LOAD_IN
        7 - dio, LAT_RLY

    8 - SPI
    9 - SPI, SPI_MOSI
    10 - SPI, SPI_MISO
    11 - SPI, SPI_CLK
    12 - composite video
    13 - composite video
    14 - composite video
    15 - toggle Port B header
    16-23 - VGA 
    24 - audio PWM
    25 - SPI
    26 - PS/2_DATA
    27 - PS/2_CLOCK. 
    28 - i2c
    29 - i2c
    30 - USB
    31 - USB
