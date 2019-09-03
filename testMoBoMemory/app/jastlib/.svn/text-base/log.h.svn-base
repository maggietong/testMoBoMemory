/*!
*\file memlib_log.h
* 
* Define create log information header file
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
* June. 09, 2010    Create this file
*/

#ifndef MEMLIB_LOG_H_
#define MEMLIB_LOG_H_

#include <syslog.h> 

/* sys/syslog.h:
 * LOG_EMERG    0   system is unusable
 * LOG_ALERT    1   action must be taken immediately
 * LOG_CRIT     2   critical conditions
 *
 * LOG_ERR      3   error conditions 
 * LOG_WARNING  4   warning conditions
 * LOG_NOTICE   5   normal but significant condition
 * LOG_INFO     6   informational
 *
 * LOG_DEBUG    7   debug-level messages
 */


#define LOG_ERROR   LOG_ERR
#define LOG_WARN    LOG_WARNING

#define LOG_NAME_DEFAULT    "memlib"
#define LOG_MSG_LENGTH      1024

void log_init(const char *name, int isdaemon, int verbose);
void log_halt(void);
void log_level_set(int level);
int log_level_get(void);
void lprintf(int level, const char *format, ...);
void lperror(int level, const char *format, ...);
void lprintf_n(int level, const char *format, ...);

#endif /* MEMLIB_LOG_H_  */
