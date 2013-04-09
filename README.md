OpenLog
=======

[![OpenLog](https://dlnmh9ip6v2uc.cloudfront.net/images/products/9/5/3/0/09530-01_i_ma.jpg)  
*OpenLog (DEV-09530)*](https://www.sparkfun.com/products/9530)

OpenLog is an open source project to create a simple to use data logger. 

OpenLog v1 is stable but only supports FAT16 and up to 2GB.  
OpenLog v2 is a bit buggy but supports FAT32 and SD cards up to 16GB.  
OpenLog v3 is stable, supports FAT32 and supports higher record speeds (115200/57600).

License Information
-------------------

The hardware design and firmware are released under [Creative Commons Share-alike 3.0](http://creativecommons.org/licenses/by-sa/3.0/).  
The FAT16/FAT32 firmware was originally designed by Roland Riegel and is released under [GPL v2](http://www.gnu.org/licenses/gpl-2.0.html).  
OpenLog v2.0 and above uses sdfatlib written by Bill Greiman and is released under [GPL v3](http://www.gnu.org/licenses/gpl-3.0.html).  
The OpenLog firmware was created by SparkFun Electronics.

Repository Contents
-------------------

* **/hardware** - Hardware design files for the OpenLog PCB. These files were designed in Eagle CAD.
* **/firmware** 
    * OpenLog_v3 - Firmware that ships with OpenLog
    * OpenLog_v3_Light - Alternative version to allow for larger buffers
    * Examples - Example Arduino code for controlling and testing OpenLog

Version History
---------------

* **v1.0**  Buggy initial release
* **v1.1**  Small changes to system settings and EEPROM storage.
* **v1.2**  Added wild card to listing and remove commands. Added read file command.
* **v1.3**  Added auto buffer record if unit sits idle for more than 5 seconds.
* **v1.4**  Increase buffer size to 900 bytes. Pinning down URU errors.
* **v1.5**  Lowered power consumption to ~2mA avg. Added 4800 and 19200 baud rates.
* **v1.51** Added configurable escape character, and escape character amount.
* **v1.6**  Added ability to configure via config.txt file.
* **v2.0**  Massive overhaul. Ported to sdfatlib. Now supports FAT16/FAT32/SD/SDHC.
* **v2.1**  Power save not working. Fixed issue 35. Dropping characters at 57600bps.
* **v2.11** Tested with 16GB microSD. Fixed issues 30 & 34. Re-enable power save.
* **v2.2**  Modified append_file() to use a single buffer. Increased HardwareSerial.cpp buffer to 512 bytes.
* **v2.21** ringp fork brought in. rm dir, cd .., and wildcards now work!
* **v2.3**  Migrated to v10.10.10 of sdfatlib. Moved to inline RX interrupt and buffer.
* **v2.4**  Merged ringp updates. Commands cd, rm, ls work again!
* **v2.41** Power loss bug fixed. Adding support for 38400bps for testing with SparkFum 9DOF IMU logging. 
* **v2.5**  Added software reset command. Modified the read command to print extended ASCII characters.
* **v2.51** Changed command prompt control to ignore \n for easier control from microcontroller.
* **v3.0**  Migration to Arduino v1.0 and better recording speed at 115200bps and 57600bps.
* **v3.1**  Better handling of recording during power loss.
* **v3.2**  Freed up RAM for larger RX ring buffer. Added support for wildcards and ability to ignore emergency override.