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

int getSizeOfFile(int fileNumber) {
    fseek(filehd, fileNumber*INODE_SIZE, SEEK_SET); // Go to the inode number
    // First 4 bytes of this inode will represent size of the file
    
    int *fileSize = malloc(sizeof(int));
    fread(fileSize, sizeof(int), 1, filehd); // get total size of the file
    return *fileSize;
}

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
void saveUsedBlockCount() {
    FILE* blockFile = fopen(BLOCK_COUNT_FILE_PATH, "r+");
    if (blockFile != NULL) {
        fwrite(&blocksUsed, sizeof(int), 1, blockFile);
    }
    fclose(blockFile);
}

void initializeLogFile() {
    
    for (int i=0 ; i<INODE_COUNT ; i++) {
        
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


#endif /* helpers_h */
