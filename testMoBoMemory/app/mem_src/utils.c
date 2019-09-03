/* 
 * utils.c
 *
 * Author: Xiang-Yu Wang <rain_wang@jabil.com>
 *
 * Copyright (c) 2008 Jabil Circuit.
 *
 * This source code and any compilation or derivative thereof is the sole
 * property of Jabil Circuit and is provided pursuant to a Software License
 * Agreement. This code is the proprietary information of Jabil Circuit and
 * is confidential in nature. Its use and dissemination by any party other
 * than Jabil Circuit is strictly limited by the confidential information
 * provisions of Software License Agreement referenced above.
 *
 * 2008-01-17 made initial version
 * 2008-11-27 modified to make it a common utilis codes by removing 'sdtset' 
 *            specific definitions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <strings.h>

#include "utils.h"


void tdebug(const char *fmt, ...)
{
#ifdef DEBUG
	va_list arg;

	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
#else
	return;
#endif
}

void tprintf(FILE *fp, const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	vfprintf(fp, fmt, arg);
	va_end(arg);
}

void tprintt(FILE *fp, const char *fmt, ...)
{
	va_list arg;
	time_t ts = time(NULL);
	struct tm *tm = localtime(&ts);

	/* the month ranges from 0 to 11 and year start from 0 */
	fprintf(fp, "%d/%d/%d-%d:%d:%d: ", 
			tm->tm_mon + 1, tm->tm_mday, tm->tm_year + 1900,
			tm->tm_hour, tm->tm_min, tm->tm_sec);
	va_start(arg, fmt);
	vfprintf(fp, fmt, arg);
	va_end(arg);
}

int CheckInstance(char *appName)
{ 
    char ps_cmd[50];
    FILE *fd;
    int bRun = 0;
    int num_instances = 0;

    strcpy(ps_cmd, "ps -e|grep ");
    strcat(ps_cmd, appName);
    strcat(ps_cmd, "|wc -l");
    if((fd = popen(ps_cmd, "r")) == NULL) {
        return -1;
    } else {
       /* while(fgets(str, 255, fd) != NULL) */ 
            fscanf(fd, "%d", &num_instances);
       if( num_instances > 1 ) {  
           bRun = 1;
           //printf("Jackie %d instances found\n", num_instances);
       }
    } 
    pclose(fd);
    return bRun;
}

int CheckAllDigits(char *str)
{
    int len, i;
    len = strlen(str);
    for(i=0; i<len; i++) {
        if( str[i] > '9' || str[i] < '0')
            return 0;
    }
    return 1;
} 
