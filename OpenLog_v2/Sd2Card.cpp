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
#include "WProgram.h"
#include "wiring.h"
#include "Sd2Card.h"
//------------------------------------------------------------------------------

#ifndef SOFTWARE_SPI
// functions for hardware SPI

/** Set input mode for MISO */
#define spiMISOInputMode() pinMode(SPI_MISO_PIN, INPUT)
/** Set output mode for MOSI */
#define spiMOSIOutputMode() pinMode(SPI_MOSI_PIN, OUTPUT)
/** Set output mode for SCK */
#define spiSCKOutputMode() pinMode(SPI_SCK_PIN, OUTPUT)
/** Set output mode for SS */
#define spiSSOutputMode() pinMode(SPI_SS_PIN, OUTPUT)
/** Set Slave Select high */
inline void spiSSHigh(void) {digitalWrite(SPI_SS_PIN, HIGH);}
/** Set Slave Select low */
inline void spiSSLow(void) {digitalWrite(SPI_SS_PIN, LOW);}
/** Send a byte to the card */
inline void spiSend(uint8_t b) {SPDR = b; while(!(SPSR & (1 << SPIF)));}
/** Receive a byte from the card */
inline uint8_t spiRec(void) {spiSend(0XFF); return SPDR;}

#else // SOFTWARE_SPI
//------------------------------------------------------------------------------
// define functions for software SPI

/** Set input mode for MISO */
#define spiMISOInputMode()  SOFT_SPI_MISO_DDR &= ~_BV(SOFT_SPI_MISO_BIT)
/** Read MISO bit */
#define spiMISOIn() (SOFT_SPI_MISO_PIN & _BV(SOFT_SPI_MISO_BIT))

/** Set output mode for MOSI */
#define spiMOSIOutputMode() SOFT_SPI_MOSI_DDR |= _BV(SOFT_SPI_MOSI_BIT)
/** Set MOSI low */
#define spiMOSILow()  SOFT_SPI_MOSI_PORT &= ~_BV(SOFT_SPI_MOSI_BIT)

#define spiMOSIHigh() SOFT_SPI_MOSI_PORT |=  _BV(SOFT_SPI_MOSI_BIT)

/** Set output mode for SCK */
#define spiSCKOutputMode() SOFT_SPI_SCK_DDR |= _BV(SOFT_SPI_SCK_BIT)
/** Set SCK low */
#define spiSCKLow()  SOFT_SPI_SCK_PORT &= ~_BV(SOFT_SPI_SCK_BIT)
/** Set SCK high */
#define spiSCKHigh() SOFT_SPI_SCK_PORT |=  _BV(SOFT_SPI_SCK_BIT)

/** Set output mode for SS */
#define spiSSOutputMode() SOFT_SPI_SS_DDR |= _BV(SOFT_SPI_SS_BIT)
/** Set Slave Select high */
#define spiSSHigh() SOFT_SPI_SS_PORT |=  _BV(SOFT_SPI_SS_BIT)
/** Set Slave Select low */
#define spiSSLow()  SOFT_SPI_SS_PORT &= ~_BV(SOFT_SPI_SS_BIT)
/** Soft SPI receive */
#define spiRec() softSpiXfer(0XFF)
// Soft SPI send */
#define spiSend(b) softSpiXfer(b)
/** nop to tune soft SPI timing */
#define nop asm volatile ("nop\n\t")
//------------------------------------------------------------------------------
/**
 * Software SPI byte transfer function.
 * SPI clock rate on a 16Mhz CPU is about 700 kHz.
 */
uint8_t softSpiXfer(uint8_t data)
{
  for (uint8_t i = 0; i < 8; i++) {
  
    // set output bit
    if (data & 0X80) {
      spiMOSIHigh();
    }
    else {
      spiMOSILow();
    }
    
    // clock high
    spiSCKHigh();
    
    // delay for symetric clock
    nop;nop;nop;nop;nop;nop;

    // shift for next bit
    data <<= 1;

    // read input bit
    if (spiMISOIn()) data |= 1;

    // clock low
    spiSCKLow();
  }
  return data;
}


#endif // SOFTWARE_SPI
//------------------------------------------------------------------------------
uint8_t Sd2Card::cardCommand(uint8_t cmd, uint32_t arg)
{
  uint8_t r1;
  
  // end read if in partialBlockRead mode
  readEnd();
  
  // select card
  spiSSLow();
  
  // wait up to 300 ms if busy
  waitNotBusy(300);
  
  // send command
  spiSend(cmd | 0x40);
  
  // send argument
  for (int8_t s = 24; s >= 0; s -= 8) spiSend(arg >> s);
  
  // send CRC
  uint8_t crc = 0XFF;
  if (cmd == CMD0) crc = 0X95; // correct crc for CMD0 with arg 0
  if (cmd == CMD8) crc = 0X87; // correct crc for CMD8 with arg 0X1AA
  spiSend(crc);
  
  // wait for response
  for (uint8_t retry = 0; ((r1 = spiRec()) & 0X80) && retry != 0XFF; retry++);
  return r1;
}
//------------------------------------------------------------------------------
/**
 * Determine the size of a SD flash memory card.
 * \return The number of 512 byte data blocks in the card
 */ 
