/*!
*\file memory_err.h
* 
* Define memory module error code
*
*\author Maggie Tonpl
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
* Version history
* Nov. 17, 2009    Create this file
*/

#ifndef MEMORY_ERR_H
#define MEMORY_ERR_H


#define ERR_MODULE_ID           0x10200001
#define ERR_ITEM_ID             0x10200002
#define ERR_GET_MEMORY_INFO     0x10200003
#define ERR_OPEN_TEMP_FILE      0x10200004
#define ERR_READ_TEMP_FILE      0x10200005
#define ERR_CORE_NUM            0x10200006
#define ERR_MALLOC_SPACE        0x10200007
#define ERR_TEST_ITEM_NUM       0x10200008
#define ERR_MLOCK_SPACE         0x10200009
#define ERR_CREATE_THREAD       0x1020000A
#define ERR_MUNLOCK_SPACE       0x1020000B
#define ERR_BIND_CPU            0x1020000C
#define ERR_NO_ENOUGH_MEM_SPACE 0x1020000D

#define ERR_ADDR_WALK_ONES      0x10200010
#define ERR_ADDR_WALK_OWN       0x10200011
#define ERR_MOVINV_10           0x10200012
#define ERR_MOVINV_8BIT         0x10200013
#define ERR_MOVINV_RANDOM       0x10200014
#define ERR_BLOCK_MOVE          0x10200015
#define ERR_MOVINV_32BIT        0x10200016
#define ERR_RANDOM_NUM_SEQ      0x10200017
#define ERR_MODULO              0x10200018
#define ERR_BIT_FADE            0x10200019
#define ERR_MARCH_C             0x1020001A

#define ERR_CACHE_WALKING       0x10101020
#define ERR_CACHE_WALK_1        0x10101021
#define ERR_CACHE_WALK_0        0x10101023
#define ERR_CACHE_STUCK         0x10100024
#define ERR_CACHE_RDWR          0x10100025
#define ERR_CACHE_RAND_DATA     0x10100026
#define ERR_TEXT_WRITE          0x10100027
#define ERR_TIMEOUT_CACHE_TEST  0x10100008
#define ERR_CACHE_SWITCH        0x10100009
#define ERR_TURNOFF_CACHE       0x1010000a

#define ERR_DD_FAIL                     0x10301001
#define ERR_DD_OPEN                     0x10301002
#define ERR_NO_ABSDIAG_DEV_PROC_FILE    0x10301003
#define ERR_SET_CPU_AFFINITY            0x10301004
#define ERR_CPU_NOT_SUPPORT_CPUID       0x10301005
#define ERR_TEST_UNIT_NUM               0x10301006
#define ERR_WRITE_DD_FILENAME           0X10301007

extern char error_info[LEN_1024];
extern void memory_error_msg(unsigned int err_no);


#endif  /* MEMORY_ERR_H  */
