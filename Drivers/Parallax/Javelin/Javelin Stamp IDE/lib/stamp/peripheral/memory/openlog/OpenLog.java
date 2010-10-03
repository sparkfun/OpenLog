package stamp.peripheral.memory.openlog;
import stamp.core.*;
import stamp.math.*;

/**
 *************************************************************************
 *    Thanks SparkFun for creating an OpenSource project like the OpenLog!
 *
 *    SparkFun homepage: www.sparkfun.com
 *    Product homepage: http://www.sparkfun.com/commerce/product_info.php?products_id=9530
 *    OpenLog homepage: http://wiki.github.com/nseidle/OpenLog/
 *************************************************************************
 *
 * This class provides support to access SparkFun OpenLog unit
 * NOTE! The driver version (1.x) only supports ASCII files; no binary files
 *
 * How to use this class:
 *
 * OpenLog openLog = new OpenLog([check out constructor parms]);
 * if (!openLog.cardRestart()) {
 *   System.out.println("*** Error: Card cannot be initialized *** ");
 *   System.out.println("Stopping...");
 *   while (true) {CPU.delay(10);}
 * }
 *
 * 1. FileInfo fileInfo;
 *    if(openLog.listDirectoryStart()) {
 *      int count = openLog.listDirectoryCount();
 *      while ((fileInfo = openLog.listDirectoryNextEntry()) != null) {
 *        <do something with fileInfo>
 *      }
 *      openLog.listDirectoryEnd();
 *    }
 * 2. if (openLog.openFile(<file name>)) {
 * 2.1    openLog.setFilePosition(<file position>);
 *      while (openLog.readFile(<buffer>, <size>){
 *        <do something with the file data>
 *      }
 *      openLog.closeFile();
 *    }
 * 3. openLog.writeFile(<file name>, <file data>, <append>);
 * 4. openLog.deleteFile(<file name>);
 * 5. if (openLog.openLogRestart()) {
 *      openLog.openLogInit();
 *    }
 * 6. FileInfo fileInfo = openLog.fileSize(<file name>)
 *    if (fileInfo != null) {
 *      <do something with the file>
 *    }
 * 7. if (openLog.createDir(<folder name>)) {
 *      if (openLog.changeDir()) {
 *        <do something here>
 *      }
 *    }
 * 8. openLog.prevDir();
 * 9. openLog.openLogSync();
 *
 * Explanation:
 *
 * 1.   List the files in the directory. Use always listDirectoryStart() to start a directory listing. To end it, use listDirectoryEnd().
 *      Some file operations, like deleteFile() or writeFile() might invalidate the directory listing. For every directory entry use
 *      listDirectoryNextEntry() and retrieve the file name and size. listDirectoryCount() will return the number of files in
 *      current directory.
 *
 * 2.   Open a file. Returns value true/false. Read the file contents into buffer <buffer>. Please note that
 *      since you are only using 2 pins (no handshaking) to connect the Javelin to OpenLog you will have to think
 *      that the UART buffer is 256 bytes (that according to the Javelin Stamp documentation version 1.0, page 188).
 *      Do not try to read more than that or you will most likely loose data. My suggestion is that you start with
 *      a small value, e.g. 50 bytes and increase this after your needs.
 *
 * 2.1  Set the file position. When the file is opened the file position is set to "0" as default so in the normal use case if
 *      your intention is to start reading from position "0" you can omit this. Feel free to change it if you want
 *      to for example start reading from another position in the file.
 *
 * 3.   Write data to a file. Use <append> if you want to append your data <file data> to an already
 *      existing file or replace the file contents with <file data>.
 *
 * 4.   Delete file <file name>. Returns true/false.
 *
 * 5.   This function will trigger a OpenLog hardware reset Please note that after you are calling this function you *must*
 *      also call openLogInit() in order to set some parameters used internally in the OpenLog firmware.
 *
 * 6.   Returns the corresponding FileInfo class containing file information for <file name> like the file size.
 *
 * 7.   Create a directory and enter that directory. Please note that this will invalidate any directory listing or open file
 *      handle.
 *
 * 8.   This function is performing a "cd .." command on the OpenLog. You will have to keep track yourself of where in the
 *      directory tree your are at the moment. This interface does not provide any "pwd" (print working directory) function.
 *
 * 9.   This function is calling the "sync" function in the OpenLog firmware to do a sync of the buffered data to the SD card
 *      You can read more about why OpenLog provides a "sync" function in the OpenLog documentation. What you have to remember is
 *      that OpenLog is using a buffer when writing data to the SD card. When the buffer is full (the 512 bytes) the data is
 *      written to the SD card. If you want to force the OpenLog to write whatever data it has at the moment then use this
 *      function.
 *
 * @author Paul Ring - hexor@coolbox.se
 * @version 1.1 - June, 2010
 * @version 1.2 - October, 2010
 */

