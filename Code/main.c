/*
 * Copyright (c) 2006-2009 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
    12-3-09
    Copyright SparkFun Electronics© 2010
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

	STAT1 LED is sitting on PD5 (Arduino D5) - toggles when character is received
	STAT2 LED is sitting on PB5 (Arduino D13) - toggles when SPI writes happen

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

	
	v1.2
	ringp added:
	Adding support for splitting command line parameters into arguments
	Adding support for reading files sequencially
		read <filename> <start> <length>
	New log now for sequencial log functions supports 0-65535 files
	Adding support for wildcard listing or deletion of files
		ls <wildcard search>
		rm <wildcard delete>

	Really great additions. Thanks ringp!
	
	Nate added error testing within newlog()
	Checks to see if we have 65534 logs. If so, error out to command prompt with "!Too many logs:1"
	
	
	v1.3
	Added sd_raw_sync() inside append_file. I believe this was why tz1's addition of the timeout buffer update feature
	was not working. Auto buffer update now working. So if you don't send anything to OpenLog for 5 seconds,
	the buffer will automatically record/update.
	
	Need to create 'Testing' page to outline the battery of tests we need to throw at any OpenLog after a firmware 
	submission and update is complete.
	
	Testing
	create 40,000 logs

	Record at full speed:
	Run at full 115200, load lotsoftext.txt and verify no characters are dropped.

	Detect too many logs:
	Create new log at 65533 (do this by editing 'zero' function to set EEPROM to 0xFF and oxFD) 
	and power cycle. Verify unit starts new log. Power cycle and verify unit errors out and drops to command prompt.

	Record buffer after timeout:
	Create new log. Type 20 characters and wait 5 seconds. Unit should auto-record buffer. Power down unit. 
	Power up unit and verify LOG has 20 characters recorded.	
	

	v1.4
	Added exit options to the two menus (set and baud)
	Also added display of current settin to two menus (Ex: "Baud currently: 57600bps")
	
	Added '!' infront of 'error opening'. This pops up if there is an error while trying to append
	to a freshly created new log (ex: LOG00548.txt is created, then errors out because it cannot append).
	'!' was added so that user can parse against it.
	
	Replicated logging errors at 57600 using 5V Arduino
	Unit would systematically glitch during logging of 111054 bytes
	
	Increasing buffer to 1000 characters caused URU error.
	URU: Unit Resets Unexpectedly
	
	To recreate URU error. Type "append ". Include the space. If you get "!error opening", then things are 
	fine. If you get "!error opening#" where # is a weird character, then type 'ls' and the unit will 
	unexpectedly reset (URU error). I believe this is attributed to a memory overrun somewhere in the
	FAT system.
	
	Changed buffer size to 900 and declared the character buffer as volatile
	#define BUFF_LEN 900
	volatile char input_buffer[BUFF_LEN];
	
	This increase to the buffer allows for clean logging of 444055 bytes with no URU errors.
	
	Experimenting with Scott's SD cards (customer gave cards on loan for recreating logging errors):
	Card with single ~740mb file produces errors when trying to open/append to new log. 
	Card with less stuff on it logs full 444055 bytes correctly.

	
	v1.5
	Added 4800bps and 19200bps support
	
	Added power saving features. Current consumption at 5V is now:
	In default append mode: 
		6.6/5.5mA while receiving characters (LED On/Off)
		2.1mA during idle
	In command mode: 3.2/2.1mA (LED On/Off)
	
	So if you're actively throwing characters at the logger, it will be ~6mA. If you send the logger
	characters then delay 5-10 seconds, current will be ~2.5mA. (Unit records the characters in the buffer
	and goes into idle more if no characters are received after 5 seconds)
	
	These power savings required some significant changes to uart.c / uart_getc()

	
	v1.51 check_emergency_reset, default break character is ctrl+z 3 times, example Arduino sketch
	
	Added function from mungewell - check_emergency_reset. This has improved testing of the RX pin.
	There was potential to get a false baud reset. There is still a chance but it's now much less likely.
	
	If OpenLog is attached to a Arduino, during bootloading of the Arduino, ctrl+z will most likely be sent
	to the Arduino from the computer. This character will cause OpenLog to drop to command mode - probably
	not what we want. So I added user selectable character (ctrl+x or '$' for example) and I added
	user selectable number of escape characters to look for in a row (example is 1 or 2 or 3, '$$$' is a
	common escape sequence). The default is now ctrl+z sent 3 times in a row.
	
	Added an example Arduino sketch (from ScottH) to GitHub so that people can easily see how to send characters to
	OpenLog. Not much to it, but it does allow us to test large amounts of text thrown at OpenLog
	at 57600bps.
	
	
	v1.6 Adding config file.
	
	What happens if I reset the system by pulling RX low, but the config file has corrupt values in it?
	
	If config file has corrupt values in it, system will default to known values 9600/ctrl+z/3/newlog
	
	If config file is empty, system resets to known values
	
	After some massive testing, and lots of code to check for illegal states, it looks to be pretty stable. 
	The only problem is that we're running out of RAM. The buffer had to be decreased from 900 bytes 
	to 700 bytes to facilitate all the config file checking. Testing at 57600bps, unit runs very well
	over 40kb test file on straight RS232 connection. That's pretty good. Testing at 115200 on straight 
	connection, unit will drop a buffer every once and a while. Not great, but not much we can do if the
	SD card times out for ~150ms while it's writing.
	8 bits to the byte plus a start/stop bit = 10 bits per byte
	
	@ 9600bps = 960 bytes per second. Buffer will last for 729ms
	@ 57600bps = 5760 bytes per second. Buffer will last for 121ms
	@ 115200bps = 11520 bytes per second. Buffer will last for 60.7ms
	
	So if the SD card pauses for more than 60ms, 115200 will have lost data, sometimes. All other baud rates
	should be covered for the most part.
	
	SD cards with larges amounts of data will have increased pause rates. Always use a clean card where possible.


	v1.61 Small PCB change. Fixed version # in help menu.

	Fixed the firmware version in the help menu to v1.61.

	Updated Eagle files to Eagle v5.9. Fixed weird airwire. Changed D1 LED from Green to Blue. 
	Will only affect new production after 4-28-10.
	
	Closed some tickets and wrote some more example Arduino code:
	http://forum.sparkfun.com/viewtopic.php?t=21438

*/

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


#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "fat.h"
#include "fat_config.h"
#include "partition.h"
#include "sd_raw.h"
#include "sd_raw_config.h"

#define BUFF_LEN 700
volatile char input_buffer[BUFF_LEN];
char general_buffer[25];
volatile uint16_t read_spot, checked_spot;

#define CFG_FILENAME	"CONFIG.txt"

#include "uart.h"


//Setting DEBUG to 1 will cause extra serial messages to appear
//such as "Media Init Complete!" indicating the SD card was initalized
#define DEBUG 0

//Function declarations
static uint8_t read_line(char* buffer, uint8_t buffer_length);
static uint32_t strtolong(const char* str);
static uint8_t find_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry, uint8_t use_wild_card);
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

