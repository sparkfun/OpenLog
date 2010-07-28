/* Arduino SdFat Library
 * Copyright (C) 2009 by William Greiman
 *  
 * This file is part of the Arduino SdFat Library
 *  
 * This Library is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with the Arduino SdFat Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef SdFat_h
#define SdFat_h
/**
 * \file
 * SdFile and SdVolume classes
 */
/** Arduino Print support if not zero */
#define SD_FAT_PRINT_SUPPORT 1

#include <avr/pgmspace.h>
#include "Sd2Card.h"
#include "FatStructs.h"

#if SD_FAT_PRINT_SUPPORT
#include "Print.h"
#endif // SD_FAT_PRINT_SUPPORT

// forward declaration since SdVolume is used in SdFile
class SdVolume;
//==============================================================================
// SdFile class

// flags for ls()
/** ls() flag to print modify date */
#define LS_DATE 1
/** ls() flag to print file size */
#define LS_SIZE 2
/** ls() flag for recursive list of subdirectories */
#define LS_R 4

// use the gnu style oflag in open()
/** open() oflag for reading */
#define O_READ    0X01
/** open() oflag - same as O_READ */
#define O_RDONLY  O_READ
/** open() oflag for write */
#define O_WRITE   0X02
/** open() oflag - same as O_WRITE */
#define O_WRONLY  O_WRITE
/** open() oflag for reading and writing */
#define O_RDWR    (O_READ | O_WRITE)
/** open() oflag mask for access modes */
#define O_ACCMODE (O_READ | O_WRITE)
/** The file offset shall be set to the end of the file prior to each write. */
#define O_APPEND  0X04
/** synchronous writes - call sync() after each write */
#define O_SYNC    0X08
/** create the file if nonexistent */
#define O_CREAT   0X10
/** If O_CREAT and O_EXCL are set, open() shall fail if the file exists */
#define O_EXCL    0X20
/** truncate the file to zero length */
#define O_TRUNC   0X40

// flags for timestamp
/** set the file's last access date */
#define T_ACCESS 1
/** set the file's creation date and time */
#define T_CREATE 2
/** Set the file's write date and time */
#define T_WRITE 4

/** date field for FAT directory entry */
#define FAT_DATE(year, month, day)\
        ((year) - 1980) << 9 | (month) << 5 | (day)
/** year part of FAT directory date field */
#define FAT_YEAR(fatDate) 1980 + ((fatDate) >> 9)
/** month part of FAT directory date field */
#define FAT_MONTH(fatDate) ((fatDate) >> 5) & 0XF
/** day part of FAT directory date field */
#define FAT_DAY(fatDate) (fatDate) & 0X1F

/** time field for FAT directory entry */
#define FAT_TIME(hour, minute, second)\
        (hour) << 11 | (minute) << 5 | (second) >> 1
/** hour part of FAT directory time field */
#define FAT_HOUR(fatTime) (fatTime) >> 11
/** minute part of FAT directory time field */
#define FAT_MINUTE(fatTime) ((fatTime) >> 5) & 0X3F
/** second part of FAT directory time field */
#define FAT_SECOND(fatTime) 2*((fatTime) & 0X1F)

/** Default date for file timestamps is 1 Jan 2000 */
#define FAT_DEFAULT_DATE  FAT_DATE(2000, 1, 1)
/** Default time for file timestamp is midnight */
#define FAT_DEFAULT_TIME  FAT_TIME(0, 0, 0)

//------------------------------------------------------------------------------
/**
 * \class SdFile
 * \brief Access FAT16 and FAT32 files on SD and SDHC cards.
 */
#if SD_FAT_PRINT_SUPPORT
class SdFile : public Print {
#else //SD_FAT_PRINT_SUPPORT
class SdFile {
#endif //SD_FAT_PRINT_SUPPORT

// bits defined in flags_
#define F_OFLAG (O_ACCMODE | O_APPEND | O_SYNC ) // should be 0XF
#define F_UNUSED 0X30  // available bits
#define F_FILE_UNBUFFERED_READ 0X40 // use unbuffered SD read
#define F_FILE_DIR_DIRTY 0X80 // sync of directory entry required

// make sure F_OFLAG is ok
#if ((F_UNUSED | F_FILE_UNBUFFERED_READ | F_FILE_DIR_DIRTY) & F_OFLAG)
#error flags_ bits conflict
#endif // flags_ bits

// values for type_
/** This SdFile has not been opened. */
#define FAT_FILE_TYPE_CLOSED 0
/** SdFile for a file */
#define FAT_FILE_TYPE_NORMAL 1
/** SdFile for a FAT16 root directory */
#define FAT_FILE_TYPE_ROOT16 2
/** SdFile for a FAT32 root directory */
#define FAT_FILE_TYPE_ROOT32 3
/** SdFile for a subdirectory */
#define FAT_FILE_TYPE_SUBDIR 4
/** Test value for directory type */
#define FAT_FILE_TYPE_MIN_DIR FAT_FILE_TYPE_ROOT16