uint32_t Sd2Card::cardSize(void)
{
  csd_t csd;
  if (!readCSD(csd)) return 0;
  if (csd.v1.csd_ver == 0) {
    uint8_t read_bl_len = csd.v1.read_bl_len;
    uint16_t c_size = (csd.v1.c_size_high << 10)
                      | (csd.v1.c_size_mid << 2) | csd.v1.c_size_low;
    uint8_t c_size_mult = (csd.v1.c_size_mult_high << 1) | csd.v1.c_size_mult_low;
    return (uint32_t)(c_size + 1) << (c_size_mult + read_bl_len - 7);
  }
  else if (csd.v2.csd_ver == 1) {
    uint32_t c_size = ((uint32_t)csd.v2.c_size_high << 16)
                      | (csd.v2.c_size_mid << 8) | csd.v2.c_size_low;
    return (c_size + 1) << 10;
  }
  else {
    error(SD_CARD_ERROR_BAD_CSD);
    return false;
  }
}
//------------------------------------------------------------------------------
/** Erase a range of blocks.
 *
 * \param[in] firstBlock The address of the first block in the range.
 * \param[in] lastBlock The address of the last block in the range.
 *
 * \note This function requests the SD card to do a flash erase for a
 * range of blocks.  The data on the card after an erase operation is
 * either 0 or 1, depends on the card vendor.  The card must support
 * single block erase.
 *  
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure. 
 */  
uint8_t Sd2Card::erase(uint32_t firstBlock, uint32_t lastBlock)
{
  if (!eraseSingleBlockEnable()) {
    error(SD_CARD_ERROR_ERASE_SINGLE_BLOCK);
    return false;
  }
  if (type_ != SD_CARD_TYPE_SDHC) {
    firstBlock <<= 9;
    lastBlock <<= 9;
  }
  if (cardCommand(CMD32, firstBlock)
      || cardCommand(CMD33, lastBlock)
      || cardCommand(CMD38, 0)) {
      error(SD_CARD_ERROR_ERASE);
      return false;
  }
  if (waitNotBusy(SD_ERASE_TIMEOUT)) return true;
  error(SD_CARD_ERROR_ERASE_TIMEOUT);
  return false;
}
//------------------------------------------------------------------------------
/** Determine if card supports single block erase. 
 *
 * \return The value one, true, is returned if single block erase is supported.
 * The value zero, false, is returned if single block erase is not supported. 
 */
uint8_t Sd2Card::eraseSingleBlockEnable(void)
{
  csd_t csd;
  return readCSD(csd) ? csd.v1.erase_blk_en : 0;
}
//------------------------------------------------------------------------------
/**
 * Initialize a SD flash memory card.
 *
 * \param[in] slow Set SPI Frequency F_CPU/4 if true else F_CPU/2.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure. 
 */  
