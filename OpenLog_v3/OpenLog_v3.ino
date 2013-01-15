/*
 12-3-09
 Nathan Seidle
 SparkFun Electronics 2012
 
 OpenLog hardware and firmware are released under the Creative Commons Share Alike v3.0 license.
 http://creativecommons.org/licenses/by-sa/3.0/
 Feel free to use, distribute, and sell varients of OpenLog. All we ask is that you include attribution of 'Based on OpenLog by SparkFun'.
 
 OpenLog is based on the work of Bill Greiman and sdfatlib: http://code.google.com/p/sdfatlib/
 
 OpenLog is a simple serial logger based on the ATmega328 running at 16MHz. The ATmega328
 is able to talk to high capacity (larger than 2GB) SD cards. The whole purpose of this
 logger was to create a logger that just powered up and worked. OpenLog ships with an Arduino/Optiboot 
 115200bps serial bootloader running at 16MHz so you can load new firmware with a simple serial
 connection.
 
 OpenLog automatically works with 512MB, 1GB, 2GB, 4GB, 8GB, and 16GB microSD cards. We recommend FAT16 for 2GB and smaller cards. We
 recommend FAT32 for 4GB and larger cards.
 	
 OpenLog runs at 9600bps by default. This is configurable to 2400, 4800, 9600, 19200, 38400, 57600, and 115200bps. You can alter 
 all settings including baud rate and escape characters by editing config.txt found on OpenLog.
 
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
 	
 Please note: The preloaded Optiboot serial bootloader is 0.5k, and begins at 0x7E00 (32,256). If the code is
 larger than 32,256 bytes, you will get verification errors during serial bootloading.
 
 STAT1 LED is sitting on PD5 (Arduino D5) - toggles when character is received
 STAT2 LED is sitting on PB5 (Arduino D13) - toggles when SPI writes happen
 
 LED Flashing errors @ 2Hz:
 No SD card - 3 blinks
 Baud rate change (requires power cycle) - 4 blinks
 
 OpenLog regularly shuts down to conserve power. If after 0.5 seconds no characters are received, OpenLog will record any unsaved characters
 and go to sleep. OpenLog will automatically wake up and continue logging the instant a new character is received.
 
 1.55mA idle
 15mA actively writing
 
 Input voltage on VCC can be 3.3 to 12V. Input voltage on RX-I pin must not exceed 6V. Output voltage on
 TX-O pin will not be greater than 3.3V. This may cause problems with some systems - for example if your
 attached microcontroller requires 4V minimum for serial communication (this is rare).
 
 OpenLog has progressed significantly over the past three years. Please see Changes.txt or GitHub for a full change log.
 
 
 v3.0 Re-write of core functions. Support under Arduino v1.0. Now supports full speed 57600 and 115200bps! Lower standby power.
 
 28354 bytes out of 32256.
 
 Be sure to check out https://github.com/nseidle/OpenLog/issues for issues that have been resolved up to this version.
 
 Update to new beta version of SerialPort lib from Bill Greiman. Update to Arduino v1.0. With Bill's library, we don't need to hack the HardwareSerial.cpp.
 Re-wrote the append function. This function is the most important function and has the most affect on record accuracy. We are 
 now able to record at 57600 and 115200bps at full blast! The performance is vastly improved.
 
 To compile v3.0 you will need Arduino v1.0 and Bill's beta library: http://beta-lib.googlecode.com/files/SerialLoggerBeta20120108.zip
 Unzip 'SerialPortBeta20120106.zip' and 'SdFatBeta20120108.zip' to the libraries directory and close and restart Arduino.
 
 Small stuff:
 Redirected all static strings to point to the new way in Arduino 1.0: NewSerial.print(F("asdf")); instead of PgmPrint
 Figured out lower standby power from the low-power tutorial: http://www.sparkfun.com/tutorials/309
 Corrected #define TRUE to built-in supported 'true'
 Re-arranged some functions
 Migrating to Uno bootloader to get an additional 1500 bytes of program space
 Replumbed everything to get away from hardware UART
 Reduced # of sub directory support from 15 levels to 2 to allow for more RAM
 
 Wildcard remove is not yet supported in v3.0
 Wildcard ls is not yet supported in v3.0
 efcount and efinfo is not yet supported in v3.0
 
 Testing at 115200. First test with clean/empty card. The second test is with 193MB across 172 files on the microSD card.
 1GB: 333075/333075, 333075/333075
 8GB: 333075/333075, 333075/333075
 16GB: 333075/333075, 333075/333075
 The card with tons of files may have problems. Whenever possible, use a clean, empty, freshly formatted card.
 
 Testing at 57600. First test with clean/empty card. The second test is with 193MB across 172 files on the microSD card.
 1GB: 111075/111075, 111075/111075
 8GB: 111075/111075, 111075/111075
 16GB: 111075/111075, 111075/111075
 The card with tons of files may have problems. Whenever possible, use a clean, empty, freshly formatted card.
 
 
 v3.1 Fixed bug where entire log data is lost when power is lost.
 
 Added fix from issue 76: https://github.com/nseidle/OpenLog/issues/76
 Added support for verbose and echo settings recorded to the config file and EEPROM.
 Bugs 101 and 102 fixed with the help of pwjansen and wilafau - thank you!!
 
 Because of the new code to the cycle sensitive append_log loop, 115200bps is not as rock solid as I'd like.
 I plan to correct this in the Light version.
 
 Added a maxLoop variable calculation to figure out how many bytes to receive before we do a forced file.sync.
 
 
 v3.11 Added freeMemory support for RAM testing.
 
 This was taken from: http://arduino.cc/playground/Code/AvailableMemory
 
 
 v3.12 Using freeMemory() function from JeeLabs. Seems to be working well.
 
 Echo on/off is working again with a reduction of the buffer from 600 to 500.
 Code size is up a bit to 29,282. We have free RAM of about 527 after "2" and 444 once we've begun append_file.
 
 
 v3.13 Changed the string compares from strings in RAM (strcmp) to strings located in flash memory (strcmp_P).
 
 We used to use this type of compare: else if(strcmp_P(command_arg, PSTR("md")) == 0)
 I'm not sure how or why I got away from it, but bringing it back frees up a lot of RAM. I bet I got rid of PSTRs
 in an attempt to reduce code size when I should have been more worried about RAM running out.
 RAM is currently at 645 after 2 and 562 once we've begun append_file. 
 Also figured out there is sprintf_P built just for PSTRs. Neat!
 RAM is currently at 779 after 2 and 696 once we've begun append_file.
 Removal of strncmp(). I believe we used it to reduce flash footprint. Once migrated to PSTR, it makes it worse.
 
 */

#include <SdFat.h> //We do not use the built-in SD.h file because it calls Serial.print
#include <SerialPort.h> //This is a new/beta library written by Bill Greiman. You rock Bill!
#include <EEPROM.h>

SerialPort<0, 780, 0> NewSerial;
//This is a very important buffer declaration. This sets the <port #, rx size, tx size>. We set
//the TX buffer to zero because we will be spending most of our time needing to buffer the incoming (RX) characters.
//900 works on minimal implementation, doesn't work with the full command prompt
//850 fails on echo on/off
//800 works with full command shell
//700 works very well at 115200
//600 works well at 115200

#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/power.h> //Needed for powering down perihperals such as the ADC/TWI and Timers
//#include <avr/wdt.h> //Needed for the reset command - uses watch dog timer to reset IC

//Debug turns on (1) or off (0) a bunch of verbose debug statements. Normally use (0)
//#define DEBUG  1
#define DEBUG  0

//The bigger the receive buffer, the less likely we are to drop characters at high speed. However, the ATmega has a limited amount of
//RAM. This debug mode allows us to view available RAM at various stages of the program
//#define RAM_TESTING  1 //On
#define RAM_TESTING  0 //Off

//#define Reset_AVR() wdt_enable(WDTO_1S); while(1) {} //Correct way of resetting the ATmega, but doesn't work with 
//Arduino pre-Optiboot bootloader
void(* Reset_AVR) (void) = 0; //Dirty way of resetting the ATmega, but it works for now

// The folder track depth is used by "cd .." and "pwd"
// This looks like an ugly hack but the sdfatlib is not supporting going back to previous folder yet.
// The current handling of "cd .." should be removed when it's available in the sdfat library.
//#define FOLDER_TRACK_DEPTH 15
#define FOLDER_TRACK_DEPTH 2 //Decreased for more RAM access
char folderTree[FOLDER_TRACK_DEPTH][12];

