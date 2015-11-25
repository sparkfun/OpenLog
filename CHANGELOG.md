#Change Log
All notable changes to this project will be documented in this file. We'll also give shout outs to folks who help fix bugs and generally make OpenLog a better experience.
 
## [4.0] - November 23 2015 

27,046 bytes compiled

Welcome to version 4! Now compiles under Arduino v1.6.x! Thanks @owenduffy for getting OpenLog ported to the new SdFatLib.

Worked on issue #168. Fixed issue #178. Thanks @jremington! Now works with Bill's latest [SD lib](https://github.com/greiman/SdFat-beta).

[Added] Escape character sequences are now removed from logs. If you hit ctrl+z three times you'll drop to the command shell and the three escape characters will not be recorded to the log.

[Removed] The pwd command has been removed from the command shell. This *shouldn't* effect anyone but you should know.

[Removed] The sync command has been removed from the command shell. This *shouldn't* effect anyone but you should know.

Fixed a bad bug found by @ystark. Thank you! New file name array is now correctly defined as static.

Fixed the LEDs so they now blink correctly when doing an emergency reset.

In order to get the maximum out of SerialPort lib you'll need to edit SerialPort.h: 

* Set BUFFERED_TX to 0
* Set ENABLE_RX_ERROR_CHECKING to 0

## [3.3] August 2014

28,350 bytes compiled

Using the latest SdFat lib from Bill: https://code.google.com/p/sdfatlib/downloads/list
Using Arduino v1.5.4

Fixed issue #160: https://github.com/sparkfun/OpenLog/issues/160
Logs now increment continously regardless of what file may be on the card. Adds 6 bytes.

Added feature/issue #166: If escape characters is set to zero, it ignores all escape characters. Adds 200 bytes.

Fixed issue 164 - Reducing RX buffer size to allow for correct baud rate changes.
SerialPort<0, 780, 0> NewSerial; //Fails
SerialPort<0, 750, 0> NewSerial; //Works

Fixed issue #163 - using dlkeng's fix to get baud rate to work at 300 baud. Adds 80 bytes.

Increased escape character limits to 0 and 254. If set to zero, it will not check for escape characters.

## [3.2] April 8 2013

28,640 bytes compiled (yay! saved a bunch by removing the text-based menu system)

Re-wrote baud rate system to support anything from 300bps to 1,000,000bps

Added support for newlines when using the 'write' command.
Should fix issue #149.

Fixed issue #134: When removing files and there is a directory, all wildcard files are now removed correctly.
Fixed issue #135: ls with wildcard works again. Thank you @dlkeng!

## [3.14]

29,354 bytes compiled

Added support for 1200bps. Issue #139.

## [3.13]

29,172 bytes compiled

Changed the string compares from strings in RAM (strcmp) to strings located in flash memory (strcmp_P).

We used to use this type of compare: else if(strcmp_P(command_arg, PSTR("md")) == 0)

I'm not sure how or why I got away from it, but bringing it back frees up a lot of RAM. I bet I got rid of PSTRs in an attempt to reduce code size when I should have been more worried about RAM running out. RAM is currently at 645 after 2 and 562 once we've begun append_file. Also figured out there is sprintf_P built just for PSTRs. Neat! RAM is currently at 779 after 2 and 696 once we've begun append_file. Removal of strncmp(). I believe we used it to reduce flash footprint. Once migrated to PSTR, it makes it worse.

## [3.12]

29,282 bytes compiled

Using freeMemory() function from JeeLabs. Seems to be working well.

Echo on/off is working again with a reduction of the buffer from 600 to 500. Code size is up a bit to 29,282. We have free RAM of about 527 after "2" and 444 once we've begun append_file.

## [3.11] March 26 2012

Added freeMemory support for RAM testing.

This was taken from: http://arduino.cc/playground/Code/AvailableMemory

## [3.1] March 23 2012

Fixed bug where entire log data is lost when power is lost.

