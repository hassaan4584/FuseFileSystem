//
//  fuse_operations.h
//  OSA3
//
//  Created by Hassaan on 25/04/2017.
//  Copyright © 2017 HaigaTech. All rights reserved.
//

#ifndef fuse_operations_h
#define fuse_operations_h
#include "Constants.h"
#include "helpers.h"

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
//static int haiga_fgetattr(const char* path, struct stat* stbuf, struct fuse_file_info * fileInfo)
//{
//    printf("F GETARR FUNCTION \n");
//    
//    return 0;
//}

//This is the same as the access(2) system call. It returns -ENOENT if the path doesn't exist, -EACCESS if the requested permission isn't available, or 0 for success. Note that it can be called on files, directories, or any other object that appears in the filesystem. This call is not required but is highly recommended.
/**
 * Check file access permissions
 *
 * This will be called for the access() system call.  If the
 * 'default_permissions' mount option is given, this method is not
 * called.
 *
 * This method is not called under Linux kernel versions 2.4.x
 *
 * Introduced in version 2.5
 */

//static int haiga_access(const char* path, int mask)
//{
//    printf("ACCESS FUNCTION with path : %s\n", path);
//    
//    return 0;
//}

//Make a special (device) file, FIFO, or socket. See mknod(2) for details. This function is rarely needed, since it's uncommon to make these objects inside special-purpose filesystems.
/** Create a file node
 *
 * This is called for creation of all non-directory, non-symlink
 * nodes.  If the filesystem defines a create() method, then for
 * regular files that will be called instead.
 */
static int haiga_mknod(const char* path, mode_t mode, dev_t rdev)
{
    printf("mknod FUNCTION with path : %s \n", path);
    mode = S_IFREG | 0666;
    return 0;
}




// readlink(const char* path, char* buf, size_t size)
// If path is a symbolic link, fill buf with its target, up to size. See readlink(2) for how to handle a too-small buffer and for error codes. Not required if you don't support symbolic links. NOTE: Symbolic-link support requires only readlink and symlink. FUSE itself will take care of tracking symbolic links in paths, so your path-evaluation code doesn't need to worry about it.



