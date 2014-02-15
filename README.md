Pcon 
==== 
*Code is under construction it is not stable*

This project uses a Parallax Propeller C3 to drive a Parallax Digital IO Board (DIOB).  The DIOB has 8 channels.  Each channel can control a 120v 8 amp load.  A DS3231 real time clock module is connected to the C3's i2c bus (pins 28,29) to provide a time reference. 

Command processor:
------------------
*    name channels
*    manually control channel state
*   set channel control mode (manual, time, time & sensor)
*    create and maintain schedules for each cahnnel
*    load/save schedules to SD card
*   load/save channel control information to SD card

 >Because the command processor is implemented by a state machine there is a lot of flexibility in they way commands can be entered.  It should be noted that the same command could have different results based on the context in which it is used.

Language:
---------

	C Propgcc

Architecture:
-------------
    The control part of the application uses 2 cogs, "rtc.cogc" and 
    "dio.cogc".  The rtc cog talks to the DS3231 and updates a time/date buffer in hub memory.  It also sets a trigger (once a minute) in hub memory to to let the dio cog know that it should update the DIOB based on the current time, the schedule for the channel and the control information for the channel.  The schedule and control information are stored on a SD card and loaded into hub memory at initialization or on command.

    The more complex part of the application is the command processor.  XMMC is required because of the code size.  It uses a fsm to parse the input character stream into tokens and a second fsm to process the tokens.  This type of command processor is probably inappropriate for a micro controller, however no one is paying me anymore so I can do what I want.

Data Structures:
----------------

Files:
------

    Schedules are stored on a SD card. There is one file for each (day,channel) tuple.

Hardware:
---------
*    Parallax - C3 micro controller 
*    Parallax - Digital IO Board (DIOB)
*    adafruit - ChronoDot real time clock module, based on the DS3231  temperature compensated RTC (TCXO).

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