Added fix from issue #76. Added support for verbose and echo settings recorded to the config file and EEPROM. Bugs #101 and #102 fixed with the help of @pwjansen and @wilafau - thank you!!

Because of the new code to the cycle sensitive append_log loop, 115200bps is not as rock solid as I'd like. I plan to correct this in the Light version.

Added a maxLoop variable calculation to figure out how many bytes to receive before we do a forced file.sync.


## [3.0] March 14 2012

28,354 bytes compiled

Re-write of core functions. Support under Arduino v1.0. Now supports full speed 57600 and 115200bps! Lower standby power.

Be sure to check out https://github.com/nseidle/OpenLog/issues for issues that have been resolved up to this version.

Update to new beta version of SerialPort lib from Bill Greiman. Update to Arduino v1.0. With Bill's library, we don't need to hack the HardwareSerial.cpp.

Re-wrote the append function. This function is the most important function and has the most affect on record accuracy. We are now able to record at 57600 and 115200bps at full blast! The performance is vastly improved.

To compile v3.0 you will need Arduino v1.0 and Bill's beta library: http://beta-lib.googlecode.com/files/SerialLoggerBeta20120108.zip Unzip 'SerialPortBeta20120106.zip' and 'SdFatBeta20120108.zip' to the libraries directory and close and restart Arduino.

Small stuff:

* Redirected all static strings to point to the new way in Arduino 1.0: Serial.print(F("asdf")); instead of PgmPrint
* Figured out lower standby power from the low-power tutorial: http://www.sparkfun.com/tutorials/309
* Corrected #define TRUE to built-in supported 'true'
* Re-arranged some functions
* Migrating to Uno bootloader to get an additional 1500 bytes of program space
* Replumbed everything to get away from hardware UART
* Reduced # of sub directory support from 15 levels to 2 to allow for more RAM
* Wildcard remove is supported in v3.1
* Wildcard is supported in v3.2
* [Remove] efcount and efinfo is not yet supported in v3.0

Testing at 115200. First test with clean/empty card. The second test is with 193MB across 172 files on the microSD card.

* 1GB: 333075/333075, 333075/333075
* 8GB: 333075/333075, 333075/333075
* 16GB: 333075/333075, 333075/333075

The card with tons of files may have problems. Whenever possible, use a clean, empty, freshly formatted card.

Testing at 57600. First test with clean/empty card. The second test is with 193MB across 172 files on the microSD card.

* 1GB: 111075/111075, 111075/111075
* 8GB: 111075/111075, 111075/111075
* 16GB: 111075/111075, 111075/111075

The card with tons of files may have problems. Whenever possible, use a clean, empty, freshly formatted card.

## [2.51] June 1 2011

Improved command prompt parsing (now ignores '\n')
 
We now ignore the \n character when parsing characters from the command prompt. This makes it easier to create code on a microcontroller that correctly controls OpenLog. Previously, println or sprintf commands were adding a \n to the end of the string that would confuse OpenLog. One way around this if you have previous versions is this 

    sprintf(buff, "new blah.txt\r");
    Serial.print(buff); //No println, use \r in string instead

Also - added a CommandTest sketch to demonstrate how you can control OpenLog from a microcontroller / automate the command prompt

## [2.5] February 2 2011

28,782 bytes compiled

Improved 'read' command. Added 'reset' command.

Modified the read command so that we can print extended ASCII characters. The function was also failing to print any value over 127 (limited to signed 8 bits - boo). Thank you @wilafau! You proposed excellent, easy fixes.

Added raw command during print operation. This allows for non-visible and extended ASCII characters to be printed correctly, if need be.
 
Issuing 'reset' command causes OpenLog to reset and re-read the config file. This is important if you want to change the config file then restart.
 
Removed some of the extraneous prints from the help menu to save on space.

## [2.41]

29,124 bytes compiled (yikes)

Power loss bug fixed. Adding support for 38400bps for testing with SparkFum 9DOF IMU logging. 

