/*
 * Copyright (c) 2006-2009 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
    12-3-09
    Copyright SparkFun Electronics© 2009
    Nathan Seidle
    spark at sparkfun.com
    
    OpenLog is a simple serial logger based on the ATmega328 running at 16MHz. The ATmega328
	should be able to talk to high capacity (larger than 2GB) SD cards. The whole purpose of this
	logger was to create a logger that just powered up and worked. OpenLog ships with standard 
	57600bps serial bootloader running at 16MHz so you can load new firmware with a simple serial
	connection. This makes it compatible with Arduino if needed.
	
	OpenLog runs at 9600bps by default. This is configurable to 2400, 9600, 57600, and 115200bps. We recommend 
	you attach a serial connection to reconfigure the unit to work at a different serial speed, but you 
	should be able to do it in software.

	Type '?' to get a list of supported commands.
	
	During power up, you will see '12>'. '1' indicates the serial connection is established. '2' indicates
	the SD card has been successfully initialized. '>' indicates OpenLog is ready to receive commands.

	Recording constant 115200bps datastreams are supported. Throw it everything you've got!
	
	'cd ..' is a bit weird. Normally it's 'cd..' but to change to a lower dir, use 'cd ..'
	
	Currently, the firmware supports creating a new file or directory up to 16 characters including the
	'.' and extension. "123456789012.txt" is the longest name. Any longer and the module will re-initialize
	as if there was a variable sizeof error.

	Capital letters, white space, and other characters are supported ("Hi there#$_.txt").
	
	Type 'set' to enter baud rate configuration menu. Select the baud rate and press enter. You will then 
	see a message 'Going to 9600bps...' or some such message. You will need to power down OpenLog, change 
	your system UART settings to match the new OpenLog baud rate and then power OpenLog back up.
	
	If you get OpenLog stuck into an unknown baudrate, there is a safety mechanism built-in. Tie the RX pin 
	to ground and power up OpenLog. You should see the LEDs blink back and forth for 2 seconds, then blink 
	in unison. Now power down OpenLog and remove the RX/GND jumper. OpenLog is now reset to 9600bps.
	
	Please note: The preloaded STK500 serial bootloader is 2k, and begins at 0x7800 (30,720). If the code is l
	arger than 30,719 bytes, you will get verification errors during serial bootloading.

	SD vs HCSD configuration is found in sd_raw_config.h - Currently only 512MB, 1GB, 2GB, and some 
	4GB cards work (not yet compatible with HCSD cards).

	STAT1 LED is sitting on PD5 (Arduino D5)
	STAT2 LED is sitting on PB5 (Arduino D13)

	LED Flashing errors @ 2Hz:
	No SD card - 3 blinks
	Baud rate change (requires power cycle) - 4 blinks
	
	During an append, OpenLog will buffer 512 characters at a time. That means that if the system loses power
	while reading in characters, you may loose up to, but no more than, 511 characters. This is important for low
	power systems where you may not know when the battery or power will die. OpenLog should record each buffer as 
	it receives each 512 byte chunk. The only way to exit an append is with Ctrl+z (ASCII 26).
	
	8mA idle
	18mA actively writing
	
	Input voltage on VCC can be 3.3 to 12V. Input voltage on RX-I pin must not exceed 6V. Output voltage on
	TX-O pin will not be greater than 3.3V. This may cause problems with some systems - for example if your
	attached microcontroller requires 4V minimum for serial communication (this is rare).
	
	v1.1
	Adding better defines for EEPROM settings
	Adding new log and sequential log functions.
	
	Code is acting very weird with what looks to be stack crashes. I can get around this by turning the optimizer off ('0').
	Found an error : EEPROM functions fail when optimizer is set to '0' or '1'. 
	sd-reader_config.h contains flag for USE_DYNAMIC_MEMORY
	
	Looks like tweaking the optimization higher causes the asm("nop"); to fail inside the append_file routine. Changing this to
	delay_us(1); works.
	
	I have a sneaking suspicion that I was having buffer overrun problems when defining the input_buffer at 1024 bytes. The
	ATmega328 should have enough RAM (2K) but weird reset errors were occuring. With the buffer at 512bytes, append_file runs 
	just fine and is able to log at 115200 at a constant data rate.
	
	Added file called 'lots-o-text.txt' to version control. This text contains easy to scan text to be used for full data
	rate checking and testing.	
*/

#include <string.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "fat.h"
#include "fat_config.h"
#include "partition.h"
#include "sd_raw.h"
#include "sd_raw_config.h"
#include "uart.h"

//Setting DEBUG to 1 will cause extra serial messages to appear
//such as "Media Init Complete!" indicating the SD card was initalized
#define DEBUG 0

/**
 * \mainpage MMC/SD/SDHC card library
 *
 * This project provides a general purpose library which implements read and write
 * support for MMC, SD and SDHC memory cards.
 *
 * It includes
 * - low-level \link sd_raw MMC, SD and SDHC read/write routines \endlink
 * - \link partition partition table support \endlink
 * - a simple \link fat FAT16/FAT32 read/write implementation \endlink
 *
 * \section circuit The circuit
 * The circuit which was mainly used during development consists of an Atmel AVR
 * microcontroller with some passive components. It is quite simple and provides
 * an easy test environment. The circuit which can be downloaded on the
 * <a href="http://www.roland-riegel.de/sd-reader/">project homepage</a> has been
 * improved with regard to operation stability.
 *
 * I used different microcontrollers during development, the ATmega8 with 8kBytes
 * of flash, and its pin-compatible alternative, the ATmega168 with 16kBytes flash.
 * The first one is the one I started with, but when I implemented FAT16 write
 * support, I ran out of flash space and switched to the ATmega168. For FAT32, an
 * ATmega328 is required.
 * 
 * The circuit board is a self-made and self-soldered board consisting of a single
 * copper layer and standard DIL components, except of the MMC/SD card connector.
 *
 * The connector is soldered to the bottom side of the board. It has a simple
 * eject button which, when a card is inserted, needs some space beyond the connector
 * itself. As an additional feature the connector has two electrical switches
 * to detect wether a card is inserted and wether this card is write-protected.
 * 
 * \section pictures Pictures
 * \image html pic01.jpg "The circuit board used to implement and test this application."
 * \image html pic02.jpg "The MMC/SD card connector on the soldering side of the circuit board."
 *
 * \section software The software
 * The software is written in pure standard ANSI-C. It might not be the smallest or
 * the fastest one, but I think it is quite flexible. See the project's
 * <a href="http://www.roland-riegel.de/sd-reader/benchmarks/">benchmark page</a> to get an
 * idea of the possible data rates.
 *
 * I implemented an example application providing a simple command prompt which is accessible
 * via the UART at 9600 Baud. With commands similiar to the Unix shell you can browse different
 * directories, read and write files, create new ones and delete them again. Not all commands are
 * available in all software configurations.
 * - <tt>cat \<file\></tt>\n
 *   Writes a hexdump of \<file\> to the terminal.
 * - <tt>cd \<directory\></tt>\n
 *   Changes current working directory to \<directory\>.
 * - <tt>disk</tt>\n
 *   Shows card manufacturer, status, filesystem capacity and free storage space.
 * - <tt>init</tt>\n
 *   Reinitializes and reopens the memory card.
 * - <tt>ls</tt>\n
 *   Shows the content of the current directory.
 * - <tt>mkdir \<directory\></tt>\n
 *   Creates a directory called \<directory\>.
 * - <tt>rm \<file\></tt>\n
 *   Deletes \<file\>.
 * - <tt>sync</tt>\n
 *   Ensures all buffered data is written to the card.
 * - <tt>touch \<file\></tt>\n
 *   Creates \<file\>.
 * - <tt>write \<file\> \<offset\></tt>\n
 *   Writes text to \<file\>, starting from \<offset\>. The text is read
 *   from the UART, line by line. Finish with an empty line.
 *
 * \htmlonly
 * <p>
 * The following table shows some typical code sizes in bytes, using the 20090330 release with a
 * buffered read-write MMC/SD configuration, FAT16 and static memory allocation:
 * </p>
 *
 * <table border="1" cellpadding="2">
 *     <tr>
 *         <th>layer</th>
 *         <th>code size</th>
 *         <th>static RAM usage</th>
 *     </tr>
 *     <tr>
 *         <td>MMC/SD</td>
 *         <td align="right">2410</td>
 *         <td align="right">518</td>
 *     </tr>
 *     <tr>
 *         <td>Partition</td>
 *         <td align="right">456</td>
 *         <td align="right">17</td>
 *     </tr>
 *     <tr>
 *         <td>FAT16</td>
 *         <td align="right">7928</td>
 *         <td align="right">188</td>
 *     </tr>
 * </table>
 *
 * <p>
 * The static RAM is mostly used for buffering memory card access, which
 * improves performance and reduces implementation complexity.
 * </p>
 * 
 * <p>
 * Please note that the numbers above do not include the C library functions
 * used, e.g. some string functions. These will raise the numbers somewhat
 * if they are not already used in other program parts.
 * </p>
 * 
 * <p>
 * When opening a partition, filesystem, file or directory, a little amount
 * of RAM is used, as listed in the following table. Depending on the library
 * configuration, the memory is either allocated statically or dynamically.
 * </p>
 *
 * <table border="1" cellpadding="2">
 *     <tr>
 *         <th>descriptor</th>
 *         <th>dynamic/static RAM</th>
 *     </tr>
 *     <tr>
 *         <td>partition</td>
 *         <td align="right">17</td>
 *     </tr>
 *     <tr>
 *         <td>filesystem</td>
 *         <td align="right">26</td>
 *     </tr>
 *     <tr>
 *         <td>file</td>
 *         <td align="right">53</td>
 *     </tr>
 *     <tr>
 *         <td>directory</td>
 *         <td align="right">49</td>
 *     </tr>
 * </table>
 * 
 * \endhtmlonly
 *
 * \section adaptation Adapting the software to your needs
 * The only hardware dependent part is the communication layer talking to the
 * memory card. The other parts like partition table and FAT support are
 * completely independent, you could use them even for managing Compact Flash
 * cards or standard ATAPI hard disks.
 *
 * By changing the MCU* variables in the Makefile, you can use other Atmel
 * microcontrollers or different clock speeds. You might also want to change
 * the configuration defines in the files fat_config.h, partition_config.h,
 * sd_raw_config.h and sd-reader_config.h. For example, you could disable
 * write support completely if you only need read support.
 *
 * For further information, visit the project's
 * <a href="http://www.roland-riegel.de/sd-reader/faq/">FAQ page</a>.
 * 
 * \section bugs Bugs or comments?
 * If you have comments or found a bug in the software - there might be some
 * of them - you may contact me per mail at feedback@roland-riegel.de.
 *
 * \section acknowledgements Acknowledgements
 * Thanks go to Ulrich Radig, who explained on his homepage how to interface
 * MMC cards to the Atmel microcontroller (http://www.ulrichradig.de/).
 * I adapted his work for my circuit. Although this is a very simple
 * solution, I had no problems using it.
 * 
 * \section copyright Copyright 2006-2009 by Roland Riegel
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation (http://www.gnu.org/copyleft/gpl.html).
 * At your option, you can alternatively redistribute and/or modify the following
 * files under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation (http://www.gnu.org/copyleft/lgpl.html):
 * - byteordering.c
 * - byteordering.h
 * - fat.c
 * - fat.h
 * - fat_config.h
 * - partition.c
 * - partition.h
 * - partition_config.h
 * - sd_raw.c
 * - sd_raw.h
 * - sd_raw_config.h
 * - sd-reader_config.h
 */