#define CFG_FILENAME "config.txt" //This is the name of the file that contains the unit settings
#define CFG_LENGTH  20 //Length of text found in config file: "115200,103,14,0,1,1\0" = 115200 bps, escape char of ASCII(103), 14 times, new log mode, verbose on, echo on. 
#define SEQ_FILENAME "SEQLOG00.TXT" //This is the name for the file when you're in sequential mode

//Internal EEPROM locations for the user settings
#define LOCATION_BAUD_SETTING		0x01
#define LOCATION_SYSTEM_SETTING		0x02
#define LOCATION_FILE_NUMBER_LSB	0x03
#define LOCATION_FILE_NUMBER_MSB	0x04
#define LOCATION_ESCAPE_CHAR		0x05
#define LOCATION_MAX_ESCAPE_CHAR	0x06
#define LOCATION_VERBOSE                0x07
#define LOCATION_ECHO                   0x08

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

//STAT1 is a general LED and indicates serial traffic
#define STAT1  5 //On PORTD
#define STAT1_PORT  PORTD
#define STAT2  5 //On PORTB
#define STAT2_PORT  PORTB
const byte statled1 = 5;  //This is the normal status LED
const byte statled2 = 13; //This is the SPI LED, indicating SD traffic

//Blinking LED error codes
#define ERROR_SD_INIT	  3
#define ERROR_NEW_BAUD	  5
#define ERROR_CARD_INIT   6
#define ERROR_VOLUME_INIT 7
#define ERROR_ROOT_INIT   8
#define ERROR_FILE_OPEN   9

#if !DEBUG
// Using OpenLog in an embedded environment only if not in debug mode. The reason for this
// is that we are out of space if the simple embedded is included
#define INCLUDE_SIMPLE_EMBEDDED
#endif

//These are bit locations used when testing for simple embedded commands.
#define ECHO			0x01
#define EXTENDED_INFO		0x02
#define OFF  0x00
#define ON   0x01

#ifdef INCLUDE_SIMPLE_EMBEDDED
#define EMBEDDED_END_MARKER	0x08
#endif

Sd2Card card;
SdVolume volume;
SdFile currentDirectory;

byte setting_uart_speed; //This is the baud rate that the system runs at, default is 9600
byte setting_system_mode; //This is the mode the system runs in, default is MODE_NEWLOG
byte setting_escape_character; //This is the ASCII character we look for to break logging, default is ctrl+z
byte setting_max_escape_character; //Number of escape chars before break logging, default is 3
byte setting_verbose; //This controls the whether we get extended or simple responses.
byte setting_echo; //This turns on/off echoing at the command prompt

//The number of command line arguments
//Increase to support more arguments but be aware of the memory restrictions
//command <arg1> <arg2> <arg3> <arg4> <arg5>
#define MAX_COUNT_COMMAND_LINE_ARGS 5

//Used for wild card delete and search
struct command_arg
{
  char* arg; //Points to first character in command line argument
  byte arg_length; //Length of command line argument
};
static struct command_arg cmd_arg[MAX_COUNT_COMMAND_LINE_ARGS];
byte feedback_mode = (ECHO | EXTENDED_INFO);

//Passes back the available amount of free RAM
int freeRam () 
{
#if RAM_TESTING
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
#endif
}
void printRam() 
{
#if RAM_TESTING
  NewSerial.print(F(" RAM:"));
  NewSerial.println(freeRam());
#endif
}

//Handle errors by printing the error type and blinking LEDs in certain way
//The function will never exit - it loops forever inside blink_error
void systemError(byte error_type)
{
  NewSerial.print(F("Error "));
  switch(error_type)
  {
  case ERROR_CARD_INIT:
    NewSerial.print(F("card.init")); 
    blink_error(ERROR_SD_INIT);
    break;
  case ERROR_VOLUME_INIT:
    NewSerial.print(F("volume.init")); 
    blink_error(ERROR_SD_INIT);
    break;
  case ERROR_ROOT_INIT:
    NewSerial.print(F("root.init")); 
    blink_error(ERROR_SD_INIT);
    break;
  case ERROR_FILE_OPEN:
    NewSerial.print(F("file.open")); 
    blink_error(ERROR_SD_INIT);
    break;
  }
}

void setup(void)
{
  pinMode(statled1, OUTPUT);

  //Power down various bits of hardware to lower power usage  
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();

  //Shut off TWI, Timer2, Timer1, ADC
  ADCSRA &= ~(1<<ADEN); //Disable ADC
  ACSR = (1<<ACD); //Disable the analog comparator
  DIDR0 = 0x3F; //Disable digital input buffers on all ADC0-ADC5 pins
  DIDR1 = (1<<AIN1D)|(1<<AIN0D); //Disable digital input buffer on AIN1/0

  power_twi_disable();
  power_timer1_disable();
  power_timer2_disable();
  power_adc_disable();

  check_emergency_reset(); //Look to see if the RX pin is being pulled low

  read_system_settings(); //Load all system settings from EEPROM

  //Setup UART
  if(setting_uart_speed == BAUD_2400) NewSerial.begin(2400);
  if(setting_uart_speed == BAUD_4800) NewSerial.begin(4800);
  if(setting_uart_speed == BAUD_9600) NewSerial.begin(9600);
  if(setting_uart_speed == BAUD_19200) NewSerial.begin(19200);
  if(setting_uart_speed == BAUD_38400) NewSerial.begin(38400);
  if(setting_uart_speed == BAUD_57600) NewSerial.begin(57600);
  if(setting_uart_speed == BAUD_115200) NewSerial.begin(115200);
  NewSerial.print(F("1"));

  //Setup SD & FAT
  if (!card.init(SPI_FULL_SPEED)) systemError(ERROR_CARD_INIT);
  if (!volume.init(&card)) systemError(ERROR_VOLUME_INIT);
  currentDirectory.close(); //We close the cD before opening root. This comes from QuickStart example. Saves 4 bytes.
  if (!currentDirectory.openRoot(&volume)) systemError(ERROR_ROOT_INIT);

  NewSerial.print(F("2"));

  printRam(); //Print the available RAM

  //Search for a config file and load any settings found. This will over-ride previous EEPROM settings if found.
  read_config_file();

  memset(folderTree, 0, sizeof(folderTree)); //Clear folder tree
}

void loop(void)
{
  //If we are in new log mode, find a new file name to write to
  if(setting_system_mode == MODE_NEWLOG)
    append_file(newlog()); //Append the file name that newlog() returns

  //If we are in sequential log mode, determine if seqlog.txt has been created or not, and then open it for logging
  if(setting_system_mode == MODE_SEQLOG)
    seqlog();

  //Once either one of these modes exits, go to normal command mode, which is called by returning to main()
  command_shell();

  while(1); //We should never get this far
}

//Log to a new file everytime the system boots
//Checks the spots in EEPROM for the next available LOG# file name
//Updates EEPROM and then appends to the new log file.
//Limited to 65535 files but this should not always be the case.
char* newlog(void)
{
  byte msb, lsb;
  uint16_t new_file_number;

  SdFile newFile; //This will contain the file for SD writing

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
    NewSerial.print(F("!Too many logs:1!"));
    return(0); //Bail!
  }

  //If we made it this far, everything looks good - let's start testing to see if our file number is the next available

  //Search for next available log spot
  //char new_file_name[] = "LOG00000.TXT";
  char new_file_name[13];
  while(1)
  {
    new_file_number++;
    if(new_file_number > 65533) //There is a max of 65534 logs
    {
      NewSerial.print(F("!Too many logs:2!"));
      return(0); //Bail!
    }

    sprintf_P(new_file_name, PSTR("LOG%05d.TXT"), new_file_number); //Splice the new file number into this file name

    //Try to open file, if fail (file doesn't exist), then break
    if (newFile.open(&currentDirectory, new_file_name, O_CREAT | O_EXCL | O_WRITE)) break;
  }
  newFile.close(); //Close this new file we just opened

  //Record new_file number to EEPROM
  lsb = (byte)(new_file_number & 0x00FF);
  msb = (byte)((new_file_number & 0xFF00) >> 8);

  EEPROM.write(LOCATION_FILE_NUMBER_LSB, lsb); // LSB

  if (EEPROM.read(LOCATION_FILE_NUMBER_MSB) != msb)
    EEPROM.write(LOCATION_FILE_NUMBER_MSB, msb); // MSB

#if DEBUG
  NewSerial.print(F("\nCreated new file: "));
  NewSerial.println(new_file_name);
#endif

  //  append_file(new_file_name);
  return(new_file_name);
}

