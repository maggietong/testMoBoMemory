 /*! 
 *\file e820.c
 *
 * e820 related operations to retrieve e820 map.
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

#include <asm/types.h>
#include <asm/e820.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "mmtest_intf.h"
#include "log.h"
#include "e820.h"
#include "fileops.h"

struct mye820map mye820map;

int e820AndCmdlineInitialization(void)
{
   int i;
   char *path="/sys/firmware/memmap";
   char cmdline[512];
   int status;
   __u64 size;
   int fd;
   #ifdef DEBUG
      printf("Enter e820 Initialization\n");
   #endif
   EnumerateDirByPath(path, e820handler);
   for(i = 0; i < mye820map.nr_map; i ++)
      lprintf(LOG_DEBUG,"entry=%d, start=0x%lx, end=0x%lx\n", i, mye820map.map[i].addr, mye820map.map[i].addr + mye820map.map[i].size );
   if( mye820map.nr_map == 0) {
      lperror(LOG_ERR, "Cannot get e820 map from /sys/firmware/memmap, test cannot continue\n");
	  return -1;
   }
   
   status = getFileString("/proc/cmdline", cmdline, sizeof(cmdline));
   if( status != 0 ) {
      lperror(LOG_ERR, "Failed to open /proc/cmdline to get cmdline parameter, cannot continue test\n");
      return -1;
   }
   lprintf(LOG_DEBUG, "cmdline is: %s", cmdline);	
   size = ParseCmdLineParaMem( cmdline );

   if ( size == 0 ) {
      lperror(LOG_ERR, "No mem=xxx option in commandline or the format is incorrect, cannot perform test\n");
	  return -1;
   }

   lprintf(LOG_DEBUG, "mem=%ld in command line\n", size);
   
   fd = open("/dev/memtest", O_RDWR);
   if( -1 != fd) {
      status = ioctl(fd, MEMORY_SET_E820_INFO, &mye820map);
      #ifdef DEBUG
         printf("ioctl is %lx\n", MEMORY_SET_E820_INFO);
      #endif
      if( !status ) {
         lprintf(LOG_DEBUG, "Successful ioctrl to set E820 information\n");
         status = ioctl(fd, MEMORY_SET_MEM_INFO, &size);
         if ( !status)
	         lprintf(LOG_DEBUG, "Set mem limit succeeds\n");
		 else
		     lperror(LOG_ERR, "Set mem limit fails, test cannot continue\n");
      } else {
	         lperror(LOG_ERR, "ioctrl failed to set E820 information\n");
	  }
      close(fd);
  } else {
    printf("Cannot open the device\n");
  }
  #ifdef DEBUG
     printf("Exit e820 initialization\n");
  #endif
  return status;
}

int e820AddEntry(__u64 start, __u64 end, int type, int entry)
{
   if(entry >= E820_X_MAX)
      return -1;
   if (end <= start)
      return -1;
   if( mye820map.map[entry].size == 0 )
      mye820map.nr_map ++;
   mye820map.map[entry].addr = start;
   mye820map.map[entry].size = end - start + 1;
   mye820map.map[entry].type = type;

   return 0;
}

int e820handler(char *path, int i)
{
   //printf("\n%s, entry=%d\n", path, i);
   FILE *file;
   char fileStr[256];
   char typeStr[30];
   __u64 startAdd, endAdd;
   char start[30], end[30];
   int type;
   sprintf(fileStr, "%s/start", path);
   file = fopen(fileStr, "r");
   if( file == NULL) {
      return -1;
   }
   //fscanf does not work well
   fgets(start, sizeof(start), file);
   fclose(file);


   sprintf(fileStr, "%s/end", path);
   file = fopen(fileStr, "r");
   if( file == NULL) {
      return -1;
   }
   fgets(end, sizeof(end), file);
   fclose(file);


   sprintf(fileStr, "%s/type", path);
   file = fopen(fileStr, "r");
   if( file == NULL) {
      return -1;
   }
   fgets(typeStr, sizeof(typeStr), file);
   fclose(file);
  
   sscanf(start, "0x%lx", &startAdd);
   sscanf(end, "0x%lx", &endAdd);
   type=e820StringToType(typeStr);
  // scanf(
    
   e820AddEntry(startAdd, endAdd, type, i);
   //printf("entry=%d, start=%lx, end=%lx, %s=%d\n", i, startAdd, endAdd, typeStr, type);
   return 0;
}

int e820StringToType(char *stringType)
{  
   if(strncasecmp("System RAM", stringType, strlen("System RAM")) == 0)
      return E820_RAM;
   else if(strncasecmp("ACPI Tables", stringType, strlen("ACPI Tables")) == 0)
      return E820_ACPI;
   else if(strncasecmp("ACPI Non-volatile Storage", stringType, strlen("ACPI Non-volatile Storage")) == 0)
      return E820_NVS;
   else
      return E820_RESERVED;
}

__u64 ParseCmdLineParaMem(char *cmdline)
{
   char cmdlineTemp[512];
   char *pMemPara, *pTemp;
   __u64 size;
   if( cmdline == NULL )
      return 0;
   
   strcpy(cmdlineTemp, cmdline);
   pMemPara = strstr(cmdlineTemp, "mem="); 
   if ( pMemPara == NULL )
      return 0;
   
   pMemPara += 4;
   pTemp = strstr(pMemPara, " ");
   if ( pTemp != 0 )
      *pTemp = '\0';
   size = strtol(pMemPara, &pTemp, 0);
   if( *pTemp == 'G' || *pTemp == 'g' )
      size <<= 30;
   else if( *pTemp == 'M' || *pTemp == 'm' )
      size <<= 20;
   else if( * pTemp == 'K' || *pTemp == 'k' )
      size <<= 10;
   return size;
}
