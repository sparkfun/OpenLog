
/*
 * Copyright (c) 2006-2009 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <avr/pgmspace.h>

extern volatile char input_buffer[900];
//I can't figure out how to pass in an external define for the BUFF_LEN in main.c, so it's hard coded. Compiler will throw error if they don't agree
extern volatile uint16_t read_spot;

#ifdef __cplusplus
extern "C"
{
#endif

void uart_init(uint8_t uart_speed);

void uart_putc(uint8_t c);

void uart_putc_hex(uint8_t b);
void uart_putw_hex(uint16_t w);
void uart_putdw_hex(uint32_t dw);

void uart_putw_dec(uint16_t w);
void uart_putdw_dec(uint32_t dw);

void uart_puts(const char* str);
void uart_puts_p(PGM_P str);

uint8_t uart_getc(void);

#ifdef __cplusplus
}
#endif

#endif