//Log to the same file every time the system boots, sequentially
//Checks to see if the file SEQLOG.txt is available
//If not, create it
//If yes, append to it
//Return 0 on error
//Return anything else on sucess
void seqlog(void)
{
  SdFile seqFile;

  char sequentialFileName[strlen(SEQ_FILENAME)]; //Limited to 8.3
  strcpy_P(sequentialFileName, PSTR(SEQ_FILENAME)); //This is the name of the config file. 'config.sys' is probably a bad idea.

  //Try to create sequential file
  if (!seqFile.open(&currentDirectory, sequentialFileName, O_CREAT | O_WRITE))
  {
    NewSerial.print(F("Error creating SEQLOG\n"));
    return;
  }

  seqFile.close(); //Close this new file we just opened

  append_file(sequentialFileName); 
}

//This is the most important function of the device. These loops have been tweaked as much as possible.
//Modifying this loop may negatively affect how well the device can record at high baud rates.
//Appends a stream of serial data to a given file
//Assumes the currentDirectory variable has been set before entering the routine
//Does not exit until Ctrl+z (ASCII 26) is received
//Returns 0 on error
//Returns 1 on success
byte append_file(char* file_name)
{
  SdFile workingFile;

  // O_CREAT - create the file if it does not exist
  // O_APPEND - seek to the end of the file prior to each write
  // O_WRITE - open for write
  if (!workingFile.open(&currentDirectory, file_name, O_CREAT | O_APPEND | O_WRITE)) systemError(ERROR_FILE_OPEN);
  if (workingFile.fileSize() == 0) {
    //This is a trick to make sure first cluster is allocated - found in Bill's example/beta code
    //workingFile.write((byte)0); //Leaves a NUL at the beginning of a file
    workingFile.rewind();
    workingFile.sync();
  }  

  NewSerial.print(F("<")); //give a different prompt to indicate no echoing
  digitalWrite(statled1, HIGH); //Turn on indicator LED

#define LOCAL_BUFF_SIZE  32
  byte localBuffer[LOCAL_BUFF_SIZE];
  byte checkedSpot;

  const uint16_t MAX_IDLE_TIME_MSEC = 500; //The number of milliseconds before unit goes to sleep
  const uint16_t MAX_TIME_BEFORE_SYNC_MSEC = 5000;
  uint32_t lastSyncTime = millis(); //Keeps track of the last time the file was synced
  byte escape_chars_received = 0;

#if DEBUG
  NewSerial.print(F("maxLoops: "));
  NewSerial.println(maxLoops);
#endif

  printRam(); //Print the available RAM

  //Start recording incoming characters
  while(escape_chars_received < setting_max_escape_character) {

    byte n = NewSerial.read(localBuffer, sizeof(localBuffer)); //Read characters from global buffer into the local buffer
    if (n > 0) //If we have characters, check for escape characters
    {

      if (localBuffer[0] == setting_escape_character) 
      {
        escape_chars_received++;

        //Scan the local buffer for esacape characters
        for(checkedSpot = 1 ; checkedSpot < n ; checkedSpot++) {
          if(localBuffer[checkedSpot] == setting_escape_character) {
            escape_chars_received++;
            //If n is greater than 3 there's a chance here where we receive three esc chars
            // and then reset the variable: 26 26 26 A T + would not escape correctly
            if(escape_chars_received == setting_max_escape_character) break;
          }
          else
            escape_chars_received = 0; 
        }
      }

      workingFile.write(localBuffer, n); //Record the buffer to the card

      STAT1_PORT ^= (1<<STAT1); //Toggle the STAT1 LED each time we record the buffer

      if((millis() - lastSyncTime) > MAX_TIME_BEFORE_SYNC_MSEC) //This will force a sync approximately every 5 seconds
      { 
        //This is here to make sure a log is recorded in the instance
        //where the user is throwing non-stop data at the unit from power on to forever
        workingFile.sync(); //Sync the card
        lastSyncTime = millis();
      }

    }
    //No characters recevied?
    else if( (millis() - lastSyncTime) > MAX_IDLE_TIME_MSEC) { //If we haven't received any characters in 2s, goto sleep
      workingFile.sync(); //Sync the card before we go to sleep

      STAT1_PORT &= ~(1<<STAT1); //Turn off stat LED to save power

      power_timer0_disable(); //Shut down peripherals we don't need
      power_spi_disable();
      sleep_mode(); //Stop everything and go to sleep. Wake up if serial character received

      power_spi_enable(); //After wake up, power up peripherals
      power_timer0_enable();

      escape_chars_received = 0; // Clear the esc flag as it has timed out
      lastSyncTime = millis(); //Reset the last sync time to now
    }

  }

  workingFile.sync();
  workingFile.close(); // Done recording, close out the file

    digitalWrite(statled1, LOW); // Turn off indicator LED

  NewSerial.print(F("~")); // Indicate a successful record

  return(1); //Success!
}

