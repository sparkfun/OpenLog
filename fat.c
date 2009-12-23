
/* 
 * Copyright (c) 2006-2009 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#include "byteordering.h"
#include "partition.h"
#include "fat.h"
#include "fat_config.h"
#include "sd-reader_config.h"

#include <string.h>

#if USE_DYNAMIC_MEMORY
    #include <stdlib.h>
#endif

/**
 * \addtogroup fat FAT support
 *
 * This module implements FAT16/FAT32 read and write access.
 * 
 * The following features are supported:
 * - File names up to 31 characters long.
 * - Unlimited depth of subdirectories.
 * - Short 8.3 and long filenames.
 * - Creating and deleting files.
 * - Reading and writing from and to files.
 * - File resizing.
 * - File sizes of up to 4 gigabytes.
 * 
 * @{
 */
/**
 * \file
 * FAT implementation (license: GPLv2 or LGPLv2.1)
 *
 * \author Roland Riegel
 */

/**
 * \addtogroup fat_config FAT configuration
 * Preprocessor defines to configure the FAT implementation.
 */

/**
 * \addtogroup fat_fs FAT access
 * Basic functions for handling a FAT filesystem.
 */

/**
 * \addtogroup fat_file FAT file functions
 * Functions for managing files.
 */

/**
 * \addtogroup fat_dir FAT directory functions
 * Functions for managing directories.
 */

/**
 * @}
 */

#define FAT16_CLUSTER_FREE 0x0000
#define FAT16_CLUSTER_RESERVED_MIN 0xfff0
#define FAT16_CLUSTER_RESERVED_MAX 0xfff6
#define FAT16_CLUSTER_BAD 0xfff7
#define FAT16_CLUSTER_LAST_MIN 0xfff8
#define FAT16_CLUSTER_LAST_MAX 0xffff

#define FAT32_CLUSTER_FREE 0x00000000
#define FAT32_CLUSTER_RESERVED_MIN 0x0ffffff0
#define FAT32_CLUSTER_RESERVED_MAX 0x0ffffff6
#define FAT32_CLUSTER_BAD 0x0ffffff7
#define FAT32_CLUSTER_LAST_MIN 0x0ffffff8
#define FAT32_CLUSTER_LAST_MAX 0x0fffffff

#define FAT_DIRENTRY_DELETED 0xe5
#define FAT_DIRENTRY_LFNLAST (1 << 6)
#define FAT_DIRENTRY_LFNSEQMASK ((1 << 6) - 1)

/* Each entry within the directory table has a size of 32 bytes
 * and either contains a 8.3 DOS-style file name or a part of a
 * long file name, which may consist of several directory table
 * entries at once.
 *
 * multi-byte integer values are stored little-endian!
 *
 * 8.3 file name entry:
 * ====================
 * offset  length  description
 *      0       8  name (space padded)
 *      8       3  extension (space padded)
 *     11       1  attributes (FAT_ATTRIB_*)
 *
 * long file name (lfn) entry ordering for a single file name:
 * ===========================================================
 * LFN entry n
 *     ...
 * LFN entry 2
 * LFN entry 1
 * 8.3 entry (see above)
 * 
 * lfn entry:
 * ==========
 * offset  length  description
 *      0       1  ordinal field
 *      1       2  unicode character 1
 *      3       3  unicode character 2
 *      5       3  unicode character 3
 *      7       3  unicode character 4
 *      9       3  unicode character 5
 *     11       1  attribute (always 0x0f)
 *     12       1  type (reserved, always 0)
 *     13       1  checksum
 *     14       2  unicode character 6
 *     16       2  unicode character 7
 *     18       2  unicode character 8
 *     20       2  unicode character 9
 *     22       2  unicode character 10
 *     24       2  unicode character 11
 *     26       2  cluster (unused, always 0)
 *     28       2  unicode character 12
 *     30       2  unicode character 13
 * 
 * The ordinal field contains a descending number, from n to 1.
 * For the n'th lfn entry the ordinal field is or'ed with 0x40.
 * For deleted lfn entries, the ordinal field is set to 0xe5.
 */

struct fat_header_struct
{
    offset_t size;

    offset_t fat_offset;
    uint32_t fat_size;

    uint16_t sector_size;
    uint16_t cluster_size;

    offset_t cluster_zero_offset;

    offset_t root_dir_offset;
#if FAT_FAT32_SUPPORT
    cluster_t root_dir_cluster;
#endif
};

struct fat_fs_struct
{
    struct partition_struct* partition;
    struct fat_header_struct header;
};

struct fat_file_struct
{
    struct fat_fs_struct* fs;
    struct fat_dir_entry_struct dir_entry;
    offset_t pos;
    cluster_t pos_cluster;
};

struct fat_dir_struct
{
    struct fat_fs_struct* fs;
    struct fat_dir_entry_struct dir_entry;
    cluster_t entry_cluster;
    uint16_t entry_offset;
};

struct fat_read_dir_callback_arg
{
    struct fat_dir_entry_struct* dir_entry;
    uintptr_t bytes_read;
    uint8_t finished;
};

struct fat_usage_count_callback_arg
{
    cluster_t cluster_count;
    uintptr_t buffer_size;
};

#if !USE_DYNAMIC_MEMORY
static struct fat_fs_struct fat_fs_handles[FAT_FS_COUNT];
static struct fat_file_struct fat_file_handles[FAT_FILE_COUNT];
static struct fat_dir_struct fat_dir_handles[FAT_DIR_COUNT];
#endif

static uint8_t fat_read_header(struct fat_fs_struct* fs);
static cluster_t fat_get_next_cluster(const struct fat_fs_struct* fs, cluster_t cluster_num);
static offset_t fat_cluster_offset(const struct fat_fs_struct* fs, cluster_t cluster_num);
static uint8_t fat_dir_entry_read_callback(uint8_t* buffer, offset_t offset, void* p);
static uint8_t fat_interpret_dir_entry(struct fat_dir_entry_struct* dir_entry, const uint8_t* raw_entry);

static uint8_t fat_get_fs_free_16_callback(uint8_t* buffer, offset_t offset, void* p);
#if FAT_FAT32_SUPPORT
static uint8_t fat_get_fs_free_32_callback(uint8_t* buffer, offset_t offset, void* p);
#endif

#if FAT_WRITE_SUPPORT
static cluster_t fat_append_clusters(const struct fat_fs_struct* fs, cluster_t cluster_num, cluster_t count);
static uint8_t fat_free_clusters(const struct fat_fs_struct* fs, cluster_t cluster_num);
static uint8_t fat_terminate_clusters(const struct fat_fs_struct* fs, cluster_t cluster_num);
static uint8_t fat_clear_cluster(const struct fat_fs_struct* fs, cluster_t cluster_num);
static uintptr_t fat_clear_cluster_callback(uint8_t* buffer, offset_t offset, void* p);
static offset_t fat_find_offset_for_dir_entry(const struct fat_fs_struct* fs, const struct fat_dir_struct* parent, const struct fat_dir_entry_struct* dir_entry);
static uint8_t fat_write_dir_entry(const struct fat_fs_struct* fs, struct fat_dir_entry_struct* dir_entry);
#if FAT_DATETIME_SUPPORT
static void fat_set_file_modification_date(struct fat_dir_entry_struct* dir_entry, uint16_t year, uint8_t month, uint8_t day);
static void fat_set_file_modification_time(struct fat_dir_entry_struct* dir_entry, uint8_t hour, uint8_t min, uint8_t sec);
#endif
#endif

/**
 * \ingroup fat_fs
 * Opens a FAT filesystem.
 *
 * \param[in] partition Discriptor of partition on which the filesystem resides.
 * \returns 0 on error, a FAT filesystem descriptor on success.
 * \see fat_close
 */
struct fat_fs_struct* fat_open(struct partition_struct* partition)
{
    if(!partition ||
#if FAT_WRITE_SUPPORT
       !partition->device_write ||
       !partition->device_write_interval
#else
       0
#endif
      )
        return 0;

#if USE_DYNAMIC_MEMORY
    struct fat_fs_struct* fs = malloc(sizeof(*fs));
    if(!fs)
        return 0;
#else
    struct fat_fs_struct* fs = fat_fs_handles;
    uint8_t i;
    for(i = 0; i < FAT_FS_COUNT; ++i)
    {
        if(!fs->partition)
            break;

        ++fs;
    }
    if(i >= FAT_FS_COUNT)
        return 0;
#endif

    memset(fs, 0, sizeof(*fs));

    fs->partition = partition;
    if(!fat_read_header(fs))
    {
#if USE_DYNAMIC_MEMORY
        free(fs);
#else
        fs->partition = 0;
#endif
        return 0;
    }
    
    return fs;
}

/**
 * \ingroup fat_fs
 * Closes a FAT filesystem.
 *
 * When this function returns, the given filesystem descriptor
 * will be invalid.
 *
 * \param[in] fs The filesystem to close.
 * \see fat_open
 */
void fat_close(struct fat_fs_struct* fs)
{
    if(!fs)
        return;

#if USE_DYNAMIC_MEMORY
    free(fs);
#else
    fs->partition = 0;
#endif
}

/**
 * \ingroup fat_fs
 * Reads and parses the header of a FAT filesystem.
 *
 * \param[inout] fs The filesystem for which to parse the header.
 * \returns 0 on failure, 1 on success.
 */
