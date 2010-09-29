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
 	
 Please note: The preloaded STK500 serial bootloader is 2k, and begins at 0x7800 (30,720). If the code is
 larger than 30,719 bytes, you will get verification errors during serial bootloading.
 
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
 
 
 v2.0 - 25986 bytes out of 30720
 Welcome to version 2! We've moved from Roland Riegel's FAT library to Bill Greiman's sdfatlib. OpenLog now works with SD cards
 up to 16GB (that is the current largest microSD card we can get our hands on). OpenLog automatically detects and works with FAT16/FAT32 
 file systems. It also automatically works with normal SD cards as well as SDHC cards.
 
 Almost all the previous commands have been ported from v1 to v2. The current commands that do not work:
 cd.. - does not work. You can change to an upper directory, but you cannot navigate back down the tree.
 cat - this command was depricated. HEX printing is now done with the 'read' command. We've added a 5th argument to select between ASCII and HEX printing.
 Wild cards do not yet work. So rm and ls do not have wild cards enabled - yet. Help us out!
 
 Porting OpenLog to work directly under Arduino to work with sdfatlib (http://code.google.com/p/sdfatlib/) by Bill Greiman.
 
 sdfatlib intrinsically supports FAT16, FAT32 as well as SD and HCSD cards. In a word, it's amazing.
 
 Needs to be done:
 Done - Get config file reading/loading working
 Document config file in wiki: if no config file is found, current system settings are used. If config is found, system switches to settings found in file. If system settings are changed, then config file is changed and system uses new settings immediately.
 Done - We don't want to constantly record a new config file on each power on. Only record when there is a change.
 Get cd.. working
 Seperate OpenLog_v2 into multiple files
 Re-test too many log files created in the newlog setting - 65535. Potentially create thousands of files and see how sdfatlib handles it.
 Done - Test sequential logging.
 Get wild card support working for ls and rm
 Get backspace working
 Test rm with directories, and directory wildcards? Maybe not.
 Get power save working
 Test compile on a computer that doesn't have WinAVR
 
 Test commands:
 new - works, also in sub dirs
 append - works, also in sub dirs
 rm - works, but does not yet support wild cards.
 md - works, also in sub dirs
 cd - you can change up to a sub-directory, but you cannot navigate back down the tree. The current work around is to type 'init'. This will re-init the card and set the directory back to root.
 ls - works pretty well but does not yet support wild cards. Shows directories, files, and file sizes. Would be cool if it listed dirs first.
 read - works well. Tested 0, 1, 2, 3, 4, and 5 arguments (included and left off). Fails gracefully. Now prints in HEX as well!
 size - works well
 disk - works well, prints more information than ever before!
 init - works well
 sync - works, but not really tested
 cat - I've decided to drop this command. You can now print in hex using the read command and using a 5th argument as '1' for ASCII (default) and '2' for HEX.
 
 set - works well
 baud - works well
 
 
 v2.1 - Power save not working. Fixed issue 35. Dropping characters at 57600bps. 
 26058 bytes out of 30720
 Fixed a bug found by Salient (Issue 35). User settings where declared at chars which allowed them to be signed. If a user went from old firmware, to v2,
 the safety checks would fail because the settings would be read at -1 instead of 255. Declaring user settings as byte fixed issue.
 
 Added "a) Clear user settings" to set menu. This allows us to completely wipe an OpenLog (user settings and config file) to see how it will respond
 to future firmware changes.
 
 Improved the file 'size' command.
 
 Sequential logging is tested and works.
 
 Receive testing: Using the Test_Sketch found on Github, I am testing the receive reliability at different UART speeds.
 We need to test a lot of received data. At 57600, 115200, and both from an Arduino (lots of time in between characters becuase of software overhead)
 and from a raw serial port (almost no time in between characters). I am hoping to make sdfatlib hiccup at 115200, full speed, across a 1MB file. If 
 I can make it fail, then we can start to increase the buffer size and look at how much RAM sdfatlib has left open for the buffer.
 
 9600bps from Arduino works fine
 57600bps from Arduino drops characters
 115200 from Arduino drops characters
 
 It seems that sdfatlib takes longer to write to the SD card than the original file system from Robert Reigel. I'm thinking perhaps
 we should create a version of OpenLog firmware that is just sequantial logging, no fancy system menus... It might open up some RAM.
 
 If only we could get control of the UART from Arduino's clutches, we could probably handle the ring buffer much better. Not sure how to handle UART
 interrupts without tearing out HardwareSerial.cpp.
 
 Added README to the Test sketch. Added 115200bps to test sketch.
 
 
 v2.11 Tested with 16GB microSD. Fixed some general bugs. Lowered power consumption.
 
 26136 bytes out of 30720
 
 Fixed issue 30. I added printing a period ('.') for non-visible ASCII characters during a 'read' command. This cleans up the output a bit. HEX 
 printing is still available. 
 
 Fixed issue 34. When issuing a long command such as "read log00056.txt 100 200 2" (read from spot 100 to spot 200 and print in HEX), the
 command shell would die at 24 spots. I increased both the general_buffer and 'buffer' in the command shell from 24 to 30. The limit is now
 30 characters, so "read log00056.txt 100 20000 2" is allowed.
 
 Works with a 16GB microSD card! High volume test: loaded 16GB card with 5GB of data. Basic serial tests work. When running at 57600, there
 is an odd delay. I think it has to do with the file system doing an initial scan for an available cluster. Takes 2-3 seconds before OpenLog
 returns to normal. This can cause significant data loss.
 
 Fixing power management in v2. Power down after no characters for 3 seconds now works. Unit drops to 2.35mA in sleep. 7.88mA in sitting 
 RX mode (awake but not receiving anything). There is still a weird bug where the unit comes on at 30mA. After sleep, it comes back at the 
 correct 7-8mA. Is something not getting shut off?
 
 
 v2.2 Modified append_file() to use a single buffer. Increased HardwareSerial.cpp buffer to 512 bytes.
 
 More testing at 57600. Record times look to be 2, 5, and zero milliseconds when doing a record. This means that the split buffer doesn't
 really make a difference? There are some records that take 150ms, 14ms, etc. At 57600bps, that's 7200 bytes/s, 138us per byte. With a 150ms
 pause, that's 1,086 bytes that need to be buffered while we wait... Grrr. Too many.
 
 I re-wrote the append_file function to use only one buffer. This allows us to more quickly pull data from the hardware serial buffer. Hardware 
 serial buffer has to be increased manually to 512. This file (hardwareserial.cpp) is stored in the Arduino directory. With testing,
 it seems like recording is working more solidly at 57600bps. But now I'm seeing glitches at 19200bps so more testing is required before we
 make this the official OpenLog release.
 
 Moved input_buffer into within the append function. Attempting to shave bytes of RAM.
 
 
 v2.21 ringp fork brought in. Wildcard remove and list commands now work. Remove directory now works! Change directory up/down the tree works again.
 
 28440 bytes used of 30720.
 
 ringp brought back many great commands! Thanks ringp!
 rm LOG*.* works
 ls *.TXT works
 cd .. works again
 ls now correctly shows directories first and files following the directories.
 
 To remove a directory, you have to navigate into that directory. For example:
 >cd TEMP (you are now in TEMP directory)
 >rm -f TEMP (you are now removing TEMP, and you will be pushed back down one level of the tree)
 >ls (shows files and directories where TEMP directory used to be, but TEMP directory should be gone)
 
 ringp added new commands:
 efcount: gives the number of files in the current directory. Example: "efcount" returns "count|3". There are 3 files in the current directory.
 efinfo <spot>: gives info about the file in <spot>. Example: "efinfo 2" reports "LOG00588.TXT|45". File number 2 is named LOG00588.TXT and is 45 bytes in size.
 verbose <"on"|"off">: sets whether command errors are verbose (long winded) or just the "!" character. Example: "verbose off" sets verbose off. Then if a 
 command like "blah" is received, then only "!>" is seen from the command line interface. This makes it easier for embedded systems to recognize there 
 was an error. This setting is not recorded to EEPROM.
 */

#include "SdFat.h"
#include "SdFatUtil.h" // use functions to print strings from flash memory
#include <EEPROM.h>

#include <avr/pgmspace.h> //Needed for strcmp_P - string compare in program space
#include <ctype.h> //Needed for isdigit
#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/power.h> //Needed for powering down perihperals such as the ADC/TWI and Timers

//Debug turns on (1) or off (0) a bunch of verbose debug statements. Normally use (0)
//#define DEBUG  1
#define DEBUG  0

//Boot time testing is used to see how long it takes to go from power on to ready to receive characters
//The problem with this is that millis() is not running while the bootloader is timing out...
//Current testing shows that time to boot is around 74ms from a hardware reset and 217ms from a power cycle
//#define BOOTTIME_TESTING 1
#define BOOTTIME_TESTING 0

//The bigger the receive buffer, the less likely we are to drop characters at high speed. However, the ATmega has a limited amount of
//RAM. This debug mode allows us to view available RAM at various stages of the program
//#define RAM_TESTING  1 //On
#define RAM_TESTING  0 //Off

#define sbi(port, port_pin)   ((port) |= (uint8_t)(1 << port_pin))
#define cbi(port, port_pin)   ((port) &= (uint8_t)~(1 << port_pin))

char general_buffer[30];
#define FOLDER_TRACK_DEPTH 15
char folderTree[FOLDER_TRACK_DEPTH][12];
#define CFG_FILENAME	"config.txt"

//EEPROM locations for the various user settings
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
#define MODE_SEQLOG     1
#define MODE_COMMAND    2
#define MAX_COUNT_COMMAND_LINE_ARGS 5

#define FALSE    -1
#define TRUE      0

//STAT1 is a general LED and indicates serial traffic
#define STAT1  5 //On PORTD
#define STAT1_PORT  PORTD
#define STAT2  5 //On PORTB
#define STAT2_PORT  PORTB
int statled1 = 5;  //This is the normal status LED
int statled2 = 13; //This is the SPI LED, indicating SD traffic

//Blinking LED error codes
#define ERROR_SD_INIT	3
#define ERROR_NEW_BAUD	5

// Using OpenLog in an embedded environment
#define INCLUDE_SIMPLE_EMBEDDED
#define ECHO			0x01
#define EXTENDED_INFO		0x02
#define READABLE_TEXT_ONLY	0x04

#ifdef INCLUDE_SIMPLE_EMBEDDED
#define EMBEDDED_END_MARKER	0x08
#endif

Sd2Card card;
SdVolume volume;
SdFile currentDirectory;
SdFile file;

byte setting_uart_speed; //This is the baud rate that the system runs at, default is 9600
byte setting_system_mode; //This is the mode the system runs in, default is MODE_NEWLOG
byte setting_escape_character; //This is the ASCII character we look for to break logging, default is ctrl+z
byte setting_max_escape_character; //Number of escape chars before break logging, default is 3

#if BOOTTIME_TESTING
unsigned long boottime; //Used to keep track of how long it takes to boot
#endif

//Used for wild card delete and search
struct command_arg
{
  char* arg; 			//Points to first character in command line argument
  uint8_t arg_length; // Length of command line argument
};
static struct command_arg cmd_arg[MAX_COUNT_COMMAND_LINE_ARGS];
uint8_t feedback_mode = (ECHO | EXTENDED_INFO);

#define MIN(a,b) ((a)<(b))?(a):(b)

// store error strings in flash to save RAM
#define error(s) error_P(PSTR(s))
void error_P(const char *str)
{
  PgmPrint("error: ");
  SerialPrintln_P(str);
  if (card.errorCode()) {
    PgmPrint("SD error: ");
    Serial.print(card.errorCode(), HEX);
    Serial.print(',');
    Serial.println(card.errorData(), HEX);
  }
  while(1);
}

void setup(void)
{
  pinMode(statled1, OUTPUT);

  //Power down various bits of hardware to lower power usage  
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();

  //Shut off TWI, Timer2, Timer1, ADC
  power_twi_disable();
  power_timer1_disable();
  power_timer2_disable();
  power_adc_disable();

  check_emergency_reset(); //Look to see if the RX pin is being pulled low

  read_system_settings(); //Load all system settings from EEPROM

  //Setup UART
  if(setting_uart_speed == BAUD_2400) Serial.begin(2400);
  if(setting_uart_speed == BAUD_4800) Serial.begin(4800);
  if(setting_uart_speed == BAUD_9600) Serial.begin(9600);
  if(setting_uart_speed == BAUD_19200) Serial.begin(19200);
  if(setting_uart_speed == BAUD_57600) Serial.begin(57600);
  if(setting_uart_speed == BAUD_115200) Serial.begin(115200);
  Serial.print("1");

#if RAM_TESTING
  PgmPrint("Free RAM start: ");
  Serial.println(FreeRam());
#endif

  //Setup SD & FAT
  if (!card.init()) {
    PgmPrint("error card.init"); 
    blink_error(ERROR_SD_INIT);
  } // initialize the SD card
  if (!volume.init(card)) {
    PgmPrint("error volume.init"); 
    blink_error(ERROR_SD_INIT);
  } // initialize a FAT volume
  if (!currentDirectory.openRoot(volume)) {
    PgmPrint("error openRoot"); 
    blink_error(ERROR_SD_INIT);
  } // open the root directory
  Serial.print("2");

#if RAM_TESTING
  PgmPrint("Free RAM post SD init: ");
  Serial.println(FreeRam());
#endif

  //Search for a config file and load any settings found. This will over-ride previous EEPROM settings if found.
  read_config_file();
  memset(folderTree, 0, sizeof(folderTree));
}

void loop(void)
{
  //If we are in new log mode, find a new file name to write to
  if(setting_system_mode == MODE_NEWLOG)
    newlog();

  //If we are in sequential log mode, determine if seqlog.txt has been created or not, and then open it for logging
  if(setting_system_mode == MODE_SEQLOG)
    seqlog();

  //Once either one of these modes exits, go to normal command mode, which is called by returning to main()
  command_shell();

  while(1); //We should never get this far
}

int8_t getNextFolderTreeIndex()
{
  int8_t i;
  for (i = 0; i < FOLDER_TRACK_DEPTH; i++)
    if (strlen(folderTree[i]) == 0)
      return i;

  if (i >= FOLDER_TRACK_DEPTH)
    i = -1;

  return i;
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
  lsb = EEPROM.read(LOCATION_FILE_NUMBER_LSB);
  msb = EEPROM.read(LOCATION_FILE_NUMBER_MSB);

  new_file_number = msb;
  new_file_number = new_file_number << 8;
  new_file_number |= lsb;

  //If both EEPROM spots are 255 (0xFF), that means they are un-initialized (first time OpenLog has been turned on)
  //Let's init them both to 0
  if((lsb == 255) && (msb == 255))
  {
    new_file_number = 0; //By default, unit will start at file number zero
    EEPROM.write(LOCATION_FILE_NUMBER_LSB, 0x00);
    EEPROM.write(LOCATION_FILE_NUMBER_MSB, 0x00);
  }

  //The above code looks like it will forever loop if we ever create 65535 logs
  //Let's quit if we ever get to 65534
  //65534 logs is quite possible if you have a system with lots of power on/off cycles
  if(new_file_number == 65534)
  {
    //Gracefully drop out to command prompt with some error
    PgmPrint("!Too many logs:1!");
    return; //Bail!
  }

  //If we made it this far, everything looks good - let's start testing to see if our file number is the next available

  //Search for next available log spot
  char new_file_name[] = "LOG00000.TXT";
  while(1)
  {
    new_file_number++;
    if(new_file_number > 65533) //There is a max of 65534 logs
    {
      PgmPrint("!Too many logs:2!");
      return;
    }

    sprintf(new_file_name, "LOG%05d.TXT", new_file_number); //Splice the new file number into this file name

    //Try to open file, if fail (file doesn't exist), then break
    if (file.open(currentDirectory, new_file_name, O_CREAT | O_EXCL | O_WRITE)) break;
  }
  file.close(); //Close this new file we just opened
  //file.writeError = false; // clear any write errors

  //Record new_file number to EEPROM
  lsb = (uint8_t)(new_file_number & 0x00FF);
  msb = (uint8_t)((new_file_number & 0xFF00) >> 8);

  EEPROM.write(LOCATION_FILE_NUMBER_LSB, lsb); // LSB

  if (EEPROM.read(LOCATION_FILE_NUMBER_MSB) != msb)
    EEPROM.write(LOCATION_FILE_NUMBER_MSB, msb); // MSB

#if DEBUG
  PgmPrint("\nCreated new file: ");
  Serial.println(new_file_name);
#endif

  //Begin writing to file
  append_file(new_file_name);
}

//Log to the same file every time the system boots, sequentially
//Checks to see if the file SEQLOG.txt is available
//If not, create it
//If yes, append to it
//Return 0 on error
//Return anything else on sucess
void seqlog(void)
{
  char seq_file_name[13] = "SEQLOG00.TXT";

  //Try to create sequential file
  if (!file.open(currentDirectory, seq_file_name, O_CREAT | O_WRITE))
  {
    PgmPrint("Error creating SEQLOG\n");
    return;
  }

  file.close(); //Close this new file we just opened

  append_file(seq_file_name);
}

//Appends a stream of serial data to a given file
//Assumes the currentDirectory variable has been set before entering the routine
//We use the RX interrupt and a circular buffer of 1024 bytes so that we can capture a full stream of 
//data even at 115200bps
//Does not exit until Ctrl+z (ASCII 26) is received
//Returns 0 on error
//Returns 1 on success
uint8_t append_file(char* file_name)
{
  //44051
  //This is the size of the receive buffer. The bigger it is, the less likely we will overflow the buffer while doing a record to SD.
  //But we have limited amounts of RAM (~1100 bytes)
#define BUFF_LEN 50 //50 works well. Too few and we will call file.write A LOT. Too many and we run out of RAM.
  //#define BUFF_LEN 400 //Fails horribly for some reason. Oh right, readSpot only goes to 255.
  //#define BUFF_LEN 100 //Works well.
  char inputBuffer[BUFF_LEN];
  char escape_chars_received = 0;
  byte readSpot = 0; //Limited to 255
  byte incomingByte;

  // O_CREAT - create the file if it does not exist
  // O_APPEND - seek to the end of the file prior to each write
  // O_WRITE - open for write
  if (!file.open(currentDirectory, file_name, O_CREAT | O_APPEND | O_WRITE)) {
    if ((feedback_mode & EXTENDED_INFO) > 0)
      error("open1");
  }

#if DEBUG
  PgmPrintln("File open");
  PgmPrintln("Recording");
#endif

  Serial.print('<'); //give a different prompt to indicate no echoing
  digitalWrite(statled1, HIGH); //Turn on indicator LED

  //Clear out the serial buffer
  Serial.flush();

  //Start recording incoming characters
  //HardwareSerial.cpp has a buffer tied to the interrupt. We increased this buffer to 512 bytes
  //As characters come in, we read them in and record them to FAT.
  while(1){
    uint16_t timeout_counter = 0;

#if BOOTTIME_TESTING
    boottime = millis(); //Get the time from power on to ready to receive
    PgmPrint("Time until ready to recieve (ms): ");
    Serial.println(boottime);
#endif

#if RAM_TESTING
    PgmPrint("Free RAM receive ready: ");
    Serial.println(FreeRam());
#endif

    //Testing to try to get rid of long delay after the first synch
    //This forces OpenLog to scan the fat table and get the pointers ready before the wave of data starts coming in
    //currentDirectory.sync();
    //file.write("123", 3);
    //file.sync();
    //file.close();
    //if (!file.open(currentDirectory, file_name, O_CREAT | O_APPEND | O_WRITE)) error("open1");

    while(!Serial.available()){ //Wait for characters to come in
      if(timeout_counter++ > 1200){ //If we haven't seen a character for about 3 seconds, then record the buffer, sync the SD, and shutdown
        timeout_counter = 0;

        if(readSpot != 0){ //There is unrecorded stuff sitting in the buffer
          //Record the buffer
          if(file.write((byte*)inputBuffer, readSpot) != readSpot)
            if ((feedback_mode & EXTENDED_INFO) > 0)
              PgmPrintln("error writing to file");
        }

        file.sync(); //Push these new file.writes to the SD card
        Serial.flush(); //Clear out the current serial buffer. This is needed if the buffer gets overrun. OpenLog will fail to read the escape character if
        //the buffer gets borked.

        //Reset the points so that we don't record these freshly recorded characters a 2nd time, when the unit receives more characters
        readSpot = 0;

        //Now power down until new characters to arrive
        while(!Serial.available()){
          digitalWrite(statled1, LOW); //Turn off stat LED to save power
          sleep_mode(); //Stop everything and go to sleep. Wake up if serial character received
        }
      }
      delay(1); //Hang out for a ms
    }

    incomingByte = Serial.read(); //Grab new character from hardwareserial.cpp buffer (could be 512 bytes)

    //Scan for escape character
    if(incomingByte == setting_escape_character){
#if DEBUG
      Serial.print("!");
#endif
      if(++escape_chars_received == setting_max_escape_character) break;
    }
    else
      escape_chars_received = 0;

    inputBuffer[readSpot++] = incomingByte; //Record character to the local buffer

    if(readSpot == BUFF_LEN){ //If we've filled the local small buffer, pass it to the sd write function.
      //Record the buffer
      if(file.write((byte*)inputBuffer, BUFF_LEN) != BUFF_LEN){
        if ((feedback_mode & EXTENDED_INFO) > 0)
          PgmPrintln("error writing to file");
        break;
      }

      readSpot = 0; //Wrap the buffer
    }

    STAT1_PORT ^= (1<<STAT1); //Toggle the STAT1 LED each time we receive a character
  } //End while - escape character received or error

  //Upon receiving the escape character, we may still have stuff left in the buffer, record the last of the buffer to memory
  if(readSpot != BUFF_LEN){
    //Record the buffer
    if(file.write((byte*)inputBuffer, readSpot) != readSpot)
      if ((feedback_mode & EXTENDED_INFO) > 0)
        PgmPrintln("error writing to file");
  }

  file.sync();
  file.close(); //Done recording, close out the file
  digitalWrite(statled1, LOW); //Turn off indicator LED

#if DEBUG
  PgmPrintln("Done!");
#endif
  PgmPrint("~"); //Indicate a successful record

  return(1); //Success!
}

uint8_t gotoDir(char *dir)
{
  SdFile subDirectory;
  uint8_t tmp_var = 0;

  //Goto parent directory
  //@NOTE: This is a fix to make this work. Should be replaced with
  //proper handling. Limitation: FOLDER_TRACK_DEPTH subfolders
  if (strncmp_P(dir, PSTR(".."), 2) == 0) {
    tmp_var = 1;

    //close file system
    currentDirectory.close();

    // open the root directory
    if (!currentDirectory.openRoot(volume)) error("openRoot");
    int8_t index = getNextFolderTreeIndex() - 1;
    if (index >= 0)
    {
      for (int8_t iTemp = 0; iTemp < index; iTemp++)
      {
        if (!(tmp_var = subDirectory.open(currentDirectory, folderTree[iTemp], O_READ)))
          break;

        currentDirectory = subDirectory; //Point to new directory
        subDirectory.close();
      }
      memset(folderTree[index], 0, 11);
    }
    if (((feedback_mode & EXTENDED_INFO) > 0) && (tmp_var == 0))
    {
      PgmPrint("cannot cd to parent directory: ");
      Serial.println(dir);
    }
  }
  else
  {
    if (!(tmp_var = subDirectory.open(currentDirectory, dir, O_READ))) {
      if ((feedback_mode & EXTENDED_INFO) > 0)
      {
        PgmPrint("directory not found: ");
        Serial.println(dir);
      }
    }
    else
    {
      currentDirectory = subDirectory; //Point to new directory
      int8_t index = getNextFolderTreeIndex();
      if (index >= 0)
        strncpy(folderTree[index], dir, 11);
    }
  }
  return tmp_var;
}
void command_shell(void)
{
  //provide a simple shell
  char buffer[30];
  uint8_t tmp_var;

#ifdef INCLUDE_SIMPLE_EMBEDDED
  uint32_t file_index;
  uint8_t command_succedded = 1;
#endif //INCLUDE_SIMPLE_EMBEDDED

  while(true)
  {
#ifdef INCLUDE_SIMPLE_EMBEDDED
    if ((feedback_mode & EMBEDDED_END_MARKER) > 0)
      Serial.print((char)0x1A); // Ctrl+Z ends the data and marks the start of result

    if (command_succedded == 0)
      Serial.print('!');
#endif
    Serial.print('>');

    //read command
    char* command = buffer;
    if(read_line(command, sizeof(buffer)) < 1)
    {
#ifdef INCLUDE_SIMPLE_EMBEDDED
      command_succedded = 1;
#endif
      continue;
    }

#ifdef INCLUDE_SIMPLE_EMBEDDED
    command_succedded = 0;
#endif

    //Argument 1: The actual command
    char* command_arg = get_cmd_arg(0);

    //execute command
    if(strcmp_P(command_arg, PSTR("init")) == 0)
    {
      PgmPrintln("Closing down file system");

      //close file system
      currentDirectory.close();

      // open the root directory
      if (!currentDirectory.openRoot(volume)) error("openRoot");

      PgmPrintln("File system initialized");
#ifdef INCLUDE_SIMPLE_EMBEDDED
      command_succedded = 1;
#endif
    }

    else if(strcmp_P(command_arg, PSTR("?")) == 0)
    {
      //Print available commands
      print_menu();

#ifdef INCLUDE_SIMPLE_EMBEDDED
      command_succedded = 1;
#endif

    }
    else if(strcmp_P(command_arg, PSTR("help")) == 0)
    {
      //Print available commands
      print_menu();

#ifdef INCLUDE_SIMPLE_EMBEDDED
      command_succedded = 1;
#endif

    }
    else if(strcmp_P(command_arg, PSTR("baud")) == 0)
    {
      //Go into baud select menu
      baud_menu();

#ifdef INCLUDE_SIMPLE_EMBEDDED
      command_succedded = 1;
#endif

    }
    else if(strcmp_P(command_arg, PSTR("set")) == 0)
    {
      //Go into system setting menu
      system_menu();

#ifdef INCLUDE_SIMPLE_EMBEDDED
      command_succedded = 1;
#endif

    }
#if DEBUG
    else if(strcmp_P(command_arg, PSTR("create")) == 0)
    {
      //Test for creating the maximum number of files that a directory can hold
      //create_lots_of_files();

    }
#endif
    else if(strcmp_P(command_arg, PSTR("ls")) == 0)
    {
      if ((feedback_mode & EXTENDED_INFO) > 0)
      {
        PgmPrint("Volume is FAT");
        Serial.println(volume.fatType(), DEC);
      }

      currentDirectory.ls(LS_SIZE, 0, &wildcmp, get_cmd_arg(1));

#ifdef INCLUDE_SIMPLE_EMBEDDED
      command_succedded = 1;
#endif

    }

    else if(strncmp_P(command_arg, PSTR("md"), 2) == 0)
    {
      //Expecting only 2 arguments
      if (too_many_arguments_error(2, command))
        continue;

      //Argument 2: Directory name
      command_arg = get_cmd_arg(1);
      if(command_arg == 0)
        continue;

      SdFile newDirectory;
      if (!newDirectory.makeDir(currentDirectory, command_arg)) {
        if ((feedback_mode & EXTENDED_INFO) > 0)
        {
          PgmPrintln("error creating directory: ");
          Serial.println(command_arg);
        }
      }
#ifdef INCLUDE_SIMPLE_EMBEDDED
      else
      {
        command_succedded = 1;
      }
#endif

    }

    else if(strncmp_P(command_arg, PSTR("rm"), 2) == 0)
    {
      //Expecting max 3 arguments
      if (too_many_arguments_error(3, command))
        continue;

      //Argument 2: Directory name
      command_arg = get_cmd_arg(1);
      if(command_arg == 0)
        continue;

      //Remove empty folder.
      //@NOTE: Unfortunately -rf does not work due to a buggy rmRfStar() function.
      //Calling rmRfStart() will result in OpenLog rebooting.
      if (strncmp_P(command_arg, PSTR("-f"), 2) == 0)
      {
        if (file.open(currentDirectory, command_arg, O_READ))
          file.close(); //There is a file called "-f"
        else
        {
          //remove and goto parent directory
          if (currentDirectory.rmDir()) gotoDir("..");
#ifdef INCLUDE_SIMPLE_EMBEDDED
          command_succedded = 1;
#endif
          continue;
        }
      }

      //Argument 2: File name or file wildcard removal
      uint32_t filesDeleted = currentDirectory.remove(&wildcmp, get_cmd_arg(1), &removeErrorCallback);
      if ((feedback_mode & EXTENDED_INFO) > 0)
      {
        Serial.print(filesDeleted);
        Serial.println(" file(s) deleted");
      }
#ifdef INCLUDE_SIMPLE_EMBEDDED
      if (filesDeleted > 0)
        command_succedded = 1;
#endif
    }

    else if(strncmp_P(command_arg, PSTR("cd"), 2) == 0)
    {
      //Expecting only 2 arguments
      if (too_many_arguments_error(2, command))
        continue;

      //Argument 2: Directory name
      command_arg = get_cmd_arg(1);
      if(command_arg == 0)
        continue;
      //open directory
      tmp_var = gotoDir(command_arg);

#ifdef INCLUDE_SIMPLE_EMBEDDED
        command_succedded = tmp_var;
#endif
    }

    else if(strncmp_P(command_arg, PSTR("read"), 4) == 0)
    {
      //Argument 2: File name
      command_arg = get_cmd_arg(1);
      if(command_arg == 0)
        continue;

      //search file in current directory and open it
      if (!file.open(currentDirectory, command_arg, O_READ)) {
        if ((feedback_mode & EXTENDED_INFO) > 0)
        {
          PgmPrint("Failed to open file ");
          Serial.println(command_arg);
        }
        continue;
      }
      //Serial.println();

      //Argument 3: File seek position
      if ((command_arg = get_cmd_arg(2)) != 0) {
        if ((command_arg = is_number(command_arg, strlen(command_arg))) != 0) {
          int32_t offset = strtolong(command_arg);
          if(!file.seekSet(offset)) {
            if ((feedback_mode & EXTENDED_INFO) > 0)
            {
              PgmPrint("Error seeking to ");
              Serial.println(command_arg);
            }
            file.close();
            continue;
          }
        }
      }

      //Argument 4: How much data (number of characters) to read from file
      uint32_t readAmount = (uint32_t)-1;
      if ((command_arg = get_cmd_arg(3)) != 0)
        if ((command_arg = is_number(command_arg, strlen(command_arg))) != 0)
          readAmount = strtolong(command_arg);

      //Argument 5: Should we print ASCII or HEX? 1 = ASCII, 2 = HEX
      uint32_t printType = 1; //Default to ASCII
      if ((command_arg = get_cmd_arg(4)) != 0)
        if ((command_arg = is_number(command_arg, strlen(command_arg))) != 0)
          printType = strtolong(command_arg);

      //Print file contents from current seek position to the end (readAmount)
      int8_t c;
      int16_t readSpot = 0;
      while ((c = file.read()) > 0) {
        if(++readSpot > readAmount) break;
        if(printType == 1) { //Printing ASCII
          //Test character to see if it is visible, if not print '.'
          if(c >= ' ' && c < 127)
            Serial.print((char)c); //Regular ASCII
          else if (c == '\n' || c == '\r')
            Serial.print((char)c); //Go ahead and print the carriage returns and new lines
          else
            Serial.print('.'); //For non visible ASCII characters, print a .
        }
        else if (printType == 2) {
          Serial.print((char)c, HEX); //Print in HEX
          Serial.print(" ");
        }

      }
      file.close();
#ifdef INCLUDE_SIMPLE_EMBEDDED
      command_succedded = 1;
      if ((feedback_mode & EMBEDDED_END_MARKER) == 0)
#endif
        Serial.println();
    }

    else if(strncmp_P(command_arg, PSTR("write"), 5) == 0)
    {
      //Argument 2: File name
      command_arg = get_cmd_arg(1);
      if(command_arg == 0)
        continue;

      //search file in current directory and open it
      if (!file.open(currentDirectory, command_arg, O_WRITE)) {
        if ((feedback_mode & EXTENDED_INFO) > 0)
        {
          PgmPrint("Failed to open file ");
          Serial.println(command_arg);
        }
        continue;
      }

      //Argument 3: File seek position
      if ((command_arg = get_cmd_arg(2)) != 0){
        if ((command_arg = is_number(command_arg, strlen(command_arg))) != 0) {
          int32_t offset = strtolong(command_arg);
          if(!file.seekSet(offset)) {
            if ((feedback_mode & EXTENDED_INFO) > 0)
            {
              PgmPrint("Error seeking to ");
              Serial.println(command_arg);
            }
            file.close();
            continue;
          }
        }
      }

      //read text from the shell and write it to the file
      uint8_t dataLen;
      while(1) {    
        Serial.print("<"); //give a different prompt

          //read one line of text
        dataLen = read_line(buffer, sizeof(buffer));
        if(!dataLen) {
#ifdef INCLUDE_SIMPLE_EMBEDDED
          command_succedded = 1;
#endif
          break;
        }

        //write text to file
        if(file.write((uint8_t*) buffer, dataLen) != dataLen) {
          if ((feedback_mode & EXTENDED_INFO) > 0)
            PgmPrint("error writing to file\n\r");
          break;
        }
      }

      file.close();
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

      //search file in current directory and open it
      if (file.open(currentDirectory, command_arg, O_READ)) {
        Serial.print(file.fileSize());
#ifdef INCLUDE_SIMPLE_EMBEDDED
        command_succedded = 1;
#endif
      }
      else
      {
        if ((feedback_mode & EXTENDED_INFO) > 0)
          PgmPrint("-1"); //Indicate no file is found*/
      }
#ifdef INCLUDE_SIMPLE_EMBEDDED
      if ((feedback_mode & EMBEDDED_END_MARKER) == 0)
#endif
        Serial.println();
    }

    else if(strcmp_P(command_arg, PSTR("disk")) == 0)
    {
      //Print card type
      PgmPrint("\nCard type: ");
      switch(card.type()) {
      case SD_CARD_TYPE_SD1:
        PgmPrintln("SD1");
        break;
      case SD_CARD_TYPE_SD2:
        PgmPrintln("SD2");
        break;
      case SD_CARD_TYPE_SDHC:
        PgmPrintln("SDHC");
        break;
      default:
        PgmPrintln("Unknown");
      }

      //Print card information
      cid_t cid;
      if (!card.readCID(cid)) {
        PgmPrint("readCID failed");
        continue;
      }

      PgmPrint("Manufacturer ID: ");
      Serial.println(cid.mid, HEX);

      PgmPrint("OEM ID: ");
      Serial.print(cid.oid[0]);
      Serial.println(cid.oid[1]);

      PgmPrint("Product: ");
      for (uint8_t i = 0; i < 5; i++) {
        Serial.print(cid.pnm[i]);
      }

      PgmPrint("\n\rVersion: ");
      Serial.print(cid.prv_n, DEC);
      Serial.print('.');
      Serial.println(cid.prv_m, DEC);

      PgmPrint("Serial number: ");
      Serial.println(cid.psn);

      PgmPrint("Manufacturing date: ");
      Serial.print(cid.mdt_month);
      Serial.print('/');
      Serial.println(2000 + cid.mdt_year_low + (cid.mdt_year_high <<4));

      csd_t csd;
      uint32_t cardSize = card.cardSize();
      if (cardSize == 0 || !card.readCSD(csd)) {
        PgmPrintln("readCSD failed");
        continue;
      }
      PgmPrint("Card Size: ");
      cardSize /= 2; //Card size is coming up as double what it should be? Don't know why. Dividing it by 2 to correct.
      Serial.print(cardSize);
      //After division
      //7761920 = 8GB card
      //994816 = 1GB card
      PgmPrintln(" KB");
#ifdef INCLUDE_SIMPLE_EMBEDDED
      command_succedded = 1;
#endif
    }
    else if(strcmp_P(command_arg, PSTR("sync")) == 0)
    {
      //Flush all current data and record it to card
      //This isn't really tested.
      file.sync();
      currentDirectory.sync();
#ifdef INCLUDE_SIMPLE_EMBEDDED
      command_succedded = 1;
#endif
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

      //Try to open file, if fail (file doesn't exist), then break
      if (file.open(currentDirectory, command_arg, O_CREAT | O_EXCL | O_WRITE)) {//Will fail if file already exsists
        file.close(); //Everything is good, Close this new file we just opened
#ifdef INCLUDE_SIMPLE_EMBEDDED
        command_succedded = 1;
#endif
      }
      else
      {
        if ((feedback_mode & EXTENDED_INFO) > 0) {
          PgmPrint("Error creating file: ");
          Serial.println(command_arg);
        }
      }
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

      //append_file: Uses circular buffer to capture full stream of text and append to file
#ifdef INCLUDE_SIMPLE_EMBEDDED
      command_succedded = append_file(command_arg);
#else
      append_file(command_arg);
#endif
    }
    // verbose <on>|<off>
    else if(strcmp_P(command_arg, PSTR("verbose")) == 0)
    {
      //Argument 2: <on>|<off>
      // Set if we are going to show extended error information when executing commands
      command_arg = get_cmd_arg(1);
      if (command_arg != 0)
      {
        if ((tmp_var = strcmp_P(command_arg, PSTR("on"))) == 0)
          feedback_mode |= EXTENDED_INFO;
        else if ((tmp_var = strcmp_P(command_arg, PSTR("off"))) == 0)
          feedback_mode &= ((uint8_t)~EXTENDED_INFO);
#ifdef INCLUDE_SIMPLE_EMBEDDED
        command_succedded = (tmp_var == 0);
#endif
      }
    }
#ifdef INCLUDE_SIMPLE_EMBEDDED
    // eem (Embedded End Marker) <on>|<off>
    else if(strcmp_P(command_arg, PSTR("eem")) == 0)
    {
      //Argument 2: <on>|<off>
      //Set if we are going to enable char 26 (Ctrl+z) as end-of-data
      //marker to separate the returned data and the actual
      //result of the operation
      command_arg = get_cmd_arg(1);
      if (command_arg != 0)
      {
        if ((tmp_var = strcmp_P(command_arg, PSTR("on"))) == 0)
          feedback_mode |= EMBEDDED_END_MARKER;
        else if ((tmp_var = strcmp_P(command_arg, PSTR("off"))) == 0)
          feedback_mode &= ((uint8_t)~EMBEDDED_END_MARKER);

        command_succedded = (tmp_var == 0);
      }
    }
    // efcount
    // returns the number of files in current folder |count|
    else if(strcmp_P(command_arg, PSTR("efcount")) == 0)
    {
      //Argument 2: wild card search
      command_arg = get_cmd_arg(1);
      file_index = 0;

      PgmPrint("count|");
      Serial.println(currentDirectory.fileInfo(FI_COUNT, 0, buffer));
      command_succedded = 1;
    }
    // efinfo <file index>
    // Returns the name and the size of a file <name>|<size>
    else if(strcmp_P(command_arg, PSTR("efinfo")) == 0)
    {
      //Argument 2: File index
      command_arg = get_cmd_arg(1);
      if (command_arg != 0)
      {
        // File index should always be a number
        if ((command_arg = is_number(command_arg, strlen(command_arg))) != 0)
        {
          file_index = strtolong(command_arg);
          memset(buffer, 0, sizeof(buffer));
          uint32_t size = currentDirectory.fileInfo(FI_INFO, file_index, buffer);
          Serial.print(buffer);
          Serial.print('|');
          Serial.println(size);
          command_succedded = 1;
        }
      }
    }
#endif //INCLUDE_SIMPLE_EMBEDDED

    else
    {
      if ((feedback_mode & EXTENDED_INFO) > 0) {
        PgmPrint("unknown command: ");
        Serial.println(command_arg);
      }
    }
  }

  //Do we ever get this far?
  PgmPrint("Exiting: closing down\n");
}

//Reads a line until the \n enter character is found
uint8_t read_line(char* buffer, uint8_t buffer_length)
{
  memset(buffer, 0, buffer_length);

  uint8_t read_length = 0;
  while(read_length < buffer_length - 1) {
    while (!Serial.available());
    uint8_t c = Serial.read();

    //PORTD ^= (1<<STAT1); //Blink the stat LED while typing - this is taken care of in the UART ISR
    //PORTB ^= (1<<STAT2); //Blink the stat LED while typing - I don't want the SPI lines toggling

    if(c == 0x08 || c == 0x7f) {
      if(read_length < 1)
        continue;

      --read_length;
      buffer[read_length] = '\0';

      Serial.print(0x08);
      Serial.print(' ');
      Serial.print(0x08);

      continue;
    }

    Serial.print(c);

    //if(c == '\n')
    if(c == '\r') {
      Serial.println();
      buffer[read_length] = '\0';
      break;
    }
    else {
      buffer[read_length] = c;
      ++read_length;
    }
  }

  //Split the command line into arguments
  split_cmd_line_args(buffer, buffer_length);

  return read_length;
}

//Reads the current system settings from EEPROM
//If anything looks weird, reset setting to default value
void read_system_settings(void)
{
  //Read what the current UART speed is from EEPROM memory
  //Default is 9600
  setting_uart_speed = EEPROM.read(LOCATION_BAUD_SETTING);
  if(setting_uart_speed > 10) 
  {
    setting_uart_speed = BAUD_9600; //Reset UART to 9600 if there is no speed stored
    EEPROM.write(LOCATION_BAUD_SETTING, setting_uart_speed);
  }

  //Determine the system mode we should be in
  //Default is NEWLOG mode
  setting_system_mode = EEPROM.read(LOCATION_SYSTEM_SETTING);
  if(setting_system_mode > 5) 
  {
    setting_system_mode = MODE_NEWLOG; //By default, unit will turn on and go to new file logging
    EEPROM.write(LOCATION_SYSTEM_SETTING, setting_system_mode);
  }

  //Read the escape_character
  //ASCII(26) is ctrl+z
  setting_escape_character = EEPROM.read(LOCATION_ESCAPE_CHAR);
  if(setting_escape_character == 0 || setting_escape_character == 255) 
  {
    setting_escape_character = 26; //Reset escape character to ctrl+z
    EEPROM.write(LOCATION_ESCAPE_CHAR, setting_escape_character);
  }

  //Read the number of escape_characters to look for
  //Default is 3
  setting_max_escape_character = EEPROM.read(LOCATION_MAX_ESCAPE_CHAR);
  if(setting_max_escape_character == 0 || setting_max_escape_character == 255) 
  {
    setting_max_escape_character = 3; //Reset number of escape characters to 3
    EEPROM.write(LOCATION_MAX_ESCAPE_CHAR, setting_max_escape_character);
  }
}

void print_menu(void)
{
  PgmPrintln("OpenLog v2.21");
  PgmPrintln("Available commands:");
  PgmPrintln("new <file>\t\t: Creates <file>");
  PgmPrintln("append <file>\t\t: Appends text to end of <file>. The text is read from the UART in a stream and is not echoed. Finish by sending Ctrl+z (ASCII 26)");
  PgmPrintln("write <file> <offset>\t: Writes text to <file>, starting from <offset>. The text is read from the UART, line by line. Finish with an empty line");
  PgmPrintln("rm <file>\t\t: Deletes <file>. Use wildcard to do a wildcard removal of files");
  PgmPrintln("md <directory>\t: Creates a directory called <directory>");
  PgmPrintln("cd <directory>\t\t: Changes current working directory to <directory>");
  PgmPrintln("cd ..\t\t: Changes to lower directory in tree");
  PgmPrintln("ls\t\t\t: Shows the content of the current directory. Use wildcard to do a wildcard listing of files in current directory");
  PgmPrintln("read <file> <start> <length> <type>\t\t: Writes ASCII <length> parts of <file> to the terminal starting at <start>. Omit <start> and <length> to read whole file. <type> 1 prints in ASCII, 2 in HEX.");
  PgmPrintln("size <file>\t\t: Write size of file to terminal");
  PgmPrintln("disk\t\t\t: Shows card manufacturer, status, filesystem capacity and free storage space");
  PgmPrintln("init\t\t\t: Reinitializes and reopens the memory card");
  PgmPrintln("sync\t\t\t: Ensures all buffered data is written to the card");

  PgmPrintln("\n\rMenus:");
  PgmPrintln("set\t\t\t: Menu to configure system mode");
  PgmPrintln("baud\t\t\t: Menu to configure baud rate");
}

//Configure what baud rate to communicate at
void baud_menu(void)
{
  char buffer[5];

  uint8_t uart_speed = EEPROM.read(LOCATION_BAUD_SETTING);

  while(1)
  {
    PgmPrintln("\n\rBaud Configuration:");

    PgmPrint("Current: ");
    if(uart_speed == BAUD_4800) PgmPrint("48");
    if(uart_speed == BAUD_2400) PgmPrint("24");
    if(uart_speed == BAUD_9600) PgmPrint("96");
    if(uart_speed == BAUD_19200) PgmPrint("192");
    if(uart_speed == BAUD_57600) PgmPrint("576");
    if(uart_speed == BAUD_115200) PgmPrint("1152");
    PgmPrintln("00 bps");

    PgmPrintln("Change to:");
    PgmPrintln("1) 2400 bps");
    PgmPrintln("2) 4800 bps");
    PgmPrintln("3) 9600 bps");
    PgmPrintln("4) 19200 bps");
    PgmPrintln("5) 57600 bps");
    PgmPrintln("6) 115200 bps");
    PgmPrintln("x) Exit");

    //print prompt
    Serial.print('>');

    //read command
    char* command = buffer;

    if(read_line(command, sizeof(buffer)) < 1)
      continue;

    //execute command
    if(strcmp_P(command, PSTR("1")) == 0)
    {
      PgmPrintln("Going to 2400bps...");

      //Set baud rate to 2400
      EEPROM.write(LOCATION_BAUD_SETTING, BAUD_2400);
      record_config_file(); //Put this new setting into the config file
      blink_error(ERROR_NEW_BAUD);
      return;
    }
    if(strcmp_P(command, PSTR("2")) == 0)
    {
      PgmPrintln("Going to 4800bps...");

      //Set baud rate to 4800
      EEPROM.write(LOCATION_BAUD_SETTING, BAUD_4800);
      record_config_file(); //Put this new setting into the config file
      blink_error(ERROR_NEW_BAUD);
      return;
    }
    if(strcmp_P(command, PSTR("3")) == 0)
    {
      PgmPrintln("Going to 9600bps...");

      //Set baud rate to 9600
      EEPROM.write(LOCATION_BAUD_SETTING, BAUD_9600);
      record_config_file(); //Put this new setting into the config file
      blink_error(ERROR_NEW_BAUD);
      return;
    }
    if(strcmp_P(command, PSTR("4")) == 0)
    {
      PgmPrintln("Going to 19200bps...");

      //Set baud rate to 19200
      EEPROM.write(LOCATION_BAUD_SETTING, BAUD_19200);
      record_config_file(); //Put this new setting into the config file
      blink_error(ERROR_NEW_BAUD);
      return;
    }
    if(strcmp_P(command, PSTR("5")) == 0)
    {
      PgmPrintln("Going to 57600bps...");

      //Set baud rate to 57600
      EEPROM.write(LOCATION_BAUD_SETTING, BAUD_57600);
      record_config_file(); //Put this new setting into the config file
      blink_error(ERROR_NEW_BAUD);
      return;
    }
    if(strcmp_P(command, PSTR("6")) == 0)
    {
      PgmPrintln("Going to 115200bps...");

      //Set baud rate to 115200
      EEPROM.write(LOCATION_BAUD_SETTING, BAUD_115200);
      record_config_file(); //Put this new setting into the config file
      blink_error(ERROR_NEW_BAUD);
      return;
    }
    if(strcmp_P(command, PSTR("x")) == 0)
    {
      PgmPrintln("Exiting");
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
  uint8_t system_mode = EEPROM.read(LOCATION_SYSTEM_SETTING);

  while(1)
  {
    PgmPrintln("\r\nSystem Configuration");

    PgmPrint("Current boot mode: ");
    if(system_mode == MODE_NEWLOG) PgmPrint("New file");
    if(system_mode == MODE_SEQLOG) PgmPrint("Append file");
    if(system_mode == MODE_COMMAND) PgmPrint("Command");
    Serial.println();//("\n");

    PgmPrint("Current escape character and amount: ");
    Serial.print(setting_escape_character, DEC);
    PgmPrint(" x ");
    Serial.println(setting_max_escape_character, DEC);

    PgmPrintln("Change to:");
    PgmPrintln("1) New file logging");
    PgmPrintln("2) Append file logging");
    PgmPrintln("3) Command prompt");
    PgmPrintln("4) Reset new file number");
    PgmPrintln("5) New escape character");
    PgmPrintln("6) Number of escape characters");

#if DEBUG
    PgmPrintln("a) Clear all user settings");
#endif

    PgmPrintln("x) Exit");
    //print prompt
    Serial.print('>');

    //read command
    char buffer[5];
    char* command = buffer;

    if(read_line(command, sizeof(buffer)) < 1)
      continue;

    //execute command
    if(strcmp_P(command, PSTR("1")) == 0)
    {
      PgmPrintln("New file logging");
      EEPROM.write(LOCATION_SYSTEM_SETTING, MODE_NEWLOG);
      record_config_file(); //Put this new setting into the config file
      return;
    }
    if(strcmp_P(command, PSTR("2")) == 0)
    {
      PgmPrintln("Append file logging");
      EEPROM.write(LOCATION_SYSTEM_SETTING, MODE_SEQLOG);
      record_config_file(); //Put this new setting into the config file
      return;
    }
    if(strcmp_P(command, PSTR("3")) == 0)
    {
      PgmPrintln("Command prompt");
      EEPROM.write(LOCATION_SYSTEM_SETTING, MODE_COMMAND);
      record_config_file(); //Put this new setting into the config file
      return;
    }
    if(strcmp_P(command, PSTR("4")) == 0)
    {
      PgmPrintln("New file number reset to zero");
      EEPROM.write(LOCATION_FILE_NUMBER_LSB, 0);
      EEPROM.write(LOCATION_FILE_NUMBER_MSB, 0);

      //65533 log testing
      //EEPROM_write(LOCATION_FILE_NUMBER_LSB, 0xFD);
      //EEPROM_write(LOCATION_FILE_NUMBER_MSB, 0xFF);
      return;
    }
    if(strcmp_P(command, PSTR("5")) == 0)
    {
      PgmPrint("Enter a new escape character: ");

      while(!Serial.available()); //Wait for user to hit character
      setting_escape_character = Serial.read();

      EEPROM.write(LOCATION_ESCAPE_CHAR, setting_escape_character);
      record_config_file(); //Put this new setting into the config file

        PgmPrint("\n\rNew escape character: ");
      Serial.println(setting_escape_character, DEC);
      return;
    }
    if(strcmp_P(command, PSTR("6")) == 0)
    {
      uint8_t choice = 10;
      while(choice > 9 || choice < 1)
      {
        PgmPrint("\n\rEnter number of escape characters to look for (1 to 9): ");
        while(!Serial.available()); //Wait for user to hit character
        choice = Serial.read() - '0';
      }

      setting_max_escape_character = choice;
      EEPROM.write(LOCATION_MAX_ESCAPE_CHAR, setting_max_escape_character);
      record_config_file(); //Put this new setting into the config file

        PgmPrint("\n\rNumber of escape characters needed: ");
      Serial.println(setting_max_escape_character, DEC);
      return;
    }

#if DEBUG
    //This allows us to reset the EEPROM and config file on a unit to see what would happen to an 
    //older unit that is upgraded/reflashed to newest firmware
    if(strcmp_P(command, PSTR("a")) == 0)
    {
      EEPROM.write(LOCATION_BAUD_SETTING, 0xFF);
      EEPROM.write(LOCATION_SYSTEM_SETTING, 0xFF);
      EEPROM.write(LOCATION_FILE_NUMBER_LSB, 0xFF);
      EEPROM.write(LOCATION_FILE_NUMBER_MSB, 0xFF);
      EEPROM.write(LOCATION_ESCAPE_CHAR, 0xFF);
      EEPROM.write(LOCATION_MAX_ESCAPE_CHAR, 0xFF);

      //Remove the config file if it is there
      SdFile rootDirectory;
      if (!rootDirectory.openRoot(volume)) error("openRoot"); // open the root directory

      char configFileName[13];
      sprintf(configFileName, CFG_FILENAME); //This is the name of the config file. 'config.sys' is probably a bad idea.

      //If there is currently a config file, trash it
      if (file.open(rootDirectory, configFileName, O_WRITE)) {
        PgmPrintln("Deleting config");
        if (!file.remove()){
          PgmPrintln("Remove config failed"); 
          return;
        }
      }

      PgmPrintln("Unit has been reset. Please power cycle");
      while(1);
    }
#endif

    if(strcmp_P(command, PSTR("x")) == 0)
    {
      //Do nothing, just exit
      PgmPrintln("Exiting");
      return;
    }
  }
}

void read_config_file(void)
{
  SdFile rootDirectory;
  if (!rootDirectory.openRoot(volume)) error("openRoot"); // open the root directory

  char configFileName[13];
  sprintf(configFileName, CFG_FILENAME); //This is the name of the config file. 'config.sys' is probably a bad idea.

  //Check to see if we have a config file
  if (file.open(rootDirectory, configFileName, O_READ)) {
    //If we found the config file then load settings from file and push them into EEPROM
#if DEBUG
    PgmPrintln("Found config file!");
#endif

    //Read up to 20 characters from the file. There may be a better way of doing this...
    char c;
    int len;
    uint8_t settings_string[20]; //"115200,103,14,0\0" = 115200 bps, escape char of ASCII(103), 14 times, new log mode. 20 characters is longer than the max (16).
    for(len = 0 ; len < 20 ; len++) {
      if( (c = file.read()) < 0) break; //We've reached the end of the file
      settings_string[len] = c;
    }
    file.close();
    rootDirectory.close();

#if DEBUG
    //Print line for debugging
    Serial.print("Text Settings: ");
    for(int i = 0; i < len; i++)
      Serial.print(settings_string[i]);
    Serial.println();
    Serial.print("Len: ");
    Serial.println(len);
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

#if DEBUG
    //This will print the found settings. Use for debugging
    PgmPrint("Settings determined to be: ");

    char temp_string[16]; //"115200,103,14,0\0" = 115200 bps, escape char of ASCII(103), 14 times, new log mode
    char temp[3];

    if(new_system_baud == BAUD_2400) strcpy(temp_string, "2400");
    if(new_system_baud == BAUD_4800) strcpy(temp_string, "4800");
    if(new_system_baud == BAUD_9600) strcpy(temp_string, "9600");
    if(new_system_baud == BAUD_19200) strcpy(temp_string, "19200");
    if(new_system_baud == BAUD_57600) strcpy(temp_string, "57600");
    if(new_system_baud == BAUD_115200) strcpy(temp_string, "115200");

    strcat(temp_string, ",");

    //Convert escape character to an ASCII visible string
    sprintf(temp, "%d", new_system_escape);
    strcat(temp_string, temp); //Add this string to the system string

    strcat(temp_string, ",");

    //Convert max escape character to an ASCII visible string
    sprintf(temp, "%d", new_system_max_escape);
    strcat(temp_string, temp); //Add this string to the system string

    strcat(temp_string, ",");

    //Convert system mode to a ASCII visible character
    sprintf(temp, "%d", new_system_mode);
    strcat(temp_string, temp); //Add this string to the system string

    strcat(temp_string, "\0");

    Serial.println(temp_string);
#endif

    //We now have the settings loaded into the global variables. Now check if they're different from EEPROM settings
    boolean recordNewSettings = FALSE;

    if(new_system_baud != setting_uart_speed) {
      //If the baud rate from the file is different from the current setting,
      //Then update the setting to the file setting
      //And re-init the UART
      EEPROM.write(LOCATION_BAUD_SETTING, new_system_baud);
      setting_uart_speed = new_system_baud;

      //Move system to new uart speed
      if(setting_uart_speed == BAUD_2400) Serial.begin(2400);
      if(setting_uart_speed == BAUD_4800) Serial.begin(4800);
      if(setting_uart_speed == BAUD_9600) Serial.begin(9600);
      if(setting_uart_speed == BAUD_19200) Serial.begin(19200);
      if(setting_uart_speed == BAUD_57600) Serial.begin(57600);
      if(setting_uart_speed == BAUD_115200) Serial.begin(115200);

      recordNewSettings = TRUE;
    }

    if(new_system_mode != setting_system_mode) {
      //Goto new system mode
      setting_system_mode = new_system_mode;
      EEPROM.write(LOCATION_SYSTEM_SETTING, setting_system_mode);

      recordNewSettings = TRUE;
    }

    if(new_system_escape != setting_escape_character) {
      //Goto new system escape char
      setting_escape_character = new_system_escape;
      EEPROM.write(LOCATION_ESCAPE_CHAR, setting_escape_character); 

      recordNewSettings = TRUE;
    }

    if(new_system_max_escape != setting_max_escape_character) {
      //Goto new max escape
      setting_max_escape_character = new_system_max_escape;
      EEPROM.write(LOCATION_MAX_ESCAPE_CHAR, setting_max_escape_character);

      recordNewSettings = TRUE;
    }

    //We don't want to constantly record a new config file on each power on. Only record when there is a change.
    if(recordNewSettings == TRUE)
      record_config_file(); //If we corrected some values because the config file was corrupt, then overwrite any corruption
#if DEBUG
    else
      PgmPrintln("Config file matches system settings");
#endif

    //All done! New settings are loaded. System will now operate off new config settings found in file.
  }
  else {
    //If we don't have a config file already, then create config file and record the current system settings to the file
#if DEBUG
    PgmPrintln("No config found - creating default:");
#endif

    //Record the current eeprom settings to the config file
    record_config_file();
  }

}

//Records the current EEPROM settings to the config file
//If a config file exists, it is trashed and a new one is created
void record_config_file(void)
{
  //I'm worried that the user may not be in the root directory when modifying config settings. If that is the case,
  //config file will not be found and it will be created in some erroneus directory. The changes to user settings may be lost on the
  //next power cycles. To prevent this, we will open another instance of the file system, then close it down when we are done.
  SdFile rootDirectory;
  if (!rootDirectory.openRoot(volume)) error("openRoot"); // open the root directory

  char configFileName[13];
  sprintf(configFileName, CFG_FILENAME); //This is the name of the config file. 'config.sys' is probably a bad idea.

  //If there is currently a config file, trash it
  if (file.open(rootDirectory, configFileName, O_WRITE)) {
#if DEBUG
    PgmPrintln("Deleting config");
#endif
    if (!file.remove()){
      PgmPrintln("Remove config failed"); 
      return;
    }
  }

  //Create config file
  if (file.open(rootDirectory, configFileName, O_CREAT | O_APPEND | O_WRITE) == 0) {
    PgmPrintln("Create config failed");
    return;
  }
  else {
    //Config was successfully created, now record current system settings to the config file

      //Baud, escape character, escape times, system mode (0 = new log)
    char settings_string[16]; //"115200,103,14,0\0" = 115200 bps, escape char of ASCII(103), 14 times, new log mode
    char temp[3];

    //Before we read the EEPROM values, they've already been tested and defaulted in the read_system_settings function
    char current_system_baud = EEPROM.read(LOCATION_BAUD_SETTING);
    char current_system_mode = EEPROM.read(LOCATION_SYSTEM_SETTING);
    char current_system_escape = EEPROM.read(LOCATION_ESCAPE_CHAR);
    char current_system_max_escape = EEPROM.read(LOCATION_MAX_ESCAPE_CHAR);

    //Determine current baud and copy it to string
    if(current_system_baud == BAUD_2400) strcpy(settings_string, "2400");
    if(current_system_baud == BAUD_4800) strcpy(settings_string, "4800");
    if(current_system_baud == BAUD_9600) strcpy(settings_string, "9600");
    if(current_system_baud == BAUD_19200) strcpy(settings_string, "19200");
    if(current_system_baud == BAUD_57600) strcpy(settings_string, "57600");
    if(current_system_baud == BAUD_115200) strcpy(settings_string, "115200");

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
    strcpy( (char*)general_buffer, settings_string);

#if DEBUG
    PgmPrint("\nSetting string: ");
    Serial.println(settings_string);
#endif

    if(file.write((uint8_t*)general_buffer, strlen(settings_string)) != strlen(settings_string))
      PgmPrintln("error writing to file");

    file.sync(); //Sync all newly written data to card
    file.close(); //Close this file
    rootDirectory.close(); //Close this file structure instance
    //Now the new config file has the current system settings, nothing else to do!
  }
}

//A rudimentary way to convert a string to a long 32 bit integer
//Used to the read command, in command shell
uint32_t strtolong(const char* str)
{
  uint32_t l = 0;
  while(*str >= '0' && *str <= '9')
    l = l * 10 + (*str++ - '0');

  return l;
}
//Blinks the status LEDs to indicate a type of error
void blink_error(uint8_t ERROR_TYPE)
{
  while(1)
  {
    for(int x = 0 ; x < ERROR_TYPE ; x++)
    {
      digitalWrite(statled1, HIGH);
      delay(200);
      digitalWrite(statled1, LOW);
      delay(200);
    }

    delay(2000);
  }
}

//Check to see if we need an emergency UART reset
//Scan the RX pin for 2 seconds
//If it's low the entire time, then return 1
void check_emergency_reset(void)
{
  pinMode(0, INPUT); //Turn the RX pin into an input
  digitalWrite(0, HIGH); //Push a 1 onto RX pin to enable internal pull-up

  //Quick pin check
  if(digitalRead(0) == HIGH) return;

  //Wait 2 seconds, blinking LEDs while we wait
  pinMode(statled2, OUTPUT);
  digitalWrite(statled2, HIGH); //Set the STAT2 LED
  for(uint8_t i = 0 ; i < 40 ; i++)
  {
    delay(25);
    STAT1_PORT ^= (1<<STAT1); //Blink the stat LEDs

    if(digitalRead(0) == HIGH) return; //Check to see if RX is not low anymore

    delay(25);
    STAT2_PORT ^= (1<<STAT2); //Blink the stat LEDs

    if(digitalRead(0) == HIGH) return; //Check to see if RX is not low anymore
  }		

  //If we make it here, then RX pin stayed low the whole time
  set_default_settings(); //Reset baud, escape characters, escape number, system mode

  //Try to setup the SD card so we can record these new settings
  if (!card.init()) error("card.init"); // initialize the SD card
  if (!volume.init(card)) error("volume.init"); // initialize a FAT volume
  if (!currentDirectory.openRoot(volume)) error("openRoot"); // open the root directory

  record_config_file(); //Record new config settings

  pinMode(statled1, OUTPUT);
  pinMode(statled2, OUTPUT);
  digitalWrite(statled1, HIGH);
  digitalWrite(statled2, HIGH);

  //Now sit in forever loop indicating system is now at 9600bps
  while(1)
  {
    delay(500);
    STAT1_PORT ^= (1<<STAT1); //Blink the stat LEDs
    STAT2_PORT ^= (1<<STAT2); //Blink the stat LEDs
  }
}

//Resets all the system settings to safe values
void set_default_settings(void)
{
  //Reset UART to 9600bps
  EEPROM.write(LOCATION_BAUD_SETTING, BAUD_9600);

  //Reset system to new log mode
  EEPROM.write(LOCATION_SYSTEM_SETTING, MODE_NEWLOG);

  //Reset escape character to ctrl+z
  EEPROM.write(LOCATION_ESCAPE_CHAR, 26); 

  //Reset number of escape characters to 3
  EEPROM.write(LOCATION_MAX_ESCAPE_CHAR, 3);

  //These settings are not recorded to the config file
  //We can't do it here because we are not sure the FAT system is init'd
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
    PgmPrint("too many arguments(");
    Serial.print(count); //uart_putw_dec(count);
    PgmPrint("): ");
    Serial.println(command);
    //uart_putc('\n');
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
void removeErrorCallback(const char* fileName)
{
  Serial.print((char *)PSTR("Remove failed: "));
  Serial.println(fileName);
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
