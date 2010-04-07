
/*
 * Copyright (c) 2006-2009 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/sfr_defs.h>
#include <avr/sleep.h>

#include "uart.h"

/* some mcus have multiple uarts */
#ifdef UDR0
#define UBRRH UBRR0H
#define UBRRL UBRR0L
#define UDR UDR0

#define UCSRA UCSR0A
#define UDRE UDRE0
#define RXC RXC0

#define UCSRB UCSR0B
#define RXEN RXEN0
#define TXEN TXEN0
#define RXCIE RXCIE0

#define UCSRC UCSR0C
#define URSEL 
#define UCSZ0 UCSZ00
#define UCSZ1 UCSZ01
#define UCSRC_SELECT 0
#else
#define UCSRC_SELECT (1 << URSEL)
#endif

#ifndef USART_RXC_vect
#if defined(UART0_RX_vect)
#define USART_RXC_vect UART0_RX_vect
#elif defined(UART_RX_vect)
#define USART_RXC_vect UART_RX_vect
#elif defined(USART0_RX_vect)
#define USART_RXC_vect USART0_RX_vect
#elif defined(USART_RX_vect)
#define USART_RXC_vect USART_RX_vect
#elif defined(USART0_RXC_vect)
#define USART_RXC_vect USART0_RXC_vect
#elif defined(USART_RXC_vect)
#define USART_RXC_vect USART_RXC_vect
#else
#error "Uart receive complete interrupt not defined!"
#endif
#endif

//#define BAUD 9600UL
#define BAUD 115200UL
//#define BAUD 19200UL
#define UBRRVAL (F_CPU/(BAUD*16)-1)

//#define USE_SLEEP 0
#define USE_SLEEP 1

void uart_init(uint8_t uart_speed)
{
	//Assume 16MHz
	uint16_t new_ubrr = 207; //Default is 9600bps
	if(uart_speed == 0) new_ubrr = 832; //2400
	if(uart_speed == 1) new_ubrr = 207; //9600
	if(uart_speed == 2) new_ubrr = 34; //57600
	if(uart_speed == 3) new_ubrr = 16; //115200
	//New speeds added 4-7-2010
	//1200bps is so rare, and is not on the ATmega328 datasheet that I skipped it
	//38400bps is also rare, and ubrr of 51 oddly causes errors at 16MHz, so I skipped it as well
	if(uart_speed == 4) new_ubrr = 416; //4800
	if(uart_speed == 5) new_ubrr = 103; //19200
	//if(uart_speed == 6) new_ubrr = 51; //38400

	UCSR0A = (1<<U2X0); //Double the UART transfer rate
	//Slightly more accurate UBRR calculation
	UBRR0L = new_ubrr;
	UBRR0H = new_ubrr >> 8;

#if !USE_SLEEP
    UCSRB = (1 << RXEN) | (1 << TXEN);
#else
    UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
#endif

}

void uart_putc(uint8_t c)
{
    if(c == '\n')
        uart_putc('\r');

    /* wait until transmit buffer is empty */
    while(!(UCSRA & (1 << UDRE)));

    /* send next byte */
    UDR = c;
}

void uart_putc_hex(uint8_t b)
{
    /* upper nibble */
    if((b >> 4) < 0x0a)
        uart_putc((b >> 4) + '0');
    else
        uart_putc((b >> 4) - 0x0a + 'a');

    /* lower nibble */
    if((b & 0x0f) < 0x0a)
        uart_putc((b & 0x0f) + '0');
    else
        uart_putc((b & 0x0f) - 0x0a + 'a');
}

void uart_putw_hex(uint16_t w)
{
    uart_putc_hex((uint8_t) (w >> 8));
    uart_putc_hex((uint8_t) (w & 0xff));
}

void uart_putdw_hex(uint32_t dw)
{
    uart_putw_hex((uint16_t) (dw >> 16));
    uart_putw_hex((uint16_t) (dw & 0xffff));
}

void uart_putw_dec(uint16_t w)
{
    uint16_t num = 10000;
    uint8_t started = 0;

    while(num > 0)
    {
        uint8_t b = w / num;
        if(b > 0 || started || num == 1)
        {
            uart_putc('0' + b);
            started = 1;
        }
        w -= b * num;

        num /= 10;
    }
}

void uart_putdw_dec(uint32_t dw)
{
    uint32_t num = 1000000000;
    uint8_t started = 0;

    while(num > 0)
    {
        uint8_t b = dw / num;
        if(b > 0 || started || num == 1)
        {
            uart_putc('0' + b);
            started = 1;
        }
        dw -= b * num;

        num /= 10;
    }
}

void uart_puts(const char* str)
{
    while(*str)
        uart_putc(*str++);
}

void uart_puts_p(PGM_P str)
{
    while(1)
    {
        uint8_t b = pgm_read_byte_near(str++);
        if(!b)
            break;

        uart_putc(b);
    }
}

uint8_t uart_getc()
{
    /* wait until receive buffer is full */
#if USE_SLEEP

	//During append file, we are disabling the RX interrupt, so we need to bring it back
	UCSR0B |= (1<<RXCIE0); //Enable receive interrupts

    sei();
	//#define STAT1	5
	//PORTD &= ~(1<<STAT1); //Turn off LED to save more power - if we turn off the LED before the ISR, the LED never comes on
	//I'd rather have the LED blink
	sleep_mode();
	cli();
	
	//Now that we've woken up, we assume that the UART ISR has done its job and loaded UDR into the buffer
	//We need to look at the last used spot in the buffer which is read_spot - 1
	char b;
	if(read_spot == 0)
		b = input_buffer[sizeof(input_buffer) - 1];
	else
		b = input_buffer[read_spot - 1];

    if(b == '\r')
        b = '\n';

	return b;
#else
    while(!(UCSRA & (1 << RXC)));

    uint8_t b = UDR;
    if(b == '\r')
        b = '\n';

    return b;
#endif

}

//EMPTY_INTERRUPT(USART_RXC_vect)
