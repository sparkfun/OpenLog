/*
 12-3-09
 Nathan Seidle
 SparkFun Electronics 2012

 OpenLog hardware and firmware are released under the Creative Commons Share Alike v3.0 license.
 http://creativecommons.org/licenses/by-sa/3.0/
 Feel free to use, distribute, and sell varients of OpenLog. All we ask is that you include attribution of 'Based on OpenLog by SparkFun'.

 OpenLog is based on the work of Bill Greiman and sdfatlib: https://github.com/greiman/SdFat-beta

 This version has the command line interface and config file stripped out in order to simplify the overall 
 program and increase the receive buffer (RAM).

 The only option is the interface baud rate and it has to be set in code, compiled, and loaded onto OpenLog. To
 see how to do this please refer to https://github.com/sparkfun/OpenLog/wiki/Flashing-Firmware

 If you go through the hassle of compiling and loading this firmware you'll benefit from

 */

#define __PROG_TYPES_COMPAT__ //Needed to get SerialPort.h to work in Arduino 1.6.x

#include <SPI.h>
#include <SdFat.h> //We do not use the built-in SD.h file because it calls Serial.print
#include <EEPROM.h>
#include <FreeStack.h> //Allows us to print the available stack/RAM size

#include <avr/sleep.h> //Needed for sleep_mode
#include <avr/power.h> //Needed for powering down perihperals such as the ADC/TWI and Timers

#include <SerialPort.h>
// port 0, 1024 byte RX buffer, 0 byte TX buffer
SerialPort<0, 1024, 0> NewSerial;

//Debug turns on (1) or off (0) a bunch of verbose debug statements. Normally use (0)
//#define DEBUG  1
#define DEBUG  0

#define SD_CHIP_SELECT 10 //On OpenLog this is pin 10

//Internal EEPROM locations for the user settings
#define LOCATION_FILE_NUMBER_LSB	0x03
#define LOCATION_FILE_NUMBER_MSB	0x04

//STAT1 is a general LED and indicates serial traffic
const byte stat1 = 5;  //This is the normal status LED
const byte stat2 = 13; //This is the SPI LED, indicating SD traffic

//Blinking LED error codes
#define ERROR_SD_INIT	  3
#define ERROR_NEW_BAUD	  5
#define ERROR_CARD_INIT   6
#define ERROR_VOLUME_INIT 7
#define ERROR_ROOT_INIT   8
#define ERROR_FILE_OPEN   9

SdFat sd;

long setting_uart_speed = 115200;

//Handle errors by printing the error type and blinking LEDs in certain way
//The function will never exit - it loops forever inside blinkError
void systemError(byte errorType)
{
  NewSerial.print(F("Error "));
  switch (errorType)
  {
    case ERROR_CARD_INIT:
      NewSerial.print(F("card.init"));
      blinkError(ERROR_SD_INIT);
      break;
    case ERROR_VOLUME_INIT:
      NewSerial.print(F("volume.init"));
      blinkError(ERROR_SD_INIT);
      break;
    case ERROR_ROOT_INIT:
      NewSerial.print(F("root.init"));
      blinkError(ERROR_SD_INIT);
      break;
    case ERROR_FILE_OPEN:
      NewSerial.print(F("file.open"));
      blinkError(ERROR_SD_INIT);
      break;
  }
}

void setup(void)
{
  pinMode(stat1, OUTPUT);

  //Power down various bits of hardware to lower power usage
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();

  //Shut off TWI, Timer2, Timer1, ADC
  ADCSRA &= ~(1 << ADEN); //Disable ADC
  ACSR = (1 << ACD); //Disable the analog comparator
  DIDR0 = 0x3F; //Disable digital input buffers on all ADC0-ADC5 pins
  DIDR1 = (1 << AIN1D) | (1 << AIN0D); //Disable digital input buffer on AIN1/0

  power_twi_disable();
  power_timer1_disable();
  power_timer2_disable();
  power_adc_disable();

  //Setup UART
  NewSerial.begin(setting_uart_speed);
  NewSerial.print(F("1"));

  //Setup SD & FAT
  if (!sd.begin(SD_CHIP_SELECT, SPI_FULL_SPEED)) systemError(ERROR_CARD_INIT);

  NewSerial.print(F("2"));

#if DEBUG
  NewSerial.print(F("\n\nFreeStack: "));
  NewSerial.println(FreeStack());
#endif

}

void loop(void)
{
  appendFile(newLog()); //Append the file name that newlog() returns
}

