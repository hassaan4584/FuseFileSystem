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
#include "helpers.h"
#include "Constants.h"

static const char *haiga_str = "Hello World!\n";
static const char *haiga_path = "/hello";
static char fileNamesArr[FILE_COUNT][MAX_FILE_SIZE];


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
    else if (strcmp(path, haiga_path) == 0) {
        stbuf->st_mode = S_IFREG | 0777;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(haiga_str);
        return res;
    }
    else {
        for (int i=0; i<FILE_COUNT ; i++) {
            if (strcmp(path, fileNamesArr[i]) == 0) {
                stbuf->st_mode = S_IFREG | 0777;
                stbuf->st_nlink = 1;
                stbuf->st_size = strlen(haiga_str);
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
	filler(buf, haiga_path + 1, NULL, 0);
    for(int i=0 ; i<FILE_COUNT ; i++) {
        filler(buf, fileNamesArr[i] + 1, NULL, 0);
    }
	return 0;
}


static int haiga_open(const char *path, struct fuse_file_info *fi)
{
    printf("OPEN FUNCTION \n");
//	if (strcmp(path, haiga_path) != 0)
//		return -ENOENT;
//
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
//	if(strcmp(path, haiga_path) != 0)
//		return -ENOENT;

	len = strlen(haiga_str);
	if (offset < len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, haiga_str + offset, size);
	} else
		size = 0;

	return (int)size;
}



// Poll for I/O readiness. If ph is non-NULL, when the filesystem is ready for I/O it should call fuse_notify_poll (possibly asynchronously) with the specified ph; this will clear all pending polls. The callee is responsible for destroying ph with fuse_pollhandle_destroy() when ph is no longer needed.
//static int haiga_poll(const char* path, struct fuse_file_info* fi, struct fuse_pollhandle* ph, unsigned* reventsp)
//{
//    return 0;
//}


static struct fuse_operations haiga_operations = {
	.getattr	= haiga_getattr,
	.readdir	= haiga_readdir,
	.open		= haiga_open,
	.read		= haiga_read,
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
//    .ioctl      = haiga_ioctl,
    .create     = haiga_create,
    
    
};

int main(int argc, char *argv[])
{
    for(int i=0 ; i<FILE_COUNT ; i++) {
        sprintf(fileNamesArr[i], "/%d", i);
    }

	return fuse_main(argc, argv, &haiga_operations, NULL);
}
