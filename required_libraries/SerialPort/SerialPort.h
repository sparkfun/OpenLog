/* Arduino SerialPort Library
 * Copyright (C) 2011 by William Greiman
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
 * \file
 * \brief SerialPort class
 */
#ifndef SerialPort_h
#define SerialPort_h
//------------------------------------------------------------------------------
/** SerialPort version YYYYMMDD */
#define SERIAL_PORT_VERSION 20120106
//------------------------------------------------------------------------------
/**
 * Set ALLOW_LARGE_BUFFERS to zero to limit buffer sizes to 254 bytes.
 * ALLOW_LARGE_BUFFERS controls whether uint16_t or uint8_t will be
 * used for buffer indices.
 */
#define ALLOW_LARGE_BUFFERS 1
//------------------------------------------------------------------------------
/**
 * Set USE_WRITE_OVERRIDES to zero to use the Arduino Print version
 * of write(const char*) and write(const uint8_t*, size_t).  This will
 * save some flash but is much slower.
 */
#define USE_WRITE_OVERRIDES 1
//------------------------------------------------------------------------------
/**
 * Set BUFFERED_RX zero to save flash and RAM if no RX buffering is used.
 * RxBufSize must be zero in all SerialPort constructors if
 * BUFFERED_RX is zero.
 */
#define BUFFERED_RX 1
//------------------------------------------------------------------------------
/**
 * Set BUFFERED_TX zero to save flash and RAM if no TX buffering is used.
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
#if ARDUINO < 100
#include <WProgram.h>
class __FlashStringHelper;
#define F(string_literal)\
        (reinterpret_cast<__FlashStringHelper *>(PSTR(string_literal)))
#else  // ARDUINO < 100
#include <Arduino.h>
#endif  // ARDUINO < 100
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
#error no serial ports
#endif
//------------------------------------------------------------------------------
#ifdef UCSR0A
// bits in UCSRA
static const uint8_t M_RXC  = 1 << RXC0;
static const uint8_t M_TXC  = 1 << TXC0;
static const uint8_t M_UDRE = 1 << UDRE0;
static const uint8_t M_FE   = 1 << FE0;
static const uint8_t M_DOR  = 1 << DOR0;
static const uint8_t M_UPE  = 1 << UPE0;
static const uint8_t M_U2X  = 1 << U2X0;
// bits in UCSRB
static const uint8_t M_RXCIE = 1 << RXCIE0;
static const uint8_t M_TXCIE = 1 << TXCIE0;
static const uint8_t M_UDRIE = 1 << UDRIE0;
static const uint8_t M_RXEN  = 1 << RXEN0;
static const uint8_t M_TXEN  = 1 << TXEN0;
// bits in UCSRC
static const uint8_t M_UPM0 = 1 << UPM00;
static const uint8_t M_UPM1 = 1 << UPM01;
static const uint8_t M_USBS = 1 << USBS0;
static const uint8_t M_UCSZ0 = 1 << UCSZ00;
static const uint8_t M_UCSZ1 = 1 << UCSZ01;
#elif defined(UCSRA)  // UCSR0A
// bits in UCSRA
static const uint8_t M_RXC  = 1 << RXC;
static const uint8_t M_TXC  = 1 << TXC;
static const uint8_t M_UDRE = 1 << UDRE;
static const uint8_t M_FE   = 1 << FE;
static const uint8_t M_DOR  = 1 << DOR;
static const uint8_t M_UPE  = 1 << PE;
static const uint8_t M_U2X  = 1 << U2X;
// bits in UCSRB
static const uint8_t M_RXCIE = 1 << RXCIE;
static const uint8_t M_TXCIE = 1 << TXCIE;
static const uint8_t M_UDRIE = 1 << UDRIE;
static const uint8_t M_RXEN  = 1 << RXEN;
static const uint8_t M_TXEN  = 1 << TXEN;
// bits in UCSRC
static const uint8_t M_UPM0 = 1 << UPM0;
static const uint8_t M_UPM1 = 1 << UPM1;
static const uint8_t M_USBS = 1 << USBS;
static const uint8_t M_UCSZ0 = 1 << UCSZ0;
static const uint8_t M_UCSZ1 = 1 << UCSZ1;
#else  // UCSR0A
#error no serial ports
#endif  // UCSR0A
//------------------------------------------------------------------------------
/** use one stop bit */
static const uint8_t SP_1_STOP_BIT = 0;
/** use two stop bits */
static const uint8_t SP_2_STOP_BIT = M_USBS;