uint8_t fat_read_header(struct fat_fs_struct* fs)
{
    if(!fs)
        return 0;

    struct partition_struct* partition = fs->partition;
    if(!partition)
        return 0;

    /* read fat parameters */
#if FAT_FAT32_SUPPORT
    uint8_t buffer[37];
#else
    uint8_t buffer[25];
#endif
    offset_t partition_offset = (offset_t) partition->offset * 512;
    if(!partition->device_read(partition_offset + 0x0b, buffer, sizeof(buffer)))
        return 0;

    uint16_t bytes_per_sector = ltoh16(*((uint16_t*) &buffer[0x00]));
    uint16_t reserved_sectors = ltoh16(*((uint16_t*) &buffer[0x03]));
    uint8_t sectors_per_cluster = buffer[0x02];
    uint8_t fat_copies = buffer[0x05];
    uint16_t max_root_entries = ltoh16(*((uint16_t*) &buffer[0x06]));
    uint16_t sector_count_16 = ltoh16(*((uint16_t*) &buffer[0x08]));
    uint16_t sectors_per_fat = ltoh16(*((uint16_t*) &buffer[0x0b]));
    uint32_t sector_count = ltoh32(*((uint32_t*) &buffer[0x15]));
#if FAT_FAT32_SUPPORT
    uint32_t sectors_per_fat32 = ltoh32(*((uint32_t*) &buffer[0x19]));
    uint32_t cluster_root_dir = ltoh32(*((uint32_t*) &buffer[0x21]));
#endif

    if(sector_count == 0)
    {
        if(sector_count_16 == 0)
            /* illegal volume size */
            return 0;
        else
            sector_count = sector_count_16;
    }
#if FAT_FAT32_SUPPORT
    if(sectors_per_fat != 0)
        sectors_per_fat32 = sectors_per_fat;
    else if(sectors_per_fat32 == 0)
        /* this is neither FAT16 nor FAT32 */
        return 0;
#else
    if(sectors_per_fat == 0)
        /* this is not a FAT16 */
        return 0;
#endif

    /* determine the type of FAT we have here */
    uint32_t data_sector_count = sector_count
                                 - reserved_sectors
#if FAT_FAT32_SUPPORT
                                 - sectors_per_fat32 * fat_copies
#else
                                 - (uint32_t) sectors_per_fat * fat_copies
#endif
                                 - ((max_root_entries * 32 + bytes_per_sector - 1) / bytes_per_sector);
    uint32_t data_cluster_count = data_sector_count / sectors_per_cluster;
    if(data_cluster_count < 4085)
        /* this is a FAT12, not supported */
        return 0;
    else if(data_cluster_count < 65525)
        /* this is a FAT16 */
        partition->type = PARTITION_TYPE_FAT16;
    else
        /* this is a FAT32 */
        partition->type = PARTITION_TYPE_FAT32;

    /* fill header information */
    struct fat_header_struct* header = &fs->header;
    memset(header, 0, sizeof(*header));
    
    header->size = (offset_t) sector_count * bytes_per_sector;

    header->fat_offset = /* jump to partition */
                         partition_offset +
                         /* jump to fat */
                         (offset_t) reserved_sectors * bytes_per_sector;
    header->fat_size = (data_cluster_count + 2) * (partition->type == PARTITION_TYPE_FAT16 ? 2 : 4);

    header->sector_size = bytes_per_sector;
    header->cluster_size = (uint16_t) bytes_per_sector * sectors_per_cluster;

#if FAT_FAT32_SUPPORT
    if(partition->type == PARTITION_TYPE_FAT16)
#endif
    {
        header->root_dir_offset = /* jump to fats */
                                  header->fat_offset +
                                  /* jump to root directory entries */
                                  (offset_t) fat_copies * sectors_per_fat * bytes_per_sector;

        header->cluster_zero_offset = /* jump to root directory entries */
                                      header->root_dir_offset +
                                      /* skip root directory entries */
                                      (offset_t) max_root_entries * 32;
    }
#if FAT_FAT32_SUPPORT
    else
    {
        header->cluster_zero_offset = /* jump to fats */
                                      header->fat_offset +
                                      /* skip fats */
                                      (offset_t) fat_copies * sectors_per_fat32 * bytes_per_sector;

        header->root_dir_cluster = cluster_root_dir;
    }
#endif

    return 1;
}

/**
 * \ingroup fat_fs
 * Retrieves the next following cluster of a given cluster.
 *
 * Using the filesystem file allocation table, this function returns
 * the number of the cluster containing the data directly following
 * the data within the cluster with the given number.
 *
 * \param[in] fs The filesystem for which to determine the next cluster.
 * \param[in] cluster_num The number of the cluster for which to determine its successor.
 * \returns The wanted cluster number, or 0 on error.
 */
cluster_t fat_get_next_cluster(const struct fat_fs_struct* fs, cluster_t cluster_num)
{
    if(!fs || cluster_num < 2)
        return 0;

#if FAT_FAT32_SUPPORT
    if(fs->partition->type == PARTITION_TYPE_FAT32)
    {
        /* read appropriate fat entry */
        uint32_t fat_entry;
        if(!fs->partition->device_read(fs->header.fat_offset + cluster_num * sizeof(fat_entry), (uint8_t*) &fat_entry, sizeof(fat_entry)))
            return 0;

        /* determine next cluster from fat */
        cluster_num = ltoh32(fat_entry);
        
        if(cluster_num == FAT32_CLUSTER_FREE ||
           cluster_num == FAT32_CLUSTER_BAD ||
           (cluster_num >= FAT32_CLUSTER_RESERVED_MIN && cluster_num <= FAT32_CLUSTER_RESERVED_MAX) ||
           (cluster_num >= FAT32_CLUSTER_LAST_MIN && cluster_num <= FAT32_CLUSTER_LAST_MAX))
            return 0;
    }
    else
#endif
    {
        /* read appropriate fat entry */
        uint16_t fat_entry;
        if(!fs->partition->device_read(fs->header.fat_offset + cluster_num * sizeof(fat_entry), (uint8_t*) &fat_entry, sizeof(fat_entry)))
            return 0;

        /* determine next cluster from fat */
        cluster_num = ltoh16(fat_entry);
        
        if(cluster_num == FAT16_CLUSTER_FREE ||
           cluster_num == FAT16_CLUSTER_BAD ||
           (cluster_num >= FAT16_CLUSTER_RESERVED_MIN && cluster_num <= FAT16_CLUSTER_RESERVED_MAX) ||
           (cluster_num >= FAT16_CLUSTER_LAST_MIN && cluster_num <= FAT16_CLUSTER_LAST_MAX))
            return 0;
    }

    return cluster_num;
}

#if DOXYGEN || FAT_WRITE_SUPPORT
/**
 * \ingroup fat_fs
 * Appends a new cluster chain to an existing one.
 *
 * Set cluster_num to zero to create a completely new one.
 *
 * \param[in] fs The file system on which to operate.
 * \param[in] cluster_num The cluster to which to append the new chain.
 * \param[in] count The number of clusters to allocate.
 * \returns 0 on failure, the number of the first new cluster on success.
 */
cluster_t fat_append_clusters(const struct fat_fs_struct* fs, cluster_t cluster_num, cluster_t count)
{
    if(!fs)
        return 0;

    device_read_t device_read = fs->partition->device_read;
    device_write_t device_write = fs->partition->device_write;
    offset_t fat_offset = fs->header.fat_offset;
    cluster_t count_left = count;
    cluster_t cluster_next = 0;
    cluster_t cluster_max;
    uint16_t fat_entry16;
#if FAT_FAT32_SUPPORT
    uint32_t fat_entry32;
    uint8_t is_fat32 = (fs->partition->type == PARTITION_TYPE_FAT32);

    if(is_fat32)
        cluster_max = fs->header.fat_size / sizeof(fat_entry32);
    else
#endif
        cluster_max = fs->header.fat_size / sizeof(fat_entry16);

    for(cluster_t cluster_new = 2; cluster_new < cluster_max; ++cluster_new)
    {
#if FAT_FAT32_SUPPORT
        if(is_fat32)
        {
            if(!device_read(fat_offset + cluster_new * sizeof(fat_entry32), (uint8_t*) &fat_entry32, sizeof(fat_entry32)))
                return 0;
        }
        else
#endif
        {
            if(!device_read(fat_offset + cluster_new * sizeof(fat_entry16), (uint8_t*) &fat_entry16, sizeof(fat_entry16)))
                return 0;
        }

#if FAT_FAT32_SUPPORT
        if(is_fat32)
        {
            /* check if this is a free cluster */
            if(fat_entry32 != HTOL32(FAT32_CLUSTER_FREE))
                continue;

            /* allocate cluster */
            if(cluster_next == 0)
                fat_entry32 = HTOL32(FAT32_CLUSTER_LAST_MAX);
            else
                fat_entry32 = htol32(cluster_next);

            if(!device_write(fat_offset + cluster_new * sizeof(fat_entry32), (uint8_t*) &fat_entry32, sizeof(fat_entry32)))
                break;
        }
        else
#endif
        {
            /* check if this is a free cluster */
            if(fat_entry16 != HTOL16(FAT16_CLUSTER_FREE))
                continue;

            /* allocate cluster */
            if(cluster_next == 0)
                fat_entry16 = HTOL16(FAT16_CLUSTER_LAST_MAX);
            else
                fat_entry16 = htol16((uint16_t) cluster_next);

            if(!device_write(fat_offset + cluster_new * sizeof(fat_entry16), (uint8_t*) &fat_entry16, sizeof(fat_entry16)))
                break;
        }

        cluster_next = cluster_new;
        if(--count_left == 0)
            break;
    }

    do
    {
        if(count_left > 0)
            break;

        /* We allocated a new cluster chain. Now join
         * it with the existing one (if any).
         */
        if(cluster_num >= 2)
        {
#if FAT_FAT32_SUPPORT
            if(is_fat32)
            {
                fat_entry32 = htol32(cluster_next);

                if(!device_write(fat_offset + cluster_num * sizeof(fat_entry32), (uint8_t*) &fat_entry32, sizeof(fat_entry32)))
                    break;
            }
            else
#endif
            {
                fat_entry16 = htol16((uint16_t) cluster_next);

                if(!device_write(fat_offset + cluster_num * sizeof(fat_entry16), (uint8_t*) &fat_entry16, sizeof(fat_entry16)))
                    break;
            }
        }

        return cluster_next;

    } while(0);

    /* No space left on device or writing error.
     * Free up all clusters already allocated.
     */
    fat_free_clusters(fs, cluster_next);

    return 0;
}
#endif

#if DOXYGEN || FAT_WRITE_SUPPORT
/**
 * \ingroup fat_fs
 * Frees a cluster chain, or a part thereof.
 *
 * Marks the specified cluster and all clusters which are sequentially
 * referenced by it as free. They may then be used again for future
 * file allocations.
 *
 * \note If this function is used for freeing just a part of a cluster
 *       chain, the new end of the chain is not correctly terminated
 *       within the FAT. Use fat_terminate_clusters() instead.
 *
 * \param[in] fs The filesystem on which to operate.
 * \param[in] cluster_num The starting cluster of the chain which to free.
 * \returns 0 on failure, 1 on success.
 * \see fat_terminate_clusters
 */
uint8_t fat_free_clusters(const struct fat_fs_struct* fs, cluster_t cluster_num)
{
    if(!fs || cluster_num < 2)
        return 0;

    offset_t fat_offset = fs->header.fat_offset;
#if FAT_FAT32_SUPPORT
    if(fs->partition->type == PARTITION_TYPE_FAT32)
    {
        uint32_t fat_entry;
        while(cluster_num)
        {
            if(!fs->partition->device_read(fat_offset + cluster_num * sizeof(fat_entry), (uint8_t*) &fat_entry, sizeof(fat_entry)))
                return 0;

            /* get next cluster of current cluster before freeing current cluster */
            uint32_t cluster_num_next = ltoh32(fat_entry);

            if(cluster_num_next == FAT32_CLUSTER_FREE)
                return 1;
            if(cluster_num_next == FAT32_CLUSTER_BAD ||
               (cluster_num_next >= FAT32_CLUSTER_RESERVED_MIN &&
                cluster_num_next <= FAT32_CLUSTER_RESERVED_MAX
               )
              )
                return 0;
            if(cluster_num_next >= FAT32_CLUSTER_LAST_MIN && cluster_num_next <= FAT32_CLUSTER_LAST_MAX)
                cluster_num_next = 0;

            /* free cluster */
            fat_entry = HTOL32(FAT32_CLUSTER_FREE);
            fs->partition->device_write(fat_offset + cluster_num * sizeof(fat_entry), (uint8_t*) &fat_entry, sizeof(fat_entry));

            /* We continue in any case here, even if freeing the cluster failed.
             * The cluster is lost, but maybe we can still free up some later ones.
             */

            cluster_num = cluster_num_next;
        }
    }
    else
#endif
    {
        uint16_t fat_entry;
        while(cluster_num)
        {
            if(!fs->partition->device_read(fat_offset + cluster_num * sizeof(fat_entry), (uint8_t*) &fat_entry, sizeof(fat_entry)))
                return 0;

            /* get next cluster of current cluster before freeing current cluster */
            uint16_t cluster_num_next = ltoh16(fat_entry);

            if(cluster_num_next == FAT16_CLUSTER_FREE)
                return 1;
            if(cluster_num_next == FAT16_CLUSTER_BAD ||
               (cluster_num_next >= FAT16_CLUSTER_RESERVED_MIN &&
                cluster_num_next <= FAT16_CLUSTER_RESERVED_MAX
               )
              )
                return 0;
            if(cluster_num_next >= FAT16_CLUSTER_LAST_MIN && cluster_num_next <= FAT16_CLUSTER_LAST_MAX)
                cluster_num_next = 0;

            /* free cluster */
            fat_entry = HTOL16(FAT16_CLUSTER_FREE);
            fs->partition->device_write(fat_offset + cluster_num * sizeof(fat_entry), (uint8_t*) &fat_entry, sizeof(fat_entry));

            /* We continue in any case here, even if freeing the cluster failed.
             * The cluster is lost, but maybe we can still free up some later ones.
             */

            cluster_num = cluster_num_next;
        }
    }

    return 1;
}
#endif

#if DOXYGEN || FAT_WRITE_SUPPORT
/**
 * \ingroup fat_fs
 * Frees a part of a cluster chain and correctly terminates the rest.
 *
 * Marks the specified cluster as the new end of a cluster chain and
 * frees all following clusters.
 *
 * \param[in] fs The filesystem on which to operate.
 * \param[in] cluster_num The new end of the cluster chain.
 * \returns 0 on failure, 1 on success.
 * \see fat_free_clusters
 */
uint8_t fat_terminate_clusters(const struct fat_fs_struct* fs, cluster_t cluster_num)
{
    if(!fs || cluster_num < 2)
        return 0;

    /* fetch next cluster before overwriting the cluster entry */
    cluster_t cluster_num_next = fat_get_next_cluster(fs, cluster_num);

    /* mark cluster as the last one */
#if FAT_FAT32_SUPPORT
    if(fs->partition->type == PARTITION_TYPE_FAT32)
    {
        uint32_t fat_entry = HTOL32(FAT32_CLUSTER_LAST_MAX);
        if(!fs->partition->device_write(fs->header.fat_offset + cluster_num * sizeof(fat_entry), (uint8_t*) &fat_entry, sizeof(fat_entry)))
            return 0;
    }
    else
#endif
    {
        uint16_t fat_entry = HTOL16(FAT16_CLUSTER_LAST_MAX);
        if(!fs->partition->device_write(fs->header.fat_offset + cluster_num * sizeof(fat_entry), (uint8_t*) &fat_entry, sizeof(fat_entry)))
            return 0;
    }

    /* free remaining clusters */
    if(cluster_num_next)
        return fat_free_clusters(fs, cluster_num_next);
    else
        return 1;
}
#endif

#if DOXYGEN || FAT_WRITE_SUPPORT
/**
 * \ingroup fat_fs
 * Clears a single cluster.
 *
 * The complete cluster is filled with zeros.
 *
 * \param[in] fs The filesystem on which to operate.
 * \param[in] cluster_num The cluster to clear.
 * \returns 0 on failure, 1 on success.
 */
uint8_t fat_clear_cluster(const struct fat_fs_struct* fs, cluster_t cluster_num)
{
    if(cluster_num < 2)
        return 0;

    offset_t cluster_offset = fat_cluster_offset(fs, cluster_num);

    uint8_t zero[16];
    memset(zero, 0, sizeof(zero));
    return fs->partition->device_write_interval(cluster_offset,
                                                zero,
                                                fs->header.cluster_size,
                                                fat_clear_cluster_callback,
                                                0
                                               );
}
#endif

#if DOXYGEN || FAT_WRITE_SUPPORT
/**
 * \ingroup fat_fs
 * Callback function for clearing a cluster.
 */
uintptr_t fat_clear_cluster_callback(uint8_t* buffer, offset_t offset, void* p)
{
    return 16;
}
#endif

/**
 * \ingroup fat_fs
 * Calculates the offset of the specified cluster.
 *
 * \param[in] fs The filesystem on which to operate.
 * \param[in] cluster_num The cluster whose offset to calculate.
 * \returns The cluster offset.
 */
offset_t fat_cluster_offset(const struct fat_fs_struct* fs, cluster_t cluster_num)
{
    if(!fs || cluster_num < 2)
        return 0;

    return fs->header.cluster_zero_offset + (offset_t) (cluster_num - 2) * fs->header.cluster_size;
}

/**
 * \ingroup fat_file
 * Retrieves the directory entry of a path.
 *
 * The given path may both describe a file or a directory.
 *
 * \param[in] fs The FAT filesystem on which to search.
 * \param[in] path The path of which to read the directory entry.
 * \param[out] dir_entry The directory entry to fill.
 * \returns 0 on failure, 1 on success.
 * \see fat_read_dir
 */
uint8_t fat_get_dir_entry_of_path(struct fat_fs_struct* fs, const char* path, struct fat_dir_entry_struct* dir_entry)
{
    if(!fs || !path || path[0] == '\0' || !dir_entry)
        return 0;

    if(path[0] == '/')
        ++path;

    /* begin with the root directory */
    memset(dir_entry, 0, sizeof(*dir_entry));
    dir_entry->attributes = FAT_ATTRIB_DIR;

    while(1)
    {
        if(path[0] == '\0')
            return 1;

        struct fat_dir_struct* dd = fat_open_dir(fs, dir_entry);
        if(!dd)
            break;

        /* extract the next hierarchy we will search for */
        const char* sub_path = strchr(path, '/');
        uint8_t length_to_sep;
        if(sub_path)
        {
            length_to_sep = sub_path - path;
            ++sub_path;
        }
        else
        {
            length_to_sep = strlen(path);
            sub_path = path + length_to_sep;
        }
        
        /* read directory entries */
        while(fat_read_dir(dd, dir_entry))
        {
            /* check if we have found the next hierarchy */
            if((strlen(dir_entry->long_name) != length_to_sep ||
                strncmp(path, dir_entry->long_name, length_to_sep) != 0))
                continue;

            fat_close_dir(dd);
            dd = 0;

            if(path[length_to_sep] == '\0')
                /* we iterated through the whole path and have found the file */
                return 1;

            if(dir_entry->attributes & FAT_ATTRIB_DIR)
            {
                /* we found a parent directory of the file we are searching for */
                path = sub_path;
                break;
            }

            /* a parent of the file exists, but not the file itself */
            return 0;
        }

        fat_close_dir(dd);
    }
    
    return 0;
}

/**
 * \ingroup fat_file
 * Opens a file on a FAT filesystem.
 *
 * \param[in] fs The filesystem on which the file to open lies.
 * \param[in] dir_entry The directory entry of the file to open.
 * \returns The file handle, or 0 on failure.
 * \see fat_close_file
 */
struct fat_file_struct* fat_open_file(struct fat_fs_struct* fs, const struct fat_dir_entry_struct* dir_entry)
{
    if(!fs || !dir_entry || (dir_entry->attributes & FAT_ATTRIB_DIR))
        return 0;

#if USE_DYNAMIC_MEMORY
    struct fat_file_struct* fd = malloc(sizeof(*fd));
    if(!fd)
        return 0;
#else
    struct fat_file_struct* fd = fat_file_handles;
    uint8_t i;
    for(i = 0; i < FAT_FILE_COUNT; ++i)
    {
        if(!fd->fs)
            break;

        ++fd;
    }
    if(i >= FAT_FILE_COUNT)
        return 0;
#endif
    
    memcpy(&fd->dir_entry, dir_entry, sizeof(*dir_entry));
    fd->fs = fs;
    fd->pos = 0;
    fd->pos_cluster = dir_entry->cluster;

    return fd;
}

/**
 * \ingroup fat_file
 * Closes a file.
 *
 * \param[in] fd The file handle of the file to close.
 * \see fat_open_file
 */
void fat_close_file(struct fat_file_struct* fd)
{
    if(fd)
#if USE_DYNAMIC_MEMORY
        free(fd);
#else
        fd->fs = 0;
#endif
}

/**
 * \ingroup fat_file
 * Reads data from a file.
 * 
 * The data requested is read from the current file location.
 *
 * \param[in] fd The file handle of the file from which to read.
 * \param[out] buffer The buffer into which to write.
 * \param[in] buffer_len The amount of data to read.
 * \returns The number of bytes read, 0 on end of file, or -1 on failure.
 * \see fat_write_file
 */
intptr_t fat_read_file(struct fat_file_struct* fd, uint8_t* buffer, uintptr_t buffer_len)
{
    /* check arguments */
    if(!fd || !buffer || buffer_len < 1)
        return -1;

    /* determine number of bytes to read */
    if(fd->pos + buffer_len > fd->dir_entry.file_size)
        buffer_len = fd->dir_entry.file_size - fd->pos;
    if(buffer_len == 0)
        return 0;
    
    uint16_t cluster_size = fd->fs->header.cluster_size;
    cluster_t cluster_num = fd->pos_cluster;
    uintptr_t buffer_left = buffer_len;
    uint16_t first_cluster_offset = (uint16_t) (fd->pos & (cluster_size - 1));

    /* find cluster in which to start reading */
    if(!cluster_num)
    {
        cluster_num = fd->dir_entry.cluster;
        
        if(!cluster_num)
        {
            if(!fd->pos)
                return 0;
            else
                return -1;
        }

        if(fd->pos)
        {
            uint32_t pos = fd->pos;
            while(pos >= cluster_size)
            {
                pos -= cluster_size;
                cluster_num = fat_get_next_cluster(fd->fs, cluster_num);
                if(!cluster_num)
                    return -1;
            }
        }
    }
    
    /* read data */
    do
    {
        /* calculate data size to copy from cluster */
        offset_t cluster_offset = fat_cluster_offset(fd->fs, cluster_num) + first_cluster_offset;
        uint16_t copy_length = cluster_size - first_cluster_offset;
        if(copy_length > buffer_left)
            copy_length = buffer_left;

        /* read data */
        if(!fd->fs->partition->device_read(cluster_offset, buffer, copy_length))
            return buffer_len - buffer_left;

        /* calculate new file position */
        buffer += copy_length;
        buffer_left -= copy_length;
        fd->pos += copy_length;

        if(first_cluster_offset + copy_length >= cluster_size)
        {
            /* we are on a cluster boundary, so get the next cluster */
            if((cluster_num = fat_get_next_cluster(fd->fs, cluster_num)))
            {
                first_cluster_offset = 0;
            }
            else
            {
                fd->pos_cluster = 0;
                return buffer_len - buffer_left;
            }
        }

        fd->pos_cluster = cluster_num;

    } while(buffer_left > 0); /* check if we are done */

    return buffer_len;
}

#if DOXYGEN || FAT_WRITE_SUPPORT
/**
 * \ingroup fat_file
 * Writes data to a file.
 * 
 * The data is written to the current file location.
 *
 * \param[in] fd The file handle of the file to which to write.
 * \param[in] buffer The buffer from which to read the data to be written.
 * \param[in] buffer_len The amount of data to write.
 * \returns The number of bytes written, 0 on disk full, or -1 on failure.
 * \see fat_read_file
 */
intptr_t fat_write_file(struct fat_file_struct* fd, const uint8_t* buffer, uintptr_t buffer_len)
{
    /* check arguments */
    if(!fd || !buffer || buffer_len < 1)
        return -1;
    if(fd->pos > fd->dir_entry.file_size)
        return -1;

    uint16_t cluster_size = fd->fs->header.cluster_size;
    cluster_t cluster_num = fd->pos_cluster;
    uintptr_t buffer_left = buffer_len;
    uint16_t first_cluster_offset = (uint16_t) (fd->pos & (cluster_size - 1));

    /* find cluster in which to start writing */
    if(!cluster_num)
    {
        cluster_num = fd->dir_entry.cluster;
        
        if(!cluster_num)
        {
            if(!fd->pos)
            {
                /* empty file */
                fd->dir_entry.cluster = cluster_num = fat_append_clusters(fd->fs, 0, 1);
                if(!cluster_num)
                    return -1;
            }
            else
            {
                return -1;
            }
        }

        if(fd->pos)
        {
            uint32_t pos = fd->pos;
            cluster_t cluster_num_next;
            while(pos >= cluster_size)
            {
                pos -= cluster_size;
                cluster_num_next = fat_get_next_cluster(fd->fs, cluster_num);
                if(!cluster_num_next && pos == 0)
                    /* the file exactly ends on a cluster boundary, and we append to it */
                    cluster_num_next = fat_append_clusters(fd->fs, cluster_num, 1);
                if(!cluster_num_next)
                    return -1;

                cluster_num = cluster_num_next;
            }
        }
    }
    
    /* write data */
    do
    {
        /* calculate data size to write to cluster */
        offset_t cluster_offset = fat_cluster_offset(fd->fs, cluster_num) + first_cluster_offset;
        uint16_t write_length = cluster_size - first_cluster_offset;
        if(write_length > buffer_left)
            write_length = buffer_left;

        /* write data which fits into the current cluster */
        if(!fd->fs->partition->device_write(cluster_offset, buffer, write_length))
            break;

        /* calculate new file position */
        buffer += write_length;
        buffer_left -= write_length;
        fd->pos += write_length;

        if(first_cluster_offset + write_length >= cluster_size)
        {
            /* we are on a cluster boundary, so get the next cluster */
            cluster_t cluster_num_next = fat_get_next_cluster(fd->fs, cluster_num);
            if(!cluster_num_next && buffer_left > 0)
                /* we reached the last cluster, append a new one */
                cluster_num_next = fat_append_clusters(fd->fs, cluster_num, 1);
            if(!cluster_num_next)
            {
                fd->pos_cluster = 0;
                break;
            }

            cluster_num = cluster_num_next;
            first_cluster_offset = 0;
        }

        fd->pos_cluster = cluster_num;

    } while(buffer_left > 0); /* check if we are done */

    /* update directory entry */
    if(fd->pos > fd->dir_entry.file_size)
    {
        uint32_t size_old = fd->dir_entry.file_size;

        /* update file size */
        fd->dir_entry.file_size = fd->pos;
        /* write directory entry */
        if(!fat_write_dir_entry(fd->fs, &fd->dir_entry))
        {
            /* We do not return an error here since we actually wrote
             * some data to disk. So we calculate the amount of data
             * we wrote to disk and which lies within the old file size.
             */
            buffer_left = fd->pos - size_old;
            fd->pos = size_old;
        }
    }

    return buffer_len - buffer_left;
}
#endif

/**
 * \ingroup fat_file
 * Repositions the read/write file offset.
 *
 * Changes the file offset where the next call to fat_read_file()
 * or fat_write_file() starts reading/writing.
 *
 * If the new offset is beyond the end of the file, fat_resize_file()
 * is implicitly called, i.e. the file is expanded.
 *
 * The new offset can be given in different ways determined by
 * the \c whence parameter:
 * - \b FAT_SEEK_SET: \c *offset is relative to the beginning of the file.
 * - \b FAT_SEEK_CUR: \c *offset is relative to the current file position.
 * - \b FAT_SEEK_END: \c *offset is relative to the end of the file.
 *
 * The resulting absolute offset is written to the location the \c offset
 * parameter points to.
 * 
 * \param[in] fd The file decriptor of the file on which to seek.
 * \param[in,out] offset A pointer to the new offset, as affected by the \c whence
 *                   parameter. The function writes the new absolute offset
 *                   to this location before it returns.
 * \param[in] whence Affects the way \c offset is interpreted, see above.
 * \returns 0 on failure, 1 on success.
 */
uint8_t fat_seek_file(struct fat_file_struct* fd, int32_t* offset, uint8_t whence)
{
    if(!fd || !offset)
        return 0;

    uint32_t new_pos = fd->pos;
    switch(whence)
    {
        case FAT_SEEK_SET:
            new_pos = *offset;
            break;
        case FAT_SEEK_CUR:
            new_pos += *offset;
            break;
        case FAT_SEEK_END:
            new_pos = fd->dir_entry.file_size + *offset;
            break;
        default:
            return 0;
    }

    if(new_pos > fd->dir_entry.file_size
#if FAT_WRITE_SUPPORT
       && !fat_resize_file(fd, new_pos)
#endif
       )
        return 0;

    fd->pos = new_pos;
    fd->pos_cluster = 0;

    *offset = (int32_t) new_pos;
    return 1;
}

#if DOXYGEN || FAT_WRITE_SUPPORT
/**
 * \ingroup fat_file
 * Resizes a file to have a specific size.
 *
 * Enlarges or shrinks the file pointed to by the file descriptor to have
 * exactly the specified size.
 *
 * If the file is truncated, all bytes having an equal or larger offset
 * than the given size are lost. If the file is expanded, the additional
 * bytes are allocated.
 *
 * \note Please be aware that this function just allocates or deallocates disk
 * space, it does not explicitely clear it. To avoid data leakage, this
 * must be done manually.
 *
 * \param[in] fd The file decriptor of the file which to resize.
 * \param[in] size The new size of the file.
 * \returns 0 on failure, 1 on success.
 */
uint8_t fat_resize_file(struct fat_file_struct* fd, uint32_t size)
{
    if(!fd)
        return 0;

    cluster_t cluster_num = fd->dir_entry.cluster;
    uint16_t cluster_size = fd->fs->header.cluster_size;
    uint32_t size_new = size;

    do
    {
        if(cluster_num == 0 && size_new == 0)
            /* the file stays empty */
            break;

        /* seek to the next cluster as long as we need the space */
        while(size_new > cluster_size)
        {
            /* get next cluster of file */
            cluster_t cluster_num_next = fat_get_next_cluster(fd->fs, cluster_num);
            if(cluster_num_next)
            {
                cluster_num = cluster_num_next;
                size_new -= cluster_size;
            }
            else
            {
                break;
            }
        }

        if(size_new > cluster_size || cluster_num == 0)
        {
            /* Allocate new cluster chain and append
             * it to the existing one, if available.
             */
            cluster_t cluster_count = (size_new + cluster_size - 1) / cluster_size;
            cluster_t cluster_new_chain = fat_append_clusters(fd->fs, cluster_num, cluster_count);
            if(!cluster_new_chain)
                return 0;

            if(!cluster_num)
            {
                cluster_num = cluster_new_chain;
                fd->dir_entry.cluster = cluster_num;
            }
        }

        /* write new directory entry */
        fd->dir_entry.file_size = size;
        if(size == 0)
            fd->dir_entry.cluster = 0;
        if(!fat_write_dir_entry(fd->fs, &fd->dir_entry))
            return 0;

        if(size == 0)
        {
            /* free all clusters of file */
            fat_free_clusters(fd->fs, cluster_num);
        }
        else if(size_new <= cluster_size)
        {
            /* free all clusters no longer needed */
            fat_terminate_clusters(fd->fs, cluster_num);
        }

    } while(0);

    /* correct file position */
    if(size < fd->pos)
    {
        fd->pos = size;
        fd->pos_cluster = 0;
    }

    return 1;
}
#endif

/**
 * \ingroup fat_dir
 * Opens a directory.
 *
 * \param[in] fs The filesystem on which the directory to open resides.
 * \param[in] dir_entry The directory entry which stands for the directory to open.
 * \returns An opaque directory descriptor on success, 0 on failure.
 * \see fat_close_dir
 */
struct fat_dir_struct* fat_open_dir(struct fat_fs_struct* fs, const struct fat_dir_entry_struct* dir_entry)
{
    if(!fs || !dir_entry || !(dir_entry->attributes & FAT_ATTRIB_DIR))
        return 0;

#if USE_DYNAMIC_MEMORY
    struct fat_dir_struct* dd = malloc(sizeof(*dd));
    if(!dd)
        return 0;
#else
    struct fat_dir_struct* dd = fat_dir_handles;
    uint8_t i;
    for(i = 0; i < FAT_DIR_COUNT; ++i)
    {
        if(!dd->fs)
            break;

        ++dd;
    }
    if(i >= FAT_DIR_COUNT)
        return 0;
#endif
    
    memcpy(&dd->dir_entry, dir_entry, sizeof(*dir_entry));
    dd->fs = fs;
    dd->entry_cluster = dir_entry->cluster;
    dd->entry_offset = 0;

    return dd;
}

/**
 * \ingroup fat_dir
 * Closes a directory descriptor.
 *
 * This function destroys a directory descriptor which was
 * previously obtained by calling fat_open_dir(). When this
 * function returns, the given descriptor will be invalid.
 *
 * \param[in] dd The directory descriptor to close.
 * \see fat_open_dir
 */
void fat_close_dir(struct fat_dir_struct* dd)
{
    if(dd)
#if USE_DYNAMIC_MEMORY
        free(dd);
#else
        dd->fs = 0;
#endif
}

/**
 * \ingroup fat_dir
 * Reads the next directory entry contained within a parent directory.
 *
 * \param[in] dd The descriptor of the parent directory from which to read the entry.
 * \param[out] dir_entry Pointer to a buffer into which to write the directory entry information.
 * \returns 0 on failure, 1 on success.
 * \see fat_reset_dir
 */
uint8_t fat_read_dir(struct fat_dir_struct* dd, struct fat_dir_entry_struct* dir_entry)
{
    if(!dd || !dir_entry)
        return 0;

    /* get current position of directory handle */
    struct fat_fs_struct* fs = dd->fs;
    const struct fat_header_struct* header = &fs->header;
    uint16_t cluster_size = header->cluster_size;
    cluster_t cluster_num = dd->entry_cluster;
    uint16_t cluster_offset = dd->entry_offset;
    struct fat_read_dir_callback_arg arg;

    /* reset directory entry */
    memset(dir_entry, 0, sizeof(*dir_entry));

    /* reset callback arguments */
    memset(&arg, 0, sizeof(arg));
    arg.dir_entry = dir_entry;

    /* check if we read from the root directory */
    if(cluster_num == 0)
    {
#if FAT_FAT32_SUPPORT
        if(fs->partition->type == PARTITION_TYPE_FAT32)
            cluster_num = header->root_dir_cluster;
        else
#endif
            cluster_size = header->cluster_zero_offset - header->root_dir_offset;
    }

    /* read entries */
    uint8_t buffer[32];
    while(!arg.finished)
    {
        /* read directory entries up to the cluster border */
        uint16_t cluster_left = cluster_size - cluster_offset;
        uint32_t pos = cluster_offset;
        if(cluster_num == 0)
            pos += header->root_dir_offset;
        else
            pos += fat_cluster_offset(fs, cluster_num);

        arg.bytes_read = 0;
        if(!fs->partition->device_read_interval(pos,
                                                buffer,
                                                sizeof(buffer),
                                                cluster_left,
                                                fat_dir_entry_read_callback,
                                                &arg)
          )
            return 0;

        cluster_offset += arg.bytes_read;

        if(cluster_offset >= cluster_size)
        {
            /* we reached the cluster border and switch to the next cluster */
            cluster_offset = 0;

            /* get number of next cluster */
            if(!(cluster_num = fat_get_next_cluster(fs, cluster_num)))
            {
                /* directory entry not found, reset directory handle */
                cluster_num = dd->dir_entry.cluster;
                break;
            }
        }
    }

    dd->entry_cluster = cluster_num;
    dd->entry_offset = cluster_offset;

    return dir_entry->long_name[0] != '\0' ? 1 : 0;
}

/**
 * \ingroup fat_dir
 * Resets a directory handle.
 *
 * Resets the directory handle such that reading restarts
 * with the first directory entry.
 *
 * \param[in] dd The directory handle to reset.
 * \returns 0 on failure, 1 on success.
 * \see fat_read_dir
 */
uint8_t fat_reset_dir(struct fat_dir_struct* dd)
{
    if(!dd)
        return 0;

    dd->entry_cluster = dd->dir_entry.cluster;
    dd->entry_offset = 0;
    return 1;
}

/**
 * \ingroup fat_fs
 * Callback function for reading a directory entry.
 */
uint8_t fat_dir_entry_read_callback(uint8_t* buffer, offset_t offset, void* p)
{
    struct fat_read_dir_callback_arg* arg = p;
    struct fat_dir_entry_struct* dir_entry = arg->dir_entry;

    arg->bytes_read += 32;

    /* skip deleted or empty entries */
    if(buffer[0] == FAT_DIRENTRY_DELETED || !buffer[0])
        return 1;

    if(!dir_entry->entry_offset)
        dir_entry->entry_offset = offset;
    
    switch(fat_interpret_dir_entry(dir_entry, buffer))
    {
        case 0: /* failure */
        {
            return 0;
        }
        case 1: /* buffer successfully parsed, continue */
        {
            return 1;
        }
        case 2: /* directory entry complete, finish */
        {
            arg->finished = 1;
            return 0;
        }
    }

    return 0;
}

/**
 * \ingroup fat_fs
 * Interprets a raw directory entry and puts the contained
 * information into the directory entry.
 * 
 * For a single file there may exist multiple directory
 * entries. All except the last one are lfn entries, which
 * contain parts of the long filename. The last directory
 * entry is a traditional 8.3 style one. It contains all
 * other information like size, cluster, date and time.
 * 
 * \param[in,out] dir_entry The directory entry to fill.
 * \param[in] raw_entry A pointer to 32 bytes of raw data.
 * \returns 0 on failure, 1 on success and 2 if the
 *          directory entry is complete.
 */
uint8_t fat_interpret_dir_entry(struct fat_dir_entry_struct* dir_entry, const uint8_t* raw_entry)
{
    if(!dir_entry || !raw_entry || !raw_entry[0])
        return 0;

    char* long_name = dir_entry->long_name;
    if(raw_entry[11] == 0x0f)
    {
        /* Lfn supports unicode, but we do not, for now.
         * So we assume pure ascii and read only every
         * second byte.
         */
        uint16_t char_offset = ((raw_entry[0] & 0x3f) - 1) * 13;
        const uint8_t char_mapping[] = { 1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30 };
        for(uint8_t i = 0; i <= 12 && char_offset + i < sizeof(dir_entry->long_name) - 1; ++i)
            long_name[char_offset + i] = raw_entry[char_mapping[i]];

        return 1;
    }
    else
    {
        /* if we do not have a long name, take the short one */
        if(long_name[0] == '\0')
        {
            uint8_t i;
            for(i = 0; i < 8; ++i)
            {
                if(raw_entry[i] == ' ')
                    break;
                long_name[i] = raw_entry[i];

                /* Windows NT and later versions do not store LFN entries
                 * for 8.3 names which have a lowercase basename, extension
                 * or both when everything else is uppercase. They use two
                 * extra bits to signal a lowercase basename or extension.
                 */
                if((raw_entry[12] & 0x08) && raw_entry[i] >= 'A' && raw_entry[i] <= 'Z')
                    long_name[i] += 'a' - 'A';
            }
            if(long_name[0] == 0x05)
                long_name[0] = (char) FAT_DIRENTRY_DELETED;

            if(raw_entry[8] != ' ')
            {
                long_name[i++] = '.';

                uint8_t j = 8;
                for(; j < 11; ++j)
                {
                    if(raw_entry[j] == ' ')
                        break;
                    long_name[i] = raw_entry[j];

                    /* See above for the lowercase 8.3 name handling of
                     * Windows NT and later.
                     */
                    if((raw_entry[12] & 0x10) && raw_entry[j] >= 'A' && raw_entry[j] <= 'Z')
                        long_name[i] += 'a' - 'A';

                    ++i;
                }
            } 

            long_name[i] = '\0';
        }
        
        /* extract properties of file and store them within the structure */
        dir_entry->attributes = raw_entry[11];
        dir_entry->cluster = ltoh16(*((uint16_t*) &raw_entry[26]));
#if FAT_FAT32_SUPPORT
        dir_entry->cluster |= ((cluster_t) ltoh16(*((uint16_t*) &raw_entry[20]))) << 16;
#endif
        dir_entry->file_size = ltoh32(*((uint32_t*) &raw_entry[28]));

#if FAT_DATETIME_SUPPORT
        dir_entry->modification_time = ltoh16(*((uint16_t*) &raw_entry[22]));
        dir_entry->modification_date = ltoh16(*((uint16_t*) &raw_entry[24]));
#endif

        return 2;
    }
}

#if DOXYGEN || FAT_WRITE_SUPPORT
/**
 * \ingroup fat_fs
 * Searches for space where to store a directory entry.
 *
 * \param[in] fs The filesystem on which to operate.
 * \param[in] parent The directory in which to search.
 * \param[in] dir_entry The directory entry for which to search space.
 * \returns 0 on failure, a device offset on success.
 */
offset_t fat_find_offset_for_dir_entry(const struct fat_fs_struct* fs, const struct fat_dir_struct* parent, const struct fat_dir_entry_struct* dir_entry)
{
    if(!fs || !dir_entry)
        return 0;

    /* search for a place where to write the directory entry to disk */
    uint8_t free_dir_entries_needed = (strlen(dir_entry->long_name) + 12) / 13 + 1;
    uint8_t free_dir_entries_found = 0;
    cluster_t cluster_num = parent->dir_entry.cluster;
    offset_t dir_entry_offset = 0;
    offset_t offset = 0;
    offset_t offset_to = 0;
#if FAT_FAT32_SUPPORT
    uint8_t is_fat32 = (fs->partition->type == PARTITION_TYPE_FAT32);
#endif

    if(cluster_num == 0)
    {
#if FAT_FAT32_SUPPORT
        if(is_fat32)
        {
            cluster_num = fs->header.root_dir_cluster;
        }
        else
#endif
        {
            /* we read/write from the root directory entry */
            offset = fs->header.root_dir_offset;
            offset_to = fs->header.cluster_zero_offset;
            dir_entry_offset = offset;
        }
    }
    
    while(1)
    {
        if(offset == offset_to)
        {
            if(cluster_num == 0)
                /* We iterated through the whole root directory and
                 * could not find enough space for the directory entry.
                 */
                return 0;

            if(offset)
            {
                /* We reached a cluster boundary and have to
                 * switch to the next cluster.
                 */

                cluster_t cluster_next = fat_get_next_cluster(fs, cluster_num);
                if(!cluster_next)
                {
                    cluster_next = fat_append_clusters(fs, cluster_num, 1);
                    if(!cluster_next)
                        return 0;

                    /* we appended a new cluster and know it is free */
                    dir_entry_offset = fs->header.cluster_zero_offset +
                                       (offset_t) (cluster_next - 2) * fs->header.cluster_size;

                    /* clear cluster to avoid garbage directory entries */
                    fat_clear_cluster(fs, cluster_next);

                    break;
                }
                cluster_num = cluster_next;
            }

            offset = fat_cluster_offset(fs, cluster_num);
            offset_to = offset + fs->header.cluster_size;
            dir_entry_offset = offset;
            free_dir_entries_found = 0;
        }
        
        /* read next lfn or 8.3 entry */
        uint8_t first_char;
        if(!fs->partition->device_read(offset, &first_char, sizeof(first_char)))
            return 0;

        /* check if we found a free directory entry */
        if(first_char == FAT_DIRENTRY_DELETED || !first_char)
        {
            /* check if we have the needed number of available entries */
            ++free_dir_entries_found;
            if(free_dir_entries_found >= free_dir_entries_needed)
                break;

            offset += 32;
        }
        else
        {
            offset += 32;
            dir_entry_offset = offset;
            free_dir_entries_found = 0;
        }
    }

    return dir_entry_offset;
}
#endif

#if DOXYGEN || FAT_WRITE_SUPPORT
/**
 * \ingroup fat_fs
 * Writes a directory entry to disk.
 *
 * \note The file name is not checked for invalid characters.
 *
 * \note The generation of the short 8.3 file name is quite
 * simple. The first eight characters are used for the filename.
 * The extension, if any, is made up of the first three characters
 * following the last dot within the long filename. If the
 * filename (without the extension) is longer than eight characters,
 * the lower byte of the cluster number replaces the last two
 * characters to avoid name clashes. In any other case, it is your
 * responsibility to avoid name clashes.
 *
 * \param[in] fs The filesystem on which to operate.
 * \param[in] dir_entry The directory entry to write.
 * \returns 0 on failure, 1 on success.
 */
