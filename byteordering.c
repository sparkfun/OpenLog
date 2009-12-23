
/*
 * Copyright (c) 2006-2009 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#include "byteordering.h"

/**
 * \addtogroup byteordering
 *
 * Architecture-dependent handling of byte-ordering.
 *
 * @{
 */
/**
 * \file
 * Byte-order handling implementation (license: GPLv2 or LGPLv2.1)
 *
 * \author Roland Riegel
 */

#if DOXYGEN || !(LITTLE_ENDIAN || __AVR__)
/**
 * Converts a 16-bit integer to little-endian byte order.
 *
 * Use this function on variable values instead of the
 * macro HTOL16(). This saves code size.
 *
 * \param[in] h A 16-bit integer in host byte order.
 * \returns The given 16-bit integer converted to little-endian byte order.
 */
uint16_t htol16(uint16_t h)
{
    return HTOL16(h);
}
#endif

#if DOXYGEN || !(LITTLE_ENDIAN || __AVR__)
/**
 * Converts a 32-bit integer to little-endian byte order.
 *
 * Use this function on variable values instead of the
 * macro HTOL32(). This saves code size.
 *
 * \param[in] h A 32-bit integer in host byte order.
 * \returns The given 32-bit integer converted to little-endian byte order.
 */
uint32_t htol32(uint32_t h)
{
    return HTOL32(h);
}
#endif

/**
 * @}
 */

