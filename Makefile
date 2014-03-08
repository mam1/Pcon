# #########################################################
# this makefile allows building and cleaning Pcon
# #########################################################

# Propeller Demo include
PROPLIB = ./common

CFLAGS = -Os -mxmmc -Wall -m32bit-doubles -fno-exceptions -std=c99
NM = propeller-elf-nm

#
# objects for this program
#
MODEL = xmmc
BOARD = C3F
NAME = Pcon
OBJS = Pcon.o channel.o schedule.o char_fsm.o cmd_fsm.o rtc_1.cog dio_2.cog


all: $(NAME).elf
	@echo "Please execute next commands:"
	@echo 'setenv PATH /usr/local/greenhills/mips5/linux86:$$PATH'


include $(PROPLIB)/xmmcdemo.mk

#
# do a partial link of all driver code
#


xmmc/rtc.cog: rtc.cogc
	$(CC) -r -Os -mcog -o $@ -xc $<

xmmc/dio.cog: dio.ccogc
	$(CC) -r -Os -mcog -o $@  -xc $<
#
# We have to avoid conflicts between symbols in the main C program and
# symbols in the local cog C programs. We do this by using objcopy to
# turn all the global symbols in the cog .text segment into local symbols
# (that's what --localize-text does).
#

%.cog: %.o
	$(OBJCOPY) --localize-text $^ $@