uint8_t Sd2Card::init(uint8_t slow)
{
  uint8_t r;
  errorCode_ = inBlock_ = partialBlockRead_ = type_ = 0;

  // set pin modes
  spiSSOutputMode();
  spiSSHigh();
  spiMISOInputMode();
  spiMOSIOutputMode();
  spiSCKOutputMode();
  
#ifndef SOFTWARE_SPI
  //Enable SPI, Master, clock rate f_osc/128
  SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
#endif //SOFTWARE_SPI

  //must supply min of 74 clock cycles with CS high.
  for (uint8_t i = 0; i < 10; i++) spiSend(0XFF);
  
  // next two lines prevent re-init hang by cards that were in partial read
  spiSSLow();
  for (uint16_t i = 0; i <= 512; i++) spiRec();
  
  // command to go idle in SPI mode
  for (uint8_t retry = 0; ; retry++) {
    if ((r = cardCommand(CMD0, 0)) ==  R1_IDLE_STATE) break;
    if (retry == 10) {
      error(SD_CARD_ERROR_CMD0, r);
      return false;
    }
  }
  // check SD version
  if ((cardCommand(CMD8, 0x1AA) & R1_ILLEGAL_COMMAND)) {
    type(SD_CARD_TYPE_SD1);
  }
  else {
    // only need last byte of r7 response
    for(uint8_t i = 0; i < 4; i++) r = spiRec();
    if (r != 0XAA) {
      error(SD_CARD_ERROR_CMD8, r);
      return false;
    }
    type(SD_CARD_TYPE_SD2);
  }
  // initialize card and send host supports SDHC if SD2
  for (uint16_t t0 = millis();;) {
    r = cardAcmd(ACMD41, type() == SD_CARD_TYPE_SD2 ? 0X40000000 : 0);
    if (r == R1_READY_STATE) break;
    // timeout after 2 seconds
    if (((uint16_t)millis() - t0) > 2000) {
      error(SD_CARD_ERROR_ACMD41);
      return false;
    }
  }
  // if SD2 read OCR register to check for SDHC card
  if (type() == SD_CARD_TYPE_SD2) {
    if(cardCommand(CMD58, 0)) {
      error(SD_CARD_ERROR_CMD58);
      return false;
    }
    if ((spiRec() & 0XC0) == 0XC0) type(SD_CARD_TYPE_SDHC);
    // discard rest of ocr 
    for (uint8_t i = 0; i < 3; i++) spiRec();
  }
#ifndef SOFTWARE_SPI
  // set SPI frequency to f_OSC/4
  SPCR &= ~((1 << SPR1) | (1 << SPR0));
  // if !slow set SPI frequency to f_OSC/2
  if (!slow) SPSR |= (1 << SPI2X); 
#endif // SOFTWARE_SPI

  spiSSHigh();
  return true;
}
//------------------------------------------------------------------------------
/**
 * Read part of a 512 byte block from a SD card.
 *  
 * \param[in] block Logical block to be read.
 * \param[in] offset Number of bytes to skip at start of block
 * \param[out] dst Pointer to the location that will receive the data. 
 * \param[in] count Number of bytes to read
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.      
 */