public class OpenLog {
  private static final int MAX_COUNT_SPLIT_STRINGS = 5;

  // Contains file information that will be return when calling getFileInfo() function
  public class FileInfo {
    public StringBuffer fileName = new StringBuffer(32);    // File name
    public Int32 fileSize = new Int32(0,0);                 // File size
    public Int32 filePosition = new Int32(0,0);             // File position
  }

  private class StringSplit {
    public int startIndex;      // Points to first character in command line argument
    public int endIndex;        // Pointes to the last character in command line argument

    public StringSplit() {
      reset();
    }
    public void reset() {
      startIndex = -1;
      endIndex = -1;
    }
  }

  private StringSplit[] stringSplit;                // Used for string parsing purposes
  private Uart txUart, rxUart;                      // Uart references
  private StringBuffer tempBuffer;                  // Buffer reference
  private Timer timer;                              // Timer reference
  private int resetPin;                             // Hardware pin to which the OpenLog reset pin is connected
  private boolean fileIsOpen = false;               // Keeps track if the file is opened or not
  private final static char shellReady = '>';
  private final static char shellReceive = '<';
  private final static char LF = '\n';              // New line
  private final static char CR = '\r';              // Carriage return
  private static final char EscChar = (char)0x1A;   // Default Ctrl-Z

  private boolean dirListingStarted = false;  // Used to indicate that a directory listning is started
  private int dirCountFiles = -1;             // Number of files in current directory
  private int dirListingIndex = -1;           // Current file index
  private StringBuffer opBuffer = new StringBuffer(32);

  private FileInfo fileInfo = new FileInfo();

  public OpenLog(Uart txUart,
                 Uart rxUart,
                 int resetPin,
                 StringBuffer buffer,
                 Timer timer) {
    this.txUart = txUart;
    this.rxUart = rxUart;
    this.tempBuffer = buffer;
    this.timer = timer;
    this.resetPin = resetPin;
    stringSplit = new StringSplit[MAX_COUNT_SPLIT_STRINGS];
    for (int i = 0; i < MAX_COUNT_SPLIT_STRINGS; i++) {
      stringSplit[i] = new StringSplit();
    }
  }

  private int countSplitStrings()
  {
    int count = 0;
    for(int i = 0; i < MAX_COUNT_SPLIT_STRINGS; i++)
      if((stringSplit[i].startIndex > -1) && (stringSplit[i].endIndex > stringSplit[i].startIndex))
        count++;

    return count;
  }

  private void addSplitString(int index, int buffer_length)
  {
    int count = countSplitStrings();
    if (count < MAX_COUNT_SPLIT_STRINGS)
    {
      stringSplit[count].startIndex = index;
      stringSplit[count].endIndex = index + buffer_length;
    }
  }

  private int splitString(char[] buffer, char split, int length)
  {
    int splitIndexStart = 0;
    int splitIndexEnd = 1;

    //Reset command line arguments
    for (int i = 0; i < MAX_COUNT_SPLIT_STRINGS; i++) {
      stringSplit[i].reset();
    }

    //Split the command line arguments
    while (splitIndexEnd < length)
    {
      // Search for the split character
      if ((buffer[splitIndexEnd] == split) || (splitIndexEnd + 1 == length))
      {
        // Fix for last character
        if (splitIndexEnd + 1 == length)
          splitIndexEnd = length;

        // Add this command line argument to the list
        addSplitString(splitIndexStart, (splitIndexEnd - splitIndexStart));
        splitIndexStart = ++splitIndexEnd;
      } else {
        splitIndexEnd++;
      }
    }

    //Return the number of available command line arguments
    return countSplitStrings();
  }

  private boolean escapeCharAck() {
    // According to the documentation we have to send
    // the escape character several times (default is 3) in order to abort if in
    // logging mode
    int i = 0;
    for (i = 0; i < 10; i++) {
      txUart.sendByte(EscChar);
      CPU.delay(500);
    }

    for (i = 0; i < 10; i++) {
      txUart.sendByte(CR);
      CPU.delay(500);
      while (rxUart.byteAvailable()) {
        if ((char)rxUart.receiveByte() == shellReady ) {return true;}
      }
    }

    return false;
  }

