/* Arduino SerialPort Library
 * Copyright (C) 2012 by William Greiman
 *
 * This file is part of the Arduino SerialPort Library
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
 * along with the Arduino SerialPort Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
/**
 * @file
 * @brief Serial Port class
 */
#ifndef SerialPort_h
#define SerialPort_h
//------------------------------------------------------------------------------
/** SerialPort version YYYYMMDD */
#define SERIAL_PORT_VERSION 20140216
//------------------------------------------------------------------------------
/**
 * Set ALLOW_LARGE_BUFFERS to zero to limit buffer sizes to 254 bytes.
 *
 * ALLOW_LARGE_BUFFERS controls whether uint16_t or uint8_t will be
 * used for buffer indices.
 */
#define ALLOW_LARGE_BUFFERS 1
//------------------------------------------------------------------------------
/**
 * Set USE_WRITE_OVERRIDES to zero to use the Arduino Print version
 * of write(const char*) and write(const uint8_t*, size_t).
 *
 * This will save some flash but is much slower.
 */
#define USE_WRITE_OVERRIDES 1
//------------------------------------------------------------------------------
/**
 * Set BUFFERED_RX zero to save flash and RAM if no RX buffering is used.
 *
 * RxBufSize must be zero in all SerialPort constructors if
 * BUFFERED_RX is zero.
 */
#define BUFFERED_RX 1
//------------------------------------------------------------------------------
/**
 * Set BUFFERED_TX zero to save flash and RAM if no TX buffering is used.
 *
 * TxBufSize must be zero in all SerialPort constructors if
 * BUFFERED_TX is zero.
 */
#define BUFFERED_TX 1
//------------------------------------------------------------------------------
/**
 * Set ENABLE_RX_ERROR_CHECKING zero to disable RX error checking.
 */
