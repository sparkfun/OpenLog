
/*
 * Copyright (c) 2006-2009 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef FAT_CONFIG_H
#define FAT_CONFIG_H

#include <stdint.h>
#include "sd_raw_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \addtogroup fat
 *
 * @{
 */
/**
 * \file
 * FAT configuration (license: GPLv2 or LGPLv2.1)
 */

/**
 * \ingroup fat_config
 * Controls FAT write support.
 *
 * Set to 1 to enable FAT write support, set to 0 to disable it.
 */
#define FAT_WRITE_SUPPORT 1

/**
 * \ingroup fat_config
 * Controls FAT date and time support.
 * 
 * Set to 1 to enable FAT date and time stamping support.
 */
#define FAT_DATETIME_SUPPORT 0

/**
 * \ingroup fat_config
 * Controls FAT32 support.
 *
 * Set to 1 to enable FAT32 support.
 */
#define FAT_FAT32_SUPPORT SD_RAW_SDHC

/**
 * \ingroup fat_config
 * Determines the function used for retrieving current date and time.
 *
 * Define this to the function call which shall be used to retrieve
 * current date and time.
 *
 * \note Used only when FAT_DATETIME_SUPPORT is 1.
 *
 * \param[out] year Pointer to a \c uint16_t which receives the current year.
 * \param[out] month Pointer to a \c uint8_t which receives the current month.
 * \param[out] day Pointer to a \c uint8_t which receives the current day.
 * \param[out] hour Pointer to a \c uint8_t which receives the current hour.
 * \param[out] min Pointer to a \c uint8_t which receives the current minute.
 * \param[out] sec Pointer to a \c uint8_t which receives the current sec.
 */
#define fat_get_datetime(year, month, day, hour, min, sec) \
    get_datetime(year, month, day, hour, min, sec)
/* forward declaration for the above */
void get_datetime(uint16_t* year, uint8_t* month, uint8_t* day, uint8_t* hour, uint8_t* min, uint8_t* sec);

/**
 * \ingroup fat_config
 * Maximum number of filesystem handles.
 */
#define FAT_FS_COUNT 1

/**
 * \ingroup fat_config
 * Maximum number of file handles.
 */
#define FAT_FILE_COUNT 1

/**
 * \ingroup fat_config
 * Maximum number of directory handles.
 */
#define FAT_DIR_COUNT 2

/**
 * @}
 */

#if FAT_FAT32_SUPPORT
    typedef uint32_t cluster_t;
#else
    typedef uint16_t cluster_t;
#endif

#ifdef __cplusplus
}
#endif

#endif

