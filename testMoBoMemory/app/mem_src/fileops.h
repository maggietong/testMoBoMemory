 /*! 
 *\file fileops.h
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
 
#ifndef _FILEOPS_H
#define _FILEOPS_H

int EnumerateDirByPath(const char *path, int (*handler)(char *, int entryNum));
int getFileString(char *fullpath, char *buff, int size);

#endif

