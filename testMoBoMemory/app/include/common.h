/*!
*\file common.h
* 
* Define common data type and common functions 
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

#ifndef _COMMON_H_
#define _COMMON_H_


#ifdef __DEBUG__
#define D_printf(fmt, arg...) printf(fmt, ##arg)
#else
#define D_printf(fmt, arg...)
#endif

typedef int BOOL;

/*!
 *\def TRUE
 * define TRUE 0x0
 */
#define TRUE    1

/*!
 *\def FALSE
 * define FALSE 0x0
 */
#define FALSE   0

/*!
 *\def CONST
 * define ERR_SUCCESS 0x0
 */
#define ERR_SUCC            0x0
#define ERR_FAIL            0X1


/*!
 * \def
 * define as
 */
#define LEN_16     16
#define LEN_32     32 
#define LEN_256    256


#endif /* _COMMON_H_  */