Found a bug in the append_file routine. If the unit is actively logging, file.writes are flying by quickly. But if OpenLog loses power, none of the data is recorded because there is no file.sync. Data would only get recorded if the unit went idle or if user entered escape command. This has since been fixed with two file.sync() commands.

## [2.4] November 17 2010

29,028 bytes compiled (yikes)

Merged @ringp updates. Commands cd, rm, ls work again! New "rm -rf" command to remove directory and its files. Thanks ringp! Great work.
 
Remember - we had to completely butcher HardwareSerial.cpp so a normal Arduino installation will not work. 

C:\arduino-xxxx\hardware\arduino\cores\arduino\HardwareSerial.cpp

I've added the modified HardwareSerial.cpp to the repo.
 
Testing at 57600

* 1GB: 110490/111000, 110490/111000
* 8GB: 111000/111000, 111000/111000, 111000/111000
* 16GB: 83890/111000, 84908/111000

The 16GB card with tons of files continue to have problems but the other cards (FAT and FAT32) are acceptable. Whenever possible, use a clean, empty, freshly formatted card.
 
To remove a directory and all its files:

    rm -rf mydirectory
 
Windows 7 stores the Arduino hex file in an awful place. Something like:

    C:\Users\Main\AppData\Local\Temp\build3390340147786739225.tmp\OpenLog_v2.cpp.hex
 
Added HardwareSerial.cpp and a readme to the main trunk.
Added OpenLog_v2.cpp.hex to the main trunk.
 

## [2.3]

27,334 bytes compiled

Migrated to v10.10.10 of sdfatlib. Moved to inline RX interrupt and buffer. Improved the ability to receive a constant stream at 57600bps.
 
Remember - we had to completely butcher HardwareSerial.cpp so a normal Arduino installation will not work. 
C:\arduino-xxxx\hardware\arduino\cores\arduino\HardwareSerial.cpp
 
I removed the receive interupt handler from HardwareSerial.cpp so that we can deal directly with USART_RX_vect in the main code. This allows us to return to the way OpenLog used to operate - by having one buffer split in half. Once one half fills, it is recorded to SD while the other half fills up. This dramatically decreases the time spent in function calls and SD recording, which leads to many fewer characters dropped.

The change log at the top of the main code got so large I've moved it to a version controlled "Changes.txt" file.

By making all these changes, I have broken many things. Ringp - I could use your help here. I apologize for stomping on so much of your work. I was not good enough to figure out how to re-link from the old function calls to the new sdfatlib setup.
 
Backspace is broken - ringp, I saw this fix in one of your commits, but looking at the code, I don't see how it is supposed to work. Either way, we still get 0x08 when trying to backspace.
 
New sdfatlib doesn't have SdFat.cpp so fileInfo doesn't work. These function calls are marked with //Error
 
I have chosen to dis-allow deprecated functions: #define ALLOW_DEPRECATED_FUNCTIONS 0
This forced some trivial changes to the SD write and new file creation function calls. I believe we've successfully migrated to the new version of sdfatlib.
 
In the command_shell / read_line function : It may be better to pull directly from the UART buffer and use the RX interrupt. For now, we brute force it.
 
Because of all these changes, I need to re-test power consumption. For now, I'm confident it's low enough.
 
Testing with 512 buffer array size

* 1GB @ 57600 - dropped very little out of 3 tests
* 1GB @ 115200 - dropped very little out of 2 tests
* 8GB @ 57600 - Formatted using the sd formater (32k byte allocation size). Dropped nothing.
* 8GB @ 115200 - dropped very little, dropped none
* 16GB w/ Lots of files @ 57600 - Drops the first part of the file because of start up delay?
* 16GB w/ Lots of files @ 115200
 
1024 array size (and 800) does not run
 
Testing with 700 buffer array size

* 1GB @ 57600 - 110300 out of 111000 bytes, 110300/111000,
* 1GB @ 115200 - 111000/111000!, 109600/111000
* 8GB @ 57600 - 109000/111000, 111000/111000!,
* 8GB @ 115200 - 111000/111000!, 111000/111000!,
* 16GB w/ Lots of files @ 57600 - 85120/111000, 85120/111000
* 16GB w/ Lots of files @ 115200 - 56420 (but once it got going, log looks good). 56420.
 