  private void dirInfoInvalid() {
    dirListingStarted = false;
    dirCountFiles = -1;
    dirListingIndex = -1;
  }

  private boolean fwrite(String fileName,
                         StringBuffer fileData,
                         boolean appendMode) {

    // In case of calling this function with append==true but the file is not created
    // make sure the file is created first using "write" or the OpenLog will return an
    // error if trying to append to a non existing file
    boolean createFile = true;

    FileInfo fileInfo = fileSize(fileName);
    if (fileInfo != null) {
      Int32 size = fileInfo.fileSize;
      if (size.compare(0) >= 0) {
        createFile = false;
      }
    }

    // Create the file
    if (createFile) {
      opBuffer.clear();
      opBuffer.append("new ");
      opBuffer.append(fileName);
      // Send the command to create a new file
      if (!openLogExecute(opBuffer.toString(), null, shellReady))
        return false;
      // The file should exist now
      if (fileSize(fileName) == null)
        return false;
    }
    // Either we want to append or to write to the file
    // Writing to the always starts at position 0 which will
    // replace any data in the file located at position 0-fileData.length()
    opBuffer.clear();
    opBuffer.append(appendMode ? "append " : "write ");
    opBuffer.append(fileName);
    if (!openLogExecute(opBuffer.toString(), null, shellReceive)) {
      return false;
    }

    txUart.sendString(fileData.toString());
    delay(appendMode ? 0 : 1); // Should be enough for most cases

    if (appendMode) {
      // Send Ctrl+Z to escape from append mode
      escapeCharAck();
    } else {
      // Send a single CR ('\r') to exit from write mode
      txUart.sendByte(CR);
    }
    // We should be alive now
    return openLogAlive();
  }

  private boolean openLogExecute(String command, StringBuffer reply, char promptChar) {

    boolean retValue = false;
    StringBuffer buff = tempBuffer;
    if (reply != null)
      buff = reply;

    // Clear the UART
    while (rxUart.byteAvailable())
      rxUart.receiveByte();

    // Send message
    buff.clear();
    buff.append(command);
    buff.append(CR);
    txUart.sendString(buff.toString());

    buff.clear();

    int count = 0;
    int capacity = buff.capacity();
    opBuffer.clear();

    // Give the module some time to respond
    // Collect the return value and the result
    timer.mark();
    while (!timer.timeout(10000) && !rxUart.byteAvailable()) {CPU.delay(10);}
    while (true) {
      char ch = 0;
      if (rxUart.byteAvailable())
        ch = (char)rxUart.receiveByte();

      if ((ch != (char)EscChar) && ((ch != 0) && (ch != -1)) && (count < capacity)) {
        buff.append(ch);
        count++;
      } else if (ch == (char)EscChar) {
        // Since we found the escape character here, we always know that
        // after the escape characters the result is sent to us
        count = 0;
        capacity = opBuffer.capacity();
        timer.mark();

        while (!timer.timeout(1500) && !rxUart.byteAvailable()) {CPU.delay(10);}
        while (rxUart.byteAvailable()) {
          ch = (char)rxUart.receiveByte();
          if (count < capacity) {
            opBuffer.append(ch);
            count++;
          } else { break; }
        }
        break;
      }
    }

    if ((count = opBuffer.length()) > 0) {
      for (int i = 0; i < count; i++) {
        if (opBuffer.charAt(i) == '!') {
          return false;
        }
      }

      if (opBuffer.charAt(count-1) == promptChar) {
        retValue = true;
      }
    }

    // If we do not get the expected prompt character then it's a failure
    return retValue;
  }

  public static boolean isNumeric(char ch) {
    return ((ch >= '0') && (ch <= '9') || (ch == '-') || (ch == '+'));
  }

  private void delay(int delay) {
    for (int i = 0; i <= delay; i++)
      CPU.delay(2500); // 2500 * 95.48us * i
  }

