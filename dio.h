/**************************************/
/*  dio.h -                           */
/*  header file for dio.cogc          */ 
/**************************************/
#ifndef _DIO_H_
#define _DIO_H_

#define dir_in(pin) ( _DIRA &= ~(1<<pin))
#define dir_out(pin) ( _DIRA |= (1<<pin))
#define pin_low(pin) ( _OUTA &= ~(1<<pin))
#define pin_high(pin) ( _OUTA |= (1<<pin))

/* forward refferences */
int test_sch(volatile uint32_t *,int,int,int,int *);



#endif