//Log to a new file everytime the system boots
//Checks the spots in EEPROM for the next available LOG# file name
//Updates EEPROM and then appends to the new log file.
//Limited to 65535 files but this should not always be the case.
char* newLog(void)
{
  byte msb, lsb;
  unsigned int newFileNumber;

  SdFile newFile; //This will contain the file for SD writing

  //Combine two 8-bit EEPROM spots into one 16-bit number
  lsb = EEPROM.read(LOCATION_FILE_NUMBER_LSB);
  msb = EEPROM.read(LOCATION_FILE_NUMBER_MSB);

  newFileNumber = msb;
  newFileNumber = newFileNumber << 8;
  newFileNumber |= lsb;

  //If both EEPROM spots are 255 (0xFF), that means they are un-initialized (first time OpenLog has been turned on)
  //Let's init them both to 0
  if ((lsb == 255) && (msb == 255))
  {
    newFileNumber = 0; //By default, unit will start at file number zero
    EEPROM.write(LOCATION_FILE_NUMBER_LSB, 0x00);
    EEPROM.write(LOCATION_FILE_NUMBER_MSB, 0x00);
  }

  //The above code looks like it will forever loop if we ever create 65535 logs
  //Let's quit if we ever get to 65534
  //65534 logs is quite possible if you have a system with lots of power on/off cycles
  if (newFileNumber == 65534)
  {
    //Gracefully drop out to command prompt with some error
    NewSerial.print(F("!Too many logs:1!"));
    return (0); //Bail!
  }

  //If we made it this far, everything looks good - let's start testing to see if our file number is the next available

  //Search for next available log spot
  static char newFileName[13];
  while(1)
  {
    sprintf_P(newFileName, PSTR("LOG%05d.TXT"), newFileNumber); //Splice the new file number into this file name

    //If we are able to create this file, then it didn't exist, we're good, break
    if (newFile.open(newFileName, O_CREAT | O_EXCL | O_WRITE)) break;

    //If file exists, see if empty. If so, use it.
    if (newFile.open(newFileName, O_READ))
    {
      if (newFile.fileSize() == 0)
      {
        newFile.close();        // Close this existing file we just opened.
        return (newFileName); // Use existing empty file.
      }
      newFile.close(); // Close this existing file we just opened.
    }

    //Try the next number
    newFileNumber++;
    if (newFileNumber > 65533) //There is a max of 65534 logs
    {
      NewSerial.print(F("!Too many logs:2!"));
      return (0); //Bail!
    }
  }

  newFileNumber++; //Increment so the next power up uses the next file #

  //Record new_file number to EEPROM
  lsb = (byte)(newFileNumber & 0x00FF);
  msb = (byte)((newFileNumber & 0xFF00) >> 8);

  EEPROM.write(LOCATION_FILE_NUMBER_LSB, lsb); // LSB

  if (EEPROM.read(LOCATION_FILE_NUMBER_MSB) != msb)
    EEPROM.write(LOCATION_FILE_NUMBER_MSB, msb); // MSB

#if DEBUG
  NewSerial.print(F("Created new file: "));
  NewSerial.println(newFileName);
#endif

  return (newFileName);
}

//This is the most important function of the device. These loops have been tweaked as much as possible.
//Modifying this loop may negatively affect how well the device can record at high baud rates.
//Appends a stream of serial data to a given file
//Assumes the currentDirectory variable has been set before entering the routine
//Does not exit until Ctrl+z (ASCII 26) is received
//Returns 0 on error
//Returns 1 on success
byte appendFile(char* fileName)
{
  SdFile workingFile;

  // O_CREAT - create the file if it does not exist
  // O_APPEND - seek to the end of the file prior to each write
  // O_WRITE - open for write
  if (!workingFile.open(fileName, O_CREAT | O_APPEND | O_WRITE)) systemError(ERROR_FILE_OPEN);

  if (workingFile.fileSize() == 0) {
    //This is a trick to make sure first cluster is allocated - found in Bill's example/beta code
    workingFile.rewind();
    workingFile.sync();
  }

  const int LOCAL_BUFF_SIZE = 128; //This is the 2nd buffer. It pulls from the larger Serial buffer as quickly as possible.

  byte buff[LOCAL_BUFF_SIZE];

  const unsigned int MAX_IDLE_TIME_MSEC = 500; //The number of milliseconds of inactivity before unit goes to sleep

#if DEBUG
  NewSerial.print(F("FreeStack: "));
  NewSerial.println(FreeStack());
#endif

  NewSerial.print(F("<")); //give a different prompt to indicate no echoing
  digitalWrite(stat1, HIGH); //Turn on indicator LED

  unsigned long lastSyncTime = millis(); //Keeps track of the last time the file was synced

  //Start recording incoming characters
  while (1)
  {
    byte charsToRecord = NewSerial.read(buff, sizeof(buff));
    if (charsToRecord > 0)
    {
      toggleLED(stat1); //Toggle the STAT1 LED each time we record the buffer

      workingFile.write(buff, charsToRecord);
    }

    //No characters recevied?
    else if ( (unsigned long)(millis() - lastSyncTime) > MAX_IDLE_TIME_MSEC) //If we haven't received any characters in 2s, goto sleep
    {
      workingFile.sync(); //Sync the card before we go to sleep

      digitalWrite(stat1, LOW); //Turn off stat LED to save power

      power_timer0_disable(); //Shut down peripherals we don't need
      power_spi_disable();
      sleep_mode(); //Stop everything and go to sleep. Wake up if serial character received

      power_spi_enable(); //After wake up, power up peripherals
      power_timer0_enable();

      lastSyncTime = millis(); //Reset the last sync time to now
    }
  }

  return (1); //We should never get here!
}

//The following are system functions needed for basic operation
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//Blinks the status LEDs to indicate a type of error
void blinkError(byte ERROR_TYPE) {
  while (1) {
    for (int x = 0 ; x < ERROR_TYPE ; x++) {
      digitalWrite(stat1, HIGH);
      delay(200);
      digitalWrite(stat1, LOW);
      delay(200);
    }

    delay(2000);
  }
}

//Given a pin, it will toggle it from high to low or vice versa
void toggleLED(byte pinNumber)
{
  if (digitalRead(pinNumber)) digitalWrite(pinNumber, LOW);
  else digitalWrite(pinNumber, HIGH);
}
