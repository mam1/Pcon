################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Pcon.c \
../channel.c \
../char_fsm.c \
../cmd_fsm.c \
../old_schedul.c \
../schedule.c \
../test_schedule.c 

OBJS += \
./Pcon.o \
./channel.o \
./char_fsm.o \
./cmd_fsm.o \
./old_schedul.o \
./schedule.o \
./test_schedule.o 

C_DEPS += \
./Pcon.d \
./channel.d \
./char_fsm.d \
./cmd_fsm.d \
./old_schedul.d \
./schedule.d \
./test_schedule.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