uint8_t fat_write_dir_entry(const struct fat_fs_struct* fs, struct fat_dir_entry_struct* dir_entry)
{
    if(!fs || !dir_entry)
        return 0;
    
#if FAT_DATETIME_SUPPORT
    {
        uint16_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t min;
        uint8_t sec;

        fat_get_datetime(&year, &month, &day, &hour, &min, &sec);
        fat_set_file_modification_date(dir_entry, year, month, day);
        fat_set_file_modification_time(dir_entry, hour, min, sec);
    }
#endif

    device_write_t device_write = fs->partition->device_write;
    offset_t offset = dir_entry->entry_offset;
    const char* name = dir_entry->long_name;
    uint8_t name_len = strlen(name);
    uint8_t lfn_entry_count = (name_len + 12) / 13;
    uint8_t buffer[32];

    /* write 8.3 entry */

    /* generate 8.3 file name */
    memset(&buffer[0], ' ', 11);
    char* name_ext = strrchr(name, '.');
    if(name_ext && *++name_ext)
    {
        uint8_t name_ext_len = strlen(name_ext);
        name_len -= name_ext_len + 1;

        if(name_ext_len > 3)
            name_ext_len = 3;
        
        memcpy(&buffer[8], name_ext, name_ext_len);
    }
    
    if(name_len <= 8)
    {
        memcpy(buffer, name, name_len);

        /* For now, we create lfn entries for all files,
         * except the "." and ".." directory references.
         * This is to avoid difficulties with capitalization,
         * as 8.3 filenames allow uppercase letters only.
         *
         * Theoretically it would be possible to leave
         * the 8.3 entry alone if the basename and the
         * extension have no mixed capitalization.
         */
        if(name[0] == '.' &&
           ((name[1] == '.' && name[2] == '\0') ||
            name[1] == '\0')
          )
            lfn_entry_count = 0;
    }
    else
    {
        memcpy(buffer, name, 8);

        /* Minimize 8.3 name clashes by appending
         * the lower byte of the cluster number.
         */
        uint8_t num = dir_entry->cluster & 0xff;

        buffer[6] = (num < 0xa0) ? ('0' + (num >> 4)) : ('a' + (num >> 4));
        num &= 0x0f;
        buffer[7] = (num < 0x0a) ? ('0' + num) : ('a' + num);
    }
    if(buffer[0] == FAT_DIRENTRY_DELETED)
        buffer[0] = 0x05;

    /* fill directory entry buffer */
    memset(&buffer[11], 0, sizeof(buffer) - 11);
    buffer[0x0b] = dir_entry->attributes;
#if FAT_DATETIME_SUPPORT
    *((uint16_t*) &buffer[0x16]) = htol16(dir_entry->modification_time);
    *((uint16_t*) &buffer[0x18]) = htol16(dir_entry->modification_date);
#endif
#if FAT_FAT32_SUPPORT
    *((uint16_t*) &buffer[0x14]) = htol16((uint16_t) (dir_entry->cluster >> 16));
#endif
    *((uint16_t*) &buffer[0x1a]) = htol16(dir_entry->cluster);
    *((uint32_t*) &buffer[0x1c]) = htol32(dir_entry->file_size);

    /* write to disk */
    if(!device_write(offset + (uint16_t) lfn_entry_count * 32, buffer, sizeof(buffer)))
        return 0;
    
    /* calculate checksum of 8.3 name */
    uint8_t checksum = buffer[0];
    for(uint8_t i = 1; i < 11; ++i)
        checksum = ((checksum >> 1) | (checksum << 7)) + buffer[i];
    
    /* write lfn entries */
    for(uint8_t lfn_entry = lfn_entry_count; lfn_entry > 0; --lfn_entry)
    {
        memset(buffer, 0xff, sizeof(buffer));
        
        /* set file name */
        const char* long_name_curr = name + (lfn_entry - 1) * 13;
        uint8_t i = 1;
        while(i < 0x1f)
        {
            buffer[i++] = *long_name_curr;
            buffer[i++] = 0;

            switch(i)
            {
                case 0x0b:
                    i = 0x0e;
                    break;
                case 0x1a:
                    i = 0x1c;
                    break;
            }

            if(!*long_name_curr++)
                break;
        }
        
        /* set index of lfn entry */
        buffer[0x00] = lfn_entry;
        if(lfn_entry == lfn_entry_count)
            buffer[0x00] |= FAT_DIRENTRY_LFNLAST;

        /* mark as lfn entry */
        buffer[0x0b] = 0x0f;

        /* set 8.3 checksum */
        buffer[0x0d] = checksum;

        /* clear reserved bytes */
        buffer[0x0c] = 0;
        buffer[0x1a] = 0;
        buffer[0x1b] = 0;

        /* write entry */
        device_write(offset, buffer, sizeof(buffer));
    
        offset += sizeof(buffer);
    }
    
    return 1;
}
#endif

#if DOXYGEN || FAT_WRITE_SUPPORT
/**
 * \ingroup fat_file
 * Creates a file.
 *
 * Creates a file and obtains the directory entry of the
 * new file. If the file to create already exists, the
 * directory entry of the existing file will be returned
 * within the dir_entry parameter.
 *
 * \note The file name is not checked for invalid characters.
 *
 * \note The generation of the short 8.3 file name is quite
 * simple. The first eight characters are used for the filename.
 * The extension, if any, is made up of the first three characters
 * following the last dot within the long filename. If the
 * filename (without the extension) is longer than eight characters,
 * the lower byte of the cluster number replaces the last two
 * characters to avoid name clashes. In any other case, it is your
 * responsibility to avoid name clashes.
 *
 * \param[in] parent The handle of the directory in which to create the file.
 * \param[in] file The name of the file to create.
 * \param[out] dir_entry The directory entry to fill for the new file.
 * \returns 0 on failure, 1 on success.
 * \see fat_delete_file
 */
uint8_t fat_create_file(struct fat_dir_struct* parent, const char* file, struct fat_dir_entry_struct* dir_entry)
{
    if(!parent || !file || !file[0] || !dir_entry)
        return 0;

    /* check if the file already exists */
    while(1)
    {
        if(!fat_read_dir(parent, dir_entry))
            break;

        if(strcmp(file, dir_entry->long_name) == 0)
        {
            fat_reset_dir(parent);
            return 0;
        }
    }

    struct fat_fs_struct* fs = parent->fs;

    /* prepare directory entry with values already known */
    memset(dir_entry, 0, sizeof(*dir_entry));
    strncpy(dir_entry->long_name, file, sizeof(dir_entry->long_name) - 1);

    /* find place where to store directory entry */
    if(!(dir_entry->entry_offset = fat_find_offset_for_dir_entry(fs, parent, dir_entry)))
        return 0;
    
    /* write directory entry to disk */
    if(!fat_write_dir_entry(fs, dir_entry))
        return 0;
    
    return 1;
}
#endif

#if DOXYGEN || FAT_WRITE_SUPPORT
/**
 * \ingroup fat_file
 * Deletes a file or directory.
 *
 * If a directory is deleted without first deleting its
 * subdirectories and files, disk space occupied by these
 * files will get wasted as there is no chance to release
 * it and mark it as free.
 * 
 * \param[in] fs The filesystem on which to operate.
 * \param[in] dir_entry The directory entry of the file to delete.
 * \returns 0 on failure, 1 on success.
 * \see fat_create_file
 */
uint8_t fat_delete_file(struct fat_fs_struct* fs, struct fat_dir_entry_struct* dir_entry)
{
    if(!fs || !dir_entry)
        return 0;

    /* get offset of the file's directory entry */
    offset_t dir_entry_offset = dir_entry->entry_offset;
    if(!dir_entry_offset)
        return 0;

    uint8_t buffer[12];
    while(1)
    {
        /* read directory entry */
        if(!fs->partition->device_read(dir_entry_offset, buffer, sizeof(buffer)))
            return 0;
        
        /* mark the directory entry as deleted */
        buffer[0] = FAT_DIRENTRY_DELETED;
        
        /* write back entry */
        if(!fs->partition->device_write(dir_entry_offset, buffer, sizeof(buffer)))
            return 0;

        /* check if we deleted the whole entry */
        if(buffer[11] != 0x0f)
            break;

        dir_entry_offset += 32;
    }

    /* We deleted the directory entry. The next thing to do is
     * marking all occupied clusters as free.
     */
    return (dir_entry->cluster == 0 || fat_free_clusters(fs, dir_entry->cluster));
}
#endif

#if DOXYGEN || FAT_WRITE_SUPPORT
/**
 * \ingroup fat_dir
 * Creates a directory.
 *
 * Creates a directory and obtains its directory entry.
 * If the directory to create already exists, its
 * directory entry will be returned within the dir_entry
 * parameter.
 *
 * \note The notes which apply to fat_create_file also
 * apply to this function.
 *
 * \param[in] parent The handle of the parent directory of the new directory.
 * \param[in] dir The name of the directory to create.
 * \param[out] dir_entry The directory entry to fill for the new directory.
 * \returns 0 on failure, 1 on success.
 * \see fat_delete_dir
 */