char check_emergency_reset(void);
void set_default_settings(void);
void read_system_settings(void);

void read_config_file(void);
void record_config_file(void);

void blink_error(uint8_t ERROR_TYPE);

void EEPROM_write(uint16_t uiAddress, unsigned char ucData);
unsigned char EEPROM_read(uint16_t uiAddress);

void delay_us(uint16_t x);
void delay_ms(uint16_t x);

//These functions were added for wild card delete and search
//======================================================
uint8_t count_cmd_args(void);
char* get_cmd_arg(uint8_t index);
void add_cmd_arg(char* buffer, uint8_t buffer_length);
uint8_t split_cmd_line_args(char* buffer, uint8_t buffer_length);
uint8_t too_many_arguments_error(uint8_t limit, char* command);
char* is_number(char* buffer, uint8_t buffer_length);
uint8_t wildcmp(const char* wild, const char* string);

struct command_arg
{
	char* arg; 			//Points to first character in command line argument
	uint8_t arg_length; // Length of command line argument
};
//End Wildcard Functions ===============================


#define sbi(port, port_pin)   ((port) |= (uint8_t)(1 << port_pin))
#define cbi(port, port_pin)   ((port) &= (uint8_t)~(1 << port_pin))
#define MIN(a,b) ((a)<(b))?(a):(b)

//STAT1 is a general LED and indicates serial traffic
#define STAT1	5
#define STAT1_PORT	PORTD

//STAT2 is tied to the SCL line and indicates volume SPI traffic
#define STAT2	5
#define STAT2_PORT	PORTB

#define ERROR_SD_INIT	3
#define ERROR_NEW_BAUD	5

#define LOCATION_BAUD_SETTING		0x01
#define LOCATION_SYSTEM_SETTING		0x02
#define LOCATION_FILE_NUMBER_LSB	0x03
#define LOCATION_FILE_NUMBER_MSB	0x04
#define LOCATION_ESCAPE_CHAR		0x05
#define LOCATION_MAX_ESCAPE_CHAR	0x06

#define BAUD_2400	0
#define BAUD_9600	1
#define BAUD_57600	2
#define BAUD_115200	3
#define BAUD_4800	4
#define BAUD_19200	5

#define MODE_NEWLOG	0
#define MODE_SEQLOG 1
#define MODE_COMMAND 2
#define MAX_COUNT_COMMAND_LINE_ARGS 4

//Global variables
struct fat_fs_struct* fs;
struct partition_struct* partition;
struct fat_dir_struct* dd;
static struct command_arg cmd_arg[MAX_COUNT_COMMAND_LINE_ARGS];

char setting_uart_speed; //This is the baud rate that the system runs at, default is 9600
char setting_system_mode; //This is the mode the system runs in, default is MODE_NEWLOG
char setting_escape_character; //This is the ASCII character we look for to break logging, default is ctrl+z
char setting_max_escape_character; //Number of escape chars before break logging, default is 3

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

	//If we are in new log mode, find a new file name to write to
	if(setting_system_mode == MODE_NEWLOG)
		newlog();

	//If we are in sequential log mode, determine if seqlog.txt has been created or not, and then open it for logging
	if(setting_system_mode == MODE_SEQLOG)
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

	//Running power is 7.66mA at 3.3V / 7.23 at 5V before power tweaking
	//Let's see if we can shut off some peripherals and save some power
	PRR |= (1<<PRTWI) | (1<<PRTIM2) | (1<<PRTIM1) | (1<<PRADC); //Shut off TWI, Timer2, Timer1, ADC
	//Running power is 7.02mA at 3.3V / 6.66mA at 5V after power tweaking - so a little bit, and it still works!
	
	set_sleep_mode(SLEEP_MODE_IDLE); //I believe this is the lowest we can go and still get woken up by UART
	sleep_enable(); //Set Sleep Enable bit to 1

    //1 = output, 0 = input
    DDRD |= (1<<STAT1); //PORTD (STAT1 on PD5)
    DDRB |= (1<<STAT2); //PORTC (STAT2 on PB5)

	if(check_emergency_reset()) //Look to see if the RX pin is being pulled low
	{
		set_default_settings(); //Reset baud, escape characters, escape number, system mode

		init_media(); //Try to setup the SD card so we can record these new settings
		
		record_config_file(); //Record new config settings

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
	
	read_system_settings(); //Read the system settings into some global variables

    //Setup uart
    uart_init(setting_uart_speed);
#if DEBUG
	uart_puts_p(PSTR("UART Init\n"));
#else
	uart_puts_p(PSTR("1"));
#endif
	
	//Setup SPI, init SD card, etc
	init_media();
	uart_puts_p(PSTR("2"));
	
	read_config_file(); //Read the system settings from the config file	
}

void read_config_file(void)
{
	char config_file_name[13];
	sprintf(config_file_name, CFG_FILENAME); //This is the name of the config file. 'config.sys' is probably a bad idea.

	struct fat_dir_entry_struct file_entry;

	//Check to see if we have a config file
	if(find_file_in_dir(fs, dd, config_file_name, &file_entry, 0))
	{
		//If we found the config file then load settings from file and push them into EEPROM
		#if DEBUG
			uart_puts("\n\nFound config file!");
		#endif
		
		//Now load settings from file
		
		//search file in current directory and open it
		struct fat_file_struct* fd = open_file_in_dir(fs, dd, config_file_name);
		if(!fd)
		{
			uart_puts_p(PSTR("error opening config file"));
			return;
		}

		//int32_t offset = 0;
		//Seeks the end of the file : offset = EOF location
		/*if(!fat_seek_file(fd, &offset, FAT_SEEK_END))
		{
			uart_puts_p(PSTR("error seeking config file"));
			fat_close_file(fd);
			return;
		}*/

		/*char mytemp[30];
		sprintf(mytemp, "\noffset: %d\n", offset);
		uart_puts(mytemp);*/


		//Read line from file
		uint8_t settings_string[16]; //"115200,103,14,0\0" = 115200 bps, escape char of ASCII(103), 14 times, new log mode
		uint8_t len;
		len = fat_read_file(fd, settings_string, sizeof(settings_string)); //Read first line of file

		fat_close_file(fd);

		//Print line for debugging
		#if DEBUG
			if(len > 0)
			{
				uart_putc('\n');
				for(uint8_t i = 0; i < len; ++i)
					uart_putc(settings_string[i]);
			}
		#endif
		
		//Default the system settings in case things go horribly wrong
		char new_system_baud = BAUD_9600;
		char new_system_mode = MODE_NEWLOG;
		char new_system_escape = 26;
		char new_system_max_escape = 3;

		//Parse the settings out
		uint8_t i = 0, j = 0, setting_number = 0;
		char new_setting[7]; //Max length of a setting is 6, the bps setting = '115200' plus '\0'
		for(i = 0 ; i < len; i++)
		{
			//Pick out one setting from the line of text
			for(j = 0 ; settings_string[i] != ',' && i < len && j < 6 ; )
			{
				new_setting[j] = settings_string[i];
				i++;
				j++;
			}
			
			new_setting[j] = '\0'; //Terminate the string for array compare
			
			if(setting_number == 0) //Baud rate
			{
				if( strcmp(new_setting, "2400") == 0) new_system_baud = BAUD_2400;
				else if( strcmp(new_setting, "4800") == 0) new_system_baud = BAUD_4800;
				else if( strcmp(new_setting, "9600") == 0) new_system_baud = BAUD_9600;
				else if( strcmp(new_setting, "19200") == 0) new_system_baud = BAUD_19200;
				else if( strcmp(new_setting, "57600") == 0) new_system_baud = BAUD_57600;
				else if( strcmp(new_setting, "115200") == 0) new_system_baud = BAUD_115200;
				else new_system_baud = BAUD_9600; //Default is 9600bps
			}
			else if(setting_number == 1) //Escape character
			{
				new_system_escape = atoi(new_setting);
				if(new_system_escape == 0 || new_system_escape > 127) new_system_escape = 26; //Default is ctrl+z
			}
			else if(setting_number == 2) //Max amount escape character
			{
				new_system_max_escape = atoi(new_setting);
				if(new_system_max_escape == 0 || new_system_max_escape > 10) new_system_max_escape = 3; //Default is 3
			}
			else if(setting_number == 3) //System mode
			{
				new_system_mode = atoi(new_setting);
				if(new_system_mode == 0 || new_system_mode > 5) new_system_mode = MODE_NEWLOG; //Default is NEWLOG
			}
			else
				//We're done! Stop looking for settings
				break;
			
			setting_number++;
		}

		//This will print the found settings. Use for debugging
		/*
		uart_puts_p(PSTR("\nSettings found: "));

		char temp_string[16]; //"115200,103,14,0\0" = 115200 bps, escape char of ASCII(103), 14 times, new log mode
		char temp[3];

		if(system_baud == BAUD_2400) strcpy(temp_string, "2400");
		if(system_baud == BAUD_4800) strcpy(temp_string, "4800");
		if(system_baud == BAUD_9600) strcpy(temp_string, "9600");
		if(system_baud == BAUD_19200) strcpy(temp_string, "19200");
		if(system_baud == BAUD_57600) strcpy(temp_string, "57600");
		if(system_baud == BAUD_115200) strcpy(temp_string, "115200");
		
		strcat(temp_string, ",");
		
		//Convert escape character to an ASCII visible string
		sprintf(temp, "%d", system_escape);
		strcat(temp_string, temp); //Add this string to the system string

		strcat(temp_string, ",");

		//Convert max escape character to an ASCII visible string
		sprintf(temp, "%d", system_max_escape);
		strcat(temp_string, temp); //Add this string to the system string

		strcat(temp_string, ",");

		//Convert system mode to a ASCII visible character
		sprintf(temp, "%d", system_mode);
		strcat(temp_string, temp); //Add this string to the system string

		strcat(temp_string, "\0");
		
		uart_puts(temp_string);

		uart_puts_p(PSTR("\n"));
		*/
		
		//We now have the settings loaded into the global variables. Now check if they're different from EEPROM settings

		if(new_system_baud != setting_uart_speed)
		{
			//If the baud rate from the file is different from the current setting,
			//Then update the setting to the file setting
			//And re-init the UART
			EEPROM_write(LOCATION_BAUD_SETTING, new_system_baud);
			setting_uart_speed = new_system_baud;
		    uart_init(setting_uart_speed);
		}

		if(new_system_mode != setting_system_mode)
		{
			//Goto new system mode
			setting_system_mode = new_system_mode;
			EEPROM_write(LOCATION_SYSTEM_SETTING, setting_system_mode);
		}
		
		if(new_system_escape != setting_escape_character)
		{
			//Goto new system escape char
			setting_escape_character = new_system_escape;
			EEPROM_write(LOCATION_ESCAPE_CHAR, setting_escape_character); 
		}
		
		if(new_system_max_escape != setting_max_escape_character)
		{
			//Goto new max escape
			setting_max_escape_character = new_system_max_escape;
			EEPROM_write(LOCATION_MAX_ESCAPE_CHAR, setting_max_escape_character);
		}
		
		record_config_file(); //If we corrected some values because the config file was corrupt, then overwrite any corruption
	
		//All done! New settings are loaded. System will now operate off new config settings found in file.
	}
	else
	{
		//If we don't have a config file already, then create config file and record the current system settings to the file
		#if DEBUG
			uart_puts("No config found - creating default:\n");
		#endif

		//Record the current eeprom settings to the config file
		record_config_file();
	}
	
}

//Records the current EEPROM settings to the config file
//If a config file exists, it is trashed and a new one is created
void record_config_file(void)
{
	struct fat_dir_entry_struct file_entry;

	char config_file_name[13];
	sprintf(config_file_name, CFG_FILENAME); //This is the name of the config file. 'config.sys' is probably a bad idea.

	//If there is currently a config file, trash it
	if(find_file_in_dir(fs, dd, config_file_name, &file_entry, 0))
	{
		#if DEBUG
			uart_puts("\n\nDeleting config\n");
		#endif
		
		fat_delete_file(fs, &file_entry);
	}

	//Create config file
	if(fat_create_file(dd, config_file_name, &file_entry) == 0)
	{
		uart_puts("Failed to create config file");
		return;
	}
	else
	{
		//Config was successfully created, so let's fill it with default settings

		struct fat_file_struct* fd = open_file_in_dir(fs, dd, config_file_name);
		if(!fd)
		{
			uart_puts("!error opening config file\n");
			return;
		}

		//Read current system settings to the config file

		//Baud, escape character, escape times, system mode (0 = new log)
		char settings_string[16]; //"115200,103,14,0\0" = 115200 bps, escape char of ASCII(103), 14 times, new log mode
		char temp[3];

		//Before we read the EEPROM values, they've already been tested and defaulted in the read_system_settings function
		char current_system_baud = EEPROM_read(LOCATION_BAUD_SETTING);
		char current_system_mode = EEPROM_read(LOCATION_SYSTEM_SETTING);
		char current_system_escape = EEPROM_read(LOCATION_ESCAPE_CHAR);
		char current_system_max_escape = EEPROM_read(LOCATION_MAX_ESCAPE_CHAR);
		
		if(current_system_baud == BAUD_2400) strcpy(settings_string,"2400");
		if(current_system_baud == BAUD_4800) strcpy(settings_string,"4800");
		if(current_system_baud == BAUD_9600) strcpy(settings_string,"9600");
		if(current_system_baud == BAUD_19200) strcpy(settings_string,"19200");
		if(current_system_baud == BAUD_57600) strcpy(settings_string,"57600");
		if(current_system_baud == BAUD_115200) strcpy(settings_string,"115200");
		
		strcat(settings_string, ",");
		
		//Convert escape character to an ASCII visible string
		sprintf(temp, "%d", current_system_escape);
		strcat(settings_string, temp); //Add this string to the system string

		strcat(settings_string, ",");

		//Convert max escape character to an ASCII visible string
		sprintf(temp, "%d", current_system_max_escape);
		strcat(settings_string, temp); //Add this string to the system string

		strcat(settings_string, ",");

		//Convert system mode to a ASCII visible character
		sprintf(temp, "%d", current_system_mode);
		strcat(settings_string, temp); //Add this string to the system string

		strcat(settings_string, "\0");

		//Record current system settings to the config file
		//strcpy( (char*)input_buffer, "9600,26,3,0\0");
		strcpy( (char*)input_buffer, settings_string);

		#if DEBUG
			uart_puts_p(PSTR("\nSetting string: "));
			uart_puts(settings_string);
		#endif

		if( fat_write_file(fd, (uint8_t*)input_buffer, strlen(settings_string) ) != strlen(settings_string) )
			uart_puts("error writing to config\n");

		fat_close_file(fd);

		sd_raw_sync(); //Sync all newly written data to card
		
		//Now the new config file has the current system settings, nothing else to do!
	}
}