I am seeing long delays on cards with lots of files. In the above tests, the 16GB test card is a good example. It has 2GB worth of random files in a sub directory.

After OpenLog boots, goes to '12<'. After I send ~500 characters OpenLog freezes for a couple seconds, then returns to normal, very fast, operation. During that down time, I believe sdfatlib is searching for an open cluster. The odd thing is that after the cluster is established (after the initial down time) OpenLog performs excellently. I am hoping to create a faux file or pre-write and save to the file or some how get this allocation done before we report the '12<' indicating we are ready. That way, if a card does fill up, as long as the host system is checking for '<', it doesn't matter how long it takes sdfatlib to find the next free cluster.
 
You can see that we drop 700 bytes at a time. That's a bit odd - I'd expect to drop half or 350 at a time. What happens if we shrink the array size to say 256? To be expected, this is bad - way more instances of dropped characters.
 
Added blink for each test to the OpenLog_Test sketch so we can see as the test progresses.
 
http://www.sdcard.org/consumers/formatter/ is the site for SD card formatting. It looks like this program takes a guess at the correct block size. This could help a lot in the future.
 

## [2.21]

28,440 bytes compiled

ringp fork brought in. Wildcard remove and list commands now work. Remove directory now works! Change directory up/down the tree works again.
 
ringp brought back many great commands! Thanks ringp!

* rm LOG*.* works
* ls *.TXT works
* cd .. works again
* ls now correctly shows directories first and files following the directories.
 
To remove a directory, you have to navigate into that directory. For example:

    >cd TEMP (you are now in TEMP directory)
    >rm -f TEMP (you are now removing TEMP, and you will be pushed back down one level of the tree)
    >ls (shows files and directories where TEMP directory used to be, but TEMP directory should be gone)
 
ringp added new commands:

* efcount: gives the number of files in the current directory. Example: "efcount" returns "count|3". There are 3 files in the current directory.
* efinfo <spot>: gives info about the file in <spot>. Example: "efinfo 2" reports "LOG00588.TXT|45". File number 2 is named LOG00588.TXT and is 45 bytes in size.
* verbose <"on"|"off">: sets whether command errors are verbose (long winded) or just the "!" character. Example: "verbose off" sets verbose off. Then if a command like "blah" is received, then only "!>" is seen from the command line interface. This makes it easier for embedded systems to recognize there was an error. This setting is not recorded to EEPROM.
 
 
## [2.2]

Modified append_file() to use a single buffer. Increased HardwareSerial.cpp buffer to 512 bytes.
 
More testing at 57600. Record times look to be 2, 5, and zero milliseconds when doing a record. This means that the split buffer doesn't really make a difference? There are some records that take 150ms, 14ms, etc. At 57600bps, that's 7200 bytes/s, 138us per byte. With a 150ms pause, that's 1,086 bytes that need to be buffered while we wait... Grrr. Too many.
 
I re-wrote the append_file function to use only one buffer. This allows us to more quickly pull data from the hardware serial buffer. Hardware serial buffer has to be increased manually to 512. This file (hardwareserial.cpp) is stored in the Arduino directory. With testing, it seems like recording is working more solidly at 57600bps. But now I'm seeing glitches at 19200bps so more testing is required before we make this the official OpenLog release.
 
Moved input_buffer into within the append function. Attempting to shave bytes of RAM.


 
## [2.11]

26,136 bytes compiled out of 30,720

Tested with 16GB microSD. Fixed some general bugs. Lowered power consumption.
 
Fixed issue #30. I added printing a period ('.') for non-visible ASCII characters during a 'read' command. This cleans up the output a bit. HEX printing is still available. 
 