#define ENABLE_RX_ERROR_CHECKING 1
//------------------------------------------------------------------------------
// Define symbols to allocate 64 byte ring buffers with capacity for 63 bytes.
/** Define NewSerial with buffering like Arduino 1.0. */
#define USE_NEW_SERIAL SerialPort<0, 63, 63> NewSerial
/** Define NewSerial1 with buffering like Arduino 1.0. */
#define USE_NEW_SERIAL1 SerialPort<1, 63, 63> NewSerial1
/** Define NewSerial2 with buffering like Arduino 1.0. */
#define USE_NEW_SERIAL2 SerialPort<2, 63, 63> NewSerial2
/** Define NewSerial3 with buffering like Arduino 1.0. */
#define USE_NEW_SERIAL3 SerialPort<3, 63, 63> NewSerial3
//------------------------------------------------------------------------------
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <Arduino.h>
//------------------------------------------------------------------------------
#if defined(UCSR3A)
static const uint8_t SERIAL_PORT_COUNT = 4;
#elif defined(UCSR2A)
static const uint8_t SERIAL_PORT_COUNT = 3;
#elif defined(UCSR1A)
static const uint8_t SERIAL_PORT_COUNT = 2;
#elif defined(UCSR0A) || defined(UCSRA)
static const uint8_t SERIAL_PORT_COUNT = 1;
#else
#error no serial ports.
#endif
//------------------------------------------------------------------------------
#ifdef UCSR0A
// Bits in UCSRA.
static const uint8_t M_RXC  = 1 << RXC0;
static const uint8_t M_TXC  = 1 << TXC0;
static const uint8_t M_UDRE = 1 << UDRE0;
static const uint8_t M_FE   = 1 << FE0;
static const uint8_t M_DOR  = 1 << DOR0;
static const uint8_t M_UPE  = 1 << UPE0;
static const uint8_t M_U2X  = 1 << U2X0;
// Bits in UCSRB.
static const uint8_t M_RXCIE = 1 << RXCIE0;
static const uint8_t M_TXCIE = 1 << TXCIE0;
static const uint8_t M_UDRIE = 1 << UDRIE0;
static const uint8_t M_RXEN  = 1 << RXEN0;
static const uint8_t M_TXEN  = 1 << TXEN0;
// Bits in UCSRC.
static const uint8_t M_UPM0 = 1 << UPM00;
static const uint8_t M_UPM1 = 1 << UPM01;
static const uint8_t M_USBS = 1 << USBS0;
static const uint8_t M_UCSZ0 = 1 << UCSZ00;
static const uint8_t M_UCSZ1 = 1 << UCSZ01;
#elif defined(UCSRA)  // UCSR0A
// Bits in UCSRA.
static const uint8_t M_RXC  = 1 << RXC;
static const uint8_t M_TXC  = 1 << TXC;
static const uint8_t M_UDRE = 1 << UDRE;
static const uint8_t M_FE   = 1 << FE;
static const uint8_t M_DOR  = 1 << DOR;
static const uint8_t M_UPE  = 1 << PE;
static const uint8_t M_U2X  = 1 << U2X;
// Bits in UCSRB.
static const uint8_t M_RXCIE = 1 << RXCIE;
static const uint8_t M_TXCIE = 1 << TXCIE;
static const uint8_t M_UDRIE = 1 << UDRIE;
static const uint8_t M_RXEN  = 1 << RXEN;
static const uint8_t M_TXEN  = 1 << TXEN;
// Bits in UCSRC.
static const uint8_t M_UPM0 = 1 << UPM0;
static const uint8_t M_UPM1 = 1 << UPM1;
static const uint8_t M_USBS = 1 << USBS;
static const uint8_t M_UCSZ0 = 1 << UCSZ0;
static const uint8_t M_UCSZ1 = 1 << UCSZ1;
#elif defined(UCSR1A)  // UCSR0A
// Bits in UCSRA.
static const uint8_t M_RXC  = 1 << RXC1;
static const uint8_t M_TXC  = 1 << TXC1;
static const uint8_t M_UDRE = 1 << UDRE1;
static const uint8_t M_FE   = 1 << FE1;
static const uint8_t M_DOR  = 1 << DOR1;
static const uint8_t M_UPE  = 1 << UPE1;
static const uint8_t M_U2X  = 1 << U2X1;
// Bits in UCSRB.
static const uint8_t M_RXCIE = 1 << RXCIE1;
static const uint8_t M_TXCIE = 1 << TXCIE1;
static const uint8_t M_UDRIE = 1 << UDRIE1;
static const uint8_t M_RXEN  = 1 << RXEN1;
static const uint8_t M_TXEN  = 1 << TXEN1;
// Bits in UCSRC.
static const uint8_t M_UPM0 = 1 << UPM10;
static const uint8_t M_UPM1 = 1 << UPM11;
static const uint8_t M_USBS = 1 << USBS1;
static const uint8_t M_UCSZ0 = 1 << UCSZ10;
static const uint8_t M_UCSZ1 = 1 << UCSZ11;
#else  // UCSR0A
#error no serial ports
#endif  // UCSR0A
//------------------------------------------------------------------------------
/** Use one stop bit. */
static const uint8_t SP_1_STOP_BIT = 0;
/** Use two stop bits. */
static const uint8_t SP_2_STOP_BIT = M_USBS;

/** No parity bit. */
static const uint8_t SP_NO_PARITY = 0;
/** Use even parity. */
static const uint8_t SP_EVEN_PARITY = M_UPM1;
/** Use odd parity. */
static const uint8_t SP_ODD_PARITY = M_UPM0 | M_UPM1;

/** Use 5-bit character size. */
static const uint8_t SP_5_BIT_CHAR = 0;
/** Use 6-bit character size. */
static const uint8_t SP_6_BIT_CHAR = M_UCSZ0;
/** Use 7-bit character size. */
static const uint8_t SP_7_BIT_CHAR = M_UCSZ1;
/** Use 8-bit character size. */
static const uint8_t SP_8_BIT_CHAR = M_UCSZ0 | M_UCSZ1;
/** Mask for all options bits. */
static const uint8_t SP_OPT_MASK = M_USBS | M_UPM0 | M_UPM1 |M_UCSZ0 | M_UCSZ1;

