/*!
*\file absdiag_dev.h
* 
* Anounce functions and data type 
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
* Version history
* Nov. 16, 2009    Create this file
*/

#ifndef ABSDIAG_DEV_H
#define ABSDIAG_DEV_H


#define ABS_DIAGDEV			    	"abs_diag"
#define ABS_DIAGDEV_VERSION			"01000818"
#define ABS_DIAGDEV_PROC_FILENAME		    ABS_DIAGDEV
#define ABS_DIAG_MECC_PROC_FILENAME		    ABS_DIAGDEV"_mecc"
#define ABS_DIAG_CECC_PROC_FILENAME		    ABS_DIAGDEV"_cecc"
#define ABS_DIAG_CACHETEST_PROC_FILENAME	ABS_DIAGDEV"_cachetest"

/*!
 *  Define some information of abs_diag_struc
 */ 
typedef struct _absdiag_struc_ {
        const char *abs_diag_ver;
        BOOL cashing;
        int mem_ecc_num;
        int cash_ecc_num;
        char cash_test_stat;
}absdiag_struc;

unsigned int verify_absdiag_dev(absdiag_struc *pabs);

unsigned int clear_mem_ecc(absdiag_struc *pabs);

#endif  /* ABSDIAG_DEV_H  */
