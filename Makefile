# #########################################################
# this makefile allows building and cleaning Pcon
# #########################################################

# Propeller Demo include
PROPLIB = .

IDIR =     ../include
CC=propeller-elf-gcc
MODEL=xmmc
BOARD = C3F
CFLAGS=-Os -m$(MODEL) -I$(IDIR)
LDFLAGS=-m$(MODEL)
OBJDIR=./
SRCDIR=./
OBJS = vgademo.o vga.o draw.o text.o text2.o
HDRS =

NM = propeller-elf-nm

#
# objects for this program
#

NAME = Pcon
OBJS = \
Pcon.o \
channel.o \
char_fsm.o \
cmd_fsm.o \
schedule.o \
rtc.cogc

all: $(NAME).elf

include $(PROPLIB)/xmmc.mk

#
# do a partial link of all driver code
#

# # Note that calling our output file *_1.cog ensures that it
# is placed in the .coguser1 section by the main program's
# linker script; similarly *_2.cog would be placed in .coguser2,
# and so on

Pcon_1.o: Pcon_cog.c
	$(CC) -r -Os -mcog -o $@ $<


#
# We have to avoid conflicts between symbols in the main C program and
# symbols in the local cog C programs. We do this by using objcopy to
# turn all the global symbols in the cog .text segment into local symbols
# (that's what --localize-text does).
#

%.cog: %.o
	$(OBJCOPY) --localize-text $^ $@
