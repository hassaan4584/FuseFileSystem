//
//  helpers.h
//  OSA3
//
//  Created by Hassaan on 25/04/2017.
//  Copyright © 2017 HaigaTech. All rights reserved.
//

#ifndef helpers_h
#define helpers_h



// Called when the filesystem exits. The private_data comes from the return value of init.
static void haiga_destroy(void* private_data)
{
    printf("DESTROY FUNCTION \n");
    
    //    return 0;
}

void* haiga_init(struct fuse_conn_info *conn)
{
    printf("INIT FUNCTION \n");
    
    
    return conn;
}


// As getattr, but called when fgetattr(2) is invoked by the user program.
static int haiga_fgetattr(const char* path, struct stat* stbuf, struct fuse_file_info * fileInfo)
{
    printf("F GETARR FUNCTION \n");
    
    return 0;
}

//This is the same as the access(2) system call. It returns -ENOENT if the path doesn't exist, -EACCESS if the requested permission isn't available, or 0 for success. Note that it can be called on files, directories, or any other object that appears in the filesystem. This call is not required but is highly recommended.
static int haiga_access(const char* path, int mask)
{
    printf("ACCESS FUNCTION \n");
    
    return 0;
}

//Make a special (device) file, FIFO, or socket. See mknod(2) for details. This function is rarely needed, since it's uncommon to make these objects inside special-purpose filesystems.
static int haiga_mknod(const char* path, mode_t mode, dev_t rdev)
{
    printf("mknod FUNCTION \n");
    return 0;
}




// readlink(const char* path, char* buf, size_t size)
// If path is a symbolic link, fill buf with its target, up to size. See readlink(2) for how to handle a too-small buffer and for error codes. Not required if you don't support symbolic links. NOTE: Symbolic-link support requires only readlink and symlink. FUSE itself will take care of tracking symbolic links in paths, so your path-evaluation code doesn't need to worry about it.


// As for read above, except that it can't return 0.
static int haiga_write(const char* path, const char *buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
    printf("WRITE FUNCTION \n");
    
    return 1;
}


// Return statistics about the filesystem. See statvfs(2) for a description of the structure contents. Usually, you can ignore the path. Not required, but handy for read/write filesystems since this is how programs like df determine the free space.
static int haiga_statfs(const char* path, struct statvfs* stbuf)
{
    printf("STATFS FUNCTION \n");
    return 0;
}


// This is the only FUSE function that doesn't have a directly corresponding system call, although close(2) is related. Release is called when FUSE is completely done with a file; at that point, you can free up any temporarily allocated data structures. The IBM document claims that there is exactly one release per open, but I don't know if that is true.
static int haiga_release(const char* path, struct fuse_file_info *fi)
{
    printf("RELEASE FUNCTION \n");
    return 0;
}

// This is like release, except for directories.
static int haiga_releaseDir(const char* path, struct fuse_file_info *fi)
{
    
    printf("RELEASE DIR FUNCTION \n");
    return 0;
}


// Flush any dirty information about the file to disk. If isdatasync is nonzero, only data, not metadata, needs to be flushed. When this call returns, all file data should be on stable storage. Many filesystems leave this call unimplemented, although technically that's a Bad Thing since it risks losing data. If you store your filesystem inside a plain file on another filesystem, you can implement this by calling fsync(2) on that file, which will flush too much data (slowing performance) but achieve the desired guarantee.
static int haiga_fsync(const char* path, int isdatasync, struct fuse_file_info* fi)
{
    printf("FSYNC FUNCTION \n");
    return 0;
}

// Like fsync, but for directories.
static int haiga_fsyncDir(const char* path, int isdatasync, struct fuse_file_info* fi)
{
    
    printf("FSYNCDIR FUNCTION \n");
    return 0;
}

// Called on each close so that the filesystem has a chance to report delayed errors. Important: there may be more than one flush call for each open. Note: There is no guarantee that flush will ever be called at all!
static int haiga_flush(const char* path, struct fuse_file_info* fi)
{
    printf("FLUSH FUNCTION \n");
    return 0;
}

// Perform a POSIX file-locking operation. See details below.
static int haiga_lock(const char* path, struct fuse_file_info* fi, int cmd, struct flock* locks)
{
    printf("LOCK FUNCTION \n");
    
    return 0;
}


// This function is similar to bmap(9). If the filesystem is backed by a block device, it converts blockno from a file-relative block number to a device-relative block. It isn't entirely clear how the blocksize parameter is intended to be used.
static int haiga_bmap(const char* path, size_t blocksize, uint64_t* blockno)
{
    printf("BMAP FUNCTION \n");
    return 0;
}


// Set an extended attribute. See setxattr(2). This should be implemented only if HAVE_SETXATTR is true.
static int haiga_setxattr(const char* path, const char* name, const char* value, size_t size, int flags, uint32_t position)
{
    printf("SETXATTR FUNCTION \n");
    
    return 0;
}

// Read an extended attribute. See getxattr(2). This should be implemented only if HAVE_SETXATTR is true.
static int haiga_getxattr(const char* path, const char* name, char* value, size_t size, uint32_t position)
{
    printf("GETXATTR FUNCTION \n");
    return 0;
}
// List the names of all extended attributes. See listxattr(2). This should be implemented only if HAVE_SETXATTR is true.
static int haiga_listxattr(const char* path, char* list, size_t size)
{
    printf("LISTXTTR FUNCTION \n");
    return 0;
}
//       Support the ioctl(2) system call. As such, almost everything is up to the filesystem. On a 64-bit machine, FUSE_IOCTL_COMPAT will be set for 32-bit ioctls. The size and direction of data is determined by _IOC_*() decoding of cmd. For _IOC_NONE, data will be NULL; for _IOC_WRITE data is being written by the user; for _IOC_READ it is being read, and if both are set the data is bidirectional. In all non-NULL cases, the area is _IOC_SIZE(cmd) bytes in size.
static int haiga_ioctl(const char* path, int cmd, void* arg, struct fuse_file_info* fi, unsigned int flags, void* data)
{
    printf("IOCTL FUNCTION \n");
    return 0;
}

static int haiga_create(const char *path, mode_t mod, struct fuse_file_info *fi)
{
    printf("CREATE FUNCTION \n");
    
    if ((fi->flags & 3) != O_RDONLY)
        return -EACCES;
    
    
    return 0;
}

// Poll for I/O readiness. If ph is non-NULL, when the filesystem is ready for I/O it should call fuse_notify_poll (possibly asynchronously) with the specified ph; this will clear all pending polls. The callee is responsible for destroying ph with fuse_pollhandle_destroy() when ph is no longer needed.
//static int haiga_poll(const char* path, struct fuse_file_info* fi, struct fuse_pollhandle* ph, unsigned* reventsp)
//{
//    return 0;
//}



#endif /* helpers_h */