// Read size bytes from the given file into the buffer buf, beginning offset bytes into the file. See read(2) for full details. Returns the number of bytes transferred, or 0 if offset was at or beyond the end of the file. Required for any sensible filesystem.
// As for read above, except that it can't return 0.
static int haiga_write(const char* path, const char *buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
    printf("WRITE FUNCTION \n");
    
    if ((int)size > (8*1024)) {
        return -EFBIG;
    }
    int iNodeNumber = getiNodeNumberForFile(path);
    if (iNodeNumber < 0 ) {
        return 0;
    }
    int iNodeLocation = getiNodeLocation(iNodeNumber);
    fseek(filehd, iNodeLocation, SEEK_SET); // Go to the inode number
    // First 4 bytes of this inode will represent size of the file
    
    int *fileSize = malloc(sizeof(int));
    *fileSize = (int)size;
    fread(fileSize, sizeof(int), 1, filehd); // get total size of the file
    *fileSize = ((*fileSize) < (int)strlen(buf)) ? *fileSize : (int)strlen(buf);
    if (offset > *fileSize) {
        return 0;
    }
    if (*fileSize == 0) {
        // if this is a new file, previously its size was 0
        *fileSize = (int) strlen(buf);
    }
    
    // e.g if the original file size is 2024B then 1024B will be in the first block and remaining 1000B will be in the 2nd/last block
    int numberOfBlocks = (*fileSize)/BLOCK_SIZE; // 2024/1024 = 1
    int blockNumber = -1;
    for (int i=0 ; i<numberOfBlocks; i++) {
        int iNodeLocation = getiNodeLocation(iNodeNumber);
        fseek(filehd, ((iNodeLocation)+4+(4*i)), SEEK_SET); // Read block number from our iNode
        // Forcefully assiging the next free block to store file data
        int nextFreeBlockNumber = blocksUsed;
        blockNumber = blocksUsed; // in this block we will store new data
        blocksUsed++;
        fwrite((void*)&nextFreeBlockNumber, sizeof(int), 1, filehd); // pointing to the data block that will store "some" data of this file

        // Testing 2.1
        size_t dataBlockLocation = DATA_BLOCKS_BASE_ADDR + ((blockNumber)*BLOCK_SIZE);
        fseek(filehd, dataBlockLocation, SEEK_SET); // Go to the ith data block of the iNode
        // we will read complete BLOCK_SIZE bytes from this block because it is not the last data block of this file and hence it is completely filled
        fwrite((void*)(buf+(i*BLOCK_SIZE)), BLOCK_SIZE, 1, filehd);
    }
    
//    fseek(filehd, ((iNodeNumber*INODE_SIZE)+4+(4*numberOfBlocks)), SEEK_SET); // Read last block number from our iNode
    
    fseek(filehd, (getiNodeLocation(iNodeNumber)+4+(numberOfBlocks*4)), SEEK_SET); // Read last block number from our iNode
    int nextFreeBlockNumber = blocksUsed;
    blockNumber = blocksUsed; // in this block we will store new data
    blocksUsed++;
    fwrite((void*)&nextFreeBlockNumber, sizeof(int), 1, filehd); // pointing to the data block that will store last
    
    int lastBlockDataSize = (*fileSize) % BLOCK_SIZE; // now we will read the remaining 1000 bytes
    size_t lastBlockStartLocation = DATA_BLOCKS_BASE_ADDR + ((blockNumber)*BLOCK_SIZE); // Read the last block number from the eight-4 bytes block numbers
    fseek(filehd, lastBlockStartLocation, SEEK_SET);
    size_t wrtieOffset = (numberOfBlocks*BLOCK_SIZE);
    fwrite((void*)(buf+wrtieOffset), lastBlockDataSize, 1, filehd);
    
    int bytesWritten = (int)strlen(buf);
    fseek(filehd, getiNodeLocation(iNodeNumber), SEEK_SET); // Again Go to the inode number
    fwrite((void*)&bytesWritten, sizeof(int), 1, filehd); // and update the total size of the file. Currently it updates for 1 block only.

    
    //clean up the previous extra data
    for (int i=numberOfBlocks+1 ; i<8; i++) {
        int iNodeLocation = getiNodeLocation(iNodeNumber);
        fseek(filehd, ((iNodeLocation)+4+(4*i)), SEEK_SET); // Read block number from our iNode
        int minus1 = -1;
        fwrite(&minus1, sizeof(int), 1, filehd);
    }
    return bytesWritten;
    
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
//static int haiga_flush(const char* path, struct fuse_file_info* fi)
//{
//    printf("FLUSH FUNCTION \n");
//    return 0;
//}

// Perform a POSIX file-locking operation. See details below.
//static int haiga_lock(const char* path, struct fuse_file_info* fi, int cmd, struct flock* locks)
//{
//    printf("LOCK FUNCTION \n");
//    
//    return 0;
//}


// This function is similar to bmap(9). If the filesystem is backed by a block device, it converts blockno from a file-relative block number to a device-relative block. It isn't entirely clear how the blocksize parameter is intended to be used.
//static int haiga_bmap(const char* path, size_t blocksize, uint64_t* blockno)
//{
//    printf("BMAP FUNCTION \n");
//    return 0;
//}


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
//static int haiga_listxattr(const char* path, char* list, size_t size)
//{
//    printf("LISTXTTR FUNCTION \n");
//    return 0;
//}
//       Support the ioctl(2) system call. As such, almost everything is up to the filesystem. On a 64-bit machine, FUSE_IOCTL_COMPAT will be set for 32-bit ioctls. The size and direction of data is determined by _IOC_*() decoding of cmd. For _IOC_NONE, data will be NULL; for _IOC_WRITE data is being written by the user; for _IOC_READ it is being read, and if both are set the data is bidirectional. In all non-NULL cases, the area is _IOC_SIZE(cmd) bytes in size.
//static int haiga_ioctl(const char* path, int cmd, void* arg, struct fuse_file_info* fi, unsigned int flags, void* data)
//{
//    printf("IOCTL FUNCTION \n");
//    return 0;
//}

/**
 * Create and open a file
 *
 * If the file does not exist, first create it with the specified
 * mode, and then open it.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the mknod() and open() methods
 * will be called instead.
 *
 * Introduced in version 2.5
 */

static int haiga_create(const char *path, mode_t mod, struct fuse_file_info *fi)
{
    printf("CREATE FUNCTION with path : %s \n", path);
    
    createNewFile(path);
//    fi->flags = O_RDWR;
//    mod = S_IFREG | 0666;

    return 0;
}

// Poll for I/O readiness. If ph is non-NULL, when the filesystem is ready for I/O it should call fuse_notify_poll (possibly asynchronously) with the specified ph; this will clear all pending polls. The callee is responsible for destroying ph with fuse_pollhandle_destroy() when ph is no longer needed.
//static int haiga_poll(const char* path, struct fuse_file_info* fi, struct fuse_pollhandle* ph, unsigned* reventsp)
//{
//    return 0;
//}

/** Read the target of a symbolic link
 *
 * The buffer should be filled with a null terminated string.  The
 * buffer size argument includes the space for the terminating
 * null character.	If the linkname is too long to fit in the
 * buffer, it should be truncated.	The return value should be 0
 * for success.
 */

static int haiga_readlink(const char *path, char *buf, size_t size)
{
    printf("READLINK FUNCTION \n");

    
    return 0;
}

/** Create a directory */
 int haiga_mkdir(const char * path, mode_t mode)
{
    printf("MKDIR FUNCTION \n");
    mode = S_IFDIR | 0755;
    return 0;
}

/** Remove a file */
static int haiga_unlink(const char *path)
{
    printf("UNLINK FUNCTION \n");
    
    return 0;
}

/** Remove a directory */
static int haiga_rmdir(const char *path)
{
    printf("RMDIR FUNCTION \n");
    
    return 0;
}

/** Create a hard link to a file */
static int haiga_link(const char *unknown1, const char *unknown2)
{
    printf("LINK FUNCTION \n");

    return 0;
}

/** Change the permission bits of a file */
static int haiga_chmod(const char *path, mode_t mode)
{
    printf("CHMOD FUNCTION \n");

    return 0;
}

/** Change the owner and group of a file */
static int haiga_chown(const char *path, uid_t uid, gid_t gid)
{
    printf("CHOWN FUNCTION \n");

    return 0;
}

/** Change the size of a file */
static int haiga_truncate(const char *path, off_t offset)
{
    printf("TRUNCATE FUNCTION with path : %s and offset : %ld\n", path, (long)offset);
    
    return 0;
}

/**
 * Change the access and modification times of a file with
 * nanosecond resolution
 *
 * Introduced in version 2.6
 */
//static int haiga_utimens(const char *path, const struct timespec tv[2])
//{
//    printf("UTIMENS FUNCTION \n");
//   
//    
//    return 0;
//}




/** Create a symbolic link */
static int haiga_symlink(const char *x, const char *y)
{
    printf("SYMLINK FUNCTION \n");
    
    return 0;
}

/** Rename a file */
int haiga_rename(const char *x, const char *y)
{
    printf("RENAME FUNCTION \n");

    return 0;
}





#endif /* fuse_helpers_h */