/** disable parity bit */
static const uint8_t SP_NO_PARITY = 0;
/** use even parity */
static const uint8_t SP_EVEN_PARITY = M_UPM1;
/** use odd parity */
static const uint8_t SP_ODD_PARITY = M_UPM0 | M_UPM1;

/** use 5-bit character size */
static const uint8_t SP_5_BIT_CHAR = 0;
/** use 6-bit character size */
static const uint8_t SP_6_BIT_CHAR = M_UCSZ0;
/** use 7-bit character size */
static const uint8_t SP_7_BIT_CHAR = M_UCSZ1;
/** use 8-bit character size */
static const uint8_t SP_8_BIT_CHAR = M_UCSZ0 | M_UCSZ1;
/** mask for all options bits */
static const uint8_t SP_OPT_MASK = M_USBS | M_UPM0 | M_UPM1 |M_UCSZ0 | M_UCSZ1;

/** USART frame error bit */
static const uint8_t SP_FRAMING_ERROR    = M_FE;
/** USART RX data overrun error bit */
static const uint8_t SP_RX_DATA_OVERRUN  = M_DOR;
/** USART parity error bit */
static const uint8_t SP_PARITY_ERROR     = M_UPE;
/** mask for all error bits in UCSRA */
static const uint8_t SP_UCSRA_ERROR_MASK = M_FE | M_DOR | M_UPE;
/** RX ring buffer full overrun */
static const uint8_t SP_RX_BUF_OVERRUN  = 1;
//------------------------------------------------------------------------------
/**
 * \class UsartRegister
 * \brief addresses of USART registers
 */