Fixed issue #34. When issuing a long command such as "read log00056.txt 100 200 2" (read from spot 100 to spot 200 and print in HEX), the command shell would die at 24 spots. I increased both the general_buffer and 'buffer' in the command shell from 24 to 30. The limit is now 30 characters, so "read log00056.txt 100 20000 2" is allowed.
 
Works with a 16GB microSD card! High volume test: loaded 16GB card with 5GB of data. Basic serial tests work. When running at 57600, there is an odd delay. I think it has to do with the file system doing an initial scan for an available cluster. Takes 2-3 seconds before OpenLog returns to normal. This can cause significant data loss.
 
Fixing power management in v2. Power down after no characters for 3 seconds now works. Unit drops to 2.35mA in sleep. 7.88mA in sitting  RX mode (awake but not receiving anything). There is still a weird bug where the unit comes on at 30mA. After sleep, it comes back at the correct 7-8mA. Is something not getting shut off?

## [2.1]

26,058 bytes out of 30,720

Power save not working. Fixed issue #35. Dropping characters at 57600bps. 

Fixed a bug found by Salient issue #35. User settings where declared at chars which allowed them to be signed. If a user went from old firmware, to v2, the safety checks would fail because the settings would be read at -1 instead of 255. Declaring user settings as byte fixed issue.
 
Added "a) Clear user settings" to set menu. This allows us to completely wipe an OpenLog (user settings and config file) to see how it will respond to future firmware changes.
 
Improved the file 'size' command.
 
Sequential logging is tested and works.
 
Receive testing: Using the Test_Sketch found on Github, I am testing the receive reliability at different UART speeds.

We need to test a lot of received data. At 57600, 115200, and both from an Arduino (lots of time in between characters becuase of software overhead) and from a raw serial port (almost no time in between characters). I am hoping to make sdfatlib hiccup at 115200, full speed, across a 1MB file. If I can make it fail, then we can start to increase the buffer size and look at how much RAM sdfatlib has left open for the buffer.
 
* 9600bps from Arduino works fine
* 57600bps from Arduino drops characters
* 115200 from Arduino drops characters

It seems that sdfatlib takes longer to write to the SD card than the original file system from Robert Reigel. I'm thinking perhaps we should create a version of OpenLog firmware that is just sequantial logging, no fancy system menus... It might open up some RAM.
 
If only we could get control of the UART from Arduino's clutches, we could probably handle the ring buffer much better. Not sure how to handle UART interrupts without tearing out HardwareSerial.cpp.
 
Added README to the Test sketch. Added 115200bps to test sketch.
 
 
## [2.0]

25,986 bytes out of 30,720

Welcome to version 2! We've moved from Roland Riegel's FAT library to Bill Greiman's sdfatlib. OpenLog now works with SD cards up to 16GB (that is the current largest microSD card we can get our hands on). OpenLog automatically detects and works with FAT16/FAT32 file systems. It also automatically works with normal SD cards as well as SDHC cards.
 
Almost all the previous commands have been ported from v1 to v2. The current commands that do not work:

* cd.. - does not work. You can change to an upper directory, but you cannot navigate back down the tree.
* cat - this command was depricated. HEX printing is now done with the 'read' command. We've added a 5th argument to select between ASCII and HEX printing.
* Wild cards do not yet work. So rm and ls do not have wild cards enabled - yet. Help us out!
 