//Resets all the system settings to safe values
void set_default_settings(void)
{
	//Reset UART to 9600bps
	EEPROM_write(LOCATION_BAUD_SETTING, BAUD_9600);

	//Reset system to new log mode
	EEPROM_write(LOCATION_SYSTEM_SETTING, MODE_NEWLOG);

	//Reset escape character to ctrl+z
	EEPROM_write(LOCATION_ESCAPE_CHAR, 26); 

	//Reset number of escape characters to 3
	EEPROM_write(LOCATION_MAX_ESCAPE_CHAR, 3);

	//These settings are not recorded to the config file
	//We can't do it here because we are not sure the FAT system is init'd
}

//Reads the current system settings from EEPROM
//If anything looks weird, reset setting to default value
void read_system_settings(void)
{
	//Read what the current UART speed is from EEPROM memory
	//Default is 9600
	setting_uart_speed = EEPROM_read(LOCATION_BAUD_SETTING);
	if(setting_uart_speed > 10) 
	{
		setting_uart_speed = BAUD_9600; //Reset UART to 9600 if there is no speed stored
		EEPROM_write(LOCATION_BAUD_SETTING, setting_uart_speed);
	}

	//Determine the system mode we should be in
	//Default is NEWLOG mode
	setting_system_mode = EEPROM_read(LOCATION_SYSTEM_SETTING);
	if(setting_system_mode > 5) 
	{
		setting_system_mode = MODE_NEWLOG; //By default, unit will turn on and go to new file logging
		EEPROM_write(LOCATION_SYSTEM_SETTING, setting_system_mode);
	}

	//Read the escape_character
	//ASCII(26) is ctrl+z
	setting_escape_character = EEPROM_read(LOCATION_ESCAPE_CHAR);
	if(setting_escape_character == 0 || setting_escape_character == 255) 
	{
		setting_escape_character = 26; //Reset escape character to ctrl+z
		EEPROM_write(LOCATION_ESCAPE_CHAR, setting_escape_character);
	}

	//Read the number of escape_characters to look for
	//Default is 3
	setting_max_escape_character = EEPROM_read(LOCATION_MAX_ESCAPE_CHAR);
	if(setting_max_escape_character == 0 || setting_max_escape_character == 255) 
	{
		setting_max_escape_character = 3; //Reset number of escape characters to 3
		EEPROM_write(LOCATION_MAX_ESCAPE_CHAR, setting_max_escape_character);
	}
}

