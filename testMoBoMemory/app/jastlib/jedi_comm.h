/*!
*\file JEDI_common.h
* 
* Define jedilib common data type and common functions 
*
*\author Maggie Tong
*
* Copyright(c) 2009 Jabil Circuit.
*
* This source code and any compilation or derivative thereof is the sole property of 
* Jabil Circuit and is provided pursuant to a Software License Agreement. This code 
* is the proprietary information of Jabil Circuit and is confidential in nature. 
* It's use and dissemination by any party other than Jabil Circuit is strictly 
* limited by the confidential information provisions of Software License Agreement 
* referenced above.
*
*\par ChangeLog:
*
* Nov. 15, 2009    Create this file
*/

#ifndef JEDI_COMMON_H_
#define JEDI_COMMON_H_


#ifdef __DEBUG__
#define D_printf(fmt, arg...) printf(fmt, ##arg)
#else
#define D_printf(fmt, arg...)
#endif


#define CPU_CACHE           1
#define MEMORY_MODULE       2
#define CACHE_MEMORY        3


typedef unsigned long UL;
typedef unsigned long volatile ULV;


typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef int BOOL;


#define TRUE    1
#define FALSE   0

#define ERR_SUCC    0        
#define ERR_FAIL    1
#define ERR_SUCCESS ERR_SUCC

#define SUCCESS     0
#define FAIL        1

#define LOW_IO_PRIVILEGE    1
#define HIGH_IO_PRIVILEGE   2
#define IO_DISABLE          0
#define IO_ENABLE           1
#define HIGH_IO_ENABLE      3

#define BASE_PORT           0x3ff

#ifdef __IODELAY__
#define IO_DELAY(usec) usleep(usec)
#else
#define IO_DELAY(usec)
#endif

#define RW_DELAY        1

#define LEN_16      16
#define LEN_32      32
#define LEN_64      64
#define LEN_128     128
#define LEN_256     256
#define LEN_1024    1024

#define JEDI_PGSZ   4*1024

void jedi_create_log_file(int err_code, char *err_info);

#endif /* JEDI_COMMON_H_  */
