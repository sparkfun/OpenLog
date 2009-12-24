
/*
 * Copyright (c) 2006-2009 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef PARTITION_H
#define PARTITION_H

#include <stdint.h>
#include "sd_raw_config.h"
#include "partition_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \addtogroup partition
 *
 * @{
 */
/**
 * \file
 * Partition table header (license: GPLv2 or LGPLv2.1)
 *
 * \author Roland Riegel
 */

/**
 * The partition table entry is not used.
 */
#define PARTITION_TYPE_FREE 0x00
/**
 * The partition contains a FAT12 filesystem.
 */
#define PARTITION_TYPE_FAT12 0x01
/**
 * The partition contains a FAT16 filesystem with 32MB maximum.
 */
#define PARTITION_TYPE_FAT16_32MB 0x04
/**
 * The partition is an extended partition with its own partition table.
 */
#define PARTITION_TYPE_EXTENDED 0x05
/**
 * The partition contains a FAT16 filesystem.
 */
#define PARTITION_TYPE_FAT16 0x06
/**
 * The partition contains a FAT32 filesystem.
 */
#define PARTITION_TYPE_FAT32 0x0b
/**
 * The partition contains a FAT32 filesystem with LBA.
 */
#define PARTITION_TYPE_FAT32_LBA 0x0c
/**
 * The partition contains a FAT16 filesystem with LBA.
 */
#define PARTITION_TYPE_FAT16_LBA 0x0e
/**
 * The partition is an extended partition with LBA.
 */
#define PARTITION_TYPE_EXTENDED_LBA 0x0f
/**
 * The partition has an unknown type.
 */
#define PARTITION_TYPE_UNKNOWN 0xff

/**
 * A function pointer used to read from the partition.
 *
 * \param[in] offset The offset on the device where to start reading.
 * \param[out] buffer The buffer into which to place the data.
 * \param[in] length The count of bytes to read.
 */
typedef uint8_t (*device_read_t)(offset_t offset, uint8_t* buffer, uintptr_t length);
/**
 * A function pointer passed to a \c device_read_interval_t.
 *
 * \param[in] buffer The buffer which contains the data just read.
 * \param[in] offset The offset from which the data in \c buffer was read.
 * \param[in] p An opaque pointer.
 * \see device_read_interval_t
 */
typedef uint8_t (*device_read_callback_t)(uint8_t* buffer, offset_t offset, void* p);
/**
 * A function pointer used to continuously read units of \c interval bytes
 * and call a callback function.
 *
 * This function starts reading at the specified offset. Every \c interval bytes,
 * it calls the callback function with the associated data buffer.
 *
 * By returning zero, the callback may stop reading.
 *
 * \param[in] offset Offset from which to start reading.
 * \param[in] buffer Pointer to a buffer which is at least interval bytes in size.
 * \param[in] interval Number of bytes to read before calling the callback function.
 * \param[in] length Number of bytes to read altogether.
 * \param[in] callback The function to call every interval bytes.
 * \param[in] p An opaque pointer directly passed to the callback function.
 * \returns 0 on failure, 1 on success
 * \see device_read_t
 */
typedef uint8_t (*device_read_interval_t)(offset_t offset, uint8_t* buffer, uintptr_t interval, uintptr_t length, device_read_callback_t callback, void* p);
/**
 * A function pointer used to write to the partition.
 *
 * \param[in] offset The offset on the device where to start writing.
 * \param[in] buffer The buffer which to write.
 * \param[in] length The count of bytes to write.
 */
typedef uint8_t (*device_write_t)(offset_t offset, const uint8_t* buffer, uintptr_t length);
/**
 * A function pointer passed to a \c device_write_interval_t.
 *
 * \param[in] buffer The buffer which receives the data to write.
 * \param[in] offset The offset to which the data in \c buffer will be written.
 * \param[in] p An opaque pointer.
 * \returns The number of bytes put into \c buffer
 * \see device_write_interval_t
 */
typedef uintptr_t (*device_write_callback_t)(uint8_t* buffer, offset_t offset, void* p);
/**
 * A function pointer used to continuously write a data stream obtained from
 * a callback function.
 *
 * This function starts writing at the specified offset. To obtain the
 * next bytes to write, it calls the callback function. The callback fills the
 * provided data buffer and returns the number of bytes it has put into the buffer.
 *
 * By returning zero, the callback may stop writing.
 *
 * \param[in] offset Offset where to start writing.
 * \param[in] buffer Pointer to a buffer which is used for the callback function.
 * \param[in] length Number of bytes to write in total. May be zero for endless writes.
 * \param[in] callback The function used to obtain the bytes to write.
 * \param[in] p An opaque pointer directly passed to the callback function.
 * \returns 0 on failure, 1 on success
 * \see device_write_t
 */
typedef uint8_t (*device_write_interval_t)(offset_t offset, uint8_t* buffer, uintptr_t length, device_write_callback_t callback, void* p);

/**
 * Describes a partition.
 */
struct partition_struct
{
    /**
     * The function which reads data from the partition.
     *
     * \note The offset given to this function is relative to the whole disk,
     *       not to the start of the partition.
     */
    device_read_t device_read;
    /**
     * The function which repeatedly reads a constant amount of data from the partition.
     *
     * \note The offset given to this function is relative to the whole disk,
     *       not to the start of the partition.
     */
    device_read_interval_t device_read_interval;
    /**
     * The function which writes data to the partition.
     *
     * \note The offset given to this function is relative to the whole disk,
     *       not to the start of the partition.
     */
    device_write_t device_write;
    /**
     * The function which repeatedly writes data to the partition.
     *
     * \note The offset given to this function is relative to the whole disk,
     *       not to the start of the partition.
     */
    device_write_interval_t device_write_interval;

    /**
     * The type of the partition.
     *
     * Compare this value to the PARTITION_TYPE_* constants.
     */
    uint8_t type;
    /**
     * The offset in blocks on the disk where this partition starts.
     */
    uint32_t offset;
    /**
     * The length in blocks of this partition.
     */
    uint32_t length;
};

struct partition_struct* partition_open(device_read_t device_read, device_read_interval_t device_read_interval, device_write_t device_write, device_write_interval_t device_write_interval, int8_t index);
uint8_t partition_close(struct partition_struct* partition);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif

