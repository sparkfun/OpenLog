OpenLog Firmware
=================

* OpenLog - Firmware that ships with OpenLog. '?' command will show the version loaded onto a unit.
* OpenLog_Light - Used for high-speed logging. By removing the menu and command mode the receive buffer is increased.
* OpenLog_Minimal - Highest speed logging. Baud rate must be set in code and uploaded. Hardest, most advanced, and best at high-speed logging.
* Examples - Arduino examples for controlling and testing OpenLog
    * Benchmarking - Used to test OpenLog. Sends large amounts of data at 115200bps over multiple files.
    * CommandTest - Example of how to create and append a file via command line control.
    * ReadExample - Example of how to control OpenLog via command line.
    * ReadExample_LargeFile - Example of how to open a large file stored on OpenLog and report it over a local bluetooth connection.
    * Test_Sketch - Used to test OpenLog with lots of serial data.
    * Test_Sketch_Binary - Used to test OpenLog with binary data and escape characters.