  // private data
  uint8_t flags_;         // See above for definition of flags_ bits
  uint8_t type_;          // type of file see above for values
  uint32_t curCluster_;   // cluster for current file position
  uint32_t curPosition_;  // current file position in bytes from beginning
  uint32_t dirBlock_;     // SD block that contains directory entry for file
  uint8_t  dirIndex_;     // index of entry in dirBlock 0 <= dirIndex_ <= 0XF
  uint32_t fileSize_;     // file size in bytes
  uint32_t firstCluster_; // first cluster of file
  SdVolume *vol_;         // volume where file is located
  
  // private functions
  uint8_t addCluster(void);
  uint8_t addDirCluster(void);
  dir_t *cacheDirEntry(uint8_t action = 0);
  static void (*dateTime_) (uint16_t &date, uint16_t &time);
  static uint8_t make83Name(char *str, uint8_t *name);
  uint8_t openCachedEntry(uint8_t cacheIndex, uint8_t oflags);
  dir_t *readDirCache(void);
public:
  /** Create an instance of SdFile. */
  SdFile(void) : type_(FAT_FILE_TYPE_CLOSED){}
  /**
   * writeError is set to true if an error occurs during a write().
   * Set writeError to false before calling print() and/or write() and check
   * for true after calls to print() and/or write().
   */
  bool writeError;
  /**
   * Cancel unbuffered reads for this file.
   * See setUnbufferedRead()
   */
  void clearUnbufferedRead(void)
    {flags_ &= ~F_FILE_UNBUFFERED_READ;}
  uint8_t close(void);
  uint8_t contiguousRange(uint32_t &bgnBlock, uint32_t &endBlock);  
  uint8_t createContiguous(SdFile &dirFile, char *fileName, uint32_t size);
  /** \return The current cluster number for a file or directory. */
  uint32_t curCluster(void) {return curCluster_;}
  /** \return The current position for a file or directory. */
  uint32_t curPosition(void) {return curPosition_;}
  /**
   * Set the date/time callback function
   *
   * \param[in] dateTime The user's call back function.  The callback
   * function is of the form:
   *
   * \code
   * void dateTime(uint16_t &date, uint16_t &time)
   * {
   *   uint16_t year;
   *   uint8_t month, day, hour, minute, second;
   *
   *   // User gets date and time from GPS or real-time clock here
   *
   *   // return date using FAT_DATE macro to format fields
   *   date = FAT_DATE(year, month, day);
   *
   *   // return time using FAT_TIME macro to format fields
   *   time = FAT_TIME(hour, minute, second);
   * }
   * \endcode
   *
   * Sets the function that is called when a file is created or when
   * a file's directory entry is modified by sync(). All timestamps,
   * access, creation, and modify, are set when a file is created.
   * sync() maintains the last access date and last modify date/time.
   *
   * See the timestamp() function.
   */
  static void dateTimeCallback(void (*dateTime) (uint16_t &date, uint16_t &time))
    {dateTime_ = dateTime;}
  /** \return Address of the block that contains this file's directory. */
  uint32_t dirBlock(void) {return dirBlock_;}
  uint8_t dirEntry(dir_t &dir);
  /** \return Index of this file's directory in the block dirBlock. */
  uint8_t dirIndex(void)  {return dirIndex_;}
  static void dirName(dir_t &dir, char *name);
  /** \return The total number of bytes in a file or directory. */
  uint32_t fileSize(void) {return fileSize_;}
  /** \return The first cluster number for a file or directory. */
  uint32_t firstCluster(void) {return firstCluster_;}
  /** \return True if this is a SdFile for a directory else false. */
  uint8_t isDir(void) {return type_ >= FAT_FILE_TYPE_MIN_DIR;}
  /** \return True if this is a SdFile for a file else false. */
  uint8_t isFile(void) {return type_ == FAT_FILE_TYPE_NORMAL;}
  /** \return True if this is a SdFile for an open file/directory else false. */
  uint8_t isOpen(void) {return type_ != FAT_FILE_TYPE_CLOSED;}
  /** \return True if this is a SdFile for a subdirectory else false. */
  uint8_t isSubDir(void) {return type_ == FAT_FILE_TYPE_SUBDIR;}
  /** \return True if this is a SdFile for the root directory. */
  uint8_t isRoot(void) {
    return type_ == FAT_FILE_TYPE_ROOT16 || type_ == FAT_FILE_TYPE_ROOT32;}
  void ls(uint8_t flags = 0, uint8_t indent = 0);
  uint8_t makeDir(SdFile &dir, char *dirName);
  uint8_t open(SdFile &dirFile, uint16_t index, uint8_t oflag);
  uint8_t open(SdFile &dirFile, char *fileName, uint8_t oflag);
  /** For compatibility - do not use in new apps */
  uint8_t open(SdFile &dirFile, char *fileName) {
    return open(dirFile, fileName, O_RDWR);}
  uint8_t openRoot(SdVolume &vol);
  static void printDirName(dir_t &dir, uint8_t width);
  static void printFatDate(uint16_t fatDate);
  static void printFatTime(uint16_t fatTime);
  static void printTwoDigits(uint8_t v);
  /**
   * Read the next byte from a file.
   *
   * \return For success read returns the next byte in the file as an int.
   * If an error occurs or end of file is reached -1 is returned.
   */
  int16_t read(void) {uint8_t b; return read(&b, 1) == 1 ? b : -1;}
  int16_t read(void *buf, uint16_t nbyte);
  int8_t readDir(dir_t &dir);
  static uint8_t remove(SdFile &dirFile, char *fileName);
  uint8_t remove(void);
  /** Set the file's current position to zero. */
  void rewind(void) {curPosition_ = curCluster_ = 0;}
  uint8_t rmDir(void);
  uint8_t rmRfStar(void);
  /** Set the files position to current position + \a pos. See seekSet(). */
  uint8_t seekCur(uint32_t pos) {return seekSet(curPosition_ + pos);}
  /**
   *  Set the files current position to end of file.  Useful to position
   *  a file for append. See seekSet().
   */
  uint8_t seekEnd(void) {return seekSet(fileSize_);}
  uint8_t seekSet(uint32_t pos);
  /**
   * Use unbuffered reads to access this file.  Used with Wave
   * Shield ISR.  Used with Sd2Card::partialBlockRead() in WaveRP.
   *
   * Not recommended for normal applications.
   */
  void setUnbufferedRead(void)
    {if(isFile()) flags_ |= F_FILE_UNBUFFERED_READ;}
  uint8_t timestamp(uint8_t flag, uint16_t year, uint8_t month, uint8_t day,
          uint8_t hour, uint8_t minute, uint8_t second);
  uint8_t sync(void);
  /** Type of this SdFile.  You should use isFile() or isDir() instead of type()
   * if possible.
   *
   * \return The file or directory type.
   */
  uint8_t type(void) {return type_;}
  uint8_t truncate(uint32_t size);
  /** \return Unbuffered read flag. */
  uint8_t unbufferedRead(void)
    {return flags_ & F_FILE_UNBUFFERED_READ;}
  /** \return SdVolume that contains this file. */
  SdVolume *volume(void) {return vol_;}
  void write(uint8_t b);
  int16_t write(const void *buf, uint16_t nbyte);
  void write(const char *str);
  void write_P(PGM_P str);
  void writeln_P(PGM_P str);
};
//==============================================================================
// SdVolume class
/**
 * \brief Cache for an SD data block
 */
union cache_t {
           /** Used to access cached file data blocks. */
  uint8_t  data[512];
           /** Used to access cached FAT16 entries. */
  uint16_t fat16[256];
           /** Used to access cached FAT32 entries. */
  uint32_t fat32[128];
           /** Used to access cached directory entries. */
  dir_t    dir[16];
           /** Used to access a cached MasterBoot Record. */
  mbr_t    mbr;
           /** Used to access to a cached FAT boot sector. */
  fbs_t    fbs;
};
// value for action argument in cacheRawBlock to indicate cache dirty
#define CACHE_FOR_WRITE  1
/**
 * \class SdVolume
 * \brief Access FAT16 and FAT32 volumes on SD and SDHC cards.
 */
class SdVolume {
  /** Allow SdFile access to SdVolume private data. */
  friend class SdFile;
//
  static cache_t cacheBuffer_;       // 512 byte cache for storage device blocks
  static uint32_t cacheBlockNumber_; // Logical number of block in the cache
  static Sd2Card *sdCard_;           // Sd2Card object for cache
  static uint8_t cacheDirty_;        // cacheFlush() will write block if true
  static uint32_t cacheMirrorBlock_; // block number for mirror FAT
//
  uint32_t allocSearchStart_;   // start cluster for alloc search
  uint8_t blocksPerCluster_;    // cluster size in blocks
  uint32_t blocksPerFat_;       // FAT size in blocks
  uint32_t clusterCount_;       // clusters in one FAT
  uint8_t clusterSizeShift_;    // shift to convert cluster count to block count
  uint32_t dataStartBlock_;     // first data block number
  uint8_t fatCount_;            // number of FATs on volume
  uint32_t fatStartBlock_;      // start block for first FAT
  uint8_t fatType_;             // volume type (12, 16, OR 32)
  uint16_t rootDirEntryCount_;  // number of entries in FAT16 root dir
  uint32_t rootDirStart_;       // root start block for FAT16, cluster for FAT32
  //----------------------------------------------------------------------------
  uint8_t allocContiguous(uint32_t &curCluster, uint32_t count);
  uint8_t blockOfCluster(uint32_t position) {
          return (position >> 9) & (blocksPerCluster_ - 1);}
  uint32_t clusterStartBlock(uint32_t cluster) {
           return dataStartBlock_ + ((cluster - 2) << clusterSizeShift_);}
  uint32_t blockNumber(uint32_t cluster, uint32_t position) {
           return clusterStartBlock(cluster) + blockOfCluster(position);}
  static uint8_t cacheFlush(void);
  static uint8_t cacheRawBlock(uint32_t blockNumber, uint8_t action = 0);
  static void cacheSetDirty(void) {cacheDirty_ |= CACHE_FOR_WRITE;}
  static uint8_t cacheZeroBlock(uint32_t blockNumber);
  uint8_t chainSize(uint32_t &size, uint32_t beginCluster);
  uint8_t fatGet(uint32_t cluster, uint32_t &value);
  uint8_t fatPut(uint32_t cluster, uint32_t value);
  uint8_t fatPutEOC(uint32_t cluster) {return fatPut(cluster, 0x0FFFFFFF);}
  uint8_t findContiguous(uint32_t &start, uint32_t count);
  uint8_t freeChain(uint32_t cluster);
  uint8_t isEOC(uint32_t cluster)
      {return  cluster >= (fatType_ == 16 ? FAT16EOC_MIN : FAT32EOC_MIN);}
  uint8_t readBlock(uint32_t block, uint8_t *dst) {
    return sdCard_->readBlock(block, dst);}
  uint8_t readData(uint32_t block, uint16_t offset, uint8_t *dst, uint16_t count)
      {return sdCard_->readData(block, offset, dst, count);}
  uint8_t writeBlock(uint32_t block, uint8_t *dst)
      {return sdCard_->writeBlock(block, dst);}
public:
  /** Create an instance of SdVolume */
  SdVolume(void) :allocSearchStart_(2), fatType_(0) {}
  /** Clear the cache and returns a pointer to the cache.  Used by the WaveRP
   *  recorder to do raw write to the SD card.  Not for normal apps.
   */
  static uint8_t *cacheClear(void)
    {cacheFlush(); cacheBlockNumber_ = 0XFFFFFFFF; return cacheBuffer_.data;}
  /**
   * Initialize a FAT volume.  Try partition one first then try super
   * floppy format.
   *
   * \param[in] dev The Sd2Card where the volume is located.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.  Reasons for
   * failure include not finding a valid partition, not finding a valid
   * FAT file system or an I/O error.
   */
  uint8_t init(Sd2Card &dev) { return init(dev, 1) ? 1 : init(dev, 0);}
  uint8_t init(Sd2Card &dev, uint8_t vol);

