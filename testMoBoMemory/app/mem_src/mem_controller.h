/*!
*\file mem_controller.h
* 
* Define memor controller sub-module data structures
*
*\author Maggie Tong
*
* Copyright(c) 2011 Jabil Circuit.
*
* This source code and any compilation or derivative thereof is the sole property of 
* Jabil Circuit and is provided pursuant to a Software License Agreement. This code 
* is the proprietary information of Jabil Circuit and is confidential in nature. 
* It's use and dissemination by any party other than Jabil Circuit is strictly 
* limited by the confidential information provisions of Software License Agreement 
* referenced above.
*
* Version history
* Nar. 4, 2011    Create this file
*/

#ifndef MEM_CONTROLLER_H
#define MEM_CONTROLLER_H

#define LOG_DEV_FILENAME    "/dev/mcelog"
#define MEM_DEV_FILENAME    "/dev/mem"


#define MCE_GET_RECORD_LEN  _IOR('M', 1, int)
#define MCE_GET_LOG_LEN     _IOR('M', 2, int)
#define MCE_GETCLEAR_FLAGS  _IOR('M', 3, int)


#endif /* MEM_CONTROLLER_H */
