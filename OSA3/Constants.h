//
//  Constants.h
//  OSA3
//
//  Created by Hassaan on 25/04/2017.
//  Copyright © 2017 HaigaTech. All rights reserved.
//

#ifndef Constants_h
#define Constants_h

const int BLOCK_SIZE        = 1024;
const int BLOCK_COUNT       = 20;

const char* LOG_FILE_PATH   = "/Users/Hassaan/Desktop/haiga_lfs.txt";


//static const char *haiga_str = "Hello World!\n";
//static const char *haiga_path = "/hello";
static char fileNamesArr[BLOCK_COUNT][BLOCK_SIZE];
//static char fileDataArr[BLOCK_COUNT][BLOCK_SIZE+1];
static FILE* filehd;



#endif /* Constants_h */
