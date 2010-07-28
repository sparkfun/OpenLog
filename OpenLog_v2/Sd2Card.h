/* Arduino Sd2Card Library
 * Copyright (C) 2009 by William Greiman
 *  
 * This file is part of the Arduino Sd2Card Library
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
 * along with the Arduino Sd2Card Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef Sd2Card_h
#define Sd2Card_h
/**
 * \file
 * Sd2Card class
 */
#include "ArduinoPins.h"
#include "SdInfo.h"
/**
 * Define MEGA_SOFT_SPI to use software SPI on Mega Arduinos.
 * Pins used are SS 10, MOSI 11, MISO 12, and SCK 13.
 *
 * MEGA_SOFT_SPI allows an unmodified Adafruit GPS Shield to be used
 * on Mega Arduinos.  Software SPI works well with GPS Shield V1.1 
 * but many SD cards will fail with GPS Shield V1.0.
 */
//#define MEGA_SOFT_SPI

//------------------------------------------------------------------------------
#ifdef MEGA_SOFT_SPI && defined(__AVR_ATmega1280__)
#define SOFTWARE_SPI
#endif // MEGA_SOFT_SPI

//------------------------------------------------------------------------------
//SPI pin definitions
// See ArduinoPins.h
//
#ifndef SOFTWARE_SPI

// hardware pin defs
/** SPI Slave Select pin */
#define SPI_SS_PIN   SS_PIN
/** SPI Master Out Slave In pin */
#define SPI_MOSI_PIN MOSI_PIN
/** SPI Master In Slave Out pin */
#define SPI_MISO_PIN MISO_PIN
/** SPI Clock pin */
#define SPI_SCK_PIN  SCK_PIN

/** optimize loops for hardware SPI */
#define OPTIMIZE_HARDWARE_SPI

#else // SOFTWARE_SPI
// define software SPI so Mega can use unmodified GPS Shield

#define SOFT_SPI_SS_DDR  PIN10_DDRREG
#define SOFT_SPI_SS_PORT PIN10_PORTREG
#define SOFT_SPI_SS_BIT  PIN10_BITNUM

#define SOFT_SPI_MOSI_DDR  PIN11_DDRREG
#define SOFT_SPI_MOSI_PORT PIN11_PORTREG
#define SOFT_SPI_MOSI_BIT  PIN11_BITNUM

#define SOFT_SPI_MISO_DDR  PIN12_DDRREG
#define SOFT_SPI_MISO_PIN  PIN12_PINREG
#define SOFT_SPI_MISO_BIT  PIN12_BITNUM

#define SOFT_SPI_SCK_DDR  PIN13_DDRREG
#define SOFT_SPI_SCK_PORT PIN13_PORTREG
#define SOFT_SPI_SCK_BIT  PIN13_BITNUM

#endif // SOFTWARE_SPI

//------------------------------------------------------------------------------
/** Protect block zero from write if nonzero */
#define SD_PROTECT_BLOCK_ZERO 1
/** erase timeout ms */
#define SD_ERASE_TIMEOUT 10000
/** read timeout ms */
#define SD_READ_TIMEOUT    300
/** write time out ms */
#define SD_WRITE_TIMEOUT   600

//------------------------------------------------------------------------------
// SD card errors
/** timeout error for command CMD0 */
#define SD_CARD_ERROR_CMD0  0X1
/** CMD8 was not accepted - not a valid SD card*/
#define SD_CARD_ERROR_CMD8  0X2
/** card returned an error response for CMD17 (read block) */
#define SD_CARD_ERROR_CMD17 0X3
/** card returned an error response for CMD24 (write block) */
#define SD_CARD_ERROR_CMD24 0X4
/** card returned an error response for CMD58 (read OCR) */
#define SD_CARD_ERROR_CMD58 0X5
/** card's ACMD41 initialization process timeout */
#define SD_CARD_ERROR_ACMD41 0X6
/** card returned a bad CSR version field */
#define SD_CARD_ERROR_BAD_CSD 0X7
/** read CID or CSD failed */
#define SD_CARD_ERROR_READ_REG 0X8
/** timeout occurred during write programming */
#define SD_CARD_ERROR_WRITE_TIMEOUT 0X9
/** attempt to write protected block zero */
#define SD_CARD_ERROR_WRITE_BLOCK_ZERO 0XA
/** card not capable of single block erase */
#define SD_CARD_ERROR_ERASE_SINGLE_BLOCK 0XB
/** erase block group command failed */
#define SD_CARD_ERROR_ERASE 0XC
/** timeout while waiting for start of read data */
#define SD_CARD_ERROR_READ_TIMEOUT 0XD
/** SET_WR_BLK_ERASE_COUNT failed */
#define SD_CARD_ERROR_ACMD23 0XE
/** Erase sequence timed out */
#define SD_CARD_ERROR_ERASE_TIMEOUT 0XF
/**  WRITE_MULTIPLE_BLOCKS command failed */
#define SD_CARD_ERROR_CMD25 0X10
/** card returned an error token instead of read data */
#define SD_CARD_ERROR_READ 0X11
/** card returned an error token as a response to a write operation */
#define SD_CARD_ERROR_WRITE 0X12
/** card returned an error to a CMD13 status check after a write */
#define SD_CARD_ERROR_WRITE_PROGRAMMING 0X13
//------------------------------------------------------------------------------
// card types
/** Standard capacity V1 SD card */
#define SD_CARD_TYPE_SD1 1
/** Standard capacity V2 SD card */
#define SD_CARD_TYPE_SD2 2
/** High Capacity SD card */
#define SD_CARD_TYPE_SDHC 3
//------------------------------------------------------------------------------
/**
 * \class Sd2Card
 * \brief Raw access to SD and SDHC flash memory cards.
 */
