@echo off
rem This batch file does not set the fuse bits. We assume you are updating a pre-existing OpenLog

if [%1]==[] goto usage

@echo Programming the SparkFun OpenLog
@pause
:loop

@echo -
@echo Programming binary: %1 on %2

@avrdude -Cavrdude.conf -v -V -patmega328p -carduino -P %2 -b115200 -D -Uflash:w:%1:i 

@echo Done programming! Ready for next board.
@pause

goto loop

:usage
@echo Missing the binary file and com port arguments. Ex: batch_program.bat OpenLog_v43.hex COM3