  private boolean listDirStart() {

    // Calling the "dirInfoInvalid" function will invalidate
    // the directory information and set the dirCountFiles to "-1"
    dirInfoInvalid();

    if (openLogExecute("efcount", null, shellReady)) {
      char[] ch = tempBuffer.toString().toCharArray();
      if (splitString(ch, '|', tempBuffer.length()) == 2) {
          try {
            // Add the characters
            opBuffer.clear();
            for (int i = stringSplit[1].startIndex; i < stringSplit[1].endIndex; i++) {
              if (!isNumeric(ch[i])) {break;} // enough here, we are not execpting anything more
              opBuffer.append(ch[i]);
            }

            dirCountFiles = Integer.parseInt(opBuffer.toString());
          } catch (Exception ex) {}
        }
      }
    boolean ret;
    if (ret = (dirCountFiles >= 0)) {
      dirListingIndex = 0; // Set index 0 as first index
    }
    return ret;
  }

  public int listDirectoryCount() {
    if (!dirListingStarted) {return -1;}
    return dirCountFiles;
  }

  public FileInfo listDirectoryNextEntry() {
    if (!dirListingStarted) {return null;}

    FileInfo fInfo = null;

    if (dirListingIndex < dirCountFiles) {
      opBuffer.clear();
      opBuffer.append("efinfo ");
      opBuffer.append(dirListingIndex++);

      if (openLogExecute(opBuffer.toString(), null, shellReady)) {
        char[] ch = tempBuffer.toString().toCharArray();
        if (splitString(ch, '|', tempBuffer.length()) == 2) {
          try {
            // Format: <name>|<size>

            // Add the file name
            fileInfo.fileName.clear();
            for (int i = stringSplit[0].startIndex; i < stringSplit[0].endIndex; i++)
              fileInfo.fileName.append(ch[i]);

            // Add the file size
            opBuffer.clear();
            for (int i = stringSplit[1].startIndex; i < stringSplit[1].endIndex; i++) {
              if (!isNumeric(ch[i])) {break;} // enough here, we are not excepting anything more
              opBuffer.append(ch[i]);
            }

            fileInfo.fileSize.set(opBuffer.toString());
            fInfo = fileInfo;

          } catch (Exception ex) {}
        }
      }
    }

    return fInfo;
  }

  public void listDirectoryEnd() {
    dirInfoInvalid();
  }

  public boolean listDirectoryStart() {

    // Cannot start a new directory listing while we have one active
    // Use listDirectoryEnd() before calling this function
    if (dirListingStarted) {return false;}

    int openLogInit = 0;
    while (!(dirListingStarted = listDirStart())) {
      if (openLogInit++ > 2) { break; };  // Made 2 tries and still the problem is not fixed
    }

    return dirListingStarted;
  }

  public boolean deleteFile(String fileName) {
    if (!openLogAlive() || fileName.length() == 0) { return false; }

    opBuffer.clear();
    opBuffer.append("rm ");
    opBuffer.append(fileName);

    return (openLogExecute(opBuffer.toString(), null, shellReady));
  }

  public void closeFile() {
    fileIsOpen = false;
  }

  public boolean openFile(String fileName) {
    // Cannot open another file
    if (fileIsOpen) { return false; }

    int openLogInit = 0;
    while (!(fileIsOpen = (fileSize(fileName) != null))) {
      if (openLogInit++ > 2) { break; }; // Made 2 tries and still the problem is not fixed
    }

    // This function will handle low level file commands
    return (fileIsOpen);
  }

  public boolean setFilePosition(int filePosition) {
    if (!fileIsOpen) { return false; }
    if (fileInfo.fileSize.compare(filePosition) >= 0) {
      fileInfo.filePosition.set(filePosition);
      return true;
    }
    return false;
  }

  public boolean readFile(StringBuffer buffer,
                          int length) {

    // File is not opened, at the end of the file or the length error
    if (!fileIsOpen || (fileInfo.filePosition.compare(fileInfo.fileSize) >= 0)) {
      return false;
    }

    buffer.clear();

    opBuffer.clear();
    opBuffer.append("read ");                           // Read command
    opBuffer.append(fileInfo.fileName.toString());      // File name

    opBuffer.append(" ");
    opBuffer.append(fileInfo.filePosition.toString());  // File position
    opBuffer.append(" ");
    opBuffer.append(length);                            // Length to read
    fileInfo.filePosition.add(length);                  // Keep track of the file position

    // Get the data from the OpenLog
    return (openLogExecute(opBuffer.toString(), buffer, shellReady));
  }

