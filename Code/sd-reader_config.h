
/*
 * Copyright (c) 2006-2009 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef SD_READER_CONFIG_H
#define SD_READER_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \addtogroup config Sd-reader configuration
 *
 * @{
 */

/**
 * \file
 * Common sd-reader configuration used by all modules (license: GPLv2 or LGPLv2.1)
 *
 * \note This file contains only configuration items relevant to
 * all sd-reader implementation files. For module specific configuration
 * options, please see the files fat_config.h, partition_config.h
 * and sd_raw_config.h.
 */

/**
 * Controls allocation of memory.
 *
 * Set to 1 to use malloc()/free() for allocation of structures
 * like file and directory handles, set to 0 to use pre-allocated
 * fixed-size handle arrays.
 */
//Default = 0
#define USE_DYNAMIC_MEMORY 0
//#define USE_DYNAMIC_MEMORY 1

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif

