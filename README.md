SparkFun OpenLog
================

<table class="table table-hover table-striped table-bordered">
  <tr align="center">
   <td><a href="https://www.sparkfun.com/products/13712"><img src="https://cdn.sparkfun.com//assets/parts/1/1/2/0/2/13712-SparkFun_OpenLog-01.jpg" alt="SparkFun OpenLog"></a></td>
   <td><a href="https://www.sparkfun.com/products/13955"><img src="https://cdn.sparkfun.com//assets/parts/1/1/6/6/3/13955-SparkFun_OpenLog_with_headers-01.jpg" alt="SparkFun OpenLog with Headers"></a></td>
  </tr>
  <tr align="center">
    <td><i>SparkFun OpenLog (<a href="https://www.sparkfun.com/products/13712">DEV-13712</a>)</i></td>
    <td><i>SparkFun OpenLog with Headers (<a href="https://www.sparkfun.com/products/13955">DEV-13955</a>)</i></td>
  </tr>
</table>

OpenLog is an open source data logger that works over a simple serial connection and supports microSD cards up to 64GB. 

Repository Contents
-------------------
* **/Documentation** - Data sheets, additional product information
* **/Firmware** - Example sketches for the OpenLog, and for an Arduino connected to the OpenLog.
* **/Hardware** - Hardware design files for the OpenLog PCB. These files were designed in Eagle CAD.
* **/Libraries** - Libraries for use with the OpenLog.
* **/Production** - Production panel files (.brd)

Documentation
--------------
* **[Hookup Guide](https://learn.sparkfun.com/tutorials/openlog-hookup-guide)** - Basic hookup guide for the OpenLog.
* **[SparkFun Fritzing repo](https://github.com/sparkfun/Fritzing_Parts)** - Fritzing diagrams for SparkFun products.
* **[SparkFun 3D Model repo](https://github.com/sparkfun/3D_Models)** - 3D models of SparkFun products. 
* **[SparkFun Graphical Datasheets](https://github.com/sparkfun/Graphical_Datasheets)** -Graphical Datasheets for various SparkFun products.


License Information
-------------------

This product is _**open source**_! 

Please review the LICENSE.md file for license information. 

If you have any questions or concerns on licensing, please contact techsupport@sparkfun.com.

Distributed as-is; no warranty is given.

- Your friends at SparkFun.

_SDFatLib-beta and SerialPort are written by Bill Greiman, and released under GPLv3._

Version History
---------------

For a full view of changes please see the [changelog](https://github.com/sparkfun/OpenLog/blob/master/CHANGELOG.md). 

OpenLog v4 is refactored with better RAM utilization for better performance at higher record speeds (115200/57600).

OpenLog v3 is stable, supports FAT32 cards up to 64GB and supports higher record speeds (115200/57600).

OpenLog v2 is a bit buggy but supports FAT32 and SD cards up to 16GB.  

OpenLog v1 is stable but only supports FAT16 and up to 2GB.  

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
* **v3.3**  Added ability to ignore escape character checking and corrected incremental log naming.
* **v4.0**  Re-worked to be compatible with Arduino v1.6.x. Freed RAM to increase RX buffer size.