  // inline functions that return volume info
  /** \return The volume's cluster size in blocks. */
  uint8_t blocksPerCluster(void) {return blocksPerCluster_;}
  /** \return The number of blocks in one FAT. */
  uint32_t blocksPerFat(void)  {return blocksPerFat_;}
  /** \return The total number of clusters in the volume. */
  uint32_t clusterCount(void) {return clusterCount_;}
  /** \return The shift count required to multiply by blocksPerCluster. */
  uint8_t clusterSizeShift(void) {return clusterSizeShift_;}
  /** \return The logical block number for the start of file data. */
  uint32_t dataStartBlock(void) {return dataStartBlock_;}
  /** \return The number of FAT structures on the volume. */
  uint8_t fatCount(void) {return fatCount_;}
  /** \return The logical block number for the start of the first FAT. */
  uint32_t fatStartBlock(void) {return fatStartBlock_;}
  /** \return The FAT type of the volume. Values are 12, 16 or 32. */
  uint8_t fatType(void) {return fatType_;}
  /** \return The number of entries in the root directory for FAT16 volumes. */
  uint32_t rootDirEntryCount(void) {return rootDirEntryCount_;}
  /** \return The logical block number for the start of the root directory
       on FAT16 volumes or the first cluster number on FAT32 volumes. */
  uint32_t rootDirStart(void) {return rootDirStart_;}
  /** return a pointer to the Sd2Card object for this volume */
  static Sd2Card *sdCard(void) {return sdCard_;}
};
#endif // SdFat_h
