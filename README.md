Pcon
====

This code is under constructions it is not stable

This project uses a Parallax Propeller C3 to drive a Parallax Digitial IO Board.  There are 8 channels.  Each channel can controll a 120v 8 amp load.  It aslo uses a DS3231 real time clock.

Hardware:
	Parallax - C3 microcontroller 
	Parallax - Digital IO Board
	adafruit - ChronoDot real time clock module, based on the DS3231 temperature compensated RTC (TCXO).

Language:
	C Propgcc

Architecture:



	There are 8 channels.  Each channel can controll a 120v 8 amp load.  A channel 
can be controlled manually from the command line or it can  be set to change state
at a given time.  There can be different schedules for different days of the week.

Propeller Pins
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
