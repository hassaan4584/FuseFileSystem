/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall hello.c `pkg-config fuse --cflags --libs` -o hello
*/

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include "fuse_operations.h"
#include "Constants.h"
#include "helpers.h"

int gettimeofday(struct timeval *restrict tp, void *restrict tzp);

static int haiga_getattr(const char *path, struct stat *stbuf)
{
    printf("Get Attr FUNCTION PATH: %s\n", path);
	int res = 0;

    struct timeval tv;
    struct timespec ts;

    // reset memory for the stat structure
	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0777;
		stbuf->st_nlink = 2;
        return res;
	}
    else if (strcmp(path, "/checkpoints") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return res;
    }
    else {
        readAllFileNamesFromiNodeZero();
        for (int i=0 ; i<totalFileCount ; i++) {
            if (strcmp(path, iNodeZeroFileNames[i].fileName) == 0) {
                stbuf->st_mode = S_IFREG | 0666;
                stbuf->st_nlink = 1;
                int iNodeNo = iNodeZeroFileNames[i].iNodeNumber;
                stbuf->st_size = getSizeOfFile(iNodeNo);
                gettimeofday(&tv, NULL);
                ts.tv_sec = tv.tv_sec;
                ts.tv_nsec = 0;
                stbuf->st_ctimespec = ts;
                stbuf->st_birthtimespec = ts;
                stbuf->st_atimespec = ts;
                stbuf->st_mtimespec = ts;

                return res;
            }

        }
    }
    
    res = -ENOENT;
    return res;

}

// Return one or more directory entries (struct dirent) to the caller. This is one of the most complex FUSE functions. It is related to, but not identical to, the readdir(2) and getdents(2) system calls, and the readdir(3) library function. Because of its complexity, it is described separately below. Required for essentially any filesystem, since it's what makes ls and a whole bunch of other things work.
static int haiga_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
    printf("READDIR FUNCTION \n");
	(void) offset;
	(void) fi;

	if (strcmp(path, "/") != 0)
		return -ENOENT;
    
    if (strcmp(path, "/checkpoints") == 0) {
        readAllCheckpoints();
        return 0;
    }


	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
    filler(buf, "checkpoints", NULL, 0);
    readAllFileNamesFromiNodeZero();
    for (int i=0 ; i<totalFileCount ; i++) {
        if (strlen(iNodeZeroFileNames[i].fileName) > 0) {
            filler(buf, iNodeZeroFileNames[i].fileName + 1, NULL, 0);
        }
    }
	return 0;
}


static int haiga_open(const char *path, struct fuse_file_info *fi)
{
    printf("OPEN FUNCTION \n");
    int isFileFound = 0;
    for (int i=0 ; i<totalFileCount ; i++) {
        if (strcmp(path, iNodeZeroFileNames[i].fileName) == 0) {
            isFileFound = 1;
        }
    }
    if (isFileFound == 0) {
        return -ENOENT;
    }
    fi->flags = O_RDWR;

	return 0;
}

// Read size bytes from the given file into the buffer buf, beginning offset bytes into the file. See read(2) for full details. Returns the number of bytes transferred, or 0 if offset was at or beyond the end of the file. Required for any sensible filesystem.
static int haiga_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
    printf("READ FUNCTION with Path: %s\n", path );
	(void) fi;
    int inodeNumber = -1;
    int isFileFound = 0;
    for (int i=0 ; i<totalFileCount ; i++) {
        if (strcmp(path, iNodeZeroFileNames[i].fileName) == 0) {
            isFileFound = 1;
            inodeNumber = iNodeZeroFileNames[i].iNodeNumber;
        }
    }
    if (isFileFound == 0) {
        return -ENOENT;
    }
    if ((int)size > (264*1024)) {
        return -EFBIG;
    }

    
    fseek(filehd, getLocationOfiNode(inodeNumber), SEEK_SET); // Go to the inode number
    // First 4 bytes of this inode will represent size of the file

    int *fileSize = malloc(sizeof(int));
    fread(fileSize, sizeof(int), 1, filehd); // get total size of the file
    *fileSize = ((*fileSize) < (int)size) ? *fileSize : (int)size;

    if (offset > *fileSize) {
        return 0;
    }
    
    // e.g if the original file size is 2024B then 1024B will be in the first block and remaining 1000B will be in the 2nd/last block
    int numberOfBlocks = (*fileSize)/BLOCK_SIZE; // 2024/1024 = 1
    int *blockNumber = malloc(sizeof(int));
    for (int i=0 ; i<numberOfBlocks && i<8; i++) {
        fseek(filehd, (getLocationOfiNode(inodeNumber)+4+(4*i)), SEEK_SET); // Read block number from our iNode
        fread(blockNumber, sizeof(int), 1, filehd); // loop will read/skip the 1st block
        if (*blockNumber == -1) { // previously this block pointed to no data
            return 0; // There is no data in this block.
        }
        else {
            size_t dataBlockLocation = DATA_BLOCKS_BASE_ADDR + ((*blockNumber)*BLOCK_SIZE);
            fseek(filehd, dataBlockLocation, SEEK_SET); // Go to the ith data block of the iNode
            // we will read complete BLOCK_SIZE bytes from this block because it is not the last data block of this file and hence it is completely filled
            fread((void*)(buf+(i*BLOCK_SIZE)), BLOCK_SIZE, 1, filehd);
        }
    }
    if (numberOfBlocks > 8) {
        // read data from indirect block
        int iNodeLocation = getLocationOfiNode(inodeNumber);
        fseek(filehd, (iNodeLocation+36), SEEK_SET); // Get to the indirect block storage point
        int indirectBlock = -1;
        fread(&indirectBlock, sizeof(int), 1, filehd);
        if (indirectBlock < 0) {
            return numberOfBlocks*BLOCK_SIZE;
        }
        size_t inDirectBlockStartLocation = DATA_BLOCKS_BASE_ADDR + (indirectBlock*BLOCK_SIZE);
        
        for (int i=0 ; i<=numberOfBlocks-8 ; i++) {
            fseek(filehd, (inDirectBlockStartLocation+(i*4)), SEEK_SET); // Read block number from our iNode
            // we will read complete BLOCK_SIZE bytes from this block because it is not the last data block of this file and hence it is completely filled
            fread((void*)(buf+((i*BLOCK_SIZE)+(8*BLOCK_SIZE))), BLOCK_SIZE, 1, filehd);
        }
        return numberOfBlocks*BLOCK_SIZE;
    }
    else {
        fseek(filehd, (getLocationOfiNode(inodeNumber)+4+(numberOfBlocks*4)), SEEK_SET); // Get to the last block number from our iNode
        fread(blockNumber, sizeof(int), 1, filehd); // Read last block number from our iNode
        if (*blockNumber == -1) {
            return numberOfBlocks*BLOCK_SIZE; // return the number of bytes that we have written so far in buffer
        }
        
        int lastBlockDataSize = (*fileSize) % BLOCK_SIZE; // now we will read the remaining 1000 bytes
        size_t lastBlockStartLocation = DATA_BLOCKS_BASE_ADDR + ((*blockNumber)*BLOCK_SIZE); // Read the last block number from the eight-4 bytes block numbers
        fseek(filehd, lastBlockStartLocation, SEEK_SET);
        char data[BLOCK_SIZE] = "\0";
        int reading = fread((void*)(data), lastBlockDataSize, 1, filehd);
        memcpy(buf, data , lastBlockDataSize);
        int totalBytesRead = (numberOfBlocks*BLOCK_SIZE) + lastBlockDataSize;
        
        return totalBytesRead;
    }

    
}



static struct fuse_operations haiga_operations = {
	.getattr	= haiga_getattr,
	.readdir	= haiga_readdir,
    .read		= haiga_read,
	.open		= haiga_open,
    .destroy    = haiga_destroy,
    .init       = haiga_init,
//    .fgetattr   = haiga_fgetattr,
//    .access     = haiga_access,
    .mknod      = haiga_mknod,
    .write      = haiga_write,
//    .statfs     = haiga_statfs,
    .release    = haiga_release,
    .releasedir = haiga_releaseDir,
    .fsync      = haiga_fsync,
    .fsyncdir   = haiga_fsyncDir,
//    .flush      = haiga_flush,
//    .lock       = haiga_lock,
//    .bmap       = haiga_bmap,
//    .setxattr   = haiga_setxattr,
//    .getxattr   = haiga_getxattr,
//    .listxattr  = haiga_listxattr,
    .create     = haiga_create,
    .readlink   = haiga_readlink,
//    .mkdir      = haiga_mkdir,
//    .unlink     = haiga_unlink,
//    .rmdir      = haiga_rmdir,
//    .link       = haiga_link,
    .chmod      = haiga_chmod,
    .chown      = haiga_chown,
    .truncate   = haiga_truncate,
//    .utimens    = haiga_utimens,
    .rename     = haiga_rename,
//    .symlink    = haiga_symlink,
    
};

int main(int argc, char *argv[])
{
    initializeUsedBlockCount(); // setting the global blocks used variable
    
    filehd = fopen(LOG_FILE_PATH, "r+");
    if (filehd == NULL) {
        filehd = fopen(LOG_FILE_PATH, "w+");
        initializeLogFile();
    }
    readAllFileNamesFromiNodeZero();
    

    
    
    
    
    
    
    
    
    
    
    
    
    int retVal = fuse_main(argc, argv, &haiga_operations, NULL);
    fclose(filehd);
    saveUsedBlockCount();
    
    return retVal;
}
