Pcon
====
Code is under construction it is not stable

This project uses a Parallax Propeller C3 to drive a Parallax Digitial IO Board.  There are 8 channels.  Each channel can controll a 120v 8 amp load.  Each channel can be controled manually or by a daily schedule of "on" and "off" times.  A DS3231 real time clock module is connected to the C3's i2c bus (pins 28,29).

Language:

	C Propgcc

Architecture:

    The control part of the application is simple.  One cog talks to the rtc and updates a time/date buffer in hub memory.  It also sets a trigger (once a minute) in hub memory to to let a seond cog know that it should update the DIOB based on the current time and the schedule for the channel.

    The more complex part of the application is the command processor.  It requires XMMC because of its size.  It uses a fsm to parce the input character stream into tokens and a second fsm to process the tokens.

Data Structures:

Files:

    Schedules are stored on a SD card. There is one file for each (day,channel) tuple.

Hardware:

    Parallax - C3 microcontroller 
    Parallax - Digital IO Board (DIOB)
    adafruit - ChronoDot real time clock module, based on the DS3231 temperature compensated RTC (TCXO).

Propeller Pins:

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