//The following are system functions needed for basic operation
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//Blinks the status LEDs to indicate a type of error
void blink_error(byte ERROR_TYPE) {
  while(1) {
    for(int x = 0 ; x < ERROR_TYPE ; x++) {
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
  for(byte i = 0 ; i < 40 ; i++)
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
  if (!card.init()) systemError(ERROR_CARD_INIT);
  if (!volume.init(&card)) systemError(ERROR_VOLUME_INIT);
  currentDirectory.close(); //We close the cD before opening root. This comes from QuickStart example. Saves 4 bytes.
  if (!currentDirectory.openRoot(&volume)) systemError(ERROR_ROOT_INIT);

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

  //Reset verbose responses to on
  EEPROM.write(LOCATION_VERBOSE, ON);

  //Reset echo to on
  EEPROM.write(LOCATION_ECHO, ON);

  //These settings are not recorded to the config file
  //We can't do it here because we are not sure the FAT system is init'd
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

  //Read whether we should use verbose responses or not
  //Default is true
  setting_verbose = EEPROM.read(LOCATION_VERBOSE);
  if(setting_verbose != ON || setting_verbose != OFF) 
  {
    setting_verbose = ON; //Reset verbose to true
    EEPROM.write(LOCATION_VERBOSE, setting_verbose);
  }

  //Read whether we should echo characters or not
  //Default is true
  setting_echo = EEPROM.read(LOCATION_ECHO);
  if(setting_echo != ON || setting_echo != OFF) 
  {
    setting_echo = ON; //Reset to echo on
    EEPROM.write(LOCATION_ECHO, setting_echo);
  }

  //Set flags for extended mode options  
  if (setting_verbose == ON)
    feedback_mode |= EXTENDED_INFO;
  else
    feedback_mode &= ((byte)~EXTENDED_INFO);

  if (setting_echo == ON)
    feedback_mode |= ECHO;
  else
    feedback_mode &= ((byte)~ECHO);
}

void read_config_file(void)
{
  SdFile rootDirectory;
  SdFile configFile;  
  if (!rootDirectory.openRoot(&volume)) systemError(ERROR_ROOT_INIT); // open the root directory

  char configFileName[strlen(CFG_FILENAME)]; //Limited to 8.3
  strcpy_P(configFileName, PSTR(CFG_FILENAME)); //This is the name of the config file. 'config.sys' is probably a bad idea.

  //Check to see if we have a config file
  if (!configFile.open(&rootDirectory, configFileName, O_READ)) {
    //If we don't have a config file already, then create config file and record the current system settings to the file
#if DEBUG
    NewSerial.println(F("No config found - creating default:"));
#endif
    configFile.close();
    rootDirectory.close(); //Close out the file system before we open a new one

      //Record the current eeprom settings to the config file
    record_config_file();
    return;
  }

  //If we found the config file then load settings from file and push them into EEPROM
#if DEBUG
  NewSerial.println(F("Found config file!"));
#endif

  //Read up to 20 characters from the file. There may be a better way of doing this...
  char c;
  int len;
  byte settings_string[CFG_LENGTH]; //"115200,103,14,0,1,1\0" = 115200 bps, escape char of ASCII(103), 14 times, new log mode, verbose on, echo on.
  for(len = 0 ; len < CFG_LENGTH ; len++) {
    if( (c = configFile.read()) < 0) break; //We've reached the end of the file
    if(c == '\0') break; //Bail if we hit the end of this string
    settings_string[len] = c;
  }
  configFile.close();
  rootDirectory.close();

#if DEBUG
  //Print line for debugging
  NewSerial.print(F("Text Settings: "));
  for(int i = 0; i < len; i++)
    NewSerial.write(settings_string[i]);
  NewSerial.println();
  NewSerial.print(F("Len: "));
  NewSerial.println(len);
#endif

  //Default the system settings in case things go horribly wrong
  char new_system_baud = BAUD_9600;
  char new_system_mode = MODE_NEWLOG;
  char new_system_escape = 26;
  char new_system_max_escape = 3;
  char new_system_verbose = ON;
  char new_system_echo = ON;

  //Parse the settings out
  byte i = 0, j = 0, setting_number = 0;
  char new_setting[7]; //Max length of a setting is 6, the bps setting = '115200' plus '\0'
  byte new_setting_int = 0;

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
    new_setting_int = atoi(new_setting); //Convert string to int

    if(setting_number == 0) //Baud rate
    {
      if(strcmp_P(new_setting, PSTR("2400")) == 0) new_system_baud = BAUD_2400;
      else if(strcmp_P(new_setting, PSTR("4800")) == 0) new_system_baud = BAUD_4800;
      else if(strcmp_P(new_setting, PSTR("9600")) == 0) new_system_baud = BAUD_9600;
      else if(strcmp_P(new_setting, PSTR("19200")) == 0) new_system_baud = BAUD_19200;
      else if(strcmp_P(new_setting, PSTR("38400")) == 0) new_system_baud = BAUD_38400;
      else if(strcmp_P(new_setting, PSTR("57600")) == 0) new_system_baud = BAUD_57600;
      else if(strcmp_P(new_setting, PSTR("115200")) == 0) new_system_baud = BAUD_115200;
      else new_system_baud = BAUD_9600; //Default is 9600bps
    }
    else if(setting_number == 1) //Escape character
    {
      new_system_escape = new_setting_int;
      if(new_system_escape == 0 || new_system_escape > 127) new_system_escape = 26; //Default is ctrl+z
    }
    else if(setting_number == 2) //Max amount escape character
    {
      new_system_max_escape = new_setting_int;
      if(new_system_max_escape == 0 || new_system_max_escape > 10) new_system_max_escape = 3; //Default is 3
    }
    else if(setting_number == 3) //System mode
    {
      new_system_mode = new_setting_int;
      if(new_system_mode == 0 || new_system_mode > MODE_COMMAND) new_system_mode = MODE_NEWLOG; //Default is NEWLOG
    }
    else if(setting_number == 4) //Verbose setting
    {
      new_system_verbose = new_setting_int;
      if(new_system_verbose != ON && new_system_verbose != OFF) new_system_verbose = ON; //Default is on
    }
    else if(setting_number == 5) //Echo setting
    {
      new_system_echo = new_setting_int;
      if(new_system_echo != ON && new_system_echo != OFF) new_system_echo = ON; //Default is on
    }
    else
      //We're done! Stop looking for settings
      break;

    setting_number++;
  }

#if DEBUG
  //This will print the found settings. Use for debugging
  NewSerial.print(F("Settings determined to be: "));

  char temp_string[CFG_LENGTH]; //"115200,103,14,0,1,1\0" = 115200 bps, escape char of ASCII(103), 14 times, new log mode, verbose on, echo on.
  char temp[CFG_LENGTH];

  if(new_system_baud == BAUD_2400) strcpy_P(temp_string, PSTR("2400"));
  if(new_system_baud == BAUD_4800) strcpy_P(temp_string, PSTR("4800"));
  if(new_system_baud == BAUD_9600) strcpy_P(temp_string, PSTR("9600"));
  if(new_system_baud == BAUD_19200) strcpy_P(temp_string, PSTR("19200"));
  if(new_system_baud == BAUD_38400) strcpy_P(temp_string, PSTR("38400"));
  if(new_system_baud == BAUD_57600) strcpy_P(temp_string, PSTR("57600"));
  if(new_system_baud == BAUD_115200) strcpy_P(temp_string, PSTR("115200"));

  sprintf_P(temp, PSTR(",%d,%d,%d,%d,%d\0"), new_system_escape, new_system_max_escape, new_system_mode, new_system_verbose, new_system_echo);
  strcat(temp_string, temp); //Add this string to the system string

  NewSerial.println(temp_string);
#endif

  //We now have the settings loaded into the global variables. Now check if they're different from EEPROM settings
  boolean recordNewSettings = false;

  if(new_system_baud != setting_uart_speed) {
    //If the baud rate from the file is different from the current setting,
    //Then update the setting to the file setting
    //And re-init the UART
    EEPROM.write(LOCATION_BAUD_SETTING, new_system_baud);
    setting_uart_speed = new_system_baud;

    //Move system to new uart speed
    if(setting_uart_speed == BAUD_2400) NewSerial.begin(2400);
    if(setting_uart_speed == BAUD_4800) NewSerial.begin(4800);
    if(setting_uart_speed == BAUD_9600) NewSerial.begin(9600);
    if(setting_uart_speed == BAUD_19200) NewSerial.begin(19200);
    if(setting_uart_speed == BAUD_38400) NewSerial.begin(38400);
    if(setting_uart_speed == BAUD_57600) NewSerial.begin(57600);
    if(setting_uart_speed == BAUD_115200) NewSerial.begin(115200);

    recordNewSettings = true;
  }

  if(new_system_mode != setting_system_mode) {
    //Goto new system mode
    setting_system_mode = new_system_mode;
    EEPROM.write(LOCATION_SYSTEM_SETTING, setting_system_mode);

    recordNewSettings = true;
  }

  if(new_system_escape != setting_escape_character) {
    //Goto new system escape char
    setting_escape_character = new_system_escape;
    EEPROM.write(LOCATION_ESCAPE_CHAR, setting_escape_character); 

    recordNewSettings = true;
  }

  if(new_system_max_escape != setting_max_escape_character) {
    //Goto new max escape
    setting_max_escape_character = new_system_max_escape;
    EEPROM.write(LOCATION_MAX_ESCAPE_CHAR, setting_max_escape_character);

    recordNewSettings = true;
  }

  if(new_system_verbose != setting_verbose) {
    //Goto new verbose setting
    setting_verbose = new_system_verbose;
    EEPROM.write(LOCATION_VERBOSE, setting_verbose);

    recordNewSettings = true;
  }

  if(new_system_echo != setting_echo) {
    //Goto new echo setting
    setting_echo = new_system_echo;
    EEPROM.write(LOCATION_ECHO, setting_echo);

    recordNewSettings = true;
  }

  //We don't want to constantly record a new config file on each power on. Only record when there is a change.
  if(recordNewSettings == true)
    record_config_file(); //If we corrected some values because the config file was corrupt, then overwrite any corruption
#if DEBUG
  else
    NewSerial.println(F("Config file matches system settings"));
#endif

  //All done! New settings are loaded. System will now operate off new config settings found in file.

  //Set flags for extended mode options  
  if (setting_verbose == ON)
    feedback_mode |= EXTENDED_INFO;
  else
    feedback_mode &= ((byte)~EXTENDED_INFO);

  if (setting_echo == ON)
    feedback_mode |= ECHO;
  else
    feedback_mode &= ((byte)~ECHO);

}

//Records the current EEPROM settings to the config file
//If a config file exists, it is trashed and a new one is created
void record_config_file(void)
{
  //I'm worried that the user may not be in the root directory when modifying config settings. If that is the case,
  //config file will not be found and it will be created in some erroneus directory. The changes to user settings may be lost on the
  //next power cycles. To prevent this, we will open another instance of the file system, then close it down when we are done.
  SdFile rootDirectory;
  SdFile myFile;
  if (!rootDirectory.openRoot(&volume)) systemError(ERROR_ROOT_INIT); // open the root directory

  char configFileName[strlen(CFG_FILENAME)];
  strcpy_P(configFileName, PSTR(CFG_FILENAME)); //This is the name of the config file. 'config.sys' is probably a bad idea.

  //If there is currently a config file, trash it
  if (myFile.open(&rootDirectory, configFileName, O_WRITE)) {
    if (!myFile.remove()){
      NewSerial.println(F("Remove config failed"));
      myFile.close(); //Close this file
      rootDirectory.close(); //Close this file structure instance
      return;
    }
  }

  //myFile.close(); //Not sure if we need to close the file before we try to reopen it

  //Create config file
  if (myFile.open(&rootDirectory, configFileName, O_CREAT | O_APPEND | O_WRITE) == 0) {
    NewSerial.println(F("Create config failed"));
    myFile.close(); //Close this file
    rootDirectory.close(); //Close this file structure instance
    return;
  }
  //Config was successfully created, now record current system settings to the config file

  char settings_string[CFG_LENGTH]; //"115200,103,14,0,1,1\0" = 115200 bps, escape char of ASCII(103), 14 times, new log mode, verbose on, echo on.

  //Before we read the EEPROM values, they've already been tested and defaulted in the read_system_settings function
  char current_system_baud = EEPROM.read(LOCATION_BAUD_SETTING);
  char current_system_escape = EEPROM.read(LOCATION_ESCAPE_CHAR);
  char current_system_max_escape = EEPROM.read(LOCATION_MAX_ESCAPE_CHAR);
  char current_system_mode = EEPROM.read(LOCATION_SYSTEM_SETTING);
  char current_system_verbose = EEPROM.read(LOCATION_VERBOSE);
  char current_system_echo = EEPROM.read(LOCATION_ECHO);

  //Determine current baud and copy it to string
  char baudRate[6];
  if(current_system_baud == BAUD_2400) strcpy_P(baudRate, PSTR("2400"));
  if(current_system_baud == BAUD_4800) strcpy_P(baudRate, PSTR("4800"));
  if(current_system_baud == BAUD_9600) strcpy_P(baudRate, PSTR("9600"));
  if(current_system_baud == BAUD_19200) strcpy_P(baudRate, PSTR("19200"));
  if(current_system_baud == BAUD_38400) strcpy_P(baudRate, PSTR("38400"));
  if(current_system_baud == BAUD_57600) strcpy_P(baudRate, PSTR("57600"));
  if(current_system_baud == BAUD_115200) strcpy_P(baudRate, PSTR("115200"));

  //Convert system settings to visible ASCII characters
  sprintf_P(settings_string, PSTR("%s,%d,%d,%d,%d,%d\0"), baudRate, current_system_escape, current_system_max_escape, current_system_mode, current_system_verbose, current_system_echo);
  //Record current system settings to the config file
  if(myFile.write(settings_string, strlen(settings_string)) != strlen(settings_string))
    NewSerial.println(F("error writing to file"));

  //Add a decoder line to the file
  char helperString[47]; //This probably should not be hard coded but we're doing it anyway!
  strcpy_P(helperString, PSTR("\n\rbaud,escape,esc#,mode,verb,echo\0")); //This is the name of the config file. 'config.sys' is probably a bad idea.
  myFile.write(helperString); //Add this string to the file

  myFile.sync(); //Sync all newly written data to card
  myFile.close(); //Close this file
  rootDirectory.close(); //Close this file structure instance
  //Now that the new config file has the current system settings, nothing else to do!
}

//End core system functions
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


void command_shell(void)
{
  //Provide a simple shell
  char buffer[30];
  byte tmp_var;

  SdFile tempFile;

#ifdef INCLUDE_SIMPLE_EMBEDDED
  uint32_t file_index;
  byte command_succedded = 1;
#endif //INCLUDE_SIMPLE_EMBEDDED

  printRam(); //Print the available RAM

  while(true)
  {
#ifdef INCLUDE_SIMPLE_EMBEDDED
    if ((feedback_mode & EMBEDDED_END_MARKER) > 0)
      NewSerial.print((char)0x1A); // Ctrl+Z ends the data and marks the start of result

    if (command_succedded == 0)
      NewSerial.print(F("!"));
#endif
    NewSerial.print(F(">"));

    //Read command
    if(read_line(buffer, sizeof(buffer)) < 1)
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

    //Execute command
    if(strcmp_P(command_arg, PSTR("init")) == 0)
    {
      if ((feedback_mode & EXTENDED_INFO) > 0)
        NewSerial.println(F("Closing down file system"));

      //Close file system
      currentDirectory.close();

      //Open the root directory
      if (!currentDirectory.openRoot(&volume)) systemError(ERROR_ROOT_INIT);
      memset(folderTree, 0, sizeof(folderTree)); //Clear folder tree
      if ((feedback_mode & EXTENDED_INFO) > 0)
        NewSerial.println(F("File system initialized"));
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

    else if(strcmp_P(command_arg, PSTR("ls")) == 0)
    {
      if ((feedback_mode & EXTENDED_INFO) > 0)
      {
        NewSerial.print(F("Volume is FAT"));
        NewSerial.println(volume.fatType(), DEC);
      }

      //ERROR
      //      currentDirectory.ls(LS_SIZE, 0, &wildcmp, get_cmd_arg(1));
      currentDirectory.ls(LS_SIZE | LS_R);
      NewSerial.println(F("Wild card ls not yet supported"));

#ifdef INCLUDE_SIMPLE_EMBEDDED
      command_succedded = 1;
#endif

    }
    else if(strcmp_P(command_arg, PSTR("md")) == 0)
    {
      //Argument 2: Directory name
      command_arg = get_cmd_arg(1);
      if(command_arg == 0)
        continue;

      SdFile newDirectory;
      if (!newDirectory.makeDir(&currentDirectory, command_arg)) {
        if ((feedback_mode & EXTENDED_INFO) > 0)
        {
          NewSerial.print(F("error creating directory: "));
          NewSerial.println(command_arg);
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
    else if(strcmp_P(command_arg, PSTR("rm")) == 0)
    {
      //Argument 2: Remove option or file name/subdirectory to remove
      command_arg = get_cmd_arg(1);
      if(command_arg == 0)
        continue;

      //Argument 2: Remove subfolder recursively?
      if ((count_cmd_args() == 3) && (strcmp_P(command_arg, PSTR("-rf")) == 0))
      {
        //Remove the subfolder
        if (tempFile.open(&currentDirectory, get_cmd_arg(2), O_READ))
        {
          tmp_var = tempFile.rmRfStar();
          tempFile.close();
#ifdef INCLUDE_SIMPLE_EMBEDDED
          command_succedded = tmp_var;
#endif
        }
        continue;
      }

      //Argument 2: Remove subfolder if empty or remove file
      if (tempFile.open(&currentDirectory, command_arg, O_READ))
      {
        tmp_var = 0;
        if (tempFile.isDir() || tempFile.isSubDir())
          tmp_var = tempFile.rmDir();
        else
        {
          tempFile.close();
          if (tempFile.open(&currentDirectory, command_arg, O_WRITE))
            tmp_var = tempFile.remove();
        }
#ifdef INCLUDE_SIMPLE_EMBEDDED
        command_succedded = tmp_var;
#endif
        tempFile.close();
        continue;
      }

      //Argument 2: File wildcard removal
      //Fixed by dlkeng - Thank you!
      uint32_t filesDeleted = 0;

      char fname[13];

      strupr(command_arg);

      currentDirectory.seekSet(0);
      while (tempFile.openNext(&currentDirectory, O_WRITE)) //Step through each object in the current directory
      {
        if (!tempFile.isDir() && !tempFile.isSubDir()) // Remove only files
        {
          if (tempFile.getFilename(fname)) // Get the filename of the object we're looking at
          {
            if (wildcmp(command_arg, fname))  // See if it matches the wildcard
            {
              if (tempFile.remove()) // Remove this file
              {
                ++filesDeleted;
              }
            }
          }
        }
        tempFile.close();
      }

      if ((feedback_mode & EXTENDED_INFO) > 0)
      {
        NewSerial.print(filesDeleted);
        NewSerial.println(F(" file(s) deleted"));
      }
#ifdef INCLUDE_SIMPLE_EMBEDDED
      if (filesDeleted > 0)
        command_succedded = 1;
#endif
    }
    else if(strcmp_P(command_arg, PSTR("cd")) == 0)
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
    else if(strcmp_P(command_arg, PSTR("read")) == 0)
    {
      //Argument 2: File name
      command_arg = get_cmd_arg(1);
      if(command_arg == 0)
        continue;

      //search file in current directory and open it
      if (!tempFile.open(&currentDirectory, command_arg, O_READ)) {
        if ((feedback_mode & EXTENDED_INFO) > 0)
        {
          NewSerial.print(F("Failed to open file "));
          NewSerial.println(command_arg);
        }
        continue;
      }

      //Argument 3: File seek position
      if ((command_arg = get_cmd_arg(2)) != 0) {
        if ((command_arg = is_number(command_arg, strlen(command_arg))) != 0) {
          int32_t offset = strtolong(command_arg);
          if(!tempFile.seekSet(offset)) {
            if ((feedback_mode & EXTENDED_INFO) > 0)
            {
              NewSerial.print(F("Error seeking to "));
              NewSerial.println(command_arg);
            }
            tempFile.close();
            continue;
          }
        }
      }

      //Argument 4: How much data (number of characters) to read from file
      uint32_t readAmount = (uint32_t)-1;
      if ((command_arg = get_cmd_arg(3)) != 0)
        if ((command_arg = is_number(command_arg, strlen(command_arg))) != 0)
          readAmount = strtolong(command_arg);

      //Argument 5: Should we print ASCII or HEX? 1 = ASCII, 2 = HEX, 3 = RAW
      uint32_t printType = 1; //Default to ASCII
      if ((command_arg = get_cmd_arg(4)) != 0)
        if ((command_arg = is_number(command_arg, strlen(command_arg))) != 0)
          printType = strtolong(command_arg);

      //Print file contents from current seek position to the end (readAmount)
      byte c;
      int16_t v;
      int16_t readSpot = 0;
      while ((v = tempFile.read()) >= 0) {
        //file.read() returns a 16 bit character. We want to be able to print extended ASCII
        //So we need 8 bit unsigned.
        c = v; //Force the 16bit signed variable into an 8bit unsigned

        if(++readSpot > readAmount) break;
        if(printType == 1) { //Printing ASCII
          //Test character to see if it is visible, if not print '.'
          if(c >= ' ' && c < 127)
            NewSerial.write(c); //Regular ASCII
          else if (c == '\n' || c == '\r')
            NewSerial.write(c); //Go ahead and print the carriage returns and new lines
          else
            NewSerial.write(F(".")); //For non visible ASCII characters, print a .
        }
        else if (printType == 2) {
          NewSerial.print(c, HEX); //Print in HEX
          NewSerial.print(F(" "));
        }
        else if (printType == 3) {
          NewSerial.write(c); //Print raw
        }

      }
      tempFile.close();
#ifdef INCLUDE_SIMPLE_EMBEDDED
      command_succedded = 1;
      if ((feedback_mode & EMBEDDED_END_MARKER) == 0)
#endif
        NewSerial.println();
    }
    else if(strcmp_P(command_arg, PSTR("write")) == 0)
    {
      //Argument 2: File name
      command_arg = get_cmd_arg(1);
      if(command_arg == 0)
        continue;

      //search file in current directory and open it
      if (!tempFile.open(&currentDirectory, command_arg, O_WRITE)) {
        if ((feedback_mode & EXTENDED_INFO) > 0)
        {
          NewSerial.print(F("Failed to open file "));
          NewSerial.println(command_arg);
        }
        continue;
      }

      //Argument 3: File seek position
      if ((command_arg = get_cmd_arg(2)) != 0){
        if ((command_arg = is_number(command_arg, strlen(command_arg))) != 0) {
          int32_t offset = strtolong(command_arg);
          if(!tempFile.seekSet(offset)) {
            if ((feedback_mode & EXTENDED_INFO) > 0)
            {
              NewSerial.print(F("Error seeking to "));
              NewSerial.println(command_arg);
            }
            tempFile.close();
            continue;
          }
        }
      }

      //read text from the shell and write it to the file
      byte dataLen;
      while(1) {
#ifdef INCLUDE_SIMPLE_EMBEDDED
        if ((feedback_mode & EMBEDDED_END_MARKER) > 0)
          NewSerial.print((char)0x1A); // Ctrl+Z ends the data and marks the start of result
#endif
        NewSerial.print(F("<")); //give a different prompt

        //read one line of text
        dataLen = read_line(buffer, sizeof(buffer));
        if(!dataLen) {
#ifdef INCLUDE_SIMPLE_EMBEDDED
          command_succedded = 1;
#endif
          break;
        }

        //write text to file
        if(tempFile.write((byte*) buffer, dataLen) != dataLen) {
          if ((feedback_mode & EXTENDED_INFO) > 0)
            NewSerial.print(F("error writing to file\n\r"));
          break;
        }
      }

      tempFile.close();
    }
    else if(strcmp_P(command_arg, PSTR("size")) == 0)
    {
      //Argument 2: File name - no wildcard search
      command_arg = get_cmd_arg(1);
      if(command_arg == 0)
        continue;

      //search file in current directory and open it
      if (tempFile.open(&currentDirectory, command_arg, O_READ)) {
        NewSerial.print(tempFile.fileSize());
        tempFile.close();
#ifdef INCLUDE_SIMPLE_EMBEDDED
        command_succedded = 1;
#endif
      }
      else
      {
        if ((feedback_mode & EXTENDED_INFO) > 0)
          NewSerial.print(F("-1")); //Indicate no file is found
      }
#ifdef INCLUDE_SIMPLE_EMBEDDED
      if ((feedback_mode & EMBEDDED_END_MARKER) == 0)
#endif
        NewSerial.println();
    }
    else if(strcmp_P(command_arg, PSTR("disk")) == 0)
    {
      //Print card type
      NewSerial.print(F("\nCard type: "));
      switch(card.type()) {
      case SD_CARD_TYPE_SD1:
        NewSerial.println(F("SD1"));
        break;
      case SD_CARD_TYPE_SD2:
        NewSerial.println(F("SD2"));
        break;
      case SD_CARD_TYPE_SDHC:
        NewSerial.println(F("SDHC"));
        break;
      default:
        NewSerial.println(F("Unknown"));
      }

      //Print card information
      cid_t cid;
      if (!card.readCID(&cid)) {
        NewSerial.print(F("readCID failed"));
        continue;
      }

      NewSerial.print(F("Manufacturer ID: "));
      NewSerial.println(cid.mid, HEX);

      NewSerial.print(F("OEM ID: "));
      NewSerial.print(cid.oid[0]);
      NewSerial.println(cid.oid[1]);

      NewSerial.print(F("Product: "));
      for (byte i = 0; i < 5; i++) {
        NewSerial.print(cid.pnm[i]);
      }

      NewSerial.print(F("\n\rVersion: "));
      NewSerial.print(cid.prv_n, DEC);
      NewSerial.print(F("."));
      NewSerial.println(cid.prv_m, DEC);

      NewSerial.print(F("Serial number: "));
      NewSerial.println(cid.psn);

      NewSerial.print(F("Manufacturing date: "));
      NewSerial.print(cid.mdt_month);
      NewSerial.print(F("/"));
      NewSerial.println(2000 + cid.mdt_year_low + (cid.mdt_year_high <<4));

      csd_t csd;
      uint32_t cardSize = card.cardSize();
      if (cardSize == 0 || !card.readCSD(&csd)) {
        NewSerial.println(F("readCSD failed"));
        continue;
      }
      NewSerial.print(F("Card Size: "));
      cardSize /= 2; //Card size is coming up as double what it should be? Don't know why. Dividing it by 2 to correct.
      NewSerial.print(cardSize);
      //After division
      //7761920 = 8GB card
      //994816 = 1GB card
      NewSerial.println(F(" KB"));
#ifdef INCLUDE_SIMPLE_EMBEDDED
      command_succedded = 1;
#endif
    }
    else if(strcmp_P(command_arg, PSTR("sync")) == 0)
    {
      //Flush all current data and record it to card
      //This isn't really tested.
      tempFile.sync();
      currentDirectory.sync();
#ifdef INCLUDE_SIMPLE_EMBEDDED
      command_succedded = 1;
#endif
    }

    //Reset the AVR
    else if(strcmp_P(command_arg, PSTR("reset")) == 0)
    {
      Reset_AVR();
    }

    //Create new file
    else if(strcmp_P(command_arg, PSTR("new")) == 0)
    {
      //Argument 2: File name
      command_arg = get_cmd_arg(1);
      if(command_arg == 0)
        continue;

      //Try to open file, if fail (file doesn't exist), then break
      if (tempFile.open(&currentDirectory, command_arg, O_CREAT | O_EXCL | O_WRITE)) {//Will fail if file already exsists
        tempFile.close(); //Everything is good, Close this new file we just opened
#ifdef INCLUDE_SIMPLE_EMBEDDED
        command_succedded = 1;
#endif
      }
      else
      {
        if ((feedback_mode & EXTENDED_INFO) > 0) {
          NewSerial.print(F("Error creating file: "));
          NewSerial.println(command_arg);
        }
      }
    }

    //Append to a current file
    else if(strcmp_P(command_arg, PSTR("append")) == 0)
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

    else if(strcmp_P(command_arg, PSTR("pwd")) == 0)
    {
      NewSerial.print(F(".\\"));
      tmp_var = getNextFolderTreeIndex();
      for (byte i = 0; i < tmp_var; i++)
      {
        NewSerial.print(folderTree[i]);
        if (i < tmp_var-1) NewSerial.print(F("\\"));
      }
      NewSerial.println();
#ifdef INCLUDE_SIMPLE_EMBEDDED
      command_succedded = 1;
#endif
    }
    // echo <on>|<off>
    else if(strcmp_P(command_arg, PSTR("echo")) == 0)
    {
      //Argument 2: <on>|<off>
      // Set if we are going to echo the characters back to the client or not
      command_arg = get_cmd_arg(1);
      if (command_arg != 0)
      {
        if ((tmp_var = strcmp_P(command_arg, PSTR("on"))) == 0) {
          setting_echo = ON;
          feedback_mode |= ECHO;
        }
        else if ((tmp_var = strcmp_P(command_arg, PSTR("off"))) == 0) {
          setting_echo = OFF;
          feedback_mode &= ((byte)~ECHO); 
        }

        EEPROM.write(LOCATION_ECHO, setting_echo); //Commit this setting to EEPROM
        record_config_file(); //Put this new setting into the config file

#ifdef INCLUDE_SIMPLE_EMBEDDED
          command_succedded = (tmp_var == 0);
#endif
      }
    }
    // verbose <on>|<off>
    else if(strcmp_P(command_arg, PSTR("verbose")) == 0)
    {
      //Argument 2: <on>|<off>
      // Set if we are going to show extended error information when executing commands
      command_arg = get_cmd_arg(1);
      if (command_arg != 0)
      {
        if ((tmp_var = strcmp_P(command_arg, PSTR("on"))) == 0) {
          setting_verbose = ON;
          feedback_mode |= EXTENDED_INFO;
        }
        else if ((tmp_var = strcmp_P(command_arg, PSTR("off"))) == 0) {
          setting_verbose = OFF;
          feedback_mode &= ((byte)~EXTENDED_INFO);
        }

        EEPROM.write(LOCATION_VERBOSE, setting_verbose); //Commit this setting to EEPROM
        record_config_file(); //Put this new setting into the config file

#ifdef INCLUDE_SIMPLE_EMBEDDED
          command_succedded = (tmp_var == 0);
#endif
      }
    }
#ifdef INCLUDE_SIMPLE_EMBEDDED
    //http://code.google.com/p/sdfatlib/issues/detail?id=11 by EdfeldtPeem (Embedded End Marker) <on>|<off>
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
          feedback_mode &= ((byte)~EMBEDDED_END_MARKER);

        command_succedded = (tmp_var == 0);
      }
    }

    /*
//ERROR
     //The function *.fileInfo is no yet supported by sdfatlib beta 20120108
     
     // ecountf
     // returns the number of files in current folder |count|
     else if(strcmp_P(command_arg, F("efcount")) == 0)
     {
     //Argument 2: wild card search
     command_arg = get_cmd_arg(1);
     file_index = 0;
     
     NewSerial.print(F("count|"));
     Serial.println(currentDirectory.fileInfo(FI_COUNT, 0, buffer));
     command_succedded = 1;
     }
     // efname <file index>
     // Returns the name and the size of a file <name>|<size>
     else if(strcmp_P(command_arg, F("efinfo")) == 0)
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
     NewSerial.print(buffer);
     NewSerial.print('|');
     NewSerial.println(size);
     command_succedded = 1;
     }
     }
     }
     */
#endif //INCLUDE_SIMPLE_EMBEDDED

    else
    {
      if ((feedback_mode & EXTENDED_INFO) > 0) {
        NewSerial.print(F("unknown command: "));
        NewSerial.println(command_arg);
      }
    }
  }

  //Do we ever get this far?
  NewSerial.print(F("Exiting: closing down\n"));
}

//Reads a line until the \n enter character is found
byte read_line(char* buffer, byte buffer_length)
{

  memset(buffer, 0, buffer_length); //Clear buffer

  byte read_length = 0;
  while(read_length < buffer_length - 1) {
    while (!NewSerial.available());
    byte c = NewSerial.read();

    STAT1_PORT ^= (1<<STAT1); //Blink the stat LED while typing

    if(c == 0x08 || c == 0x7f) { //Backspace characters
      if(read_length < 1)
        continue;

      --read_length;
      buffer[read_length] = '\0'; //Put a terminator on the string in case we are finished

      NewSerial.print((char)0x08); //Move back one space
      NewSerial.print(F(" ")); //Put a blank there to erase the letter from the terminal
      NewSerial.print((char)0x08); //Move back again

      continue;
    }

    // Only echo back if this is enabled
    if ((feedback_mode & ECHO) > 0)
      NewSerial.print((char)c);

    if(c == '\r') {
      NewSerial.println();
      buffer[read_length] = '\0';
      break;
    }
    else if (c == '\n') {
      //Do nothing - ignore newlines
      //This was added to v2.51 to make command line control easier from a micro
      //You never know what fprintf or sprintf is going to throw at the buffer
      //See issue 66: https://github.com/nseidle/OpenLog/issues/66
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

//We no longer use hardware receive, instead, we use the original .receive and .available
/*byte uart_getc(void)
 {
 while(!(UCSR0A & _BV(RXC0)));
 
 byte b = UDR0;
 
 return b;
 }*/

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

byte gotoDir(char *dir)
{
  SdFile subDirectory;
  byte tmp_var = 0;

  //Goto parent directory
  //@NOTE: This is a fix to make this work. Should be replaced with
  //proper handling. Limitation: FOLDER_TRACK_DEPTH subfolders
  //ERROR  if (strcmp_P(dir, F("..")) == 0) {
  if (strcmp_P(dir, PSTR("..")) == 0) {
    tmp_var = 1;

    //close file system
    currentDirectory.close();

    // open the root directory
    if (!currentDirectory.openRoot(&volume)) systemError(ERROR_ROOT_INIT);
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
      NewSerial.print(F("cannot cd to parent directory: "));
      NewSerial.println(dir);
    }
  }
  else
  {
    if (!(tmp_var = subDirectory.open(&currentDirectory, dir, O_READ))) {
      if ((feedback_mode & EXTENDED_INFO) > 0)
      {
        NewSerial.print(F("directory not found: "));
        NewSerial.println(dir);
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

void print_menu(void)
{
  NewSerial.println(F("OpenLog v3.13"));
  NewSerial.println(F("Basic commands:"));
  NewSerial.println(F("new <file>\t\t: Creates <file>"));
  NewSerial.println(F("append <file>\t\t: Appends text to end of <file>"));

  //NewSerial.println(F("write <file> <offset>\t: Writes text to <file>, starting from <offset>. The text is read from the UART, line by line. Finish with an empty line"));
  //NewSerial.println(F("rm <file>\t\t: Deletes <file>. Use wildcard to do a wildcard removal of files"));

  NewSerial.println(F("md <directory>\t\t: Creates a <directory>"));

  //NewSerial.println(F("cd <directory>\t\t: Changes current working directory to <directory>"));
  //NewSerial.println(F("cd ..\t\t: Changes to lower directory in tree"));

  NewSerial.println(F("ls\t\t\t: Shows the content of the current directory.."));
  NewSerial.println(F("read <file> <start> <length> <type>: Outputs <length> bytes of <file> to the terminal starting at <start>. Omit <start> and <length> to read whole file. <type> 1 prints in ASCII, 2 in HEX."));
  NewSerial.println(F("size <file>\t\t: Write size of <file> to terminal"));
  NewSerial.println(F("disk\t\t\t: Shows card information"));

  //NewSerial.println(F("init\t\t\t: Reinitializes and reopens the memory card"));
  //NewSerial.println(F("sync\t\t\t: Ensures all buffered data is written to the card"));

  NewSerial.println(F("reset\t\t\t: Causes unit to reset, uses parameters in config file"));
  NewSerial.println(F("set\t\t\t: Menu to configure system mode"));
  NewSerial.println(F("baud\t\t\t: Menu to configure baud rate"));
}

//Configure what baud rate to communicate at
void baud_menu(void)
{
  byte uart_speed = EEPROM.read(LOCATION_BAUD_SETTING);

  while(1)
  {
    NewSerial.println(F("\n\rBaud Configuration:"));

    NewSerial.print(F("Current: "));
    if(uart_speed == BAUD_4800) NewSerial.print(F("48"));
    if(uart_speed == BAUD_2400) NewSerial.print(F("24"));
    if(uart_speed == BAUD_9600) NewSerial.print(F("96"));
    if(uart_speed == BAUD_19200) NewSerial.print(F("192"));
    if(uart_speed == BAUD_38400) NewSerial.print(F("384"));
    if(uart_speed == BAUD_57600) NewSerial.print(F("576"));
    if(uart_speed == BAUD_115200) NewSerial.print(F("1152"));
    NewSerial.println(F("00 bps"));

    NewSerial.println(F("Change to:"));
    NewSerial.println(F("1) 2400 bps"));
    NewSerial.println(F("2) 4800 bps"));
    NewSerial.println(F("3) 9600 bps"));
    NewSerial.println(F("4) 19200 bps"));
    NewSerial.println(F("5) 38400 bps"));
    NewSerial.println(F("6) 57600 bps"));
    NewSerial.println(F("7) 115200 bps"));
    NewSerial.println(F("x) Exit"));

    //Print prompt
    NewSerial.print(F(">"));

    //Read command
    while(!NewSerial.available());
    char command = NewSerial.read();

    //Execute command
    if(command == '1')
    {
      NewSerial.println(F("Going to 2400bps"));

      //Set baud rate to 2400
      EEPROM.write(LOCATION_BAUD_SETTING, BAUD_2400);
      record_config_file(); //Put this new setting into the config file
      blink_error(ERROR_NEW_BAUD);
      return;
    }
    if(command == '2')
    {
      NewSerial.println(F("Going to 4800bps"));

      //Set baud rate to 4800
      EEPROM.write(LOCATION_BAUD_SETTING, BAUD_4800);
      record_config_file(); //Put this new setting into the config file
      blink_error(ERROR_NEW_BAUD);
      return;
    }
    if(command == '3')
    {
      NewSerial.println(F("Going to 9600bps"));

      //Set baud rate to 9600
      EEPROM.write(LOCATION_BAUD_SETTING, BAUD_9600);
      record_config_file(); //Put this new setting into the config file
      blink_error(ERROR_NEW_BAUD);
      return;
    }
    if(command == '4')
    {
      NewSerial.println(F("Going to 19200bps"));

      //Set baud rate to 19200
      EEPROM.write(LOCATION_BAUD_SETTING, BAUD_19200);
      record_config_file(); //Put this new setting into the config file
      blink_error(ERROR_NEW_BAUD);
      return;
    }
    if(command == '5')
    {
      NewSerial.println(F("Going to 38400bps"));

      //Set baud rate to 38400
      EEPROM.write(LOCATION_BAUD_SETTING, BAUD_38400);
      record_config_file(); //Put this new setting into the config file
      blink_error(ERROR_NEW_BAUD);
      return;
    }
    if(command == '6')
    {
      NewSerial.println(F("Going to 57600bps"));

      //Set baud rate to 57600
      EEPROM.write(LOCATION_BAUD_SETTING, BAUD_57600);
      record_config_file(); //Put this new setting into the config file
      blink_error(ERROR_NEW_BAUD);
      return;
    }
    if(command == '7')
    {
      NewSerial.println(F("Going to 115200bps"));

      //Set baud rate to 115200
      EEPROM.write(LOCATION_BAUD_SETTING, BAUD_115200);
      record_config_file(); //Put this new setting into the config file
      blink_error(ERROR_NEW_BAUD);
      return;
    }
    if(command == 'x')
    {
      NewSerial.println(F("Exiting"));
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
  byte system_mode = EEPROM.read(LOCATION_SYSTEM_SETTING);

  while(1)
  {
    NewSerial.println(F("\r\nSystem Configuration"));

    NewSerial.print(F("Current boot mode: "));
    if(system_mode == MODE_NEWLOG) NewSerial.print(F("New file"));
    if(system_mode == MODE_SEQLOG) NewSerial.print(F("Append file"));
    if(system_mode == MODE_COMMAND) NewSerial.print(F("Command"));
    NewSerial.println();

    NewSerial.print(F("Current escape character and amount: "));
    NewSerial.print(setting_escape_character, DEC);
    NewSerial.print(F(" x "));
    NewSerial.println(setting_max_escape_character, DEC);

    NewSerial.println(F("Change to:"));
    NewSerial.println(F("1) New file logging"));
    NewSerial.println(F("2) Append file logging"));
    NewSerial.println(F("3) Command prompt"));
    NewSerial.println(F("4) Reset new file number"));
    NewSerial.println(F("5) New escape character"));
    NewSerial.println(F("6) Number of escape characters"));

#if DEBUG
    NewSerial.println(F("a) Clear all user settings"));
#endif

    NewSerial.println(F("x) Exit"));
    //Print prompt
    NewSerial.print(F(">"));

    //Read command
    while(!NewSerial.available());
    char command = NewSerial.read();

    //Execute command
    if(command == '1')
    {
      NewSerial.println(F("New file logging"));
      EEPROM.write(LOCATION_SYSTEM_SETTING, MODE_NEWLOG);
      record_config_file(); //Put this new setting into the config file
      return;
    }
    if(command == '2')
    {
      NewSerial.println(F("Append file logging"));
      EEPROM.write(LOCATION_SYSTEM_SETTING, MODE_SEQLOG);
      record_config_file(); //Put this new setting into the config file
      return;
    }
    if(command == '3')
    {
      NewSerial.println(F("Command prompt"));
      EEPROM.write(LOCATION_SYSTEM_SETTING, MODE_COMMAND);
      record_config_file(); //Put this new setting into the config file
      return;
    }
    if(command == '4')
    {
      NewSerial.println(F("New file number reset to zero"));
      EEPROM.write(LOCATION_FILE_NUMBER_LSB, 0);
      EEPROM.write(LOCATION_FILE_NUMBER_MSB, 0);

      //65533 log testing
      //EEPROM_write(LOCATION_FILE_NUMBER_LSB, 0xFD);
      //EEPROM_write(LOCATION_FILE_NUMBER_MSB, 0xFF);
      return;
    }
    if(command == '5')
    {
      NewSerial.print(F("Enter a new escape character: "));

      while(!NewSerial.available()); //Wait for user to hit character
      setting_escape_character = NewSerial.read();

      EEPROM.write(LOCATION_ESCAPE_CHAR, setting_escape_character);
      record_config_file(); //Put this new setting into the config file

        NewSerial.print(F("\n\rNew escape character: "));
      NewSerial.println(setting_escape_character, DEC);
      return;
    }
    if(command == '6')
    {
      byte choice = 10;
      while(choice > 9 || choice < 1)
      {
        NewSerial.print(F("\n\rEnter number of escape characters to look for (1 to 9): "));
        while(!NewSerial.available()); //Wait for user to hit character
        choice = NewSerial.read() - '0';
      }

      setting_max_escape_character = choice;
      EEPROM.write(LOCATION_MAX_ESCAPE_CHAR, setting_max_escape_character);
      record_config_file(); //Put this new setting into the config file

        NewSerial.print(F("\n\rNumber of escape characters needed: "));
      NewSerial.println(setting_max_escape_character, DEC);
      return;
    }

#if DEBUG
    //This allows us to reset the EEPROM and config file on a unit to see what would happen to an 
    //older unit that is upgraded/reflashed to newest firmware
    if(command == 'a')
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

      char configFileName[strlen(CFG_FILENAME)];
      strcpy_P(configFileName, PSTR(CFG_FILENAME)); //This is the name of the config file. 'config.sys' is probably a bad idea.

      //If there is currently a config file, trash it
      if (file.open(&rootDirectory, configFileName, O_WRITE)) {
        NewSerial.println(F("Deleting config"));
        if (!file.remove()){
          NewSerial.println(F("Remove config failed")); 
          return;
        }
      }

      NewSerial.println(F("Unit has been reset. Please power cycle"));
      while(1);
    }
#endif

    if(command == 'x')
    {
      //Do nothing, just exit
      NewSerial.println(F("Exiting"));
      return;
    }
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

//Returns the number of command line arguments
byte count_cmd_args(void)
{
  byte count = 0;
  byte i = 0;
  for(; i < MAX_COUNT_COMMAND_LINE_ARGS; i++)
    if((cmd_arg[i].arg != 0) && (cmd_arg[i].arg_length > 0))
      count++;

  return count;
}

//Safe index handling of command line arguments
char general_buffer[30]; //Needed for command shell
#define MIN(a,b) ((a)<(b))?(a):(b)
char* get_cmd_arg(byte index)
{
  memset(general_buffer, 0, sizeof(general_buffer));
  if (index < MAX_COUNT_COMMAND_LINE_ARGS)
    if ((cmd_arg[index].arg != 0) && (cmd_arg[index].arg_length > 0))
      return strncpy(general_buffer, cmd_arg[index].arg, MIN(sizeof(general_buffer), cmd_arg[index].arg_length));

  return 0;
}

//Safe adding of command line arguments
void add_cmd_arg(char* buffer, byte buffer_length)
{
  byte count = count_cmd_args();
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
byte split_cmd_line_args(char* buffer, byte buffer_length)
{
  byte arg_index_start = 0;
  byte arg_index_end = 1;

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

//The following functions are required for wildcard use
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//Returns char* pointer to buffer if buffer is a valid number or
//0(null) if not.
char* is_number(char* buffer, byte buffer_length)
{
  for (int i = 0; i < buffer_length; i++)
    if (!isdigit(buffer[i]))
      return 0;

  return buffer;
}
void removeErrorCallback(const char* fileName)
{
  NewSerial.print((char *)F("Remove failed: "));
  NewSerial.println(fileName);
}

//Wildcard string compare.
//Written by Jack Handy - jakkhandy@hotmail.com
//http://www.codeproject.com/KB/string/wildcmp.aspx
byte wildcmp(const char* wild, const char* string)
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

