//
//  helpers.h
//  OSA3
//
//  Created by Hassaan on 25/04/2017.
//  Copyright © 2017 HaigaTech. All rights reserved.
//

#ifndef helpers_h
#define helpers_h

#include "Constants.h"

void initializeINodeZero();
void readAllFileNamesFromiNodeZero();
void readFileNamesFromBlock();
int canBlockSaveFileName(int dataBlockNumber, const char* filename);
int saveFileNameAndiNodeInBlock(int dataBlockNumber, const char* filename, int iNodeNumber);
int doesFileExistWithFileName(const char* filename);
int createNewFile(const char* filename);

/**
 * Return size in bytes, size of the file represented by the fileNumber
 */
int getSizeOfFile(int fileNumber) {
    fseek(filehd, fileNumber*INODE_SIZE, SEEK_SET); // Go to the inode number
    // First 4 bytes of this inode will represent size of the file
    
    int *fileSize = malloc(sizeof(int));
    fread(fileSize, sizeof(int), 1, filehd); // get total size of the file
    return *fileSize;
}


// MARK: - Initializing

/**
 * Read and initialize how many blocks were used previously to store data
 * of the files present in our filesystem.
 */
void initializeUsedBlockCount() {
    FILE* blockFile = fopen(BLOCK_COUNT_FILE_PATH, "r+");
    if (blockFile == NULL) {
        printf("\nBlock count file does not exist, creating new one");
        blockFile = fopen(BLOCK_COUNT_FILE_PATH, "w+");
        int blocks = 0;
        fwrite(&blocks, sizeof(int), 1, blockFile);
        blocksUsed = 0;
    }
    else {
        fread(&blocksUsed, sizeof(int), 1, blockFile);
    }
    fclose(blockFile);
}


/**
 * To store the number of blocks in which we have stored some data
 * of the files present in our filesystem.
 */
void saveUsedBlockCount() {
    FILE* blockFile = fopen(BLOCK_COUNT_FILE_PATH, "r+");
    if (blockFile != NULL) {
        fwrite(&blocksUsed, sizeof(int), 1, blockFile);
    }
    fclose(blockFile);
}

void initializeLogFile() {
    
    initializeINodeZero();
    
    for (int i=1 ; i<INODE_COUNT ; i++) {
        
        fseek(filehd, i*INODE_SIZE, SEEK_SET);
        int *fileSize = (int*) malloc(sizeof(int));
        *fileSize = 0 ;
        fwrite((void*)fileSize, sizeof(int), 1, filehd); // writing size of the file in the 1st 4 bytes of the inode
        
        int blockNumber = -1;
        for(int j=0 ; j<8 ; j++) {
            fwrite((void*)&blockNumber, sizeof(int), 1, filehd); // initializing next 32 bytes with eigh 4 byte block numbers
        }
        
        fseek(filehd, 4, SEEK_CUR); // Leaving the last 4 bytes of the inode empty
    }
    
}

/**
 * Setting iNode0 data block pointers to "-1"
 * Should be called only the very first time our filesystem comes into life,
 * not each time filesystem is mounted
 */
void initializeINodeZero() {

    fseek(filehd, 4, SEEK_SET); // Go to the 4th byte of the 0th iNode
    int blockNumber = -1;
    for(int j=0 ; j<8 ; j++) {
        fwrite((void*)&blockNumber, sizeof(int), 1, filehd); // initializing next 32 bytes with eigh 4 byte block numbers to be null
    }
    fseek(filehd, 4, SEEK_CUR); // Leaving the last 4 bytes of the first inode empty
}


// MARK: Reading Data of iNode0

/**
 * Read and store all the data of iNode0 in the global 
 * data structure representing iNodeNumber and filename
 */
void readAllFileNamesFromiNodeZero() {
    
    fseek(filehd, 0, SEEK_SET); // Go to the 0th inode
    // First 4 bytes of this inode will represent number of files present in the filesystem
    
    int *fileCount = malloc(sizeof(int));
    fread(fileCount, sizeof(int), 1, filehd); // get total number of files present in the filesystem
    if (*fileCount == 0) {
        return; // there are currently no files present in the filesystem.
    }
    
    for (int i=0 ; i<8; i++) {
        // Go to the next block number
        fseek(filehd, (i*INODE_SIZE)+4, SEEK_SET);
        
        // read data block number
        int blockNumber = -1;
        fread(&blockNumber, sizeof(int), 1, filehd);
        if (blockNumber < 0) {
            return; // This block and all the future blocks contain no data
        }
        
        // go to the data block
        // read, parse and save the filenames in the global iNodeZeroFilesNames array
        readFileNamesFromBlock(blockNumber);
    } // end for
}




/**
 * Read and store data of specific blockNumber of iNode0 in the global
 * data structure representing iNodeNumber and filename
 */
void readFileNamesFromBlock(int dataBlockNumber) {
    
    // go to the data block
    int dataBlockLocation = DATA_BLOCKS_BASE_ADDR + ((dataBlockNumber)*BLOCK_SIZE);
    fseek(filehd, dataBlockLocation, SEEK_SET); // Go to the ith data block of the 0th iNode
    
    // read, parse and save the filenames in the global iNodeZeroFilesNames array
    char data[BLOCK_SIZE];
    fread((void*)(data), BLOCK_SIZE, 1, filehd);
    
    char iNodeStr[10];
    char fileNameStr[MAX_FILENAME_LENGTH];
    
    
    // we will loop and read all iNode and FileName combinations and store in iNodeZeroFileNames array
#warning i am not sure if the following line of code would work as i think it would. Please verify this.
    sscanf (data, "%s %s\n%s", iNodeStr, fileNameStr, data);
    int iNodeNumber = atoi(iNodeStr);
    strcpy(iNodeZeroFileNames[0].fileName, fileNameStr);
    iNodeZeroFileNames[0].iNodeNumber = iNodeNumber;


}