/** USART framing error bit. */
static const uint8_t SP_FRAMING_ERROR    = M_FE;
/** USART RX data overrun error bit. */
static const uint8_t SP_RX_DATA_OVERRUN  = M_DOR;
/** USART parity error bit. */
static const uint8_t SP_PARITY_ERROR     = M_UPE;
/** Mask for all error bits in UCSRA. */
static const uint8_t SP_UCSRA_ERROR_MASK = M_FE | M_DOR | M_UPE;
/** RX ring buffer full overrun. */
static const uint8_t SP_RX_BUF_OVERRUN  = 1;
#if 1 & ((1 << FE0) | (1 << DOR0) |(1 << UPE0))
#error Invalid SP_RX_BUF_OVERRUN bit
#endif  // SP_RX_BUF_OVERRUN
//------------------------------------------------------------------------------
/**
 * @class UsartRegister
 * @brief Addresses of USART registers.
 */
struct UsartRegister {
  volatile uint8_t* ucsra;  /**< USART Control and Status Register A. */
  volatile uint8_t* ucsrb;  /**< USART Control and Status Register B. */
  volatile uint8_t* ucsrc;  /**< USART Control and Status Register C. */
  volatile uint8_t* ubrrl;  /**< USART Baud Rate Register Low. */
  volatile uint8_t* ubrrh;  /**< USART Baud Rate Register High. */
  volatile uint8_t* udr;    /**< USART I/O Data Register. */
};
//------------------------------------------------------------------------------
/**
 * Pointers to USART registers.  This static const array allows the compiler
 * to generate very efficient code if the array index is a constant.
 */
static const UsartRegister usart[] = {
#ifdef UCSR0A
  {&UCSR0A, &UCSR0B, &UCSR0C, &UBRR0L, &UBRR0H, &UDR0},
#elif defined(UCSRA)
  {&UCSRA, &UCSRB, &UCSRC, &UBRRL, &UBRRH, &UDR},
#else  // UCSR0A
  {0, 0, 0, 0, 0, 0},
#endif  // UCSR0A

#ifdef UCSR1A
  {&UCSR1A, &UCSR1B, &UCSR1C, &UBRR1L, &UBRR1H, &UDR1},
#else  // UCSR1A
  {0, 0, 0, 0, 0, 0},
#endif  // UCSR1A

#ifdef UCSR2A
  {&UCSR2A, &UCSR2B, &UCSR2C, &UBRR2L, &UBRR2H, &UDR2},
#else  // UCSR2A
  {0, 0, 0, 0, 0, 0},
#endif  // UCSR2A

#ifdef UCSR3A
  {&UCSR3A, &UCSR3B, &UCSR3C, &UBRR3L, &UBRR3H, &UDR3}
#else  // UCSR3A
  {0, 0, 0, 0, 0, 0}
#endif  // UCSR3A
};
//------------------------------------------------------------------------------
/**
 * @class SerialRingBuffer
 * @brief Ring buffer for RX and TX data.
 */
class SerialRingBuffer {
 public:
  /** Define type for buffer indices */
#if ALLOW_LARGE_BUFFERS
  typedef uint16_t buf_size_t;
#else  // ALLOW_LARGE_BUFFERS
  typedef uint8_t buf_size_t;
#endif  // ALLOW_LARGE_BUFFERS
  int available();
  /** @return @c true if the ring buffer is empty else @c false. */
  bool empty() {return head_ == tail_;}
  void flush();
  bool get(uint8_t* b);
  buf_size_t get(uint8_t* b, buf_size_t n);
  void init(uint8_t* b, buf_size_t s);
  int peek();
  bool put(uint8_t b);
  buf_size_t put(const uint8_t* b, buf_size_t n);
  buf_size_t put_P(PGM_P b, buf_size_t n);
 private:
  uint8_t* buf_;              /**< Pointer to start of buffer. */
  volatile buf_size_t head_;  /**< Index to next empty location. */
  volatile buf_size_t tail_;  /**< Index to last entry if head_ != tail_. */
  buf_size_t size_;           /**< Size of the buffer. Capacity is size -1. */
};
//------------------------------------------------------------------------------
/** RX ring buffers. */
extern SerialRingBuffer rxRingBuf[];
/** TX ring buffers. */
extern SerialRingBuffer txRingBuf[];
/** RX error bits. */
extern uint8_t rxErrorBits[];
//------------------------------------------------------------------------------
/** Cause error message for bad port number.
 * @return Never returns since it is never called.
 */
