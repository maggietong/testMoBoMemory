/*! 
 *\file fileops.c
 *
 * Some user mode file functions.
 *
 * Author: Jackie Liu
 *
 * Copyright (c) 2012 Jabil Circuit.
 *
 * This source code and any compilation or derivative thereof is the sole
 * property of Jabil Circuit and is provided pursuant to a Software License
 * Agreement. This code is the proprietary information of Jabil Circuit and
 * is confidential in nature. Its use and dissemination by any party other
 * than Jabil Circuit is strictly limited by the confidential information
 * provisions of Software License Agreement referenced above.
 
 *\par ChangeLog:
 * July 25, 2012      The initial version
 */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <asm-generic/errno-base.h>
#include <unistd.h>
#include "fileops.h"

void List(const char *path, int level); 
/*int main(void)
{
    List("/sys/firmware/memmap", 0);
}*/
//Specially for e820 entries under /sys/firmware/memmap
int EnumerateDirByPath(const char *path, int (*handler)(char *, int entryNum)) 
{
    struct dirent* ent = NULL;
    DIR *pDir;
	int i;
	char fullDirPath[256];
	
	pDir = opendir(path);
    if( pDir == NULL ) {
      return -EINVAL;
    }
    while(NULL != (ent = readdir(pDir))) {
        if(ent->d_type == 8) {
        //file
        } else {
            //directory
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }
			//printf("%s\n", ent->d_name);
            i = atoi(ent->d_name);
			sprintf(fullDirPath,"%s/%s",path, ent->d_name);
			if(handler != NULL)
			    handler(fullDirPath, i);
        }
    }
	return 0;
}

int getFileString(char *fullpath, char *buff, int size)
{
   FILE *file;

   //stat function does not work here
   file = fopen(fullpath, "r");
   if( file == NULL) {
      return -1;
   }
   
   if( fgets(buff, size, file) == NULL )
      return -1;
   fclose(file);
   return 0;
}
