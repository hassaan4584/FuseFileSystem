//
//  Constants.h
//  OSA3
//
//  Created by Hassaan on 25/04/2017.
//  Copyright © 2017 HaigaTech. All rights reserved.
//

#ifndef Constants_h
#define Constants_h

#include <stdlib.h>
#include "dataStructures.h"

/**
 * Size of a single block.
 */
const int BLOCK_SIZE        = 1024;

/**
 * Only for Part 1. The total number of blocks to store the information of files.
 */
const int BLOCK_COUNT       = 2;

/**
 * Size of each iNode in bytes.
 */
const int INODE_SIZE        = 40;

/**
 * The total number of iNodes.
 */
const int INODE_COUNT       = 1024;

/**
 * The size in bytes of super block.
 */
const int SUPER_BLOCK_SIZE  = 1024;

/**
 * The size in bytes of metadata of filesystem.
 */
const int METADATA_SIZE       = 1024;

/**
 * The maximum number of files that the filesystem can handle.
 */
const int MAX_SUPPORTED_FILE_COUNT  = INODE_COUNT;

/**
 * The maximum length of filename that the filesystem can handle.
 */
const int MAX_FILENAME_LENGTH       = 100;

/**
 * Base address of the data blcoks. Data blocks start after the super block and metadata and iNodes.
 */
const int iNODES_BASE_ADDR          = SUPER_BLOCK_SIZE + METADATA_SIZE;

/**
 * Base address of the data blcoks. Data blocks start after the super block and metadata and iNodes.
 */
const int DATA_BLOCKS_BASE_ADDR     = (INODE_SIZE*INODE_COUNT)+SUPER_BLOCK_SIZE+METADATA_SIZE;

/**
 * The number of blocks of data used after INODE_SIZE*INODE_COUNT bytes of data.
 * e.g The number of blocks after the initail 40KB of reserved data.
 */
static int blocksUsed   = 0;

/**
 * For Part 3. The number of files that have metadata in the iNode0.
 */
static int currentFileNameCount  = 0;

/**
 * For Part 3. Number of files that are currently present in the filesystem.
 */
static int totalFileCount  = 0;

const char* LOG_FILE_PATH   = "/Users/Hassaan/Desktop/haiga_lfs-Part4.txt";
const char* BLOCK_COUNT_FILE_PATH   = "/Users/Hassaan/Desktop/blockCount-P4.txt";

//static char fileNamesArr[INODE_COUNT][MAX_FILENAME_LENGTH];

/**
 * For Part 3. Names of all the files saved in iNode0
 */
static struct iNodeZero iNodeZeroFileNames [MAX_SUPPORTED_FILE_COUNT];
//static char iNodeZeroFileNames[MAX_SUPPORTED_FILE_COUNT][MAX_FILENAME_LENGTH];

static FILE* filehd;



#endif /* Constants_h */
