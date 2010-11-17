/*
 12-3-09
 Nathan Seidle
 SparkFun Electronics© 2010
 spark at sparkfun.com
 
 OpenLog hardware and firmware are released under the Creative Commons Share Alike v3.0 license.
 http://creativecommons.org/licenses/by-sa/3.0/
 Feel free to use, distribute, and sell varients of OpenLog. All we ask is that you include attribution of 'Based on OpenLog by SparkFun'.
 
 OpenLog is based on the work of Bill Greiman and sdfatlib:
 http://code.google.com/p/sdfatlib/
 
 OpenLog is a simple serial logger based on the ATmega328 running at 16MHz. The ATmega328
 should be able to talk to high capacity (larger than 2GB) SD cards. The whole purpose of this
 logger was to create a logger that just powered up and worked. OpenLog ships with standard 
 57600bps serial bootloader running at 16MHz so you can load new firmware with a simple serial
 connection. This makes it compatible with Arduino if needed.
 
 OpenLog automatically works with 512MB, 1GB, 2GB, 4GB, 8GB, and 16GB microSD cards. We recommend FAT16 for 2GB and smaller cards. We 
 recommend FAT32 for 4GB and larger cards.
 	
 OpenLog runs at 9600bps by default. This is configurable to 2400, 4800, 9600, 19200, 57600, and 115200bps. You can alter all settings 
 including baud rate and escape characters by editing config.txt found on OpenLog.
 
 Type '?' to get a list of supported commands.
 	
 During power up, you will see '12<'. '1' indicates the serial connection is established. '2' indicates
 the SD card has been successfully initialized. '<' indicates OpenLog is ready to receive serial characters.
 
 Recording constant 115200bps datastreams are supported. Throw it everything you've got! To acheive this maximum record rate, please use the
 SD card formatter from : http://www.sdcard.org/consumers/formatter/. The fewer files on the card, the faster OpenLog is able to begin logging.
 200 files is ok. 2GB worth of music and pictures is not.
 	
 To a lower dir, use 'cd ..' instead of 'cd..'.
 	
 Only standard 8.3 file names are supported. "12345678.123" is acceptable. "123456789.123" is not.
 
 All file names are pushed to upper case. "NewLog.txt" will become "NEWLOG.TXT".
 
 Type 'set' to enter baud rate configuration menu. Select the baud rate and press enter. You will then 
 see a message 'Going to 9600bps...' or some such message. You will need to power down OpenLog, change 
 your system UART settings to match the new OpenLog baud rate and then power OpenLog back up.
 
 If you get OpenLog stuck into an unknown baudrate, there is a safety mechanism built-in. Tie the RX pin 
 to ground and power up OpenLog. You should see the LEDs blink back and forth for 2 seconds, then blink 
 in unison. Now power down OpenLog and remove the RX/GND jumper. OpenLog is now reset to 9600bps.
 	
 Please note: The preloaded STK500 serial bootloader is 2k, and begins at 0x7800 (30,720). If the code is
 larger than 30,719 bytes, you will get verification errors during serial bootloading.
 
 STAT1 LED is sitting on PD5 (Arduino D5) - toggles when character is received
 STAT2 LED is sitting on PB5 (Arduino D13) - toggles when SPI writes happen
 
 LED Flashing errors @ 2Hz:
 No SD card - 3 blinks
 Baud rate change (requires power cycle) - 4 blinks
 
 OpenLog regularly shuts down to conserve power. If after 1.5 seconds no characters are received, OpenLog will record any unsaved characters
 and go to sleep. OpenLog will automatically wake up and continue logging the instand a new character is received. If power is lost during normal
 operation, OpenLog should lose a maximum of the RX_BUFF / 2. This is currently a max of 350 characters. It's actually quite hard to get into this scenario
 and is rare with normal operation. 
 
 8mA idle
 18mA actively writing
 
 Input voltage on VCC can be 3.3 to 12V. Input voltage on RX-I pin must not exceed 6V. Output voltage on
 TX-O pin will not be greater than 3.3V. This may cause problems with some systems - for example if your
 attached microcontroller requires 4V minimum for serial communication (this is rare).
 
 OpenLog has progressed significantly over the past year. Please see Changes.txt or GitHub for a full change log.
 
 
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
 
 
 v2.3 Migrated to v10.10.10 of sdfatlib. Moved to inline RX interrupt and buffer. Improved the ability to receive a constant stream at 57600bps.
 
 27334 bytes out of 30720.
 
 Remember - we had to completely butcher HardwareSerial.cpp so a normal Arduino installation will not work. 
 C:\arduino-xxxx\hardware\arduino\cores\arduino\HardwareSerial.cpp
 
 I removed the receive interupt handler from HardwareSerial.cpp so that we can deal directly with USART_RX_vect in the main code. This allows
 us to return to the way OpenLog used to operate - by having one buffer split in half. Once one half fills, it is recorded to SD while the other
 half fills up. This dramatically decreases the time spent in function calls and SD recording, which leads to many fewer characters dropped.
 
 The change log at the top of the main code got so large I've moved it to a version controlled "Changes.txt" file.
 
 By making all these changes, I have broken many things. Ringp - I could use your help here. I apologize for stomping on so much of your work. I was not
 good enough to figure out how to re-link from the old function calls to the new sdfatlib setup.
 
 Backspace is broken - ringp, I saw this fix in one of your commits, but looking at the code, I don't see how it is supposed to work. Either way, we still
 get 0x08 when trying to backspace.

 New sdfatlib doesn't have SdFat.cpp so fileInfo doesn't work. These function calls are marked with //Error
 
 I have chosen to dis-allow deprecated functions:
 #define ALLOW_DEPRECATED_FUNCTIONS 0
 This forced some trivial changes to the SD write and new file creation function calls. I believe we've successfully migrated to the new version of sdfatlib.
 
 In the command_shell / read_line function : It may be better to pull directly from the UART buffer and use the RX interrupt. For now, we brute force it.
 
 Because of all these changes, I need to re-test power consumption. For now, I'm confident it's low enough.
 
 Testing with 512 buffer array size
 1GB @ 57600 - dropped very little out of 3 tests
 1GB @ 115200 - dropped very little out of 2 tests
 8GB @ 57600 - Formatted using the sd formater (32k byte allocation size). Dropped nothing.
 8GB @ 115200 - dropped very little, dropped none
 16GB w/ Lots of files @ 57600 - Drops the first part of the file because of start up delay?
 16GB w/ Lots of files @ 115200
 
 1024 array size (and 800) does not run
 
 Testing with 700 buffer array size
 1GB @ 57600 - 110300 out of 111000 bytes, 110300/111000,
 1GB @ 115200 - 111000/111000!, 109600/111000
 8GB @ 57600 - 109000/111000, 111000/111000!,
 8GB @ 115200 - 111000/111000!, 111000/111000!,
 16GB w/ Lots of files @ 57600 - 85120/111000, 85120/111000
 16GB w/ Lots of files @ 115200 - 56420 (but once it got going, log looks good). 56420.
 
 I am seeing long delays on cards with lots of files. In the above tests, the 16GB test card is a good example. It has 2GB worth of random files in a sub directory.
 After OpenLog boots, goes to '12<'. After I send ~500 characters OpenLog freezes for a couple seconds, then returns to normal, very fast, operation. During
 that down time, I believe sdfatlib is searching for an open cluster. The odd thing is that after the cluster is established (after the initial down time) OpenLog
 performs excellently. I am hoping to create a faux file or pre-write and save to the file or some how get this allocation done before we report the 
 '12<' indicating we are ready. That way, if a card does fill up, as long as the host system is checking for '<', it doesn't matter how long it takes 
 sdfatlib to find the next free cluster.
 
 You can see that we drop 700 bytes at a time. That's a bit odd - I'd expect to drop half or 350 at a time. 
 What happens if we shrink the array size to say 256? To be expected, this is bad - way more instances of dropped characters.
 
 Added blink for each test to the OpenLog_Test sketch so we can see as the test progresses.
 
 http://www.sdcard.org/consumers/formatter/ is the site for SD card formatting. It looks like this program takes a guess at the correct block size. This could
 help a lot in the future.


 v2.4 Merged ringp updates. Commands cd, rm, ls work again! New "rm -rf" command to remove directory and its files.

 29028 bytes of 30720 (yikes).
 
 Thanks ringp! Great work.

 Remember - we had to completely butcher HardwareSerial.cpp so a normal Arduino installation will not work. 
 C:\arduino-xxxx\hardware\arduino\cores\arduino\HardwareSerial.cpp
 I've added the modified HardwareSerial.cpp to the 
 
 Testing at 57600
 1GB: 110490/111000, 110490/111000
 8GB: 111000/111000, 111000/111000, 111000/111000
 16GB: 83890/111000, 84908/111000
 The 16GB card with tons of files continue to have problems but the other cards (FAT and FAT32) are acceptable. Whenever possible, use a clean, 
 empty, freshly formatted card.
 
 "rm -rf mydirectory" to remove a directory and all its files
 
 Windows 7 stores the Arduino hex file is an aweful place. Something like:
 C:\Users\Main\AppData\Local\Temp\build3390340147786739225.tmp\OpenLog_v2.cpp.hex
 
 Added HardwareSerial.cpp and a readme to the main trunk.
 Added OpenLog_v2.cpp.hex to the main trunk.
 
 
 v2.41 Power loss bug fixed. Adding support for 38400bps for testing with SparkFum 9DOF IMU logging. 
 
 29124 bytes of 30720 (yikes)
 
 Found a bug in the append_file routine. If the unit is actively logging, file.writes are flying by quickly. But if OpenLog loses power, none of
 the data is recorded because there is no file.sync. Data would only get recorded if the unit went idle or if user entered escape command. This
 has since been fixed with two file.sync() commands.
 
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

//#define RX_BUFF_SIZE  1024 //This larger buffer size does work but may not work in full OpenLog bells and whistles
//#define RX_BUFF_SIZE  800 //Bad
//#define RX_BUFF_SIZE  700 //Works
#define RX_BUFF_SIZE  512
//#define RX_BUFF_SIZE  256 //Drops lots
char rxBuffer[RX_BUFF_SIZE];
int rxSpot;

//The very important receive interrupt handler
SIGNAL(USART_RX_vect)
{
  rxBuffer[rxSpot++] = UDR0;
  if(rxSpot == RX_BUFF_SIZE) rxSpot = 0;
}

char general_buffer[30];

// The folder track depth is used by "cd .." and "pwd"
// This looks like an ugly hack but the sdfatlib is not
// supporting going back to previous folder yet.
// The current handling of "cd .." should be removed when
// it's available in the sdfat library.
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
#define BAUD_38400	6

#define MODE_NEWLOG	0
#define MODE_SEQLOG     1
#define MODE_COMMAND    2

// The number of command line arguments
// Increase to support more arguments but be aware of the
// memory restrictions
//     command <arg1> <arg2> <arg3> <arg4> <arg5>
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

#if !DEBUG
  // Using OpenLog in an embedded environment
  // only if not in debug mode. The reason for this
  // is that we are out of space if the simple embedded is
  // included
  #define INCLUDE_SIMPLE_EMBEDDED
#endif

#define ECHO			0x01
#define EXTENDED_INFO		0x02

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
  if(setting_uart_speed == BAUD_38400) Serial.begin(38400);
  if(setting_uart_speed == BAUD_57600) Serial.begin(57600);
  if(setting_uart_speed == BAUD_115200) Serial.begin(115200);
  Serial.print("1");

#if RAM_TESTING
  PgmPrint("Free RAM start: ");
  Serial.println(FreeRam());
#endif

  //Setup SD & FAT
  if (!card.init(SPI_FULL_SPEED)) {
    PgmPrint("error card.init"); 
    blink_error(ERROR_SD_INIT);
  } // initialize the SD card
  if (!volume.init(&card)) {
    PgmPrint("error volume.init"); 
    blink_error(ERROR_SD_INIT);
  } // initialize a FAT volume
  if (!currentDirectory.openRoot(&volume)) {
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
    if (file.open(&currentDirectory, new_file_name, O_CREAT | O_EXCL | O_WRITE)) break;
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
  if (!file.open(&currentDirectory, seq_file_name, O_CREAT | O_WRITE))
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
  int checkedSpot;
  char escape_chars_received = 0;

  // O_CREAT - create the file if it does not exist
  // O_APPEND - seek to the end of the file prior to each write
  // O_WRITE - open for write
  if (!file.open(&currentDirectory, file_name, O_CREAT | O_APPEND | O_WRITE)) {
    if ((feedback_mode & EXTENDED_INFO) > 0)
      error("open1");
  }

#if DEBUG
  PgmPrintln("File open");
  PgmPrintln("Recording");
#endif
#ifdef INCLUDE_SIMPLE_EMBEDDED
  if ((feedback_mode & EMBEDDED_END_MARKER) > 0)
    Serial.print((char)0x1A); // Ctrl+Z ends the data and marks the start of result
#endif

  Serial.print('<'); //give a different prompt to indicate no echoing
  digitalWrite(statled1, HIGH); //Turn on indicator LED

  //Clear out the serial buffer
  rxSpot = 0;
  checkedSpot = 0;

  //Start UART buffered interrupts
  UCSR0B |= (1<<RXCIE0); //Enable receive interrupts
  sei(); //Enable interrupts

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

    while(checkedSpot == rxSpot){ //Wait for characters to come in
      if(timeout_counter++ > 1200){ //If we haven't seen a character for about 3 seconds, then record the buffer, sync the SD, and shutdown
        timeout_counter = 0;

        if(checkedSpot != 0 && checkedSpot != (RX_BUFF_SIZE/2)) //There is stuff in buffer to record before we go to sleep
        {
          if(checkedSpot < (RX_BUFF_SIZE/2)) {
            file.write(rxBuffer, checkedSpot); //Record first half the buffer
          } 
          else { //checked_spot > (BUFF_LEN/2)
            file.write(rxBuffer + (RX_BUFF_SIZE/2), checkedSpot - (RX_BUFF_SIZE/2)); //Record second half the buffer
          }

          // rxSpot may have moved while we , copy
          unsigned spot = checkedSpot > RX_BUFF_SIZE/2 ? RX_BUFF_SIZE/2 : 0;
          unsigned sp = spot; // start of new buffer

          cli();

          while(checkedSpot != rxSpot) 
          {
            rxBuffer[spot++] = rxBuffer[checkedSpot++]; //Error - We are not checking for escape characters in here
            if( checkedSpot >= RX_BUFF_SIZE )
              checkedSpot = 0;
          }

          rxSpot = spot; // set insertion to end of copy
          checkedSpot = sp; // reset checked to beginning of copy

          sei();

          file.sync(); //Push these new file.writes to the SD card
        }

        //Now power down until new characters to arrive
        while(checkedSpot == rxSpot){
          digitalWrite(STAT1, LOW); //Turn off stat LED to save power
          sleep_mode(); //Stop everything and go to sleep. Wake up if serial character received
        }
      }
      delay(1); //Hang out for a ms
    }

    //Scan for escape character
    if(rxBuffer[checkedSpot] == setting_escape_character){
#if DEBUG
      Serial.print("!");
#endif
      if(++escape_chars_received == setting_max_escape_character){
        //Disable interrupt and we're done!
        cli();
        UCSR0B &= ~(1<<RXCIE0); //Clear receive interrupt enable
        break;
      }
    }
    else
      escape_chars_received = 0;

    checkedSpot++;

    if(checkedSpot == (RX_BUFF_SIZE/2)) { //We've finished checking the first half the buffer
      file.write(rxBuffer, RX_BUFF_SIZE/2); //Record first half the buffer
      file.sync(); //Push these new file.writes to the SD card
    }

    if(checkedSpot == RX_BUFF_SIZE){ //We've finished checking the second half the buffer
      checkedSpot = 0;
      file.write(rxBuffer + (RX_BUFF_SIZE/2), RX_BUFF_SIZE/2); //Record second half the buffer
      file.sync(); //Push these new file.writes to the SD card
    }

    STAT1_PORT ^= (1<<STAT1); //Toggle the STAT1 LED each time we receive a character
  } //End while - escape character received or error

  //Upon receiving the escape character, we may still have stuff left in the buffer, record the last of the buffer to memory
  if(checkedSpot == 0 || checkedSpot == (RX_BUFF_SIZE/2))
  {
    //Do nothing, we already recorded the buffers right before catching the escape character
  }
  else if(checkedSpot < (RX_BUFF_SIZE/2))
  {
    file.write(rxBuffer, checkedSpot); //Record first half the buffer
  }
  else //checkedSpot > (RX_BUFF_SIZE/2)
  {
    file.write(rxBuffer + (RX_BUFF_SIZE/2), checkedSpot - (RX_BUFF_SIZE/2)); //Record second half the buffer
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
    if (!currentDirectory.openRoot(&volume)) error("openRoot");
    int8_t index = getNextFolderTreeIndex() - 1;
    if (index >= 0)
    {
      for (int8_t iTemp = 0; iTemp < index; iTemp++)
      {
        if (!(tmp_var = subDirectory.open(&currentDirectory, folderTree[iTemp], O_READ)))
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
    if (!(tmp_var = subDirectory.open(&currentDirectory, dir, O_READ))) {
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
  //Serial.begin causes the receive interrupt to turn on
  //This is bad for command_shell that relies on non-interrupt driven line reads
  cli();
  UCSR0B &= ~(1<<RXCIE0); //Clear receive interrupt enable

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
      if ((feedback_mode & EXTENDED_INFO) > 0)
        PgmPrintln("Closing down file system");

      //close file system
      currentDirectory.close();

      // open the root directory
      if (!currentDirectory.openRoot(&volume)) error("openRoot");
      memset(folderTree, 0, sizeof(folderTree));
      if ((feedback_mode & EXTENDED_INFO) > 0)
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
      //Argument 2: Directory name
      command_arg = get_cmd_arg(1);
      if(command_arg == 0)
        continue;

      SdFile newDirectory;
      if (!newDirectory.makeDir(&currentDirectory, command_arg)) {
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
    //NOTE on using "rm <option>/<file> <subfolder>"
    // "rm -rf <subfolder>" removes the <subfolder> and all contents recursively
    // "rm <subfolder>" removes the <subfolder> only if its empty
    // "rm <filename>" removes the <filename>
    else if(strncmp_P(command_arg, PSTR("rm"), 2) == 0)
    {
      //Argument 2: Remove option or file name/subdirectory to remove
      command_arg = get_cmd_arg(1);
      if(command_arg == 0)
        continue;

      //Argument 2: Remove subfolder recursively?
      if ((count_cmd_args() == 3) && (strncmp_P(command_arg, PSTR("-rf"), 3) == 0))
      {
        //Remove the subfolder
        if (file.open(&currentDirectory, get_cmd_arg(2), O_READ))
        {
          tmp_var = file.rmRfStar();
          file.close();
#ifdef INCLUDE_SIMPLE_EMBEDDED
          command_succedded = tmp_var;
#endif
        }
        continue;
      }

      //Argument 2: Remove subfolder if empty or remove file
      if (file.open(&currentDirectory, command_arg, O_READ))
      {
        tmp_var = 0;
        if (file.isDir() || file.isSubDir())
          tmp_var = file.rmDir();
        else
        {
          file.close();
          if (file.open(&currentDirectory, command_arg, O_WRITE))
            tmp_var = file.remove();
        }
#ifdef INCLUDE_SIMPLE_EMBEDDED
        command_succedded = tmp_var;
#endif
        file.close();
        continue;
      }

      //Argument 2: File wildcard removal
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
      if (!file.open(&currentDirectory, command_arg, O_READ)) {
        if ((feedback_mode & EXTENDED_INFO) > 0)
        {
          PgmPrint("Failed to open file ");
          Serial.println(command_arg);
        }
        continue;
      }

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
      if (!file.open(&currentDirectory, command_arg, O_WRITE)) {
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
#ifdef INCLUDE_SIMPLE_EMBEDDED
        if ((feedback_mode & EMBEDDED_END_MARKER) > 0)
          Serial.print((char)0x1A); // Ctrl+Z ends the data and marks the start of result
#endif
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
      //Argument 2: File name - no wildcard search
      command_arg = get_cmd_arg(1);
      if(command_arg == 0)
        continue;

      //search file in current directory and open it
      if (file.open(&currentDirectory, command_arg, O_READ)) {
        Serial.print(file.fileSize());
        file.close();
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
      if (!card.readCID(&cid)) {
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
      if (cardSize == 0 || !card.readCSD(&csd)) {
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
      //Argument 2: File name
      command_arg = get_cmd_arg(1);
      if(command_arg == 0)
        continue;

      //Try to open file, if fail (file doesn't exist), then break
      if (file.open(&currentDirectory, command_arg, O_CREAT | O_EXCL | O_WRITE)) {//Will fail if file already exsists
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
    else if(strncmp_P(command_arg, PSTR("pwd"), 3) == 0)
    {
      Serial.print(".\\");
      tmp_var = getNextFolderTreeIndex();
      for (uint8_t i = 0; i < tmp_var; i++)
      {
        Serial.print(folderTree[i]);
        if (i < tmp_var-1) Serial.print("\\");
      }
      Serial.println("");
#ifdef INCLUDE_SIMPLE_EMBEDDED
      command_succedded = 1;
#endif
    }
    // echo <on>|<off>
    else if(strncmp_P(command_arg, PSTR("echo"), 4) == 0)
    {
      //Argument 2: <on>|<off>
      // Set if we are going to echo the characters back to the client or not
      command_arg = get_cmd_arg(1);
      if (command_arg != 0)
      {
        if ((tmp_var = strncmp_P(command_arg, PSTR("on"), 2)) == 0)
          feedback_mode |= ECHO;
        else if ((tmp_var = strncmp_P(command_arg, PSTR("off"), 3)) == 0)
          feedback_mode &= ((uint8_t)~ECHO);

#ifdef INCLUDE_SIMPLE_EMBEDDED
        command_succedded = (tmp_var == 0);
#endif
      }
    }
    // verbose <on>|<off>
    else if(strncmp_P(command_arg, PSTR("verbose"), 7) == 0)
    {
      //Argument 2: <on>|<off>
      // Set if we are going to show extended error information when executing commands
      command_arg = get_cmd_arg(1);
      if (command_arg != 0)
      {
        if ((tmp_var = strncmp_P(command_arg, PSTR("on"), 2)) == 0)
          feedback_mode |= EXTENDED_INFO;
        else if ((tmp_var = strncmp_P(command_arg, PSTR("off"), 3)) == 0)
          feedback_mode &= ((uint8_t)~EXTENDED_INFO);
#ifdef INCLUDE_SIMPLE_EMBEDDED
        command_succedded = (tmp_var == 0);
#endif
      }
    }
#ifdef INCLUDE_SIMPLE_EMBEDDED
    // eem (Embedded End Marker) <on>|<off>
    else if(strncmp_P(command_arg, PSTR("eem"), 3) == 0)
    {
      //Argument 2: <on>|<off>
      //Set if we are going to enable char 26 (Ctrl+z) as end-of-data
      //marker to separate the returned data and the actual
      //result of the operation
      command_arg = get_cmd_arg(1);
      if (command_arg != 0)
      {
        if ((tmp_var = strncmp_P(command_arg, PSTR("on"), 2)) == 0)
          feedback_mode |= EMBEDDED_END_MARKER;
        else if ((tmp_var = strncmp_P(command_arg, PSTR("off"), 3)) == 0)
          feedback_mode &= ((uint8_t)~EMBEDDED_END_MARKER);

        command_succedded = (tmp_var == 0);
      }
    }
    // ecountf
    // returns the number of files in current folder |count|
    else if(strncmp_P(command_arg, PSTR("efcount"), 7) == 0)
    {
      //Argument 2: wild card search
      command_arg = get_cmd_arg(1);
      file_index = 0;

      PgmPrint("count|");
      Serial.println(currentDirectory.fileInfo(FI_COUNT, 0, buffer));
      command_succedded = 1;
    }
    // efname <file index>
    // Returns the name and the size of a file <name>|<size>
    else if(strncmp_P(command_arg, PSTR("efinfo"), 6) == 0)
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
    //while (!Serial.available()); //We've destroyed the standard way Arduino reads characters so this no longer works
    //uint8_t c = Serial.read();
    uint8_t c = uart_getc(); //It would be better to pull directly from the UART buffer and use the RX interrupt, but this works

    STAT1_PORT ^= (1<<STAT1); //Blink the stat LED while typing

    if(c == 0x08 || c == 0x7f) { //Backspace characters
      if(read_length < 1)
        continue;

      --read_length;
      buffer[read_length] = '\0';

      Serial.print((char)0x08);
      Serial.print(' ');
      Serial.print((char)0x08);

      continue;
    }

    // Only echo back if this is enabled
    if ((feedback_mode & ECHO) > 0)
      Serial.print((char)c);

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

uint8_t uart_getc(void)
{
  while(!(UCSR0A & _BV(RXC0)));

  uint8_t b = UDR0;

  return b;
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
  PgmPrintln("OpenLog v2.41");
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
    if(uart_speed == BAUD_38400) PgmPrint("384");
    if(uart_speed == BAUD_57600) PgmPrint("576");
    if(uart_speed == BAUD_115200) PgmPrint("1152");
    PgmPrintln("00 bps");

    PgmPrintln("Change to:");
    PgmPrintln("1) 2400 bps");
    PgmPrintln("2) 4800 bps");
    PgmPrintln("3) 9600 bps");
    PgmPrintln("4) 19200 bps");
    PgmPrintln("5) 38400 bps");
    PgmPrintln("6) 57600 bps");
    PgmPrintln("7) 115200 bps");
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
      PgmPrintln("Going to 38400bps...");

      //Set baud rate to 38400
      EEPROM.write(LOCATION_BAUD_SETTING, BAUD_38400);
      record_config_file(); //Put this new setting into the config file
      blink_error(ERROR_NEW_BAUD);
      return;
    }
    if(strcmp_P(command, PSTR("6")) == 0)
    {
      PgmPrintln("Going to 57600bps...");

      //Set baud rate to 57600
      EEPROM.write(LOCATION_BAUD_SETTING, BAUD_57600);
      record_config_file(); //Put this new setting into the config file
      blink_error(ERROR_NEW_BAUD);
      return;
    }
    if(strcmp_P(command, PSTR("7")) == 0)
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

      //while(!Serial.available()); //Wait for user to hit character
      //setting_escape_character = Serial.read();
      setting_escape_character = uart_getc();

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
        //while(!Serial.available()); //Wait for user to hit character
        //choice = Serial.read() - '0';
        choice = uart_getc() - '0';
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
      if (!rootDirectory.openRoot(&volume)) error("openRoot"); // open the root directory

      char configFileName[13];
      sprintf(configFileName, CFG_FILENAME); //This is the name of the config file. 'config.sys' is probably a bad idea.

      //If there is currently a config file, trash it
      if (file.open(&rootDirectory, configFileName, O_WRITE)) {
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
  if (!rootDirectory.openRoot(&volume)) error("openRoot"); // open the root directory

  char configFileName[13];
  sprintf(configFileName, CFG_FILENAME); //This is the name of the config file. 'config.sys' is probably a bad idea.

  //Check to see if we have a config file
  if (file.open(&rootDirectory, configFileName, O_READ)) {
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
        else if( strcmp(new_setting, "38400") == 0) new_system_baud = BAUD_38400;
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
    if(new_system_baud == BAUD_38400) strcpy(temp_string, "38400");
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
      if(setting_uart_speed == BAUD_38400) Serial.begin(38400);
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
  if (!rootDirectory.openRoot(&volume)) error("openRoot"); // open the root directory

  char configFileName[13];
  sprintf(configFileName, CFG_FILENAME); //This is the name of the config file. 'config.sys' is probably a bad idea.

  //If there is currently a config file, trash it
  if (file.open(&rootDirectory, configFileName, O_WRITE)) {
#if DEBUG
    PgmPrintln("Deleting config");
#endif
    if (!file.remove()){
      PgmPrintln("Remove config failed"); 
      return;
    }
  }

  //Create config file
  if (file.open(&rootDirectory, configFileName, O_CREAT | O_APPEND | O_WRITE) == 0) {
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
    if(current_system_baud == BAUD_38400) strcpy(settings_string, "38400");
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
  if (!volume.init(&card)) error("volume.init"); // initialize a FAT volume
  if (!currentDirectory.openRoot(&volume)) error("openRoot"); // open the root directory

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