uint8_t Sd2Card::readData(uint32_t block, uint16_t offset, uint8_t *dst, uint16_t count)
{
  if (count == 0) return true;
  if ((count + offset) > 512) {
    return false;
  }
  if (!inBlock_ || block != block_ || offset < offset_) {
    block_ = block;
    //use address if not SDHC card
    if (type()!= SD_CARD_TYPE_SDHC) block <<= 9;
    if (cardCommand(CMD17, block)) {
      error(SD_CARD_ERROR_CMD17);
      return false;
    }
    if (!waitStartBlock()) {
      return false;
    }
    offset_ = 0;
    inBlock_ = 1;
  }
  
#ifdef OPTIMIZE_HARDWARE_SPI

  //start first spi transfer
  SPDR = 0XFF;
  
  //skip data before offset
  for (;offset_ < offset; offset_++) {
    while(!(SPSR & (1 << SPIF)));
    SPDR = 0XFF;
  }
  
  //transfer data
  uint16_t n = count - 1;
  for (uint16_t i = 0; i < n; i++) {
    while(!(SPSR & (1 << SPIF)));
    dst[i] = SPDR;
    SPDR = 0XFF;
  }
  
  // wait for last byte
  while(!(SPSR & (1 << SPIF)));
  dst[n] = SPDR;
  
#else // OPTIMIZE_HARDWARE_SPI
  
  // skip data before offset
  for (;offset_ < offset; offset_++) {
    spiRec();
  }
  //transfer data
  for (uint16_t i = 0; i < count; i++) {
    dst[i] = spiRec();
  }
#endif // OPTIMIZE_HARDWARE_SPI

  offset_ += count;
  if (!partialBlockRead_ || offset_ >= 512) readEnd();
  return true;
}
//------------------------------------------------------------------------------
/** Skip remaining data in a block when in partial block read mode. */
void Sd2Card::readEnd(void)
{
  if (inBlock_) {
      // skip data and crc
#ifdef OPTIMIZE_HARDWARE_SPI

    // optimize skip for hardware
    SPDR = 0XFF;
    while (offset_++ < 513) {
      while(!(SPSR & (1 << SPIF)));
      SPDR = 0XFF;
    }
    // wait for last crc byte
    while(!(SPSR & (1 << SPIF)));
    
#else // OPTIMIZE_HARDWARE_SPI

    while (offset_++ < 514) spiRec();
#endif // OPTIMIZE_HARDWARE_SPI

    spiSSHigh();
    inBlock_ = 0;
  }
}
//------------------------------------------------------------------------------
/** read CID or CSR register */
uint8_t Sd2Card::readRegister(uint8_t cmd, uint8_t *dst)
{
  if (cardCommand(cmd, 0)) {
    error(SD_CARD_ERROR_READ_REG);
    return false;
  }
  if(!waitStartBlock()) return false;
  //transfer data
  for (uint16_t i = 0; i < 16; i++) dst[i] = spiRec();
  spiRec();// get first crc byte
  spiRec();// get second crc byte
  spiSSHigh();
  return true;
}
//------------------------------------------------------------------------------
// wait for card to go not busy
uint8_t Sd2Card::waitNotBusy(uint16_t timeoutMillis)
{
  uint16_t t0 = millis();
  while (spiRec() != 0XFF) {
    if (((uint16_t)millis() - t0) > timeoutMillis) return false;
  }
  return true;
}
//------------------------------------------------------------------------------
/** Wait for start block token */
uint8_t Sd2Card::waitStartBlock(void)
{
  uint8_t r;
  uint16_t t0 = millis();
  while ((r = spiRec()) == 0XFF) {
    if (((uint16_t)millis() - t0) > SD_READ_TIMEOUT) {
      error(SD_CARD_ERROR_READ_TIMEOUT);
      return false;
    }
  }
  if (r == DATA_START_BLOCK) return true;
  error(SD_CARD_ERROR_READ, r);
  return false;
}
//------------------------------------------------------------------------------
/**
 * Writes a 512 byte block to a SD card.
 *
 * \param[in] blockNumber Logical block to be written.
 * \param[in] src Pointer to the location of the data to be written.
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
uint8_t Sd2Card::writeBlock(uint32_t blockNumber, uint8_t *src)
{
#if SD_PROTECT_BLOCK_ZERO
  //don't allow write to first block
  if (blockNumber == 0) {
    error(SD_CARD_ERROR_WRITE_BLOCK_ZERO);
    return false;
  }
#endif //SD_PROTECT_BLOCK_ZERO

  //use address if not SDHC card
  if (type() != SD_CARD_TYPE_SDHC) blockNumber <<= 9;
  if (cardCommand(CMD24, blockNumber)) {
    error(SD_CARD_ERROR_CMD24);
    return false;
  }
  if(!writeData(DATA_START_BLOCK, src)) return false;
  
  // wait for flash programming to complete
  if (!waitWriteDone()) return false;
  
  // response is r2 so get and check byte two for nonzero
  if (cardCommand(CMD13, 0) || spiRec()) {
    error(SD_CARD_ERROR_WRITE_PROGRAMMING);
    return false;
  }
  spiSSHigh();
  return true;
}
//------------------------------------------------------------------------------
/** send one block of data for write block or write multiple blocks */
uint8_t Sd2Card::writeData(uint8_t token, uint8_t *src)
{
#ifdef OPTIMIZE_HARDWARE_SPI

 // send data - optimized loop
  SPDR = token;

  // send two byte per iteration
  for (uint16_t i = 0; i < 512; i += 2) {
    while(!(SPSR & (1 << SPIF)));
    SPDR = src[i];
    while(!(SPSR & (1 << SPIF)));
    SPDR = src[i+1];
  }

  // wait for last data byte
  while(!(SPSR & (1 << SPIF)));
  
#else // OPTIMIZE_HARDWARE_SPI

  spiSend(token);
  for (uint16_t i = 0; i < 512; i++) {
    spiSend(src[i]);
  }
#endif //OPTIMIZE_HARDWARE_SPI

  spiSend(0xff);// dummy crc
  spiSend(0xff);// dummy crc
  
  uint8_t r1 = spiRec();
  if ((r1 & DATA_RES_MASK) == DATA_RES_ACCEPTED) return true;
  error(SD_CARD_ERROR_WRITE, r1);
  return false;
}
//------------------------------------------------------------------------------
/** Start a write multiple blocks sequence. 
 *
 * \param[in] blockNumber Address of first block in sequence.
 * \param[in] eraseCount The number of blocks to be pre-erased.
 *  
 * \note This function is used with writeData() and writeStop()
 * for optimized multiple block writes.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
uint8_t Sd2Card::writeStart(uint32_t blockNumber, uint32_t eraseCount)
{
#if SD_PROTECT_BLOCK_ZERO
  //don't allow write to first block
  if (blockNumber == 0) {
    error(SD_CARD_ERROR_WRITE_BLOCK_ZERO);
    return false;
  }
#endif //SD_PROTECT_BLOCK_ZERO
  // send pre-erase count
  if (cardAcmd(ACMD23, eraseCount)) {
    error(SD_CARD_ERROR_ACMD23);
    return false;
  }
  // use address if not SDHC card
  if (type() != SD_CARD_TYPE_SDHC) blockNumber <<= 9;  
  if (cardCommand(CMD25, blockNumber)) {
    error(SD_CARD_ERROR_CMD25);
    return false;
  }
  return true;
}
//------------------------------------------------------------------------------
/** End a write multiple blocks sequence. 
 *
* \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
uint8_t Sd2Card::writeStop(void)
{
  if (!waitWriteDone()) return false;
  spiSend(STOP_TRAN_TOKEN);
  if (!waitWriteDone()) return false;
  spiSSHigh();
  return true;
}
