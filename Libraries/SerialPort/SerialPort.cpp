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
#include <SerialPort.h>
//------------------------------------------------------------------------------
/** \return the number of bytes in the ring buffer */
int SerialRingBuffer::available() {
  uint8_t s = SREG;
  cli();
  int n = head_ - tail_;
  SREG = s;
  return n < 0 ? size_ + n : n;
}
//------------------------------------------------------------------------------
/** Discard all data in the ring buffer. */
void SerialRingBuffer::flush() {
  uint8_t s = SREG;
  cli();
  head_ = tail_ = 0;
  SREG = s;
}
//------------------------------------------------------------------------------
/** get the next byte
 * \param[in] b location for the returned byte
 * \return true if a byte was returned or false if the ring buffer is empty
 */
bool SerialRingBuffer::get(uint8_t* b) {
  buf_size_t t = tail_;
  if (head_ == t) return false;
  *b = buf_[t++];
  tail_ = t < size_ ? t : 0;
  return true;
}
//------------------------------------------------------------------------------
/**
 * Get the maximum number of contiguous bytes from the ring buffer
 * with one call to memcpy. Do not use this function with interrupts
 * disabled.
 *
 * \param[in] b pointer to data
 * \param[in] n number of bytes to transfer from the ring buffer
 * \return number of bytes transferred
 */
SerialRingBuffer::buf_size_t SerialRingBuffer::get(uint8_t* b, buf_size_t n) {
  buf_size_t nr;
  cli();
  buf_size_t h = head_;
  sei();
  buf_size_t t = tail_;
  if (h < t) {
    nr = size_ - t;
  } else if (t < h) {
    nr = h - t;
  } else {
    return 0;
  }
  if (nr > n) nr = n;
  memcpy(b, &buf_[t], nr);
  t += nr;
  tail_ = t < size_ ? t : t - size_;
  return nr;
}
//------------------------------------------------------------------------------
/** initialize the ring buffer
 * \param[in] b buffer for data
 * \param[in] s size of the buffer
 */
void SerialRingBuffer::init(uint8_t* b, buf_size_t s) {
  buf_ = b;
  size_ = s;
  head_ = tail_ = 0;
}
//------------------------------------------------------------------------------
/** peek at the next byte in the ring buffer
 * \return the next byte that would ber read or -1 if the ring buffer is empty
 */
int SerialRingBuffer::peek() {
  return empty() ? -1 : buf_[tail_];
}
//------------------------------------------------------------------------------
/** put a byte into the ring buffer
 * \param[in] b the byte
 * \return true if byte was transferred or false if the ring buffer is full
 */
bool SerialRingBuffer::put(uint8_t b) {
  buf_size_t h = head_;
  // OK to store here even if ring is full
  buf_[h++] = b;
  if (h >= size_) h = 0;
  if (h == tail_) return false;
  head_ = h;
  return true;
}
//------------------------------------------------------------------------------
/**
 * Put the maximum number of contiguous bytes into the ring buffer
 * with one call to memcpy.
 *
 * \param[in] b pointer to data
 * \param[in] n number of bytes to transfer to the ring buffer
 * \return number of bytes transferred
 */
SerialRingBuffer::buf_size_t
  SerialRingBuffer::put(const uint8_t* b, buf_size_t n) {
  cli();
  buf_size_t t = tail_;
  sei();
  buf_size_t space;  // space in ring buffer
  buf_size_t h = head_;
  if (h < t) {
    space = t - h - 1;
  } else {
    space = size_ - h;
    if (t == 0) space -= 1;
  }
  if (n > space) n = space;
  memcpy(&buf_[h], b, n);
  h += n;
  head_ = h < size_ ? h : h - size_;
  return n;
}
//------------------------------------------------------------------------------
/**
 * Put the maximum number of contiguous bytes into the ring buffer
 * with one call to memcpy.
 *
 * \param[in] b pointer to data
 * \param[in] n number of bytes to transfer to the ring buffer
 * \return number of bytes transferred
 */