uint8_t badPortNumber(void)
  __attribute__((error("Bad port number")));
/** Cause error message for bad port number.
 * @return Never returns since it is never called.
 */
uint8_t badRxBufSize(void)
  __attribute__((error("RX buffer size too large")));
/** Cause error message for bad port number.
 * @return Never returns since it is never called.
 */
uint8_t badTxBufSize(void)
  __attribute__((error("TX buffer size too large")));
//------------------------------------------------------------------------------
/**
 * @class SerialPort
 * @brief Class for avr hardware USART ports.
 */
template<uint8_t PortNumber, size_t RxBufSize, size_t TxBufSize>
class SerialPort : public Stream {
 public:
  //----------------------------------------------------------------------------
  /** Constructor */
  SerialPort() {
    if (PortNumber >= SERIAL_PORT_COUNT || !usart[PortNumber].ucsra) {
      badPortNumber();
    }
    if (sizeof(SerialRingBuffer::buf_size_t) == 1) {
      if (RxBufSize > 254) badRxBufSize();
      if (TxBufSize > 254) badTxBufSize();
    }
    if (RxBufSize) rxRingBuf[PortNumber].init(rxBuffer_, sizeof(rxBuffer_));
    if (TxBufSize) txRingBuf[PortNumber].init(txBuffer_, sizeof(txBuffer_));
  }
  //----------------------------------------------------------------------------
  /**
   * @return The number of bytes (characters) available for reading from
   *  the serial port.
   */
  int available(void) {
    if (!RxBufSize) {
      return *usart[PortNumber].ucsra & M_RXC ? 1 : 0;
    } else {
      return rxRingBuf[PortNumber].available();
    }
  }
  //----------------------------------------------------------------------------
  /**
   * Sets the data rate in bits per second (baud) for serial data transmission.
   *
   * @param[in] baud Rate in bits per second (baud).
   * @param[in] options constructed by a bitwise-inclusive
   * OR of values from the following list.  Choose one value for stop bit,
   * parity, and character size.
   *
   * - SP_1_STOP_BIT - Use one stop bit (default if stop bit not specified).
   * - SP_2_STOP_BIT - Use two stop bits.
   * - SP_NO_PARITY - No parity bit (default if parity not specified).
   * - SP_EVEN_PARITY - Add even parity bit.
   * - SP_ODD_PARITY - Add odd parity bit.
   * - SP_5_BIT_CHAR - Use 5-bit characters (default if size not specified).
   * - SP_6_BIT_CHAR - Use 6-bit characters.
   * - SP_7_BIT_CHAR - Use 7-bit characters.
   * - SP_8_BIT_CHAR - Use 8-bit characters.
   * .
   * The default is SP_8_BIT_CHAR which results in one stop bit, no parity,
   * and 8-bit characters.
   */
  void begin(uint32_t baud, uint8_t options = SP_8_BIT_CHAR) {
    uint16_t baud_setting;

    // disable USART interrupts.  Set UCSRB to reset values.
    *usart[PortNumber].ucsrb = 0;

    // set option bits
    *usart[PortNumber].ucsrc = options & SP_OPT_MASK;

    baud_setting = F_CPU/4/baud;
    
    if (baud_setting > 8192 || (F_CPU == 16000000UL && baud == 57600)) {
      // Hardcoded exception for compatibility with the bootloader shipped
      // with the Duemilanove and previous boards and the firmware on the
      // 8U2/16U2 on the Uno and Mega 2560.    
      // Prevent overflow of 12-bit UBRR at 300 baud.    
      *usart[PortNumber].ucsra = 0;
      baud_setting /= 2;      
    } else {
      // Use U2X for better high baud rates.
      *usart[PortNumber].ucsra = M_U2X;
    }
    // Rounded value for datasheet expression.
    baud_setting = (baud_setting - 1)/2;    

    // assign the baud_setting, a.k.a. ubbr (USART Baud Rate Register)
    *usart[PortNumber].ubrrh = baud_setting >> 8;
    *usart[PortNumber].ubrrl = baud_setting;

    // enable RX and TX
    uint8_t bits = M_TXEN | M_RXEN;

    // enable receive interrupt if buffered
    if (RxBufSize) bits |= M_RXCIE;
    *usart[PortNumber].ucsrb = bits;
  }
  //----------------------------------------------------------------------------
  #if ENABLE_RX_ERROR_CHECKING
  /** Clear RX error bits. */
  void clearRxError() {rxErrorBits[PortNumber] = 0;}
  /** @return RX error bits. Possible error bits are:
   * - @ref SP_RX_BUF_OVERRUN
   * - @ref SP_RX_DATA_OVERRUN
   * - @ref SP_FRAMING_ERROR
   * - @ref SP_PARITY_ERROR
   * .
   */
  uint8_t getRxError() {return rxErrorBits[PortNumber];}
  #endif  // ENABLE_RX_ERROR_CHECKING
  //----------------------------------------------------------------------------
  /**
   * Disables serial communication, allowing the RX and TX pins to be used for
   * general input and output. To re-enable serial communication,
   * call SerialPort::begin().
   */
  void end() {
    // wait for transmission of outgoing data
    flushTx();
    // disable USART
    cli();
    *usart[PortNumber].ucsrb &= ~(M_RXEN | M_TXEN | M_RXCIE | M_UDRIE);
    sei();
    // clear any received data
    flushRx();
  }
  //----------------------------------------------------------------------------
  /**
   * For Arduino 1.0 and greater call flushTx().
   */
  void flush() {flushTx();}
  //----------------------------------------------------------------------------
  /**
   * Discard any buffered incoming serial data.
   */
  void flushRx() {
    if (RxBufSize) {
      rxRingBuf[PortNumber].flush();
    } else {
      uint8_t b;
      while (*usart[PortNumber].ucsra & M_RXC) b = *usart[PortNumber].udr;
    }
  }
  //----------------------------------------------------------------------------
  /**
   * Waits for the transmission of outgoing serial data to complete.
   */
  void flushTx() {
    if (TxBufSize) {
      while (!txRingBuf[PortNumber].empty()) {}
    }
  }
  //----------------------------------------------------------------------------
  /**
   * @return The first byte of incoming serial data available or
   *  -1 if no data is available.  -1 is always returned for unbuffered RX.
   */
  int peek(void) {
    return RxBufSize ? rxRingBuf[PortNumber].peek() : -1;
  }
  //----------------------------------------------------------------------------
  /**
   * Read incoming serial data.
   *
   * @return The first byte of incoming serial data available
   *  or -1 if no data is available.
   */
  __attribute__((noinline))
  int read() {
    if (!RxBufSize) {
      uint8_t s = *usart[PortNumber].ucsra;
  #if ENABLE_RX_ERROR_CHECKING
      rxErrorBits[PortNumber] |= s & SP_UCSRA_ERROR_MASK;
  #endif  // ENABLE_RX_ERROR_CHECKING
      return  s & M_RXC ? *usart[PortNumber].udr : -1;
    } else {
      uint8_t b;
      return rxRingBuf[PortNumber].get(&b) ? b : -1;
    }
  }
  //----------------------------------------------------------------------------
  /**
   * Read incoming serial data. Stop when RX buffer is empty or n
   * bytes have been read.
   *
   * @param[in] b The location to receive the data.
   * @param[in] n Maximum number of bytes to read.
   * @return The number of bytes read.
   */
  __attribute__((noinline))
  size_t read(uint8_t* b, size_t n) {
    uint8_t* limit = b + n;
    uint8_t* p = b;
    if (RxBufSize) {
      while (p < limit && !rxRingBuf[PortNumber].empty()) {
        size_t nr = limit - p;
        if (sizeof(SerialRingBuffer::buf_size_t) == 1 && nr > 255) nr = 255;
        p += rxRingBuf[PortNumber].get(p, nr);
      }
    } else {
      while (p < limit) {
        int rb = read();
        if (rb < 0) break;
        *p++ = rb;
      }
    }
    return p - b;
  }
  //----------------------------------------------------------------------------
  /**
   * Write binary data to the serial port.
   *
   * @param[in] b The byte to be written.
   * @return The number of bytes written to the serial port.
   */
  __attribute__((noinline))
  size_t write(uint8_t b) {
    if (!TxBufSize) {
      while (!(*usart[PortNumber].ucsra & M_UDRE)) {}
      *usart[PortNumber].udr = b;
    } else {
      // Wait for TX ISR if buffer is full.
      while (!txRingBuf[PortNumber].put(b)) {}

      // Enable interrupts.
      *usart[PortNumber].ucsrb |= M_UDRIE;
    }
    return 1;
  }
  //----------------------------------------------------------------------------
  /** Write CR/LF.
   * @return 2
   */
  __attribute__((noinline))
  size_t writeln() {
    write('\r');
    write('\n');
    return 2;
  }
  //----------------------------------------------------------------------------
  /**
   * Write a string to the serial port followed by CR/LF.
   *
   * @param[in] s The string to be written.
   * @return The number of bytes written to the serial port.
   */
  __attribute__((noinline))
  size_t writeln(const char* s) {
    return write(s) + writeln();
  }