Porting OpenLog to work directly under Arduino to work with sdfatlib (http://code.google.com/p/sdfatlib/) by Bill Greiman.
 
sdfatlib intrinsically supports FAT16, FAT32 as well as SD and HCSD cards. In a word, it's amazing.
 
Needs to be done:

* (Done) Get config file reading/loading working
* Document config file in wiki: if no config file is found, current system settings are used. If config is found, system switches to settings found in file. If system settings are changed, then config file is changed and system uses new settings immediately.
* (Done) We don't want to constantly record a new config file on each power on. Only record when there is a change.
* Get cd.. working
* Seperate OpenLog_v2 into multiple files
* Re-test too many log files created in the newlog setting - 65535. Potentially create thousands of files and see how sdfatlib handles it.
* (Done) Test sequential logging.
* Get wild card support working for ls and rm
* Get backspace working
* Test rm with directories, and directory wildcards? Maybe not.
* Get power save working
* Test compile on a computer that doesn't have WinAVR
 
Test commands:

* new - works, also in sub dirs
* append - works, also in sub dirs
* rm - works, but does not yet support wild cards.
* md - works, also in sub dirs
* cd - you can change up to a sub-directory, but you cannot navigate back down the tree. The current work around is to type 'init'. This will re-init the card and set the directory back to root.
* ls - works pretty well but does not yet support wild cards. Shows directories, files, and file sizes. Would be cool if it listed dirs first.
* read - works well. Tested 0, 1, 2, 3, 4, and 5 arguments (included and left off). Fails gracefully. Now prints in HEX as well!
* size - works well
* disk - works well, prints more information than ever before!
* init - works well
* sync - works, but not really tested
* cat - I've decided to drop this command. You can now print in hex using the read command and using a 5th argument as '1' for ASCII (default) and '2' for HEX.
* set - works well
* baud - works well
 
  
## [1.61]

Small PCB change. Fixed version # in help menu.
 
Fixed the firmware version in the help menu to v1.61.
 
Updated Eagle files to Eagle v5.9. Fixed weird airwire. Changed D1 LED from Green to Blue. Will only affect new production after 4-28-10.
 
Closed some tickets and wrote some more example Arduino code: http://forum.sparkfun.com/viewtopic.php?t=21438
 
 
## [1.6]

Adding config file.
 
What happens if I reset the system by pulling RX low, but the config file has corrupt values in it?
 
If config file has corrupt values in it, system will default to known values 9600/ctrl+z/3/newlog
 
If config file is empty, system resets to known values
 
After some massive testing, and lots of code to check for illegal states, it looks to be pretty stable. The only problem is that we're running out of RAM. The buffer had to be decreased from 900 bytes to 700 bytes to facilitate all the config file checking. Testing at 57600bps, unit runs very well over 40kb test file on straight RS232 connection. That's pretty good. Testing at 115200 on straight  connection, unit will drop a buffer every once and a while. Not great, but not much we can do if the SD card times out for ~150ms while it's writing. 8 bits to the byte plus a start/stop bit = 10 bits per byte
 
* 9600bps = 960 bytes per second. Buffer will last for 729ms
* 57600bps = 5760 bytes per second. Buffer will last for 121ms
* 115200bps = 11520 bytes per second. Buffer will last for 60.7ms
 
So if the SD card pauses for more than 60ms, 115200 will have lost data, sometimes. All other baud rates should be covered for the most part.
 	
SD cards with larges amounts of data will have increased pause rates. Always use a clean card where possible.
 

## [1.51]

check_emergency_reset, default break character is ctrl+z 3 times, example Arduino sketch
 
Added function from @mungewell - check_emergency_reset. This has improved testing of the RX pin. There was potential to get a false baud reset. There is still a chance but it's now much less likely.
 
If OpenLog is attached to a Arduino, during bootloading of the Arduino, ctrl+z will most likely be sent to the Arduino from the computer. This character will cause OpenLog to drop to command mode - probably not what we want. So I added user selectable character (ctrl+x or '$' for example) and I added user selectable number of escape characters to look for in a row (example is 1 or 2 or 3, '$$$' is a common escape sequence). The default is now ctrl+z sent 3 times in a row.
 
Added an example Arduino sketch (from @ScottH) to GitHub so that people can easily see how to send characters to OpenLog. Not much to it, but it does allow us to test large amounts of text thrown at OpenLog at 57600bps.
 
## [1.5]

Added 4800bps and 19200bps support
 
Added power saving features. Current consumption at 5V is now:

In default append mode: 

* 6.6/5.5mA while receiving characters (LED On/Off)
* 2.1mA during idle

In command mode: 

* 3.2/2.1mA (LED On/Off)
 
So if you're actively throwing characters at the logger, it will be ~6mA. If you send the logger characters then delay 5-10 seconds, current will be ~2.5mA. (Unit records the characters in the buffer and goes into idle more if no characters are received after 5 seconds)
 
These power savings required some significant changes to uart.c / uart_getc()
 
 
## [1.4]

Added exit options to the two menus (set and baud)
Also added display of current settin to two menus (Ex: "Baud currently: 57600bps")
 
Added '!' infront of 'error opening'. This pops up if there is an error while trying to append to a freshly created new log (ex: LOG00548.txt is created, then errors out because it cannot append). '!' was added so that user can parse against it.
 
Replicated logging errors at 57600 using 5V Arduino. Unit would systematically glitch during logging of 111054 bytes.
 
Increasing buffer to 1000 characters caused URU error. URU: Unit Resets Unexpectedly
 
To recreate URU error. Type "append ". Include the space. If you get "!error opening", then things are fine. If you get "!error opening#" where # is a weird character, then type 'ls' and the unit will unexpectedly reset (URU error). I believe this is attributed to a memory overrun somewhere in the FAT system.
 
Changed buffer size to 900 and declared the character buffer as volatile

    #define BUFF_LEN 900
    volatile char input_buffer[BUFF_LEN];
 
This increase to the buffer allows for clean logging of 444055 bytes with no URU errors.
 
Experimenting with Scott's SD cards (customer gave cards on loan for recreating logging errors):

* Card with single ~740mb file produces errors when trying to open/append to new log. 
* Card with less stuff on it logs full 444055 bytes correctly.
 
 
## [1.3]

Added sd_raw_sync() inside append_file. I believe this was why tz1's addition of the timeout buffer update feature was not working. Auto buffer update now working. So if you don't send anything to OpenLog for 5 seconds, the buffer will automatically record/update.
 
Need to create 'Testing' page to outline the battery of tests we need to throw at any OpenLog after a firmware submission and update is complete.
 
Testing create 40,000 logs
 
Record at full speed: Run at full 115200, load lotsoftext.txt and verify no characters are dropped.
 
Detect too many logs: Create new log at 65533 (do this by editing 'zero' function to set EEPROM to 0xFF and oxFD) and power cycle. Verify unit starts new log. Power cycle and verify unit errors out and drops to command prompt.
 
Record buffer after timeout: Create new log. Type 20 characters and wait 5 seconds. Unit should auto-record buffer. Power down unit. Power up unit and verify LOG has 20 characters recorded.	
 
 
## [1.2]

@ringp added:

* Adding support for splitting command line parameters into arguments
* Adding support for reading files sequencially
** read <filename> <start> <length>
* New log now for sequencial log functions supports 0-65535 files
* Adding support for wildcard listing or deletion of files
** ls <wildcard search>
** rm <wildcard delete>
 
Really great additions. Thanks ringp!
 
Nate added error testing within newlog(). Checks to see if we have 65534 logs. If so, error out to command prompt with "!Too many logs:1"
 
## [1.1]

Adding better defines for EEPROM settings
Adding new log and sequential log functions.
 
Code is acting very weird with what looks to be stack crashes. I can get around this by turning the optimizer off ('0'). Found an error : EEPROM functions fail when optimizer is set to '0' or '1'. sd-reader_config.h contains flag for USE_DYNAMIC_MEMORY
 
Looks like tweaking the optimization higher causes the asm("nop"); to fail inside the append_file routine. Changing this to delay_us(1); works.
 
I have a sneaking suspicion that I was having buffer overrun problems when defining the input_buffer at 1024 bytes. The ATmega328 should have enough RAM (2K) but weird reset errors were occuring. With the buffer at 512bytes, append_file runs just fine and is able to log at 115200 at a constant data rate.
 
Added file called 'lots-o-text.txt' to version control. This text contains easy to scan text to be used for full data rate checking and testing.
 
## [1.0] December 3 2009

Original version written in C.
 
 