//Check to see if we need an emergency UART reset
//Scan the RX pin for 2 seconds
//If it's low the entire time, then return 1
char check_emergency_reset(void)
{
	DDRD |= (1<<0); //Turn the RX pin into an input
	PORTD |= (1<<0); //Push a 1 onto RX pin to enable internal pull-up

	//Quick pin check
	if( (PIND & (1<<0)) == 1) return 0;

	//Wait 2 seconds, blinking LEDs while we wait
	sbi(PORTC, STAT2); //Set the STAT2 LED
	for(uint8_t i = 0 ; i < 40 ; i++)
	{
		delay_ms(25);
		PORTD ^= (1<<STAT1); //Blink the stat LEDs

		if( (PIND & (1<<0)) == 1) return 0;

		delay_ms(25);
		PORTB ^= (1<<STAT2); //Blink the stat LEDs

		if( (PIND & (1<<0)) == 1) return 0;
	}		

	//If we make it here, then RX pin stayed low the whole time
	return 1;
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
//Limited to 65535 files but this should not always be the case.
void newlog(void)
{
	uint8_t msb, lsb;
	uint16_t new_file_number;

	//Combine two 8-bit EEPROM spots into one 16-bit number
	lsb = EEPROM_read(LOCATION_FILE_NUMBER_LSB);
	msb = EEPROM_read(LOCATION_FILE_NUMBER_MSB);

	new_file_number = msb;
	new_file_number = new_file_number << 8;
	new_file_number |= lsb;
	
	//If both EEPROM spots are 255 (0xFF), that means they are un-initialized (first time OpenLog has been turned on)
	//Let's init them both to 0
	if((lsb == 255) && (msb == 255))
	{
		new_file_number = 0; //By default, unit will start at file number zero
		EEPROM_write(LOCATION_FILE_NUMBER_LSB, 0x00);
		EEPROM_write(LOCATION_FILE_NUMBER_MSB, 0x00);
	}

	//The above code looks like it will forever loop if we ever create 65535 logs
	//Let's quit if we ever get to 65534
	//65534 logs is quite possible if you have a system with lots of power on/off cycles
	if(new_file_number == 65534)
	{
		//Gracefully drop out to command prompt with some error
		uart_puts_p(PSTR("!Too many logs:1!"));

		return; //Bail!
	}
	
	//If we made it this far, everything looks good - let's create the new LOG and write to it

	char* new_file_name = general_buffer;
	sprintf(new_file_name, "LOG%05u.txt", new_file_number);

	struct fat_dir_entry_struct file_entry;
	while(!fat_create_file(dd, new_file_name, &file_entry))
	{
		//Increment the file number because this file name is already taken
		new_file_number++;
		sprintf(new_file_name, "LOG%05u.txt", new_file_number);
		
		//Shoot! There's still a chance that we can have too many logs here
		//For example, if all the way up to LOG65533 was already on card, 
		//then reset the EEPROM log number, 65533 would be skipped, 65534 would be created
		//and the above 65534 test would be skipped
		
		if(new_file_number > 65533)
		{
			//Gracefully drop out to command prompt with some error
			uart_puts_p(PSTR("!Too many logs:2!"));
			return; //Bail!
		}
	}

	//Add one the new_file_number for the next power-up
	new_file_number++;

	//Record new_file number to EEPROM but do not waste too many
	//write cycles to the EEPROM as it will wear out. Only write if
	//needed
	lsb = (uint8_t)(new_file_number & 0x00FF);
	msb = (uint8_t)((new_file_number & 0xFF00) >> 8);

	EEPROM_write(LOCATION_FILE_NUMBER_LSB, lsb); // LSB

	if (EEPROM_read(LOCATION_FILE_NUMBER_MSB) != msb)
		EEPROM_write(LOCATION_FILE_NUMBER_MSB, msb); // MSB
	
#if DEBUG
	uart_puts_p(PSTR("\nCreated new file: "));
	uart_puts(new_file_name);
	uart_puts_p(PSTR("\n"));
#endif

	//Begin writing to file
	append_file(new_file_name);
}

#if DEBUG
//This function creates 40,000 directories when called. It is used to test
//how many files a root directory on OpenLog can be created.
//Only enabled during debugging

//Update: for some reason, creating sequential files like this takes longer, and longer, and longer.
//But a power up goes much faster (to discover and create a new file).
//Not sure why it takes so long, but creating 250 files takes many many many minutes.
void create_lots_of_files(void)
{
	uart_puts_p(PSTR("\nCreating tons of files!"));

	uint16_t files_to_create;
	uint16_t new_file_number;
	
	for(files_to_create = 0 ; files_to_create < 100 ; files_to_create++)
	{
		new_file_number = files_to_create;
		
		//Generic file creation routine:
		//=====================

		char* new_file_name = general_buffer;
		sprintf(new_file_name, "LOG%05u.txt", new_file_number);

		uart_puts_p(PSTR("1"));

		struct fat_dir_entry_struct file_entry;
		while(!fat_create_file(dd, new_file_name, &file_entry))
		{
			uart_puts_p(PSTR("2"));

			//Increment the file number because this file name is already taken
			new_file_number++;
			sprintf(new_file_name, "LOG%05u.txt", new_file_number);
			
			//Shoot! There's still a chance that we can have too many logs here
			//For example, if all the way up to LOG65533 was already on card, 
			//then reset the EEPROM log number, 65533 would be skipped, 65534 would be created
			//and the above 65534 test would be skipped
			
			if(new_file_number > 65533)
			{
				//Gracefully drop out to command prompt with some error
				uart_puts_p(PSTR("!Too many logs:2!"));
				return; //Bail!
			}
		}

		//=====================

		//File created!
		uart_puts_p(PSTR("\nF:"));
		uart_puts(new_file_name);
		uart_puts_p(PSTR("\n"));
	}
}
#endif


void command_shell(void)
{
	//provide a simple shell
	char buffer[24];
	uint8_t tmp_var;

	while(1)
	{
		//print prompt
		uart_putc('>');

		//read command
		char* command = buffer;
		if(read_line(command, sizeof(buffer)) < 1)
			continue;

		//Argument 1: The actual command
		char* command_arg = get_cmd_arg(0);

		//execute command
		if(strcmp_P(command_arg, PSTR("init")) == 0)
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
		else if(strcmp_P(command_arg, PSTR("?")) == 0)
		{
			//Print available commands
			print_menu();
		}
		else if(strcmp_P(command_arg, PSTR("help")) == 0)
		{
			//Print available commands
			print_menu();
		}
		else if(strcmp_P(command_arg, PSTR("baud")) == 0)
		{
			//Go into baud select menu
			baud_menu();
		}
		else if(strcmp_P(command_arg, PSTR("set")) == 0)
		{
			//Go into system setting menu
			system_menu();
		}
		#if DEBUG
		else if(strcmp_P(command_arg, PSTR("create")) == 0)
		{
			//Go into system setting menu
			create_lots_of_files();
		}
		#endif
		else if(strncmp_P(command_arg, PSTR("cd"), 2) == 0)
		{
			//Expecting only 2 arguments
			if (too_many_arguments_error(2, command))
				continue;

			//Argument 2: Directory name
			command_arg = get_cmd_arg(1);
			if(command_arg == 0)
				continue;

			//change directory, do not use wildcards
			struct fat_dir_entry_struct subdir_entry;
			if(find_file_in_dir(fs, dd, command_arg, &subdir_entry, 0))
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
			uart_puts(command_arg);
			uart_putc('\n');
		}
		else if(strcmp_P(command_arg, PSTR("ls")) == 0)
		{
			//Argument 2: wild card search
			command_arg = get_cmd_arg(1);

			/* print directory listing */
			struct fat_dir_entry_struct dir_entry;
			while(fat_read_dir(dd, &dir_entry))
			{
				//Check if we are to do a wild card search
				tmp_var = (command_arg == 0);
				if(command_arg != 0)
					if (wildcmp(command_arg, dir_entry.long_name))
						tmp_var = 1;

				//If no arguments list all files, otherwise we only list the files
				//being matched by the wildcard search
				if (tmp_var)
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
		}
		else if(strncmp_P(command_arg, PSTR("cat"), 3) == 0)
		{
			//Expecting only 2 arguments
			if (too_many_arguments_error(2, command))
				continue;

			//Argument 2: File name
			command_arg = get_cmd_arg(1);
			if(command_arg == 0)
				continue;

			/* search file in current directory and open it */
			struct fat_file_struct* fd = open_file_in_dir(fs, dd, command_arg);
			if(!fd)
			{
				uart_puts_p(PSTR("error opening "));
				uart_puts(command_arg);
				uart_putc('\n');
				continue;
			}

			/* print file contents */
			uint8_t buffer[8];
			uint32_t offset = 0;
			uint8_t len;
			while((len = fat_read_file(fd, buffer, sizeof(buffer))) > 0)
			{
				uart_putdw_hex(offset);
				uart_putc(':');
				for(uint8_t i = 0; i < len; ++i)
				{
					uart_putc(' ');
					uart_putc_hex(buffer[i]);
				}
				uart_putc('\n');
				offset += 8;
			}

			fat_close_file(fd);
		}
		else if(strncmp_P(command_arg, PSTR("read"), 4) == 0)
		{
			//Argument 2: File name
			command_arg = get_cmd_arg(1);
			if(command_arg == 0)
				continue;

			/* search file in current directory and open it */
			struct fat_file_struct* fd = open_file_in_dir(fs, dd, command_arg);
			if(!fd)
			{
				uart_puts_p(PSTR("error opening "));
				uart_puts(command_arg);
				uart_putc('\n');
				continue;
			}

			//Argument 3: File seek position
			if ((command_arg = get_cmd_arg(2)) != 0)
			{
				if ((command_arg = is_number(command_arg, strlen(command_arg))) != 0)
				{
					int32_t offset = strtolong(command_arg);
					if(!fat_seek_file(fd, &offset, FAT_SEEK_SET))
					{
						uart_puts_p(PSTR("error seeking on "));
						uart_puts(command);
						uart_putc('\n');

						fat_close_file(fd);
						continue;
					}
				}
			}

			//Argument 4: How much data (number of characters) to read from file
			uint32_t chunk_to_read = (uint32_t)-1;
			if ((command_arg = get_cmd_arg(3)) != 0)
				if ((command_arg = is_number(command_arg, strlen(command_arg))) != 0)
					chunk_to_read = strtolong(command_arg);

			/* print file contents */
			uint8_t buffer;
			while((fat_read_file(fd, &buffer, 1) > 0) && (chunk_to_read > 0))
			{
				if( buffer >= ' ' && buffer < 127 )
					uart_putc(buffer);
				else if (buffer == '\n' )
					uart_putc(buffer);
				else
					uart_putc('.');

				chunk_to_read--;
			}
			uart_putc('\n');
			fat_close_file(fd);
		}
		else if(strcmp_P(command_arg, PSTR("disk")) == 0)
		{
			if(!print_disk_info(fs))
				uart_puts_p(PSTR("error reading disk info\n"));
		}
		else if(strncmp_P(command_arg, PSTR("size"), 4) == 0)
		{
			//Expecting only 2 arguments
			if (too_many_arguments_error(2, command))
				continue;

			//Argument 2: File name - no wildcard search
			command_arg = get_cmd_arg(1);
			if(command_arg == 0)
				continue;

			struct fat_dir_entry_struct file_entry;
			if(find_file_in_dir(fs, dd, command_arg, &file_entry, 0))
			{
				uart_putdw_dec(file_entry.file_size);
				uart_putc('\n');
			}
            else
				uart_puts("-1\n");
		}
#if FAT_WRITE_SUPPORT
		else if(strncmp_P(command_arg, PSTR("rm"), 2) == 0)
		{
			//Expecting max 3 arguments
			if (too_many_arguments_error(3, command))
				continue;

			//Argument 2: File name or wildcard removal
			command_arg = get_cmd_arg(1);
			if(command_arg == 0)
				continue;

			struct fat_dir_entry_struct file_entry;
			while(find_file_in_dir(fs, dd, command_arg, &file_entry, 1))
			{
				if(!fat_delete_file(fs, &file_entry))
				{
					//Some kind of error, but continue anyway
					uart_puts_p(PSTR("error deleting file: "));
					uart_puts(command);
					uart_putc('\n');
				}
			}

		}
		else if(strncmp_P(command_arg, PSTR("new"), 3) == 0)
		{
			//Expecting only 2 arguments
			if (too_many_arguments_error(2, command))
				continue;

			//Argument 2: File name
			command_arg = get_cmd_arg(1);
			if(command_arg == 0)
				continue;

			struct fat_dir_entry_struct file_entry;
			if(!fat_create_file(dd, command_arg, &file_entry))
			{
				uart_puts_p(PSTR("error creating file: "));
				uart_puts(command);
				uart_putc('\n');
			}
		}
		else if(strncmp_P(command_arg, PSTR("write"), 5) == 0)
		{
			//Argument 2: File name
			command_arg = get_cmd_arg(1);
			if(command_arg == 0)
				continue;

			//Argument 3: Offset value - do not continue if the value is not correct
			char* offset_buffer;
			if ((offset_buffer = get_cmd_arg(2)) != 0)
				if ((offset_buffer = is_number(offset_buffer, strlen(offset_buffer))) == 0)
					continue;


			/* search file in current directory and open it */
			struct fat_file_struct* fd = open_file_in_dir(fs, dd, command_arg);
			if(!fd)
			{
				uart_puts_p(PSTR("error opening "));
				uart_puts(command_arg);
				uart_putc('\n');
				continue;
			}

			//Seek file position
			int32_t offset = strtolong(offset_buffer);
			if(!fat_seek_file(fd, &offset, FAT_SEEK_SET))
			{
				uart_puts_p(PSTR("error seeking on "));
				uart_puts(command_arg);
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

		else if(strncmp_P(command_arg, PSTR("append"), 6) == 0)
		{
			//Expecting only 2 arguments
			if (too_many_arguments_error(2, command))
				continue;

			//Argument 2: File name
			//Find the end of a current file and begins writing to it
			//Ends only when the user inputs Ctrl+z (ASCII 26)
			command_arg = get_cmd_arg(1);
			if(command_arg == 0)
				continue;
				
			append_file(command_arg); //Uses circular buffer to capture full stream of text and append to file
		}
		else if(strncmp_P(command_arg, PSTR("md"), 2) == 0)
		{
			//Argument 2: Directory name
			command_arg = get_cmd_arg(1);
			if(command_arg == 0)
				continue;

			struct fat_dir_entry_struct dir_entry;
			if(!fat_create_dir(dd, command_arg, &dir_entry))
			{
				uart_puts_p(PSTR("error creating directory: "));
				uart_puts(command_arg);
				uart_putc('\n');
			}
		}
#endif
#if SD_RAW_WRITE_BUFFERING
		else if(strcmp_P(command_arg, PSTR("sync")) == 0)
		{
			if(!sd_raw_sync())
				uart_puts_p(PSTR("error syncing disk\n"));
		}
#endif
		else
		{
			uart_puts_p(PSTR("unknown command: "));
			uart_puts(command_arg);
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
		uart_puts_p(PSTR("!error opening "));
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
		uart_puts_p(PSTR("!error seeking on "));
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

	char escape_chars_received = 0;
	
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

		uint16_t timeout_counter = 0;

		while(checked_spot == read_spot) 
		{ 
			if( ++timeout_counter > 5000 ) 
			{
				timeout_counter = 0;

				if(checked_spot != 0 && checked_spot != (BUFF_LEN/2)) // stuff in buff
				{
					if(checked_spot < (BUFF_LEN/2))
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
					unsigned spot = checked_spot > BUFF_LEN/2 ? BUFF_LEN/2 : 0;
					unsigned sp = spot; // start of new buffer
					
					// read_spot may have moved, copy
					cli();
					
					while(checked_spot != read_spot) 
					{
						input_buffer[spot++] = input_buffer[checked_spot++];
						if( checked_spot >= BUFF_LEN )
							checked_spot = 0;
					}
					
					read_spot = spot; // set insertion to end of copy
					checked_spot = sp; // reset checked to beginning of copy
					
					sei();
				}
				
				sd_raw_sync(); //Sync all newly written data to card

				//Hang out while we wait for the interrupt to occur and advance read_spot
				while(checked_spot == read_spot)
				{
					PORTD &= ~(1<<STAT1); //Turn off LED to save more power

					sleep_mode(); //Put CPU to sleep, UART ISR wakes us up
					//delay_ms(1); 
				}
			}

			delay_ms(1); //Hang out while we wait for the interrupt to occur and advance read_spot
		}

		if(input_buffer[checked_spot] == setting_escape_character) //Scan for escape character
		{
			escape_chars_received++;
			
			if(escape_chars_received == setting_max_escape_character)
			{
				//Disable interrupt and we're done!
				cli();
				UCSR0B &= ~(1<<RXCIE0); //Clear receive interrupt enable
				
				break;
			}
		}
		else
			escape_chars_received = 0;
		
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

	//Make sure all file handles are cleared
	fat_clear_handles();

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
        uint8_t c = uart_getc();

		//PORTD ^= (1<<STAT1); //Blink the stat LED while typing - this is taken care of in the UART ISR
		//PORTB ^= (1<<STAT2); //Blink the stat LED while typing - I don't want the SPI lines toggling

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

	//Split the command line into arguments
	split_cmd_line_args(buffer, buffer_length);

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

uint8_t find_file_in_dir(struct fat_fs_struct* fs, struct fat_dir_struct* dd, const char* name, struct fat_dir_entry_struct* dir_entry, uint8_t use_wild_card)
{
    while(fat_read_dir(dd, dir_entry))
    {
        if((strcmp(dir_entry->long_name, name) == 0) || ((use_wild_card == 1) && (wildcmp(name, dir_entry->long_name) == 1)))
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
	//Do not use wildcards here
	if(!find_file_in_dir(fs, dd, name, &file_entry, 0))
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
	uart_puts_p(PSTR("\nOpenLog v1.61\n"));
	uart_puts_p(PSTR("Available commands:\n"));
	uart_puts_p(PSTR("new <file>\t\t: Creates <file>\n"));
	uart_puts_p(PSTR("append <file>\t\t: Appends text to end of <file>. The text is read from the UART in a stream and is not echoed. Finish by sending Ctrl+z (ASCII 26)\n"));
	uart_puts_p(PSTR("write <file> <offset>\t: Writes text to <file>, starting from <offset>. The text is read from the UART, line by line. Finish with an empty line\n"));
	uart_puts_p(PSTR("rm <file>\t\t: Deletes <file>. Use wildcard to do a wildcard removal of files\n"));
	uart_puts_p(PSTR("md <directory>\t: Creates a directory called <directory>\n"));
	uart_puts_p(PSTR("cd <directory>\t\t: Changes current working directory to <directory>\n"));
	uart_puts_p(PSTR("cd ..\t\t: Changes to lower directory in tree\n"));
	uart_puts_p(PSTR("ls\t\t\t: Shows the content of the current directory. Use wildcard to do a wildcard listing of files in current directory\n"));
	uart_puts_p(PSTR("cat <file>\t\t: Writes a hexdump of <file> to the terminal\n"));
	uart_puts_p(PSTR("read <file> <start> <length>\t\t: Writes ASCII <length> parts of <file> to the terminal starting at <start>. Ommit <start> and <length> to read whole file\n"));
	uart_puts_p(PSTR("size <file>\t\t: Write size of file to terminal\n"));
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

	uint8_t uart_speed = EEPROM_read(LOCATION_BAUD_SETTING);
	
	while(1)
	{
		uart_puts_p(PSTR("\nBaud Configuration:\n"));
	
		uart_puts_p(PSTR("Current: "));
		if(uart_speed == BAUD_4800) uart_puts_p(PSTR("48"));
		if(uart_speed == BAUD_2400) uart_puts_p(PSTR("24"));
		if(uart_speed == BAUD_9600) uart_puts_p(PSTR("96"));
		if(uart_speed == BAUD_19200) uart_puts_p(PSTR("192"));
		if(uart_speed == BAUD_57600) uart_puts_p(PSTR("576"));
		if(uart_speed == BAUD_115200) uart_puts_p(PSTR("1152"));
		uart_puts_p(PSTR("00 bps\n"));
	
		uart_puts_p(PSTR("Change to:\n"));
		uart_puts_p(PSTR("1) 2400 bps\n"));
		uart_puts_p(PSTR("2) 4800 bps\n"));
		uart_puts_p(PSTR("3) 9600 bps\n"));
		uart_puts_p(PSTR("4) 19200 bps\n"));
		uart_puts_p(PSTR("5) 57600 bps\n"));
		uart_puts_p(PSTR("6) 115200 bps\n"));
		uart_puts_p(PSTR("x) Exit\n"));

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
			record_config_file(); //Put this new setting into the config file
			blink_error(ERROR_NEW_BAUD);
			return;
		}
		if(strcmp_P(command, PSTR("2")) == 0)
		{
			uart_puts_p(PSTR("\nGoing to 4800bps...\n"));

			//Set baud rate to 4800
			EEPROM_write(LOCATION_BAUD_SETTING, BAUD_4800);
			record_config_file(); //Put this new setting into the config file
			blink_error(ERROR_NEW_BAUD);
			return;
		}
		if(strcmp_P(command, PSTR("3")) == 0)
		{
			uart_puts_p(PSTR("\nGoing to 9600bps...\n"));

			//Set baud rate to 9600
			EEPROM_write(LOCATION_BAUD_SETTING, BAUD_9600);
			record_config_file(); //Put this new setting into the config file
			blink_error(ERROR_NEW_BAUD);
			return;
		}
		if(strcmp_P(command, PSTR("4")) == 0)
		{
			uart_puts_p(PSTR("\nGoing to 19200bps...\n"));

			//Set baud rate to 19200
			EEPROM_write(LOCATION_BAUD_SETTING, BAUD_19200);
			record_config_file(); //Put this new setting into the config file
			blink_error(ERROR_NEW_BAUD);
			return;
		}
		if(strcmp_P(command, PSTR("5")) == 0)
		{
			uart_puts_p(PSTR("\nGoing to 57600bps...\n"));

			//Set baud rate to 57600
			EEPROM_write(LOCATION_BAUD_SETTING, BAUD_57600);
			record_config_file(); //Put this new setting into the config file
			blink_error(ERROR_NEW_BAUD);
			return;
		}
		if(strcmp_P(command, PSTR("6")) == 0)
		{
			uart_puts_p(PSTR("\nGoing to 115200bps...\n"));

			//Set baud rate to 115200
			EEPROM_write(LOCATION_BAUD_SETTING, BAUD_115200);
			record_config_file(); //Put this new setting into the config file
			blink_error(ERROR_NEW_BAUD);
			return;
		}
		if(strcmp_P(command, PSTR("x")) == 0)
		{
			uart_puts_p(PSTR("\nExiting\n"));
			//Do nothing, just exit
			return;
		}
	}
}

//Change how OpenLog works
//1) Turn on unit, unit will create new file, and just start logging
//2) Turn on, append to known file, and just start logging
//3) Turn on, sit at command prompt
//4) Resets the newlog file number to zero
void system_menu(void)
{
	char buffer[5];

	uint8_t system_mode = EEPROM_read(LOCATION_SYSTEM_SETTING);

	while(1)
	{
		uart_puts_p(PSTR("\nSystem Configuration\n"));

		uart_puts_p(PSTR("Current boot mode: "));
		if(system_mode == MODE_NEWLOG) uart_puts_p(PSTR("New file"));
		if(system_mode == MODE_SEQLOG) uart_puts_p(PSTR("Append file"));
		if(system_mode == MODE_COMMAND) uart_puts_p(PSTR("Command"));
		uart_puts_p(PSTR("\n"));

		uart_puts_p(PSTR("Current escape character and amount: "));
		uart_putdw_dec(setting_escape_character);
		uart_puts_p(PSTR(" x "));
		uart_putdw_dec(setting_max_escape_character);
		uart_puts_p(PSTR("\n"));
		
		uart_puts_p(PSTR("Change to:\n"));
		uart_puts_p(PSTR("1) New file logging\n"));
		uart_puts_p(PSTR("2) Append file logging\n"));
		uart_puts_p(PSTR("3) Command prompt\n"));
		uart_puts_p(PSTR("4) Reset new file number\n"));
		uart_puts_p(PSTR("5) New escape character\n"));
		uart_puts_p(PSTR("6) Number of escape characters\n"));
		uart_puts_p(PSTR("x) Exit\n"));

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
			record_config_file(); //Put this new setting into the config file
			return;
		}
		if(strcmp_P(command, PSTR("2")) == 0)
		{
			uart_puts_p(PSTR("Append file logging\n"));
			EEPROM_write(LOCATION_SYSTEM_SETTING, MODE_SEQLOG);
			record_config_file(); //Put this new setting into the config file
			return;
		}
		if(strcmp_P(command, PSTR("3")) == 0)
		{
			uart_puts_p(PSTR("Command prompt\n"));
			EEPROM_write(LOCATION_SYSTEM_SETTING, MODE_COMMAND);
			record_config_file(); //Put this new setting into the config file
			return;
		}
		if(strcmp_P(command, PSTR("4")) == 0)
		{
			uart_puts_p(PSTR("New file number reset to zero\n"));
			EEPROM_write(LOCATION_FILE_NUMBER_LSB, 0);
			EEPROM_write(LOCATION_FILE_NUMBER_MSB, 0);

			//65533 log testing
			//EEPROM_write(LOCATION_FILE_NUMBER_LSB, 0xFD);
			//EEPROM_write(LOCATION_FILE_NUMBER_MSB, 0xFF);
			return;
		}
		if(strcmp_P(command, PSTR("5")) == 0)
		{
			uart_puts_p(PSTR("Enter a new escape character: "));
			setting_escape_character = uart_getc();
			EEPROM_write(LOCATION_ESCAPE_CHAR, setting_escape_character);
			record_config_file(); //Put this new setting into the config file

			uart_puts_p(PSTR("\nNew escape character: "));
			uart_putdw_dec(setting_escape_character);
			uart_puts_p(PSTR("\n"));
			return;
		}
		if(strcmp_P(command, PSTR("6")) == 0)
		{
			char choice = 255;
			while(choice > 9)
			{
				uart_puts_p(PSTR("Enter number of escape characters to look for: "));
				choice = uart_getc() - '0';
			}
			
			setting_max_escape_character = choice;
			EEPROM_write(LOCATION_MAX_ESCAPE_CHAR, setting_max_escape_character);
			record_config_file(); //Put this new setting into the config file

			uart_puts_p(PSTR("\nNumber of escape characters needed: "));
			uart_putdw_dec(setting_max_escape_character);
			uart_puts_p(PSTR("\n"));
			return;
		}
		if(strcmp_P(command, PSTR("x")) == 0)
		{
			//Do nothing, just exit
			uart_puts_p(PSTR("Exiting\n"));
			return;
		}
	}
}


//These functions were added for wild card delete and search
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//Returns the number of command line arguments
uint8_t count_cmd_args(void)
{
	uint8_t count = 0;
	uint8_t i = 0;
	for(; i < MAX_COUNT_COMMAND_LINE_ARGS; i++)
		if((cmd_arg[i].arg != 0) && (cmd_arg[i].arg_length > 0))
			count++;

	return count;
}

//Safe index handling of command line arguments
char* get_cmd_arg(uint8_t index)
{
	memset(general_buffer, 0, sizeof(general_buffer));
	if (index < MAX_COUNT_COMMAND_LINE_ARGS)
		if ((cmd_arg[index].arg != 0) && (cmd_arg[index].arg_length > 0))
			return strncpy(general_buffer, cmd_arg[index].arg, MIN(sizeof(general_buffer), cmd_arg[index].arg_length));

	return 0;
}

//Safe adding of command line arguments
void add_cmd_arg(char* buffer, uint8_t buffer_length)
{
	uint8_t count = count_cmd_args();
	if (count < MAX_COUNT_COMMAND_LINE_ARGS)
	{
		cmd_arg[count].arg = buffer;
		cmd_arg[count].arg_length = buffer_length;
	}
}

//Split the command line arguments
//Example:
//	read <filename> <start> <length>
//	arg[0] -> read
//	arg[1] -> <filename>
//	arg[2] -> <start>
//	arg[3] -> <end>
uint8_t split_cmd_line_args(char* buffer, uint8_t buffer_length)
{
	uint8_t arg_index_start = 0;
	uint8_t arg_index_end = 1;

	//Reset command line arguments
	memset(cmd_arg, 0, sizeof(cmd_arg));

	//Split the command line arguments
	while (arg_index_end < buffer_length)
	{
		//Search for ASCII 32 (Space)
		if ((buffer[arg_index_end] == ' ') || (arg_index_end + 1 == buffer_length))
		{
			//Fix for last character
			if (arg_index_end + 1 == buffer_length)
				arg_index_end = buffer_length;

			//Add this command line argument to the list
			add_cmd_arg(&(buffer[arg_index_start]), (arg_index_end - arg_index_start));
			arg_index_start = ++arg_index_end;
		}

		arg_index_end++;
	}

	//Return the number of available command line arguments
	return count_cmd_args();
}

//Call this function to ensure the number of parameters do not
//exceed limit. The main purpose of this function is to avoid
//entering file names containing spaces.
uint8_t too_many_arguments_error(uint8_t limit, char* command)
{
	uint8_t count;
	if ((count = count_cmd_args()) > limit)
	{
		uart_puts_p(PSTR("too many arguments("));
		uart_putw_dec(count);
		uart_puts_p(PSTR("): "));
		uart_puts(command);
		uart_putc('\n');
		return 1;
	}

	return 0;
}

//Returns char* pointer to buffer if buffer is a valid number or
//0(null) if not.
char* is_number(char* buffer, uint8_t buffer_length)
{
	for (int i = 0; i < buffer_length; i++)
		if (!isdigit(buffer[i]))
			return 0;

	return buffer;
}

//Wildcard string compare.
//Written by Jack Handy - jakkhandy@hotmail.com
//http://www.codeproject.com/KB/string/wildcmp.aspx
uint8_t wildcmp(const char* wild, const char* string)
{

	const char *cp = 0;
	const char *mp = 0;

	while (*string && (*wild != '*'))
	{
		if ((*wild != *string) && (*wild != '?'))
		return 0;

		wild++;
		string++;
	}

	while (*string)
	{
		if (*wild == '*')
		{
			if (!(*(++wild)))
				return 1;

			mp = wild;
			cp = string+1;
		}
		else if ((*wild == *string) || (*wild== '?'))
		{
			wild++;
			string++;
		}
		else
		{
			wild = mp;
			string = cp++;
		}
	}

	while (*wild == '*')
		wild++;
  return !(*wild);
}

//End wildcard functions
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


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
