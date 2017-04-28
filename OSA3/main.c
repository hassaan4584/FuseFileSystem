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


static int haiga_getattr(const char *path, struct stat *stbuf)
{
    printf("Get Attr FUNCTION Moun\n");
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
        return res;
	}
//    else if (strcmp(path, haiga_path) == 0) {
//        stbuf->st_mode = S_IFREG | 0666;
//        stbuf->st_nlink = 1;
//        stbuf->st_size = strlen(haiga_str);
//        return res;
//    }
    else {
        for (int i=0; i<BLOCK_COUNT ; i++) {
            if (strcmp(path, fileNamesArr[i]) == 0) {
                stbuf->st_mode = S_IFREG | 0666;
                stbuf->st_nlink = 1;
                stbuf->st_size = strlen(fileDataArr[i]);
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
//	filler(buf, haiga_path + 1, NULL, 0);
    for(int i=0 ; i<BLOCK_COUNT ; i++) {
        filler(buf, fileNamesArr[i] + 1, NULL, 0);
    }
	return 0;
}


static int haiga_open(const char *path, struct fuse_file_info *fi)
{
    printf("OPEN FUNCTION \n");
    int isFileFound = 0;
//    if (strcmp(path, haiga_path) != 0) {
        for (int i=0; i<BLOCK_COUNT ; i++) {
            if (strcmp(path, fileNamesArr[i]) == 0) {
                isFileFound = 1;
            }
        }
        if (isFileFound == 0) {
            return -ENOENT;
        }
//    }
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
    int fileNumber = -1;
    int isFileFound = 0;
//    if (strcmp(path, haiga_path) != 0) {
        for (int i=0; i<BLOCK_COUNT ; i++) {
            if (strcmp(path, fileNamesArr[i]) == 0) {
                isFileFound = 1;
                fileNumber = i;
            }
        }
        if (isFileFound == 0) {
            return -ENOENT;
        }
//    }

	len = strlen(fileDataArr[fileNumber]);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, fileDataArr[fileNumber] + offset, size);
	} else
		size = 0;

	return (int)size;
}



static struct fuse_operations haiga_operations = {
	.getattr	= haiga_getattr,
	.readdir	= haiga_readdir,
	.open		= haiga_open,
	.read		= haiga_read,
    .destroy    = haiga_destroy,
    .init       = haiga_init,
    .fgetattr   = haiga_fgetattr,
    .access     = haiga_access,
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
//    .ioctl      = haiga_ioctl,
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
    for(int i=0 ; i<BLOCK_COUNT ; i++) {
        sprintf(fileNamesArr[i], "/%d", i);
    }
    
//    logFd = open(LOG_FILE_PATH, O_RDWR);
    FILE *file = fopen(LOG_FILE_PATH, "w+");
    if (file == NULL) {
        printf("LOG FILE COULD FAILED TO OPEN");

    }
    
    if (logFd < 0) {
        printf("LOG FILE COULD FAILED TO OPEN");
    }
    for (int i=0 ; i<BLOCK_COUNT ; i++) {
        fseek(file, i*1024, SEEK_SET);
        const char buff[] = "This is some text\n";
        fwrite((void*)buff, sizeof(char), sizeof(buff), file);
        
        sprintf(fileDataArr[i], "This is plain text on block number :"
                "%d %d %d %d %d %d %d %d %d %d\n", i, i, i, i, i, i ,i, i, i, i);
    }
    fclose(file);

    int retVal = fuse_main(argc, argv, &haiga_operations, NULL);
//    close(logFd);
    return retVal;
}