struct UsartRegister {
  volatile uint8_t* ucsra;  /**< USART Control and Status Register A */
  volatile uint8_t* ucsrb;  /**< USART Control and Status Register B */
  volatile uint8_t* ucsrc;  /**< USART Control and Status Register C */
  volatile uint8_t* ubrrl;  /**< USART Baud Rate Register Low */
  volatile uint8_t* ubrrh;  /**< USART Baud Rate Register High */
  volatile uint8_t* udr;    /**< USART I/O Data Register */
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
 * \class SerialRingBuffer
 * \brief ring buffer for RX and TX data
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
  /** \return true if the ring buffer is empty else false */
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
/** RX ring buffers */
extern SerialRingBuffer rxRingBuf[];
/** TX ring buffers */
extern SerialRingBuffer txRingBuf[];
/** RX error bits */
extern uint8_t rxErrorBits[];
//------------------------------------------------------------------------------
/** Cause error message for bad port number
 * \return Never returns since it is never called
 */
uint8_t badPortNumber(void)
  __attribute__((error("Bad port number")));
/** Cause error message for bad port number
 * \return Never returns since it is never called
 */
uint8_t badRxBufSize(void)
  __attribute__((error("RX buffer size too large")));
/** Cause error message for bad port number
 * \return Never returns since it is never called
 */
uint8_t badTxBufSize(void)
  __attribute__((error("TX buffer size too large")));
//------------------------------------------------------------------------------
/**
 * \class SerialPort
 * \brief class for avr hardware USART ports
 */
template<uint8_t PortNumber, size_t RxBufSize, size_t TxBufSize>
class SerialPort : public Stream {
 public:
  //----------------------------------------------------------------------------
  SerialPort() {
    if (PortNumber >= SERIAL_PORT_COUNT) badPortNumber();
    if (sizeof(SerialRingBuffer::buf_size_t) == 1) {
      if (RxBufSize >254) badRxBufSize();
      if (TxBufSize >254) badTxBufSize();
    }
    if (RxBufSize) rxRingBuf[PortNumber].init(rxBuffer_, sizeof(rxBuffer_));
    if (TxBufSize) txRingBuf[PortNumber].init(txBuffer_, sizeof(txBuffer_));
  }
  //----------------------------------------------------------------------------
  /**
   * \return The number of bytes (characters) available for reading from
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
   * \param[in] baud rate in bits per second (baud)
   * \param[in] options constructed by a bitwise-inclusive
   * OR of values from the following list.  Choose one value for stop bit,
   * parity, and character size.
   *
   * The default is SP_8_BIT_CHAR which results in one stop bit, no parity,
   * and 8-bit characters.
   *
   * SP_1_STOP_BIT - use one stop bit (default if stop bit not specified)
   *
   * SP_2_STOP_BIT - use two stop bits
   *
   * SP_NO_PARITY - no parity bit (default if parity not specified)
   *
   * SP_EVEN_PARITY - add even parity bit
   *
   * SP_ODD_PARITY - add odd parity bit
   *
   * SP_5_BIT_CHAR - use 5-bit characters (default if size not specified)
   *
   * SP_6_BIT_CHAR - use 6-bit characters
   *
   * SP_7_BIT_CHAR - use 7-bit characters
   *
   * SP_8_BIT_CHAR - use 8-bit characters
   */
  void begin(uint32_t baud, uint8_t options = SP_8_BIT_CHAR) {
    uint16_t baud_setting;

    // disable USART interrupts.  Set UCSRB to reset values.
    *usart[PortNumber].ucsrb = 0;

    // set option bits
    *usart[PortNumber].ucsrc = options & SP_OPT_MASK;

    if (F_CPU == 16000000UL && baud == 57600) {
      // hardcoded exception for compatibility with the bootloader shipped
      // with the Duemilanove and previous boards and the firmware on the 8U2
      // on the Uno and Mega 2560.
      *usart[PortNumber].ucsra = 0;
      baud_setting = (F_CPU / 8 / baud - 1) / 2;
    } else {
      *usart[PortNumber].ucsra = M_U2X;
      baud_setting = (F_CPU / 4 / baud - 1) / 2;
    }
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
  /** clear RX error bits */
  void clearRxError() {rxErrorBits[PortNumber] = 0;}
  /** \return RX error bits */
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
   * For Arduino 0023 and before call flushRx().
   */
  #if ARDUINO < 100
  void flush() {flushRx();}
  #else  // ARDUINO < 100
  void flush() {flushTx();}
  #endif  // ARDUINO < 100
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
   * \return the first byte of incoming serial data available or
   *  -1 if no data is available.  Peek() always return -1 for unbuffered RX.
   */
  int peek(void) {
    return RxBufSize ? rxRingBuf[PortNumber].peek() : -1;
  }
  //----------------------------------------------------------------------------
  /**
   * Read incoming serial data.
   *
   * \return the first byte of incoming serial data available
   *  or -1 if no data is available
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
   * \param[in] b location to receive the data
   * \param[in] n maximum number of bytes to read
   * \return number of bytes read
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
      return p - b;
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
   * \param[in] b byte to be written.
   * \return number of bytes written to the serial port
   */
  __attribute__((noinline))
  #if ARDUINO < 100
  void write(uint8_t b) {
  #else  // ARDUINO < 100
  size_t write(uint8_t b) {
  #endif  // ARDUINO < 100
    if (!TxBufSize) {
      while (!(*usart[PortNumber].ucsra & M_UDRE)) {}
      *usart[PortNumber].udr = b;
    } else {
      // wait for TX ISR if buffer is full
      while (!txRingBuf[PortNumber].put(b)) {}
      // enable interrupts
      *usart[PortNumber].ucsrb |= M_UDRIE;
    }
    #if ARDUINO > 99
    return 1;
    #endif  // ARDUINO > 99
  }
  //----------------------------------------------------------------------------
  /** write CR LF
   * \return 2
   */
  __attribute__((noinline))
  #if ARDUINO < 100
  void writeln() {
    write('\r');
    write('\n');
  }
  #else  // ARDUINO < 100
  size_t writeln() {
    write('\r');
    write('\n');
    return 2;
  }
  #endif  // ARDUINO >= 100
  //----------------------------------------------------------------------------
  /**
   * Write a string to the serial port followed by CF LF
   *
   * \param[in] s string to be written.
   * \return number of bytes written to the serial port
   */
  __attribute__((noinline))
  #if ARDUINO < 100
  void writeln(const char* s) {
    write(s);
    writeln();
  }
  #else  // ARDUINO < 100
  size_t writeln(const char* s) {
    return write(s) + writeln();
  }
  #endif  // ARDUINO >= 100
  //----------------------------------------------------------------------------
  /**
   * Write binary data from flash memory to the serial port.
   *
   * \param[in] b bytes to be written
   * \param[in] n number of bytes to write
   * \return number of bytes written to the serial port
   */
  __attribute__((noinline))
  #if ARDUINO < 100
  void write_P(PGM_P b, size_t n) {
  #else  // ARDUINO < 100
  size_t write_P(PGM_P b, size_t n) {
  #endif  // ARDUINO < 100
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
  #if ARDUINO >= 100
    return n;
  #endif  // ARDUINO >= 100
  }
  //----------------------------------------------------------------------------
  /**
   * Write a flash string to the serial port.
   *
   * \param[in] s string to be written.
   * \return number of bytes written to the serial port
   */
  __attribute__((noinline))
  #if ARDUINO < 100
  void write(const __FlashStringHelper* s) {
    const prog_char* p = (const prog_char*)s;
    size_t n = strlen_P(p);
    write_P(p, n);
  }
  #else  // ARDUINO < 100
  size_t write(const __FlashStringHelper* s) {
    const prog_char* p = (const prog_char*)s;
    size_t n = strlen_P(p);
    return write_P(p, n);
  }
  #endif  // ARDUINO >= 100
  //----------------------------------------------------------------------------
  /**
   * Write a flash string to the serial port followed by CF LF
   *
   * \param[in] s string to be written.
   * \return number of bytes written to the serial port
   */
  __attribute__((noinline))
  #if ARDUINO < 100
  void writeln(const __FlashStringHelper* s) {
    write(s);
    writeln();
  }
  #else  // ARDUINO < 100
  size_t writeln(const __FlashStringHelper* s) {
    return write(s) + writeln();
  }
  #endif  // ARDUINO >= 100
  #if USE_WRITE_OVERRIDES
  //----------------------------------------------------------------------------
  /**
   * Write binary data to the serial port.
   *
   * \param[in] b bytes to be written
   * \param[in] n number of bytes to write
   * \return number of bytes written to the serial port
   */
  __attribute__((noinline))
  #if ARDUINO < 100
  void write(const uint8_t* b, size_t n) {
  #else  // ARDUINO < 100
  size_t write(const uint8_t* b, size_t n) {
  #endif  // ARDUINO < 100
    if (!TxBufSize) {
      for (size_t i = 0; i < n; i++) write(b[i]);
    } else {
      size_t w = n;
      while (w) {
        size_t nw = w;
        if (sizeof(SerialRingBuffer::buf_size_t) == 1 && nw > 255) nw = 255;
        size_t m = txRingBuf[PortNumber].put(b, nw);
        // enable interrupts
        *usart[PortNumber].ucsrb |= M_UDRIE;
        w -= m;
        b += m;
      }
    }
  #if ARDUINO >= 100
    return n;
  #endif  // ARDUINO >= 100
  }
  //----------------------------------------------------------------------------
  /**
   * Write a string to the serial port.
   *
   * \param[in] s string to be written.
   * \return number of bytes written to the serial port
   */
  __attribute__((noinline))
  #if ARDUINO < 100
  void write(const char* s) {
    size_t n = strlen(s);
    write(reinterpret_cast<const uint8_t*>(s), n);
  }
  #else  // ARDUINO < 100
  size_t write(const char* s) {
    size_t n = strlen(s);
    return write(reinterpret_cast<const uint8_t*>(s), n);
  }
  #endif  // ARDUINO >= 100
  #else  // USE_WRITE_OVERRIDES
  using Print::write;  // use write(str) and write(buf, size) from Print
  #endif  // USE_WRITE_OVERRIDES
  //----------------------------------------------------------------------------
 private:
  // RX buffer with a capacity of RxBufSize.
  uint8_t rxBuffer_[RxBufSize + 1];
  // TX buffer with a capacity of TxBufSize
  uint8_t txBuffer_[TxBufSize + 1];
};
//------------------------------------------------------------------------------
#endif  // SerialPort_h
