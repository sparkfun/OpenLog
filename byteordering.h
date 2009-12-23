
/*
 * Copyright (c) 2006-2009 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef BYTEORDERING_H
#define BYTEORDERING_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \addtogroup byteordering
 *
 * @{
 */
/**
 * \file
 * Byte-order handling header (license: GPLv2 or LGPLv2.1)
 *
 * \author Roland Riegel
 */

/**
 * \def HTOL16(val)
 *
 * Converts a 16-bit integer to little-endian byte order.
 *
 * Use this macro for compile time constants only. For variable values
 * use the function htol16() instead. This saves code size.
 *
 * \param[in] val A 16-bit integer in host byte order.
 * \returns The given 16-bit integer converted to little-endian byte order.
 */
/**
 * \def HTOL32(val)
 *
 * Converts a 32-bit integer to little-endian byte order.
 *
 * Use this macro for compile time constants only. For variable values
 * use the function htol32() instead. This saves code size.
 *
 * \param[in] val A 32-bit integer in host byte order.
 * \returns The given 32-bit integer converted to little-endian byte order.
 */

#if DOXYGEN || LITTLE_ENDIAN || __AVR__
#define HTOL16(val) (val)
#define HTOL32(val) (val)
#elif BIG_ENDIAN
#define HTOL16(val) ((((uint16_t) (val)) << 8) | \
                     (((uint16_t) (val)) >> 8)   \
                    )
#define HTOL32(val) (((((uint32_t) (val)) & 0x000000ff) << 24) | \
                     ((((uint32_t) (val)) & 0x0000ff00) <<  8) | \
                     ((((uint32_t) (val)) & 0x00ff0000) >>  8) | \
                     ((((uint32_t) (val)) & 0xff000000) >> 24)   \
                    )
#else
#error "Endianess undefined! Please define LITTLE_ENDIAN=1 or BIG_ENDIAN=1."
#endif

uint16_t htol16(uint16_t h);
uint32_t htol32(uint32_t h);

/**
 * Converts a 16-bit integer to host byte order.
 *
 * Use this macro for compile time constants only. For variable values
 * use the function ltoh16() instead. This saves code size.
 *
 * \param[in] val A 16-bit integer in little-endian byte order.
 * \returns The given 16-bit integer converted to host byte order.
 */
#define LTOH16(val) HTOL16(val)

/**
 * Converts a 32-bit integer to host byte order.
 *
 * Use this macro for compile time constants only. For variable values
 * use the function ltoh32() instead. This saves code size.
 *
 * \param[in] val A 32-bit integer in little-endian byte order.
 * \returns The given 32-bit integer converted to host byte order.
 */
#define LTOH32(val) HTOL32(val)

/**
 * Converts a 16-bit integer to host byte order.
 *
 * Use this function on variable values instead of the
 * macro LTOH16(). This saves code size.
 *
 * \param[in] l A 16-bit integer in little-endian byte order.
 * \returns The given 16-bit integer converted to host byte order.
 */
#if DOXYGEN
uint16_t ltoh16(uint16_t l);
#else
#define ltoh16(l) htol16(l)
#endif

/**
 * Converts a 32-bit integer to host byte order.
 *
 * Use this function on variable values instead of the
 * macro LTOH32(). This saves code size.
 *
 * \param[in] l A 32-bit integer in little-endian byte order.
 * \returns The given 32-bit integer converted to host byte order.
 */
#if DOXYGEN
uint32_t ltoh32(uint32_t l);
#else
#define ltoh32(l) htol32(l)
#endif

/**
 * @}
 */

#if LITTLE_ENDIAN || __AVR__
#define htol16(h) (h)
#define htol32(h) (h)
#else
uint16_t htol16(uint16_t h);
uint32_t htol32(uint32_t h);
#endif

#ifdef __cplusplus
}
#endif

#endif

