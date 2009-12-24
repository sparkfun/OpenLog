
/*
 * Copyright (c) 2006-2009 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef SD_RAW_H
#define SD_RAW_H

#include <stdint.h>
#include "sd_raw_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \addtogroup sd_raw
 *
 * @{
 */
/**
 * \file
 * MMC/SD/SDHC raw access header (license: GPLv2 or LGPLv2.1)
 *
 * \author Roland Riegel
 */

/**
 * The card's layout is harddisk-like, which means it contains
 * a master boot record with a partition table.
 */
#define SD_RAW_FORMAT_HARDDISK 0
/**
 * The card contains a single filesystem and no partition table.
 */
#define SD_RAW_FORMAT_SUPERFLOPPY 1
/**
 * The card's layout follows the Universal File Format.
 */
#define SD_RAW_FORMAT_UNIVERSAL 2
/**
 * The card's layout is unknown.
 */
#define SD_RAW_FORMAT_UNKNOWN 3

/**
 * This struct is used by sd_raw_get_info() to return
 * manufacturing and status information of the card.
 */
struct sd_raw_info
{
    /**
     * A manufacturer code globally assigned by the SD card organization.
     */
    uint8_t manufacturer;
    /**
     * A string describing the card's OEM or content, globally assigned by the SD card organization.
     */
    uint8_t oem[3];
    /**
     * A product name.
     */
    uint8_t product[6];
    /**
     * The card's revision, coded in packed BCD.
     *
     * For example, the revision value \c 0x32 means "3.2".
     */
    uint8_t revision;
    /**
     * A serial number assigned by the manufacturer.
     */
    uint32_t serial;
    /**
     * The year of manufacturing.
     *
     * A value of zero means year 2000.
     */
    uint8_t manufacturing_year;
    /**
     * The month of manufacturing.
     */
    uint8_t manufacturing_month;
    /**
     * The card's total capacity in bytes.
     */
    offset_t capacity;
    /**
     * Defines wether the card's content is original or copied.
     *
     * A value of \c 0 means original, \c 1 means copied.
     */
    uint8_t flag_copy;
    /**
     * Defines wether the card's content is write-protected.
     *
     * \note This is an internal flag and does not represent the
     *       state of the card's mechanical write-protect switch.
     */
    uint8_t flag_write_protect;
    /**
     * Defines wether the card's content is temporarily write-protected.
     *
     * \note This is an internal flag and does not represent the
     *       state of the card's mechanical write-protect switch.
     */
    uint8_t flag_write_protect_temp;
    /**
     * The card's data layout.
     *
     * See the \c SD_RAW_FORMAT_* constants for details.
     *
     * \note This value is not guaranteed to match reality.
     */
    uint8_t format;
};

typedef uint8_t (*sd_raw_read_interval_handler_t)(uint8_t* buffer, offset_t offset, void* p);
typedef uintptr_t (*sd_raw_write_interval_handler_t)(uint8_t* buffer, offset_t offset, void* p);

uint8_t sd_raw_init(void);
uint8_t sd_raw_available(void);
uint8_t sd_raw_locked(void);

uint8_t sd_raw_read(offset_t offset, uint8_t* buffer, uintptr_t length);
uint8_t sd_raw_read_interval(offset_t offset, uint8_t* buffer, uintptr_t interval, uintptr_t length, sd_raw_read_interval_handler_t callback, void* p);
uint8_t sd_raw_write(offset_t offset, const uint8_t* buffer, uintptr_t length);
uint8_t sd_raw_write_interval(offset_t offset, uint8_t* buffer, uintptr_t length, sd_raw_write_interval_handler_t callback, void* p);
uint8_t sd_raw_sync(void);

uint8_t sd_raw_get_info(struct sd_raw_info* info);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif

