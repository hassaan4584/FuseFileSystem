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
#include "fuse_operations.h"
#include "Constants.h"
#include <time.h>

static int haiga_getattr(const char *path, struct stat *stbuf)
{
    printf("Get Attr FUNCTION Moun\n");
	int res = 0;

    struct timeval tv;
    struct timespec ts;

    // reset memory for the stat structure
	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
        return res;
	}
    else {
        for (int i=0; i<INODE_COUNT ; i++) {
            if (strcmp(path, fileNamesArr[i]) == 0) {
                stbuf->st_mode = S_IFREG | 0666;
                stbuf->st_nlink = 1;
#warning fix st_size and make it generic
                stbuf->st_size = 0;
                gettimeofday(&tv, NULL);
                ts.tv_sec = tv.tv_sec;
                ts.tv_nsec = 0;
                stbuf->st_ctimespec = ts;
//                stbuf->st_birthtimespec = ts;
//                stbuf->st_atimespec = ts;
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

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
    for(int i=0 ; i<INODE_COUNT ; i++) {
        filler(buf, fileNamesArr[i] + 1, NULL, 0);
    }
	return 0;
}


static int haiga_open(const char *path, struct fuse_file_info *fi)
{
    printf("OPEN FUNCTION \n");
    int isFileFound = 0;
    for (int i=0; i<INODE_COUNT ; i++) {
        if (strcmp(path, fileNamesArr[i]) == 0) {
            isFileFound = 1;
        }
    }
    if (isFileFound == 0) {
        return -ENOENT;
    }
    fi->flags = O_RDWR;

//	if ((fi->flags & 3) != O_RDONLY)
//		return -EACCES;

	return 0;
}

// Read size bytes from the given file into the buffer buf, beginning offset bytes into the file. See read(2) for full details. Returns the number of bytes transferred, or 0 if offset was at or beyond the end of the file. Required for any sensible filesystem.
static int haiga_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
    printf("READ FUNCTION \n");
	size_t len;
	(void) fi;
    int inodeNumber = -1;
    int isFileFound = 0;
    for (int i=0; i<INODE_COUNT ; i++) {
        if (strcmp(path, fileNamesArr[i]) == 0) {
            isFileFound = 1;
            inodeNumber = i;
        }
    }
    if (isFileFound == 0) {
        return -ENOENT;
    }
    
    fseek(filehd, inodeNumber*INODE_SIZE, SEEK_SET); // Go to the inode number
    // First 4 bytes of this inode will represent size of the file

    int *fileSize = malloc(sizeof(int));
    fread(fileSize, sizeof(int), 1, filehd); // get total size of the file

#warning Assuming the file can be of 1KB only
//    int numberOfBlocks = (*fileSize)/BLOCK_SIZE; // 2024/1024 = 1
//    for (int i=0 ; i<numberOfBlocks; i++) {
//        fread(blockNumber, sizeof(int), 1, filehd); // loop will read/skip the 1st block
//    }
    int *blockNumber = malloc(sizeof(int)); // Assuming for now, iNode can point to one block of data only
    fread(blockNumber, sizeof(int), 1, filehd); // get the block number

    int location = (INODE_SIZE*INODE_COUNT) + (*blockNumber)*BLOCK_SIZE;
    fseek(filehd, location, SEEK_SET);

    len = *fileSize; // Assuming for now, this filesize is less than 1KB
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
        fseek(filehd, offset, SEEK_CUR);
        fread(buf, size, 1, filehd);
//		memcpy(buf, fileDataArr[fileNumber] + offset, size);
	} else
		size = 0;

	return (int)size;
}



static struct fuse_operations haiga_operations = {
	.getattr	= haiga_getattr,
	.readdir	= haiga_readdir,
    .read		= haiga_read,
	.open		= haiga_open,
    .destroy    = haiga_destroy,
    .init       = haiga_init,
    .fgetattr   = haiga_fgetattr,
//    .access     = haiga_access,
    .mknod      = haiga_mknod,
    .write      = haiga_write,
    .statfs     = haiga_statfs,
    .release    = haiga_release,
    .releasedir = haiga_releaseDir,
    .fsync      = haiga_fsync,
    .fsyncdir   = haiga_fsyncDir,
    .flush      = haiga_flush,
    .lock       = haiga_lock,
    .bmap       = haiga_bmap,
    .setxattr   = haiga_setxattr,
    .getxattr   = haiga_getxattr,
    .listxattr  = haiga_listxattr,
    .create     = haiga_create,
    .readlink   = haiga_readlink,
    .mkdir      = haiga_mkdir,
    .unlink     = haiga_unlink,
    .rmdir      = haiga_rmdir,
    .link       = haiga_link,
    .chmod      = haiga_chmod,
    .chown      = haiga_chown,
    .truncate   = haiga_truncate,
    .utimens    = haiga_utimens,
    .rename     = haiga_rename,
    .symlink    = haiga_symlink,
    
};

int main(int argc, char *argv[])
{
    for(int i=0 ; i<INODE_COUNT ; i++) {
        sprintf(fileNamesArr[i], "/%d", i);
    }
    
    filehd = fopen(LOG_FILE_PATH, "w+");
    if (filehd == NULL) {
        printf("LOG FILE COULD FAILED TO OPEN");
    }
    
    for (int i=0 ; i<INODE_COUNT ; i++) {
        
        fseek(filehd, i*INODE_SIZE, SEEK_SET);
        int *fileSize = (int*) malloc(sizeof(int));
        *fileSize = 0;
        fwrite((void*)fileSize, sizeof(int), 1, filehd); // writing size of the file in the 1st 4 bytes of the inode
        
        int blockNumber = -1;
        for(int j=0 ; j<8 ; j++) {
            fwrite((void*)&blockNumber, sizeof(int), 1, filehd); // initializing next 32 bytes with eigh 4 byte block numbers
        }
        
        fseek(filehd, 4, SEEK_CUR); // Leaving the last 4 bytes of the inode empty
    }

    
    
    
    
    
    
    
    
    
    
    
    
    int retVal = fuse_main(argc, argv, &haiga_operations, NULL);
    fclose(filehd);
    return retVal;
}
