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


const int BLOCK_SIZE        = 1024;
const int BLOCK_COUNT       = 2;
const int INODE_SIZE        = 40;
const int INODE_COUNT       = 1024;
const int DATA_BLOCKS_BASE_ADDR = (INODE_SIZE*INODE_COUNT);

/**
 * The number of blocks of data used after INODE_SIZE*INODE_COUNT bytes of data.
 * e.g The number of blocks after the initail 40KB of reserved data.
 */
static int blocksUsed   = 0;

/**
 *
 * Number of the last iNode that we updated.
 *
 */
static int lastInodeWritten = 1;
#warning each time we start the filesystem this would be set to 0

const char* LOG_FILE_PATH   = "/Users/Hassaan/Desktop/haiga_lfs.txt";
const char* BLOCK_COUNT_FILE_PATH   = "/Users/Hassaan/Desktop/blockCount.txt";

static char fileNamesArr[INODE_COUNT][BLOCK_SIZE];
static FILE* filehd;



#endif /* Constants_h */