//Function declarations
static uint8_t read_line(char* buffer, uint8_t buffer_length);
static uint32_t strtolong(const char* str);
static uint8_t find_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry);
static struct fat_file_struct* open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name); 
static uint8_t print_disk_info(const struct fat_fs_struct* fs);

void ioinit(void);
void print_menu(void);
void init_media(void);
void baud_menu(void);
void system_menu(void);
uint8_t read_buffer(char* buffer, uint8_t buffer_length);
uint8_t append_file(char* file_name);
void newlog(void);
void seqlog(void);
void command_shell(void);

void blink_error(uint8_t ERROR_TYPE);

void EEPROM_write(uint16_t uiAddress, unsigned char ucData);
unsigned char EEPROM_read(uint16_t uiAddress);

void delay_us(uint16_t x);
void delay_ms(uint16_t x);

#define sbi(port, port_pin)   ((port) |= (uint8_t)(1 << port_pin))
#define cbi(port, port_pin)   ((port) &= (uint8_t)~(1 << port_pin))

//STAT1 is a general LED and indicates serial traffic
#define STAT1	5
#define STAT1_PORT	PORTD

//STAT2 is tied to the SCL line and indicates volume SPI traffic
#define STAT2	5
#define STAT2_PORT	PORTB

#define ERROR_SD_INIT	3
#define ERROR_NEW_BAUD	5

#define LOCATION_BAUD_SETTING	0x01
#define LOCATION_SYSTEM_SETTING	0x02
#define LOCATION_FILE_NUMBER	0x03

#define BAUD_2400	0
#define BAUD_9600	1
#define BAUD_57600	2
#define BAUD_115200	3

#define MODE_NEWLOG	0
#define MODE_SEQLOG 1
#define MODE_COMMAND 2

//Global variables
struct fat_fs_struct* fs;
struct partition_struct* partition;
struct fat_dir_struct* dd;

#define BUFF_LEN 512
char input_buffer[BUFF_LEN];
uint16_t read_spot, checked_spot;


//Circular buffer UART RX interrupt
//Is only used during append
ISR(USART_RX_vect)
{
	input_buffer[read_spot] = UDR0;
	read_spot++;
	STAT1_PORT ^= (1<<STAT1); //Toggle the STAT1 LED each time we receive a character
	if(read_spot == BUFF_LEN) read_spot = 0;
}

int main(void)
{
	ioinit();

	//Determine the system mode we should be in
	uint8_t system_mode;
	system_mode = EEPROM_read(LOCATION_SYSTEM_SETTING);
	if(system_mode > 5) 
	{
		system_mode = MODE_NEWLOG; //By default, unit will turn on and go to new file logging
		EEPROM_write(LOCATION_SYSTEM_SETTING, MODE_NEWLOG);
	}

	//If we are in new log mode, find a new file name to write to
	if(system_mode == MODE_NEWLOG)
		newlog();

	//If we are in sequential log mode, determine if seqlog.txt has been created or not, and then open it for logging
	if(system_mode == MODE_SEQLOG)
		seqlog();
	
	//Once either one of these modes exits, go to normal command mode, which is called by returning to main()
	command_shell();

    return 0;
}

void ioinit(void)
{
    //Init Timer0 for delay_us
    //TCCR0B = (1<<CS00); //Set Prescaler to clk/1 (assume we are running at internal 1MHz). CS00=1 
    TCCR0B = (1<<CS01); //Set Prescaler to clk/8 : 1click = 1us(assume we are running at internal 8MHz). CS01=1 
    //Since we are running at 16MHz, this is a hack job. We will double the count during delay_us function.
	//TCCR0B = (1<<CS01)|(1<<CS00); //Set Prescaler to clk/64

    //1 = output, 0 = input
    DDRD |= (1<<STAT1); //PORTD (STAT1 on PD5)
    DDRB |= (1<<STAT2); //PORTC (STAT2 on PB5)

	//Check to see if we need an emergency UART reset
	DDRD |= (1<<0); //Turn the RX pin into an input
	PORTD |= (1<<0); //Push a 1 onto RX pin to enable internal pull-up
	if( (PIND & (1<<0)) == 0) //Now check the RX pin. If it's being held low
	{
		//Wait 2 seconds, blinking LEDs while we wait
		sbi(PORTC, STAT2); //Set the STAT2 LED
		for(uint8_t i = 0 ; i < 40 ; i++)
		{
			delay_ms(25);
			PORTD ^= (1<<STAT1); //Blink the stat LEDs
			delay_ms(25);
			PORTB ^= (1<<STAT2); //Blink the stat LEDs
		}
		
		//Check pin again
		if( (PIND & (1<<0)) == 0)
		{
			//If the pin is still low, then reset UART to 9600bps
			EEPROM_write(0x01, 1);

			//Now sit in forever loop indicating system is now at 9600bps
			sbi(PORTD, STAT1); 
			sbi(PORTB, STAT2);
			while(1)
			{
				delay_ms(500);
				PORTD ^= (1<<STAT1); //Blink the stat LEDs
				PORTB ^= (1<<STAT2); //Blink the stat LEDs
			}
		}
	}

	//Read what the current UART speed is from EEPROM memory
	uint8_t uart_speed;
	uart_speed = EEPROM_read(LOCATION_BAUD_SETTING);
	if(uart_speed > 5) 
	{
		uart_speed = BAUD_9600; //Reset UART to 9600 if there is no speed stored
		EEPROM_write(LOCATION_BAUD_SETTING, BAUD_9600);
	}

    //Setup uart
    uart_init(uart_speed);
#if DEBUG
	uart_puts_p(PSTR("UART Init\n"));
#else
	uart_puts_p(PSTR("1"));
#endif
	
	//Setup SPI, init SD card, etc
	init_media();
	uart_puts_p(PSTR("2"));
}