  public boolean writeFile(String fileName,
                           StringBuffer fileData,
                           boolean append) {

    // Basic checking - the file should not be open when we call this function
    if ((fileName.length() == 0) || (fileData.length() == 0)) { return false; }
    boolean success = false;

    int openLogInit = 0;
    while (!(success = fwrite(fileName, fileData, append))) {
      CPU.delay(5000);
      if (openLogInit++ > 1) { break; }; // Made 2 tries and still the problem is not fixed
    }

    return success;
  }

  public boolean createDir(String dirName) {
    if (!openLogAlive() || dirName.length() == 0) { return false; };
    dirInfoInvalid();

    // Create the directory
    opBuffer.clear();
    opBuffer.append("md ");
    opBuffer.append(dirName);

    return (openLogExecute(opBuffer.toString(), null, shellReady));
  }
  public boolean prevDir() {
    if (!openLogAlive()) { return false; }
    dirInfoInvalid();

    // Do a "cd .."
    opBuffer.clear();
    opBuffer.append("cd ..");

    boolean result = false;
    if (result = openLogExecute(opBuffer.toString(), null, shellReady)) {
      fileIsOpen = false;
    }

    return result;
  }

  public boolean changeDir(String dirName) {
    if (!openLogAlive() || dirName.length() == 0) { return false; }
    dirInfoInvalid();

    // Change dir
    opBuffer.clear();
    opBuffer.append("cd ");
    opBuffer.append(dirName);

    boolean result = false;
    if (result = openLogExecute(opBuffer.toString(), null, shellReady)) {
      fileIsOpen = false;
    }

    return result;
  }

  public boolean openLogSync() {
    if (!openLogAlive()) { return false; }

    opBuffer.clear();
    opBuffer.append("sync");

    boolean result = false;
    if (result = openLogExecute(opBuffer.toString(), null, shellReady)) {
      fileIsOpen = false;
    }

    return result;
  }

  public FileInfo fileSize(String fileName) {
    if (!openLogAlive() || fileName.length() == 0) { return null; }

    opBuffer.clear();
    opBuffer.append("size ");
    opBuffer.append(fileName);
    if (!openLogExecute(opBuffer.toString(), null, shellReady)) {
      return null;
    }

    int length = tempBuffer.length();
    opBuffer.clear();
    for (int i = 0; i < length; i++) {
      char ch = tempBuffer.charAt(i);
      if ((ch == CR) || (ch == LF)) // Skip '\n' and '\r'
        continue;
      if (!isNumeric(ch)) { // enough here, we are not excepting anything more
        break;
      }
      opBuffer.append(ch);
    }
    // We got an answer here, check the value
    if (opBuffer.length() > 0) {
      if (!fileInfo.fileName.equals(fileName)) {  // File name
        fileInfo.fileName.clear();
        fileInfo.fileName.append(fileName);
      }
      fileInfo.fileSize.set(opBuffer.toString()); // File size
      fileInfo.filePosition.set(0,0);             // File position
      return fileInfo;
     }
    return null;
  }

  public boolean openLogRestart() {
    // Hardware reset according to OpenLog documentation
    CPU.writePin(resetPin, false);
    delay(0);
    CPU.writePin(resetPin, true);
    delay(6); // Give the OpenLog some time to initialize itself
    //CPU.writePin(this.resetPin, false);
    //CPU.delay(1000);
    //CPU.writePin(this.resetPin, true);

    // Send escape char in case it enters append mode
    escapeCharAck();

    // Enable the embedded mode
    opBuffer.clear();
    opBuffer.append("eem on");
    if (!openLogExecute(opBuffer.toString(), null, shellReady))
      return false;

    opBuffer.clear();
    return openLogAlive();
  }

  private boolean openLogAlive() {
    // Sending an empty string to the OpenLog will force it to
    // respond with the shell in which case we know that it's alive
    opBuffer.clear();
    return (openLogExecute(opBuffer.toString(), null, shellReady));
  }

  public boolean openLogInit() {

    // Do no echo
    opBuffer.clear();
    opBuffer.append("echo off");
    if (!openLogExecute(opBuffer.toString(), null, shellReady))
      return false;

    // Do not show extended error information
    opBuffer.clear();
    opBuffer.append("verbose off");
    if (!openLogExecute(opBuffer.toString(), null, shellReady))
      return false;

    // Turn off the binary mode
//    opBuffer.clear();
//    opBuffer.append("binary off");
//    if (!openLogExecute(opBuffer.toString(), null, shellReady))
//      return false;

    return true;
  }
}