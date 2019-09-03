/*!
*\file absdiag_dev.c
* 
* Define functions for operating abs_diag proc file 
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
* Nov. 12, 2009    Change coding stytle from c++ to linux c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#include "jedi_comm.h"
#include "memory_err.h"
#include "absdiag_dev.h"

#define VERSION "0.1.1"

/*!
 * clear memory ecc number
 * \param 
 *
 * \return 0  -	 success
 * 	not zero - error code
 */
unsigned int clear_mem_ecc(absdiag_struc *pabs)
{
    FILE *fp;
    unsigned int ret = ERR_SUCCESS;
    
    pabs->mem_ecc_num = 0;

    fp = fopen("/proc/"ABS_DIAG_MECC_PROC_FILENAME, "w");
    if (NULL != fp) {
        if (1 != fputs("0", fp)){
            ret = ERR_WRITE_DD_FILENAME;
            sprintf(error_info, "Function %s, Line %d", __FUNCTION__, __LINE__);
            fclose(fp);
        }else
            fclose(fp);
    }else{
        ret = ERR_DD_OPEN;
        sprintf(error_info, "Function %s, Line %d", __FUNCTION__, __LINE__);
    }
    return ret;    
} 
/*!
 * verify abs_diag proc file
 * 
 * \param	input  None
 * \parma   output caching      - is or not support caching
 *                 mem_ecc_num  - the number of memory ECC
 *                 cash_ecc_num - the number of cash ECC 
 * \Return  0 -         success
 *          not zero -  error code
 */
unsigned int verify_absdiag_dev(absdiag_struc *pabs) 
{
    int i = 0;
    char buf[24];
    FILE *fp;
    char *s;
	unsigned int ret = ERR_SUCCESS;
 
    memset(buf, 0, sizeof(buf));

    fp = fopen("/proc/"ABS_DIAGDEV_PROC_FILENAME, "r");
	if (NULL != fp) {
		while (NULL != fgets(buf, sizeof buf, fp)) {
			i++;
			s = strrchr(buf, '\n');
			if (NULL != s) {
				*s = '\0';
				if (1 == i && 8 == s - buf) {
					/* ver code */
					if (!strcmp(ABS_DIAGDEV_VERSION, buf)){
						/* match */
						pabs->abs_diag_ver = ABS_DIAGDEV_VERSION;
					}
				} else if (2 == i) {
					/* cashing stat */
					pabs->cashing = 1 == (buf[0] - '0') ? TRUE : FALSE;
					break;
				}
			} else {
				ret = ERR_DD_FAIL;
                sprintf(error_info, "Function %s, Line %d", __FUNCTION__, __LINE__);
                break;
			}
        }
		fclose(fp);
	} else {	
	    ret = ERR_DD_OPEN;
        sprintf(error_info, "Function %s, Line %d", __FUNCTION__, __LINE__);
	}

	if (ret) {
		return ret;
	}

	i = 0;
	fp = fopen("/proc/"ABS_DIAG_MECC_PROC_FILENAME, "r");
	if (NULL != fp) {
		while (NULL != fgets(buf, sizeof buf, fp)) {
			i ++;
			s = strrchr(buf, '\n');
			if (NULL != s) {
				*s = '\0';
				if (1 == i) {
					/* stat */
				} else if (2 == i) {
					/* count */
					pabs->mem_ecc_num = strtoul(buf, NULL, 10);
					break;
				} 
			} else {
				ret = ERR_DD_FAIL;
                sprintf(error_info, "Function %s, Line %d", __FUNCTION__, __LINE__);
				break;
			}
		}
		fclose(fp);
	} else {	
        ret = ERR_DD_OPEN;
        sprintf(error_info, "Function %s, Line %d", __FUNCTION__, __LINE__);
	}

	if (ret) {
		return ret;
	}

	i = 0;
	fp = fopen("/proc/"ABS_DIAG_CECC_PROC_FILENAME, "r");
	if (NULL != fp) {
		while (NULL != fgets(buf, sizeof buf, fp)) {
			i ++;
			s = strrchr(buf, '\n');
			if (NULL != s) {
				*s = '\0';
				if (1 == i) {
					/* stat */
				} else if (2 == i) {
					/* count */
					pabs->cash_ecc_num = strtoul(buf, NULL, 10);
					break;
				} 
			} else {
				ret = ERR_DD_FAIL;
                sprintf(error_info,"Function %s, Line %d", __FUNCTION__, __LINE__);
				break;
			}
		}
		fclose(fp);
	} else {	
        ret = ERR_DD_OPEN;
        sprintf(error_info, "Function %s, Line %d", __FUNCTION__, __LINE__);
	}

	if (ret) {
		return ret;
	}
	fp = fopen("/proc/"ABS_DIAG_CACHETEST_PROC_FILENAME, "r");
	if (NULL != fp) {
		if (NULL != fgets(buf, sizeof buf, fp)) {
			s = strrchr(buf, '\n');
			if (NULL != s) {
				pabs->cash_test_stat = buf[0] - '0';
			} else {
				ret = ERR_DD_FAIL;
                sprintf(error_info, "Function %s, Line %d", __FUNCTION__, __LINE__);
			}
		}
		fclose(fp);
	} else {	
        ret = ERR_DD_OPEN;
        sprintf(error_info, "Function %s, Line %d", __FUNCTION__, __LINE__);
	}


	return ret;
}