  //----------------------------------------------------------------------------
  /**
   * Write binary data from flash memory to the serial port.
   *
   * @param[in] b Location of the bytes to be written.
   * @param[in] n The number of bytes to write.
   * @return The number of bytes written to the serial port.
   */
  __attribute__((noinline))
  size_t write_P(PGM_P b, size_t n) {
    if (!TxBufSize) {
      for (size_t i = 0; i < n; i++) write(pgm_read_byte(b + i));
    } else {
      size_t w = n;
      while (w) {
        size_t nw = w;
        if (sizeof(SerialRingBuffer::buf_size_t) == 1 && nw > 255) nw = 255;
        size_t m = txRingBuf[PortNumber].put_P(b, nw);

        // enable interrupts
        *usart[PortNumber].ucsrb |= M_UDRIE;
        w -= m;
        b += m;
      }
    }
    return n;
  }
  //----------------------------------------------------------------------------
  /**
   * Write a flash string to the serial port.
   *
   * @param[in] s The string to be written.
   * @return The number of bytes written to the serial port.
   */
  __attribute__((noinline))
  size_t write(const __FlashStringHelper* s) {
    const char PROGMEM* p = (const char PROGMEM*)s;
    size_t n = strlen_P(p);
    return write_P(p, n);
  }
  //----------------------------------------------------------------------------
  /**
   * Write a flash string to the serial port followed by CR/LF.
   *
   * @param[in] s The string to be written.
   * @return The number of bytes written to the serial port.
   */
  __attribute__((noinline))
  size_t writeln(const __FlashStringHelper* s) {
    return write(s) + writeln();
  }
  #if USE_WRITE_OVERRIDES
  //----------------------------------------------------------------------------
  /**
   * Write binary data to the serial port.
   *
   * @param[in] b Location of the bytes to be written.
   * @param[in] n The number of bytes to write.
   * @return The number of bytes written to the serial port.
   */
  __attribute__((noinline))
  size_t write(const uint8_t* b, size_t n) {
    if (!TxBufSize) {
      for (size_t i = 0; i < n; i++) write(b[i]);
    } else {
      size_t w = n;
      while (w) {
        size_t nw = w;
        if (sizeof(SerialRingBuffer::buf_size_t) == 1 && nw > 255) nw = 255;
        size_t m = txRingBuf[PortNumber].put(b, nw);

        // Enable interrupts.
        *usart[PortNumber].ucsrb |= M_UDRIE;
        w -= m;
        b += m;
      }
    }
    return n;
  }
  //----------------------------------------------------------------------------
  /**
   * Write a string to the serial port.
   *
   * @param[in] s The string to be written.
   * @return The number of bytes written to the serial port
   */
  __attribute__((noinline))
  size_t write(const char* s) {
    size_t n = strlen(s);
    return write(reinterpret_cast<const uint8_t*>(s), n);
  }
  #else  // USE_WRITE_OVERRIDES
  using Print::write;  // use write(str) and write(buf, size) from Print
  #endif  // USE_WRITE_OVERRIDES
  //----------------------------------------------------------------------------
 private:
  // RX buffer with a capacity of RxBufSize.
  uint8_t rxBuffer_[RxBufSize + 1];
  // TX buffer with a capacity of TxBufSize.
  uint8_t txBuffer_[TxBufSize + 1];
};
//------------------------------------------------------------------------------
#endif  // SerialPort_h
