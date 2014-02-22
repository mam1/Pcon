###Pcon
- - - - - - - - - 
#####Code is under construction it is not stable
- - - - - - - - -
Eight channel programmable controller. Channels can be controlled by time, time and a sensor value or manually. This project uses a Parallax Propeller C3 to drive a Parallax Digital IO Board (DIOB).  The DIOB has 8 channels.  Each channel can control a 120v 8 amp load.  A DS3231 real time clock module is connected to the C3's i2c bus (pins 28,29) to provide a time reference. 
- - - - - - - - - 
####Language:
---------
C Propgcc
---------
####Architecture:
-------------
The control part of the application uses 2 cogs, "rtc.cogc" and "dio.cogc".  The rtc cog talks to the DS3231 and updates a time/date buffer in hub memory.  It also sets a trigger (once a minute) in hub memory to let the dio cog know that it should update the DIOB based on the current time, the schedule for the channel and the control information for the channel.  The schedule and control information are stored on a SD card and loaded into hub memory at initialization or on command.

The complex part of the application is the command processor.  XMMC is required because of the code size.  It uses a fsm to parse the input character stream into tokens and a second fsm to process the tokens.  This type of command processor is probably inappropriate for a micro controller, however no one is paying me anymore so I can do what I want.
-------------
####Command processor:
------------------
* name channels  
* manually control channel state  
* set channel control mode (manual, time, time & sensor)
* load/save channel control information to SD card  
* create and maintain schedules for each channel
* load/save schedules to SD card    
 
 Because the command processor is implemented by a state machine there is a lot of flexibility in they way tokens can be entered.  Entering a '?' will display the current state of the fsm and a list of commands and tokens (INT for a integer and STR for a quoted string) valid in that state. Commands can be entered individually or strung together. If the fsm requires addition information to take an action a prompt will be displayed.  




####Data Structures:
----------------
####Files:
------

Schedules and the channel control information are stored on a SD card. There are 57 schedule files, one file for each (day,channel) tuple. The file names are generated in the following format: (s<tag>d<day #>c<chanel #>.SCH).  The tag is a user supplied 3 digit number, it is currently implemented as a preprocessor variable.

Channel information (name, control mode, state) for all channels is stored in a single file.  The file name is generated in the following format: (s<tag<>.CH).

Hardware:
---------
* Parallax - C3 micro controller 
* Parallax - Digital IO Board (DIOB)
* adafruit - ChronoDot real time clock module, based on a DS3231.

Propeller Pins:
---------------

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