SerialRingBuffer::buf_size_t SerialRingBuffer::put_P(PGM_P b, buf_size_t n) {
  cli();
  buf_size_t t = tail_;
  sei();
  buf_size_t space;  // space in ring buffer
  buf_size_t h = head_;
  if (h < t) {
    space = t - h - 1;
  } else {
    space = size_ - h;
    if (t == 0) space -= 1;
  }
  if (n > space) n = space;
  memcpy_P(&buf_[h], b, n);
  h += n;
  head_ = h < size_ ? h : h - size_;
  return n;
}
//==============================================================================
// global data and ISRs
#if ENABLE_RX_ERROR_CHECKING
//
uint8_t rxErrorBits[SERIAL_PORT_COUNT];
#endif  // ENABLE_RX_ERROR_CHECKING
//------------------------------------------------------------------------------
#if BUFFERED_RX
//------------------------------------------------------------------------------
SerialRingBuffer rxRingBuf[SERIAL_PORT_COUNT];
//------------------------------------------------------------------------------
#if ENABLE_RX_ERROR_CHECKING
inline static void rx_isr(uint8_t n) {
  uint8_t e = *usart[n].ucsra & SP_UCSRA_ERROR_MASK;
  uint8_t b = *usart[n].udr;
  if (!rxRingBuf[n].put(b)) e |= SP_RX_BUF_OVERRUN;
  rxErrorBits[n] |= e;
}
#else  // ENABLE_RX_ERROR_CHECKING
inline static void rx_isr(uint8_t n) {
  uint8_t b = *usart[n].udr;
  rxRingBuf[n].put(b);
}
#endif  // ENABLE_RX_ERROR_CHECKING
//------------------------------------------------------------------------------
// SerialRingBuffer rxbuf0;
#if defined(USART_RX_vect)
ISR(USART_RX_vect) {
#elif defined(SIG_USART0_RECV)
ISR(SIG_USART0_RECV) {
#elif defined(SIG_UART0_RECV)
ISR(SIG_UART0_RECV) {
#elif defined(USART0_RX_vect)
ISR(USART0_RX_vect) {
#elif defined(SIG_UART_RECV)
ISR(SIG_UART_RECV) {
#else  // vector
#error No ISR rx vector for UART0
#endif  // vector
  rx_isr(0);
}
#ifdef USART1_RX_vect
ISR(USART1_RX_vect) {
  rx_isr(1);
}
#endif  // USART1_RX_vect

#ifdef USART2_RX_vect
ISR(USART2_RX_vect) {
  rx_isr(2);
}
#endif  // USART2_RX_vect

#ifdef USART3_RX_vect
ISR(USART3_RX_vect) {
  rx_isr(3);
}
#endif  // USART3_RX_vect
#endif  // BUFFERED_RX
//------------------------------------------------------------------------------
#if BUFFERED_TX
//------------------------------------------------------------------------------
SerialRingBuffer txRingBuf[SERIAL_PORT_COUNT];
//------------------------------------------------------------------------------
inline static void tx_isr(uint8_t n) {
  uint8_t b;
  if (txRingBuf[n].get(&b)) {
    *usart[n].udr = b;
  } else {
    // no data - disable interrupts
    *usart[n].ucsrb &= ~M_UDRIE;
  }
}
#if defined(UART0_UDRE_vect)
ISR(UART0_UDRE_vect) {
#elif defined(UART_UDRE_vect)
ISR(UART_UDRE_vect) {
#elif defined(USART0_UDRE_vect)
ISR(USART0_UDRE_vect) {
#elif defined(USART_UDRE_vect)
ISR(USART_UDRE_vect) {
#else
#error N0 ISR tx vector for UART0
#endif
  tx_isr(0);
}
#ifdef USART1_UDRE_vect
ISR(USART1_UDRE_vect) {
  tx_isr(1);
}
#endif  // USART1_UDRE_vect

#ifdef USART2_UDRE_vect
ISR(USART2_UDRE_vect) {
  tx_isr(2);
}
#endif  // USART2_UDRE_vect

#ifdef USART3_UDRE_vect
ISR(USART3_UDRE_vect) {
  tx_isr(3);
}
#endif  // USART3_UDRE_vect
#endif  // BUFFERED_TX