class Sd2Card {
private:
  uint32_t block_;
  uint8_t errorCode_;
  uint8_t errorData_;
  uint8_t inBlock_;
  uint16_t offset_;
  uint8_t partialBlockRead_;
  uint8_t type_;
  // private functions
  uint8_t cardAcmd(uint8_t cmd, uint32_t arg) {
    cardCommand(CMD55, 0); return cardCommand(cmd, arg);}
  uint8_t cardCommand(uint8_t cmd, uint32_t arg);
  void error(uint8_t code){errorCode_ = code;}
  void error(uint8_t code, uint8_t data) {errorCode_ = code; errorData_ = data;}
  uint8_t readRegister(uint8_t cmd, uint8_t *dst);
  uint8_t sendWriteCommand(uint32_t blockNumber, uint32_t eraseCount);
  void type(uint8_t value) {type_ = value;}
  uint8_t waitNotBusy(uint16_t timeoutMillis);
  uint8_t waitWriteDone(void) {if (waitNotBusy(SD_WRITE_TIMEOUT)) return true; 
          error(SD_CARD_ERROR_WRITE_TIMEOUT); return false;}
  uint8_t writeData(uint8_t token, uint8_t *src);
  uint8_t waitStartBlock(void);
public:
  /** Construct an instance of Sd2Card. */
  Sd2Card(void) :  errorCode_(0), inBlock_(0), partialBlockRead_(0), type_(0) {};
  uint32_t cardSize(void);
  uint8_t erase(uint32_t firstBlock, uint32_t lastBlock);
  uint8_t eraseSingleBlockEnable(void);
  /** \return error code for last error */
  uint8_t errorCode(void) {return errorCode_;}
  /** \return error data for last error */
  uint8_t errorData(void) {return errorData_;}
  uint8_t init(uint8_t slow = 0);
  /**
   * Enable or disable partial block reads.
   * 
   * Enabling partial block reads improves performance by allowing a block
   * to be read over the spi bus as several sub-blocks.  Errors may occur
   * if the time between reads is too long since the SD card may timeout.
   *
   * Use this for applications like the Adafruit Wave Shield.
   *
   * \param[in] value The value TRUE (non-zero) or FALSE (zero).)   
   */     
  void partialBlockRead(uint8_t value) {readEnd(); partialBlockRead_ = value;}
  /** Returns the current value, true or false, for partial block read. */
  uint8_t partialBlockRead(void) {return partialBlockRead_;}
  uint8_t readData(uint32_t block, uint16_t offset, uint8_t *dst, uint16_t count);
  /**
 * Read a 512 byte block from a SD card device.
 *
 * \param[in] block Logical block to be read.
 * \param[out] dst Pointer to the location that will receive the data. 

 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.      
 */ 
  uint8_t readBlock(uint32_t block, uint8_t *dst){ 
           return readData(block, 0, dst, 512);}
  /** 
   * Read a cards CID register. The CID contains card identification information
   * such as Manufacturer ID, Product name, Product serial number and
   * Manufacturing date. */
   
  uint8_t readCID(cid_t &cid) {return readRegister(CMD10, (uint8_t *)&cid);}
  /** 
   * Read a cards CSD register. The CSD contains Card-Specific Data that
   * provides information regarding access to the card contents. */
  uint8_t readCSD(csd_t &csd) {return readRegister(CMD9, (uint8_t *)&csd);}
  void readEnd(void);
  /** Return the card type: SD V1, SD V2 or SDHC */
  uint8_t type(void) {return type_;}
  uint8_t writeBlock(uint32_t blockNumber, uint8_t *src);
  /** Write one data block in a multiple block write sequence */
  uint8_t writeData(uint8_t *src) { 
    return waitWriteDone() ? writeData(WRITE_MULTIPLE_TOKEN, src) : false;}
  uint8_t writeStart(uint32_t blockNumber, uint32_t eraseCount);
  uint8_t writeStop(void);
};
#endif //Sd2Card_h