uint8_t fat_create_dir(struct fat_dir_struct* parent, const char* dir, struct fat_dir_entry_struct* dir_entry)
{
    if(!parent || !dir || !dir[0] || !dir_entry)
        return 0;

    /* check if the file or directory already exists */
    while(fat_read_dir(parent, dir_entry))
    {
        if(strcmp(dir, dir_entry->long_name) == 0)
        {
            fat_reset_dir(parent);
            return 0;
        }
    }

    struct fat_fs_struct* fs = parent->fs;

    /* allocate cluster which will hold directory entries */
    cluster_t dir_cluster = fat_append_clusters(fs, 0, 1);
    if(!dir_cluster)
        return 0;

    /* clear cluster to prevent bogus directory entries */
    fat_clear_cluster(fs, dir_cluster);
    
    memset(dir_entry, 0, sizeof(*dir_entry));
    dir_entry->attributes = FAT_ATTRIB_DIR;

    /* create "." directory self reference */
    dir_entry->entry_offset = fs->header.cluster_zero_offset +
                              (offset_t) (dir_cluster - 2) * fs->header.cluster_size;
    dir_entry->long_name[0] = '.';
    dir_entry->cluster = dir_cluster;
    if(!fat_write_dir_entry(fs, dir_entry))
    {
        fat_free_clusters(fs, dir_cluster);
        return 0;
    }

    /* create ".." parent directory reference */
    dir_entry->entry_offset += 32;
    dir_entry->long_name[1] = '.';
    dir_entry->cluster = parent->dir_entry.cluster;
    if(!fat_write_dir_entry(fs, dir_entry))
    {
        fat_free_clusters(fs, dir_cluster);
        return 0;
    }

    /* fill directory entry */
    strncpy(dir_entry->long_name, dir, sizeof(dir_entry->long_name) - 1);
    dir_entry->cluster = dir_cluster;

    /* find place where to store directory entry */
    if(!(dir_entry->entry_offset = fat_find_offset_for_dir_entry(fs, parent, dir_entry)))
    {
        fat_free_clusters(fs, dir_cluster);
        return 0;
    }

    /* write directory to disk */
    if(!fat_write_dir_entry(fs, dir_entry))
    {
        fat_free_clusters(fs, dir_cluster);
        return 0;
    }

    return 1;
}
#endif

/**
 * \ingroup fat_dir
 * Deletes a directory.
 *
 * This is just a synonym for fat_delete_file().
 * If a directory is deleted without first deleting its
 * subdirectories and files, disk space occupied by these
 * files will get wasted as there is no chance to release
 * it and mark it as free.
 * 
 * \param[in] fs The filesystem on which to operate.
 * \param[in] dir_entry The directory entry of the directory to delete.
 * \returns 0 on failure, 1 on success.
 * \see fat_create_dir
 */
#ifdef DOXYGEN
uint8_t fat_delete_dir(struct fat_fs_struct* fs, struct fat_dir_entry_struct* dir_entry);
#endif

#if DOXYGEN || FAT_DATETIME_SUPPORT
/**
 * \ingroup fat_file
 * Returns the modification date of a file.
 *
 * \param[in] dir_entry The directory entry of which to return the modification date.
 * \param[out] year The year the file was last modified.
 * \param[out] month The month the file was last modified.
 * \param[out] day The day the file was last modified.
 */
void fat_get_file_modification_date(const struct fat_dir_entry_struct* dir_entry, uint16_t* year, uint8_t* month, uint8_t* day)
{
    if(!dir_entry)
        return;

    *year = 1980 + ((dir_entry->modification_date >> 9) & 0x7f);
    *month = (dir_entry->modification_date >> 5) & 0x0f;
    *day = (dir_entry->modification_date >> 0) & 0x1f;
}
#endif

#if DOXYGEN || FAT_DATETIME_SUPPORT
/**
 * \ingroup fat_file
 * Returns the modification time of a file.
 *
 * \param[in] dir_entry The directory entry of which to return the modification time.
 * \param[out] hour The hour the file was last modified.
 * \param[out] min The min the file was last modified.
 * \param[out] sec The sec the file was last modified.
 */
void fat_get_file_modification_time(const struct fat_dir_entry_struct* dir_entry, uint8_t* hour, uint8_t* min, uint8_t* sec)
{
    if(!dir_entry)
        return;

    *hour = (dir_entry->modification_time >> 11) & 0x1f;
    *min = (dir_entry->modification_time >> 5) & 0x3f;
    *sec = ((dir_entry->modification_time >> 0) & 0x1f) * 2;
}
#endif

#if DOXYGEN || (FAT_WRITE_SUPPORT && FAT_DATETIME_SUPPORT)
/**
 * \ingroup fat_file
 * Sets the modification time of a date.
 *
 * \param[in] dir_entry The directory entry for which to set the modification date.
 * \param[in] year The year the file was last modified.
 * \param[in] month The month the file was last modified.
 * \param[in] day The day the file was last modified.
 */
void fat_set_file_modification_date(struct fat_dir_entry_struct* dir_entry, uint16_t year, uint8_t month, uint8_t day)
{
    if(!dir_entry)
        return;

    dir_entry->modification_date =
        ((year - 1980) << 9) |
        ((uint16_t) month << 5) |
        ((uint16_t) day << 0);
}
#endif

#if DOXYGEN || (FAT_WRITE_SUPPORT && FAT_DATETIME_SUPPORT)
/**
 * \ingroup fat_file
 * Sets the modification time of a file.
 *
 * \param[in] dir_entry The directory entry for which to set the modification time.
 * \param[in] hour The year the file was last modified.
 * \param[in] min The month the file was last modified.
 * \param[in] sec The day the file was last modified.
 */
void fat_set_file_modification_time(struct fat_dir_entry_struct* dir_entry, uint8_t hour, uint8_t min, uint8_t sec)
{
    if(!dir_entry)
        return;

    dir_entry->modification_time =
        ((uint16_t) hour << 11) |
        ((uint16_t) min << 5) |
        ((uint16_t) sec >> 1) ;
}
#endif

/**
 * \ingroup fat_fs
 * Returns the amount of total storage capacity of the filesystem in bytes.
 *
 * \param[in] fs The filesystem on which to operate.
 * \returns 0 on failure, the filesystem size in bytes otherwise.
 */
offset_t fat_get_fs_size(const struct fat_fs_struct* fs)
{
    if(!fs)
        return 0;

#if FAT_FAT32_SUPPORT
    if(fs->partition->type == PARTITION_TYPE_FAT32)
        return (offset_t) (fs->header.fat_size / 4 - 2) * fs->header.cluster_size;
    else
#endif
        return (offset_t) (fs->header.fat_size / 2 - 2) * fs->header.cluster_size;
}

/**
 * \ingroup fat_fs
 * Returns the amount of free storage capacity on the filesystem in bytes.
 *
 * \note As the FAT filesystem is cluster based, this function does not
 *       return continuous values but multiples of the cluster size.
 *
 * \param[in] fs The filesystem on which to operate.
 * \returns 0 on failure, the free filesystem space in bytes otherwise.
 */
offset_t fat_get_fs_free(const struct fat_fs_struct* fs)
{
    if(!fs)
        return 0;

    uint8_t fat[32];
    struct fat_usage_count_callback_arg count_arg;
    count_arg.cluster_count = 0;
    count_arg.buffer_size = sizeof(fat);

    offset_t fat_offset = fs->header.fat_offset;
    uint32_t fat_size = fs->header.fat_size;
    while(fat_size > 0)
    {
        uintptr_t length = UINTPTR_MAX - 1;
        if(fat_size < length)
            length = fat_size;

        if(!fs->partition->device_read_interval(fat_offset,
                                                fat,
                                                sizeof(fat),
                                                length,
#if FAT_FAT32_SUPPORT
                                                (fs->partition->type == PARTITION_TYPE_FAT16) ?
                                                    fat_get_fs_free_16_callback :
                                                    fat_get_fs_free_32_callback,
#else
                                                fat_get_fs_free_16_callback,
#endif
                                                &count_arg
                                               )
          )
            return 0;

        fat_offset += length;
        fat_size -= length;
    }

    return (offset_t) count_arg.cluster_count * fs->header.cluster_size;
}

/**
 * \ingroup fat_fs
 * Callback function used for counting free clusters in a FAT.
 */
uint8_t fat_get_fs_free_16_callback(uint8_t* buffer, offset_t offset, void* p)
{
    struct fat_usage_count_callback_arg* count_arg = (struct fat_usage_count_callback_arg*) p;
    uintptr_t buffer_size = count_arg->buffer_size;

    for(uintptr_t i = 0; i < buffer_size; i += 2, buffer += 2)
    {
        uint16_t cluster = *((uint16_t*) &buffer[0]);
        if(cluster == HTOL16(FAT16_CLUSTER_FREE))
            ++(count_arg->cluster_count);
    }

    return 1;
}

#if DOXYGEN || FAT_FAT32_SUPPORT
/**
 * \ingroup fat_fs
 * Callback function used for counting free clusters in a FAT32.
 */
uint8_t fat_get_fs_free_32_callback(uint8_t* buffer, offset_t offset, void* p)
{
    struct fat_usage_count_callback_arg* count_arg = (struct fat_usage_count_callback_arg*) p;
    uintptr_t buffer_size = count_arg->buffer_size;

    for(uintptr_t i = 0; i < buffer_size; i += 4, buffer += 4)
    {
        uint32_t cluster = *((uint32_t*) &buffer[0]);
        if(cluster == HTOL32(FAT32_CLUSTER_FREE))
            ++(count_arg->cluster_count);
    }

    return 1;
}
#endif