//Log to the same file every time the system boots, sequentially
//Checks to see if the file SEQLOG.txt is available
//If not, create it
//If yes, append to it
//Return 0 on error
//Return anything else on sucess
void seqlog(void)
{
	char seq_file_name[13];
	sprintf(seq_file_name, "SEQLOG.txt");

	struct fat_file_struct* fd = open_file_in_dir(fs, dd, seq_file_name);
	if(!fd)
	{
		uart_puts_p(PSTR("Creating SEQLOG\n"));

		struct fat_dir_entry_struct file_entry;
		if(!fat_create_file(dd, seq_file_name, &file_entry))
		{
			uart_puts_p(PSTR("Error creating SEQLOG\n"));
			return;
		}
	}

	fat_close_file(fd); //Close the file so we can re-open it in append_file

	append_file(seq_file_name);
}

//Log to a new file everytime the system boots
//Checks the spots in EEPROM for the next available LOG# file name
//Updates EEPROM and then appends to the new log file.
//Currently limited to 255 files but this should not always be the case.
void newlog(void)
{
	uint8_t new_file_number;
	new_file_number = EEPROM_read(LOCATION_FILE_NUMBER);

	if(new_file_number == 255) 
	{
		new_file_number = 0; //By default, unit will start at file number zero
		EEPROM_write(LOCATION_FILE_NUMBER, new_file_number);
	}

	char new_file_name[13];
	sprintf(new_file_name, "LOG%03d.txt", new_file_number);

	struct fat_dir_entry_struct file_entry;
	while(!fat_create_file(dd, new_file_name, &file_entry))
	{
		//Increment the file number because this file name is already taken
		new_file_number++;

		sprintf(new_file_name, "LOG%03d.txt", new_file_number);
	}

	//Record new_file number to EEPROM
	EEPROM_write(LOCATION_FILE_NUMBER, new_file_number++); //Add one the new_file_number for the next power-up
	
#if DEBUG
	uart_puts_p(PSTR("\nCreated new file: "));
	uart_puts(new_file_name);
	uart_puts_p(PSTR("\n"));
#endif

	//Begin writing to file
	append_file(new_file_name);
}

void command_shell(void)
{
	//provide a simple shell
	char buffer[24];
	while(1)
	{
		//print prompt
		uart_putc('>');

		//read command
		char* command = buffer;
		if(read_line(command, sizeof(buffer)) < 1)
			continue;

		//execute command
		if(strcmp_P(command, PSTR("init")) == 0)
		{
			uart_puts_p(PSTR("Closing down file system\n"));

			/* close file system */
			fat_close(fs);

			/* close partition */
			partition_close(partition);
			
			//Setup SPI, init SD card, etc
			init_media();				

			uart_puts_p(PSTR("File system initialized\n"));
		}
		else if(strcmp_P(command, PSTR("?")) == 0)
		{
			//Print available commands
			print_menu();
		}
		else if(strcmp_P(command, PSTR("help")) == 0)
		{
			//Print available commands
			print_menu();
		}
		else if(strcmp_P(command, PSTR("baud")) == 0)
		{
			//Go into baud select menu
			baud_menu();
		}
		else if(strcmp_P(command, PSTR("set")) == 0)
		{
			//Go into system setting menu
			system_menu();
		}
		else if(strncmp_P(command, PSTR("cd "), 3) == 0)
		{
			command += 3;
			if(command[0] == '\0')
				continue;

			/* change directory */
			struct fat_dir_entry_struct subdir_entry;
			if(find_file_in_dir(fs, dd, command, &subdir_entry))
			{
				struct fat_dir_struct* dd_new = fat_open_dir(fs, &subdir_entry);
				if(dd_new)
				{
					fat_close_dir(dd);
					dd = dd_new;
					continue;
				}
			}

			uart_puts_p(PSTR("directory not found: "));
			uart_puts(command);
			uart_putc('\n');
		}
		else if(strcmp_P(command, PSTR("ls")) == 0)
		{
			/* print directory listing */
			struct fat_dir_entry_struct dir_entry;
			while(fat_read_dir(dd, &dir_entry))
			{
				uint8_t spaces = sizeof(dir_entry.long_name) - strlen(dir_entry.long_name) + 4;

				uart_puts(dir_entry.long_name);
				uart_putc(dir_entry.attributes & FAT_ATTRIB_DIR ? '/' : ' ');
				while(spaces--)
					uart_putc(' ');
				uart_putdw_dec(dir_entry.file_size);
				uart_putc('\n');
			}
		}
		else if(strncmp_P(command, PSTR("cat "), 4) == 0)
		{
			command += 4;
			if(command[0] == '\0')
				continue;
			
			/* search file in current directory and open it */
			struct fat_file_struct* fd = open_file_in_dir(fs, dd, command);
			if(!fd)
			{
				uart_puts_p(PSTR("error opening "));
				uart_puts(command);
				uart_putc('\n');
				continue;
			}

			/* print file contents */
			uint8_t buffer[8];
			uint32_t offset = 0;
			while(fat_read_file(fd, buffer, sizeof(buffer)) > 0)
			{
				uart_putdw_hex(offset);
				uart_putc(':');
				for(uint8_t i = 0; i < 8; ++i)
				{
					uart_putc(' ');
					uart_putc_hex(buffer[i]);
				}
				uart_putc('\n');
				offset += 8;
			}

			fat_close_file(fd);
		}
		else if(strcmp_P(command, PSTR("disk")) == 0)
		{
			if(!print_disk_info(fs))
				uart_puts_p(PSTR("error reading disk info\n"));
		}
#if FAT_WRITE_SUPPORT
		else if(strncmp_P(command, PSTR("rm "), 3) == 0)
		{
			command += 3;
			if(command[0] == '\0')
				continue;
			
			struct fat_dir_entry_struct file_entry;
			if(find_file_in_dir(fs, dd, command, &file_entry))
			{
				if(fat_delete_file(fs, &file_entry))
					continue;
			}

			uart_puts_p(PSTR("error deleting file: "));
			uart_puts(command);
			uart_putc('\n');
		}
		else if(strncmp_P(command, PSTR("new "), 4) == 0)
		{
			command += 4;
			if(command[0] == '\0')
				continue;

			struct fat_dir_entry_struct file_entry;
			if(!fat_create_file(dd, command, &file_entry))
			{
				uart_puts_p(PSTR("error creating file: "));
				uart_puts(command);
				uart_putc('\n');
			}
		}
		else if(strncmp_P(command, PSTR("write "), 6) == 0)
		{
			command += 6;
			if(command[0] == '\0')
				continue;

			char* offset_value = command;
			while(*offset_value != ' ' && *offset_value != '\0')
				++offset_value;

			if(*offset_value == ' ')
				*offset_value++ = '\0';
			else
				continue;

			/* search file in current directory and open it */
			struct fat_file_struct* fd = open_file_in_dir(fs, dd, command);
			if(!fd)
			{
				uart_puts_p(PSTR("error opening "));
				uart_puts(command);
				uart_putc('\n');
				continue;
			}

			int32_t offset = strtolong(offset_value);
			if(!fat_seek_file(fd, &offset, FAT_SEEK_SET))
			{
				uart_puts_p(PSTR("error seeking on "));
				uart_puts(command);
				uart_putc('\n');

				fat_close_file(fd);
				continue;
			}

			/* read text from the shell and write it to the file */
			uint8_t data_len;
			while(1)
			{
				/* give a different prompt */
				uart_putc('<');
				//uart_putc(' ');

				/* read one line of text */
				data_len = read_line(buffer, sizeof(buffer));
				if(!data_len)
					break;

				/* write text to file */
				if(fat_write_file(fd, (uint8_t*) buffer, data_len) != data_len)
				{
					uart_puts_p(PSTR("error writing to file\n"));
					break;
				}
			}

			fat_close_file(fd);
		}

		else if(strncmp_P(command, PSTR("append "), 7) == 0)
		{
			//Find the end of a current file and begins writing to it
			//Ends only when the user inputs Ctrl+z (ASCII 26)
			command += 7;
			if(command[0] == '\0')
				continue;
				
			append_file(command); //Uses circular buffer to capture full stream of text and append to file
		}
		else if(strncmp_P(command, PSTR("md "), 3) == 0)
		{
			command += 3;
			if(command[0] == '\0')
				continue;

			struct fat_dir_entry_struct dir_entry;
			if(!fat_create_dir(dd, command, &dir_entry))
			{
				uart_puts_p(PSTR("error creating directory: "));
				uart_puts(command);
				uart_putc('\n');
			}
		}
#endif
#if SD_RAW_WRITE_BUFFERING
		else if(strcmp_P(command, PSTR("sync")) == 0)
		{
			if(!sd_raw_sync())
				uart_puts_p(PSTR("error syncing disk\n"));
		}
#endif
		else
		{
			uart_puts_p(PSTR("unknown command: "));
			uart_puts(command);
			uart_putc('\n');
		}
    }
	
	//Do we ever get this far?
	uart_puts_p(PSTR("Exiting: closing down\n"));
}


//Appends a stream of serial data to a given file
//We use the RX interrupt and a circular buffer of 1024 bytes so that we can capture a full stream of 
//data even at 115200bps
//Does not exit until Ctrl+z (ASCII 26) is received
//Returns 0 on error
//Returns 1 on success
uint8_t append_file(char* file_name)
{

	//search file in current directory and open it 
	struct fat_file_struct* fd = open_file_in_dir(fs, dd, file_name);
	if(!fd)
	{
		uart_puts_p(PSTR("error opening "));
		uart_puts(file_name);
		uart_putc('\n');
		return(0);
	}

#if DEBUG
	uart_puts_p(PSTR("File open\n"));
#endif
	int32_t offset = 0;
	//Seeks the end of the file : offset = EOF location
	if(!fat_seek_file(fd, &offset, FAT_SEEK_END))
	{
		uart_puts_p(PSTR("error seeking on "));
		uart_puts(file_name);
		uart_putc('\n');

		fat_close_file(fd);
		return(0);
	}

#if DEBUG
	uart_puts_p(PSTR("Recording\n"));
#endif
	//give a different prompt to indicate no echoing
	uart_putc('<');

	sbi(STAT1_PORT, STAT1); //Turn on indicator LED

	read_spot = 0;
	checked_spot = 0;

	//Clear circular buffer
	for(uint16_t i = 0 ; i < BUFF_LEN ; i++)
		input_buffer[i] = 0;
		
	//Start UART buffered interrupts
	UCSR0B |= (1<<RXCIE0); //Enable receive interrupts
	sei(); //Enable interrupts
	
	//Start checking buffer
	//This gets kind of wild. We receive characters up to 115200bps using the UART interrupt
	//They get stored in one big array 1024 wide. As the array fills, we store half the array (512 bytes)
	//at a time so that we are storing half the array while the other half is filling.

	while(1)
	{
//fail		while(checked_spot == read_spot) asm("nop"); //Hang out while we wait for the interrupt to occur and advance read_spot
		while(checked_spot == read_spot) delay_us(1); //Hang out while we wait for the interrupt to occur and advance read_spot

		if(input_buffer[checked_spot] == 26) //Scan for escape character
		{
			//Disable interrupt and we're done!
			cli();
			UCSR0B &= ~(1<<RXCIE0); //Clear receive interrupt enable
			
			break;
		}
		
		checked_spot++;

		if(checked_spot == (BUFF_LEN/2)) //We've finished checking the first half the buffer
		{
			//Record first half the buffer
			if(fat_write_file(fd, (uint8_t*) input_buffer, (BUFF_LEN/2) ) != (BUFF_LEN/2) )
			{
				uart_puts_p(PSTR("error writing to file\n"));
				break;
			}
		}

		if(checked_spot == BUFF_LEN) //We've finished checking the second half the buffer
		{
			checked_spot = 0;
			
			//Record second half the buffer
			if(fat_write_file(fd, (uint8_t*) input_buffer + (BUFF_LEN/2), (BUFF_LEN/2) ) != (BUFF_LEN/2) )
			{
				uart_puts_p(PSTR("error writing to file\n"));
				break;
			}
		}
	}

	//Upon receiving the escape character, we may still have stuff left in the buffer
	//Record the last of the buffer to memory
	if(checked_spot == 0 || checked_spot == (BUFF_LEN/2))
	{
		//Do nothing, we already recorded the buffers right before catching the escape character
	}
	else if(checked_spot < (BUFF_LEN/2))
	{
		//Record first half the buffer
		if(fat_write_file(fd, (uint8_t*) input_buffer, checked_spot) != checked_spot)
			uart_puts_p(PSTR("error writing to file\n"));
	}
	else //checked_spot > (BUFF_LEN/2)
	{
		//Record second half the buffer
		if(fat_write_file(fd, (uint8_t*) input_buffer + (BUFF_LEN/2), (checked_spot - (BUFF_LEN/2)) ) != (checked_spot - (BUFF_LEN/2)) )
			uart_puts_p(PSTR("error writing to file\n"));
	}

	fat_close_file(fd);

	cbi(STAT1_PORT, STAT1); //Turn off indicator LED

#if DEBUG
	uart_puts_p(PSTR("Done!\n"));
#endif
	uart_puts_p(PSTR("~")); //Indicate a successful record
	
	return(1); //Success!
}


//Inits the SD interface, opens file system, opens root dir, and checks card info if wanted
void init_media(void)
{
	/* setup sd card slot */
	if(!sd_raw_init())
	{
#if DEBUG
		uart_puts_p(PSTR("MMC/SD initialization failed\n"));
#endif
		blink_error(ERROR_SD_INIT);
		//continue;
	}

	/* open first partition */
	partition = partition_open(sd_raw_read,
														sd_raw_read_interval,
#if SD_RAW_WRITE_SUPPORT
														sd_raw_write,
														sd_raw_write_interval,
#else
														0,
														0,
#endif
														0
													   );

	if(!partition)
	{
		/* If the partition did not open, assume the storage device
		 * is a "superfloppy", i.e. has no MBR.
		 */
		partition = partition_open(sd_raw_read,
								   sd_raw_read_interval,
#if SD_RAW_WRITE_SUPPORT
								   sd_raw_write,
								   sd_raw_write_interval,
#else
								   0,
								   0,
#endif
								   -1
								  );
		if(!partition)
		{
#if DEBUG
			uart_puts_p(PSTR("Opening partition failed\n"));
#endif
			//continue;
		}
	}

	/* open file system */
	fs = fat_open(partition);

	if(!fs)
	{
#if DEBUG
		uart_puts_p(PSTR("Opening file system failed\n"));
#endif
		//continue;
	}

	/* open root directory */
	struct fat_dir_entry_struct directory;
	fat_get_dir_entry_of_path(fs, "/", &directory);

	dd = fat_open_dir(fs, &directory);
	if(!dd)
	{
#if DEBUG
		uart_puts_p(PSTR("Opening root directory failed\n"));
#endif
		//continue;
	}
	
	/* print some card information as a boot message */
	//print_disk_info(fs);

#if DEBUG
	uart_puts_p(PSTR("Media Init Complete!\n"));
#endif
	
}

//Reads a line until the \n enter character is found
uint8_t read_line(char* buffer, uint8_t buffer_length)
{
    memset(buffer, 0, buffer_length);

    uint8_t read_length = 0;
    while(read_length < buffer_length - 1)
    {
		PORTD ^= (1<<STAT1); //Blink the stat LED while typing
PORTB ^= (1<<STAT2); //Blink the stat LED while typing

        uint8_t c = uart_getc();

        if(c == 0x08 || c == 0x7f)
        {
            if(read_length < 1)
                continue;

            --read_length;
            buffer[read_length] = '\0';

            uart_putc(0x08);
            uart_putc(' ');
            uart_putc(0x08);

            continue;
        }

        uart_putc(c);

        if(c == '\n')
        {
            buffer[read_length] = '\0';
            break;
        }
        else
        {
            buffer[read_length] = c;
            ++read_length;
        }
    }

    return read_length;
}

//Basic EEPROM functions to read/write to the internal EEPROM
void EEPROM_write(uint16_t uiAddress, unsigned char ucData)
{
	while(EECR & (1<<EEPE)); //Wait for completion of previous write
	EEARH = uiAddress >> 8; //Set up address and data registers
	EEARL = uiAddress; //Set up address and data registers
	EEDR = ucData;
	EECR |= (1<<EEMPE); //Write logical one to EEMWE
	EECR |= (1<<EEPE); //Start eeprom write by setting EEWE
}

