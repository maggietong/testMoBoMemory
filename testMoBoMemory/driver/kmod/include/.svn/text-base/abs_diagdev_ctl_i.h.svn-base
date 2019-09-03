/*!
 * \file abs_diagdev_ctl_i.h
 *
 * \brief Device ctrl file.
 *
 * \note
 * Naming *_i.h stands for it's an interface header.
 *
 * \author Pat Huang
 *
 * \version r0.1b01
 * \par ChangeLog:
 * \verbatim
 *  rev 0.1-
 * \endverbatim
 */

#ifndef ABS_DIAGDEV_CTL_I_H__
#define ABS_DIAGDEV_CTL_I_H__

#define ABS_DIAGDEV			"abs_diag"
#define ABS_DIAGDEV_PROC_FILENAME	ABS_DIAGDEV
#define ABS_DIAG_MECC_PROC_FILENAME	ABS_DIAGDEV"_mecc"
#define ABS_DIAG_CECC_PROC_FILENAME	ABS_DIAGDEV"_cecc"
#define ABS_DIAG_CACHETEST_PROC_FILENAME	ABS_DIAGDEV"_cachetest"

enum abs_diag_cachetest_stats {
	ABS_DIAG_CACHETEST_READY = 0,
	ABS_DIAG_CACHETEST_INPROCESS,
	ABS_DIAG_CACHETEST_STOP_SUCC, 
	ABS_DIAG_CACHETEST_STOP_FAIL,
	ABS_DIAG_CACHETEST_STOP_NOMEM
};

/* #define ABS_DIAGDEV_MAGIC	108*/
/* Version 1.0.8-24, 0x01,0x00,0x08,0x18 */
#define ABS_DIAGDEV_VERSION	"01000818"

/*enum dd_cmds
{
	DDCMD_NONE,
	DDCMD_VERSION,
	DDCMD_CACHE_IS_ON,
	DDCMD_CACHE_SWITCH,
	DDCMD_UNKNOWN
};
*/

/* _IOR: ioctl command number from user process 
 * to the kernel module. 
 * The second argument: the command 
 */
/*#define ABS_DIAGDEV_IOT_VERSION 	_IO(ABS_DIAGDEV_MAGIC, DDCMD_VERSION)
#define ABS_DIAGDEV_IOT_CACHE_IS_ON	_IO(ABS_DIAGDEV_MAGIC, DDCMD_CACHE_IS_ON)
#define ABS_DIAGDEV_IOS_CACHE_SWITCH	_IOW(ABS_DIAGDEV_MAGIC, DDCMD_CACHE_SWITCH, int)
 */

/* The name of the device file */
/*#define ABS_DIAGDEV_FILE 	"/dev/"ABS_DIAGDEV */

#endif


