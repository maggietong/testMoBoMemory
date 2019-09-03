 /*! 
 *\file e820.h
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
 
#ifndef _e820_H_H
#define _e820_H_H
#include <linux/types.h>

int e820AndCmdlineInitialization(void);
__u64 ParseCmdLineParaMem(char *cmdline);
int e820handler(char *path, int i);
int e820StringToType(char *stringType);
int e820AddEntry(__u64 start, __u64 end, int type, int entry);
__u64 ParseCmdLineParaMem(char *cmdline);

#endif