unsigned char EEPROM_read(uint16_t uiAddress)
{
	while(EECR & (1<<EEPE)); //Wait for completion of previous write
	EEARH = uiAddress >> 8; //Set up address and data registers
	EEARL = uiAddress; //Set up address and data registers
	EECR |= (1<<EERE); //Start eeprom read by writing EERE
	return EEDR; //Return data from data register
}


//Blinks the status LEDs to indicate a type of error
void blink_error(uint8_t ERROR_TYPE)
{
	while(1)
	{
		for(int x = 0 ; x < ERROR_TYPE ; x++)
		{
			sbi(STAT1_PORT, STAT1);
			delay_ms(100);
			cbi(STAT1_PORT, STAT1);
			delay_ms(100);
		}
		
		delay_ms(2000);
	}
}

//General short delays
//Uses internal timer do a fairly accurate 1us
//Because we are using 16MHz and a prescalar of 8 on Timer0, we have to double x
void delay_us(uint16_t x)
{
	//External 16MHz resonator hack.
	x *= 2;	
	
	while(x > 256)
	{
		TIFR0 = (1<<TOV0); //Clear any interrupt flags on Timer0
		TCNT0 = 0; //Preload Timer0 for 256 clicks. Should be 1us per click
		while( (TIFR0 & (1<<TOV0)) == 0);
		
		x -= 256;
	}

	TIFR0 = (1<<TOV0); //Clear any interrupt flags on Timer0
	TCNT0 = 256 - x; //256 - 125 = 131 : Preload Timer0 for x clicks. Should be 1us per click
	while( (TIFR0 & (1<<TOV0)) == 0);
}

//General short delays
void delay_ms(uint16_t x)
{
	for ( ; x > 0 ; x--)
		delay_us(1000);
}


//Reads a larger buffer of characters until the ctrl+z (ascii 26) character is received
uint8_t read_buffer(char* buffer, uint8_t buffer_length)
{
    memset(buffer, 0, buffer_length);

    uint8_t read_length = 0;
    while(read_length < buffer_length - 1)
    {
        uint8_t c = uart_getc();

        if(c == 0x08 || c == 0x7f)
        {
            if(read_length < 1)
                continue;

            --read_length;
            buffer[read_length] = '\0';

            uart_putc(0x08);
            uart_putc(' ');
            uart_putc(0x08);

            continue;
        }

        uart_putc(c);

        //if(c == '\n')
        if(c == 26)
        {
            buffer[read_length] = '\0';
            break;
        }
        else
        {
            buffer[read_length] = c;
            ++read_length;
        }
    }

    return read_length;
}

uint32_t strtolong(const char* str)
{
    uint32_t l = 0;
    while(*str >= '0' && *str <= '9')
        l = l * 10 + (*str++ - '0');

    return l;
}

uint8_t find_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry)
{
    while(fat_read_dir(dd, dir_entry))
    {
        if(strcmp(dir_entry->long_name, name) == 0)
        {
            fat_reset_dir(dd);
            return 1;
        }
    }

    return 0;
}

struct fat_file_struct* open_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name)
{
    struct fat_dir_entry_struct file_entry;
    if(!find_file_in_dir(fs, dd, name, &file_entry))
        return 0;

    return fat_open_file(fs, &file_entry);
}

uint8_t print_disk_info(const struct fat_fs_struct* fs)
{
    if(!fs)
        return 0;

    struct sd_raw_info disk_info;
    if(!sd_raw_get_info(&disk_info))
        return 0;

    uart_puts_p(PSTR("manuf:  0x")); uart_putc_hex(disk_info.manufacturer); uart_putc('\n');
    uart_puts_p(PSTR("oem:    ")); uart_puts((char*) disk_info.oem); uart_putc('\n');
    uart_puts_p(PSTR("prod:   ")); uart_puts((char*) disk_info.product); uart_putc('\n');
    uart_puts_p(PSTR("rev:    ")); uart_putc_hex(disk_info.revision); uart_putc('\n');
    uart_puts_p(PSTR("serial: 0x")); uart_putdw_hex(disk_info.serial); uart_putc('\n');
    uart_puts_p(PSTR("date:   ")); uart_putw_dec(disk_info.manufacturing_month); uart_putc('/');
                                   uart_putw_dec(disk_info.manufacturing_year); uart_putc('\n');
    uart_puts_p(PSTR("size:   ")); uart_putdw_dec(disk_info.capacity / 1024 / 1024); uart_puts_p(PSTR("MB\n"));
    uart_puts_p(PSTR("copy:   ")); uart_putw_dec(disk_info.flag_copy); uart_putc('\n');
    uart_puts_p(PSTR("wr.pr.: ")); uart_putw_dec(disk_info.flag_write_protect_temp); uart_putc('/');
                                   uart_putw_dec(disk_info.flag_write_protect); uart_putc('\n');
    uart_puts_p(PSTR("format: ")); uart_putw_dec(disk_info.format); uart_putc('\n');
    uart_puts_p(PSTR("free:   ")); uart_putdw_dec(fat_get_fs_free(fs)); uart_putc('/');
                                   uart_putdw_dec(fat_get_fs_size(fs)); uart_putc('\n');

    return 1;
}