// MARK: Creating new file

/**
 * Creates an entry in the iNode0 for the given filename
 * and assign an inode for this file
 */
int createNewFile(const char* filename) {
#warning Incompete method 
    
    // Find the first free inode and then link this file with this inode.
    return 0;
}

// MARK: Writing filenames to datablocks pointed by iNode0

int addFileNameforiNode(const char* fileName, int iNodeNumber) {
    
    fseek(filehd, 0, SEEK_SET); // Go to the 0th inode
    // First 4 bytes of this inode will represent number of files present in the filesystem
    
    int *fileCount = malloc(sizeof(int));
    fread(fileCount, sizeof(int), 1, filehd); // get total number of files present in the filesystem
    if (*fileCount == 0) {
        return 0; // there are currently no files present in the filesystem.
    }
    
    for (int i=0 ; i<8; i++) {
        // Go to the next block number
        fseek(filehd, (i*INODE_SIZE)+4, SEEK_SET);
        
        // read data block number
        int blockNumber = -1;
        fread(&blockNumber, sizeof(int), 1, filehd);
        if (blockNumber < 0) {
            blockNumber = blocksUsed; // in this block we will store new data
            blocksUsed++;
            fseek(filehd, -sizeof(int), SEEK_CUR); // moving back the file pointer so that we can assign this inode a new block to write its data to.
            fwrite((void*)&blockNumber, sizeof(int), 1, filehd); // pointing to the data block that will store "some" data of this file
        }
        if (canBlockSaveFileName(blockNumber, fileName) == 1) {
            saveFileNameAndiNodeInBlock(blockNumber, fileName, iNodeNumber);
            return 1;
        }
        
        
    } // end for
    return 0;
}


/**
 * Go to the block refferenced by dataBlockNumber
 * save iNodeNumber and filename at the end of this dataBlock
 */
int saveFileNameAndiNodeInBlock(int dataBlockNumber, const char* filename, int iNodeNumber) {
    
    if (canBlockSaveFileName(dataBlockNumber, filename) == 0) {
        return 0;
    }
    
    // go to the data block
    int dataBlockLocation = DATA_BLOCKS_BASE_ADDR + ((dataBlockNumber)*BLOCK_SIZE);
    fseek(filehd, dataBlockLocation, SEEK_SET);
    
    char data[BLOCK_SIZE];
    fread((void*)(data), BLOCK_SIZE, 1, filehd);
    int dataLength = (int)strlen(data);
    
    // Get to the place where we will write new data
    fseek(filehd, dataBlockLocation+dataLength, SEEK_SET);
    
    // Create buffer that would be written in the file
    int bytesForBlockNumber = 0;
    if (iNodeNumber < 10)
        bytesForBlockNumber = 1; // single digit
    else if (iNodeNumber < 100)
        bytesForBlockNumber = 2; // 2 digit number
    else if(iNodeNumber < 100)
        bytesForBlockNumber = 3; // 3 digit number
    else
        bytesForBlockNumber = 4; // 4 digit number
    
    int bufferLength = bytesForBlockNumber + (int)strlen(filename) + 1; // iNodeNumber FileName NewLine
    char buffer[bufferLength+1];
    snprintf(buffer, bufferLength, "%d %s\n", iNodeNumber, filename);
    
    // write iNodeNumber and filename to the logFile
    fwrite(buffer, bufferLength, 1, filehd);
    return 1;
}


/**
 * Verify if this dataBlock has enough free space to save
 * information of filename and iNode
 */
int canBlockSaveFileName(int dataBlockNumber, const char* filename) {
    
    if (dataBlockNumber < 1) {
        return 0; // return false
    }
    
    // go to the data block
    int dataBlockLocation = DATA_BLOCKS_BASE_ADDR + ((dataBlockNumber)*BLOCK_SIZE);
    fseek(filehd, dataBlockLocation, SEEK_SET);
    
    char data[BLOCK_SIZE];
    fread((void*)(data), BLOCK_SIZE, 1, filehd);
    
    int dataLength = (int)strlen(data);
    int fileInfoLength = 6 + (int)strlen(filename); // Total bytes required to store iNode number and filename
    if (dataLength < (BLOCK_SIZE-fileInfoLength)) {
        return 1; // return true
    }
    else {
        return 0; // return false
    }
    
}



/**
 * Check if our filesystem already has this file.
 * Returns the iNode number that knows where the contents of this file are present
 * or returns -1 if the file with "filename" is not found.
 */
int getiNodeNumberForFile(const char* filename) {
    
    int inodeNumber = doesFileExistWithFileName(filename);
    if (inodeNumber == 0) {
        return -1;
    }
    return inodeNumber;
}

/**
 * Check if our filesystem already has this file.
 * Returns > 0 on success.
 * return 0 on failure.
 */
int doesFileExistWithFileName(const char* filename) {
    
    for (int i=0 ; i<currentFileNameCount; i++) {
        if (strcmp(filename, iNodeZeroFileNames[i].fileName) == 0) {
            return iNodeZeroFileNames[i].iNodeNumber; // return iNode 0 of the file
        }
    }
    
    return 0; // return false
}


#endif /* helpers_h */