void print_menu(void)
{
	uart_puts_p(PSTR("\nOpenLog v1.1\n"));
	uart_puts_p(PSTR("Available commands:\n"));
	uart_puts_p(PSTR("new <file>\t\t: Creates <file>\n"));
	uart_puts_p(PSTR("append <file>\t\t: Appends text to end of <file>. The text is read from the UART in a stream and is not echoed. Finish by sending Ctrl+z (ASCII 26)\n"));
	uart_puts_p(PSTR("write <file> <offset>\t: Writes text to <file>, starting from <offset>. The text is read from the UART, line by line. Finish with an empty line\n"));
	uart_puts_p(PSTR("rm <file>\t\t: Deletes <file>\n"));
	uart_puts_p(PSTR("md <directory>\t: Creates a directory called <directory>\n"));

	uart_puts_p(PSTR("cd <directory>\t\t: Changes current working directory to <directory>\n"));
	uart_puts_p(PSTR("cd ..\t\t: Changes to lower directory in tree\n"));
	uart_puts_p(PSTR("ls\t\t\t: Shows the content of the current directory\n"));
	uart_puts_p(PSTR("cat <file>\t\t: Writes a hexdump of <file> to the terminal\n"));
	uart_puts_p(PSTR("disk\t\t\t: Shows card manufacturer, status, filesystem capacity and free storage space\n"));
	uart_puts_p(PSTR("init\t\t\t: Reinitializes and reopens the memory card\n"));

	uart_puts_p(PSTR("sync\t\t\t: Ensures all buffered data is written to the card\n"));
	uart_puts_p(PSTR("\nMenus:\n"));
	uart_puts_p(PSTR("set\t\t\t: Menu to configure system boot mode\n"));
	uart_puts_p(PSTR("baud\t\t\t: Menu to configure baud rate\n"));
}

//Configure what baud rate to communicate at
void baud_menu(void)
{
	char buffer[5];

	while(1)
	{
		uart_puts_p(PSTR("\nBaud Configuration:\n"));
		
		uart_puts_p(PSTR("1) Set Baud rate 2400\n"));
		uart_puts_p(PSTR("2) Set Baud rate 9600\n"));
		uart_puts_p(PSTR("3) Set Baud rate 57600\n"));
		uart_puts_p(PSTR("4) Set Baud rate 115200\n"));

		//print prompt
		uart_putc('>');

		//read command
		char* command = buffer;

		if(read_line(command, sizeof(buffer)) < 1)
			continue;

		//execute command
		if(strcmp_P(command, PSTR("1")) == 0)
		{
			uart_puts_p(PSTR("\nGoing to 2400bps...\n"));

			//Set baud rate to 2400
			EEPROM_write(LOCATION_BAUD_SETTING, BAUD_2400);
			blink_error(ERROR_NEW_BAUD);
			return;
		}
		if(strcmp_P(command, PSTR("2")) == 0)
		{
			uart_puts_p(PSTR("\nGoing to 9600bps...\n"));

			//Set baud rate to 9600
			EEPROM_write(LOCATION_BAUD_SETTING, BAUD_9600);
			blink_error(ERROR_NEW_BAUD);
			return;
		}
		if(strcmp_P(command, PSTR("3")) == 0)
		{
			uart_puts_p(PSTR("\nGoing to 57600bps...\n"));

			//Set baud rate to 57600
			EEPROM_write(LOCATION_BAUD_SETTING, BAUD_57600);
			blink_error(ERROR_NEW_BAUD);
			return;
		}
		if(strcmp_P(command, PSTR("4")) == 0)
		{
			uart_puts_p(PSTR("\nGoing to 115200bps...\n"));

			//Set baud rate to 115200
			EEPROM_write(LOCATION_BAUD_SETTING, BAUD_115200);
			blink_error(ERROR_NEW_BAUD);
			return;
		}
	}
}

//Change how OpenLog works
//1) Turn on unit, unit will create new file, and just start logging
//2) Turn on, append to known file, and just start logging
//3) Turn on, sit at command prompt
void system_menu(void)
{
	char buffer[5];

	while(1)
	{
		uart_puts_p(PSTR("\nSystem Configuration\n"));
		uart_puts_p(PSTR("\nBoot to:\n"));
		
		uart_puts_p(PSTR("1) New file logging\n"));
		uart_puts_p(PSTR("2) Append file logging\n"));
		uart_puts_p(PSTR("3) Command prompt\n"));
		uart_puts_p(PSTR("4) Reset new file number\n"));

		//print prompt
		uart_putc('>');

		//read command
		char* command = buffer;

		if(read_line(command, sizeof(buffer)) < 1)
			continue;

		//execute command
		if(strcmp_P(command, PSTR("1")) == 0)
		{
			uart_puts_p(PSTR("New file logging\n"));
			EEPROM_write(LOCATION_SYSTEM_SETTING, MODE_NEWLOG);
			return;
		}
		if(strcmp_P(command, PSTR("2")) == 0)
		{
			uart_puts_p(PSTR("Append file logging\n"));
			EEPROM_write(LOCATION_SYSTEM_SETTING, MODE_SEQLOG);
			return;
		}
		if(strcmp_P(command, PSTR("3")) == 0)
		{
			uart_puts_p(PSTR("Command prompt\n"));
			EEPROM_write(LOCATION_SYSTEM_SETTING, MODE_COMMAND);
			return;
		}
		if(strcmp_P(command, PSTR("4")) == 0)
		{
			uart_puts_p(PSTR("New file number reset to zero\n"));
			EEPROM_write(LOCATION_FILE_NUMBER, 0);
			return;
		}
	}
}

#if FAT_DATETIME_SUPPORT
void get_datetime(uint16_t* year, uint8_t* month, uint8_t* day, uint8_t* hour, uint8_t* min, uint8_t* sec)
{
    *year = 2007;
    *month = 1;
    *day = 1;
    *hour = 0;
    *min = 0;
    *sec = 0;
}
#endif
