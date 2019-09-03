/*!
*\file cache_spill.c
* 
* Define testing functions
*
*\author Maggie Tong & Pat Huang
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
* Feb. 2, 2010    Change coding stytle from c++ to linux c
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h> /* uintptr_t */
#include <time.h>

#include "jedi_comm.h"
#include "memory.h"
#include "memory_err.h"
#include "absdiag_dev.h"

#define VERSION "0.1.1"

#define ERR_FAIL    1

/* diagdev_states */
#define    DD_SUCC    0
#define    DD_EFAIL   1
#define    DD_EOPEN   2
#define    DD_ENOMEM  3
#define    DD_INPROC  4

/* abs_diag_cachetest_stats */
#define    ABS_DIAG_CACHETEST_READY       0
#define    ABS_DIAG_CACHETEST_INPROCESS   1
#define    ABS_DIAG_CACHETEST_STOP_SUCC   2
#define    ABS_DIAG_CACHETEST_STOP_FAIL   3
#define    ABS_DIAG_CACHETEST_STOP_NOMEM  4


void nsleep(const UL secs, const UL nsecs, const char intr)
{
    struct timespec req;
    struct timespec rem;

    req.tv_sec = secs;
    req.tv_nsec = nsecs;

    if (intr) {
        nanosleep(&req, &rem);
    } else {
        while (nanosleep(&req, &rem) < 0) {
            if ( EINTR == errno && (rem.tv_nsec > 0 || rem.tv_sec > 0)) {
                req = rem;
            } else {
                break;
            }
        }
    }
}


unsigned int text_write(const char *p, const char *p_mode, const char *p_cont)
{
    FILE *fp;
    fp = fopen(p, p_mode);
    if (NULL != fp) {
        fputs(p_cont, fp);
        fclose(fp);
        return ERR_SUCCESS;
    }
    return ERR_TEXT_WRITE;
}

unsigned int cache_switch(absdiag_struc *p_abs)
{
    char p_cont[4] = "0\n";
    unsigned int ret = ERR_SUCCESS;

    /* Change from caching to not caching, or vice versa.  */
    p_cont[0] = p_abs->cashing ? '0' : '1';
    if (text_write("/proc/"ABS_DIAGDEV_PROC_FILENAME, "w", p_cont) == ERR_SUCCESS) {
        p_abs->cashing = !(p_abs->cashing);

        ret = ERR_SUCCESS;     
    } else
        ret = ERR_CACHE_SWITCH;
    return ret;
}

unsigned int cache_test(absdiag_struc *p_abs)
{
    if (ABS_DIAG_CACHETEST_INPROCESS == p_abs->cash_test_stat)
        return DD_INPROC;
    if (ABS_DIAG_CACHETEST_READY == p_abs->cash_test_stat) {
        if (text_write("/proc/"ABS_DIAG_CACHETEST_PROC_FILENAME, "w", "0\n") == ERR_SUCCESS)
            return DD_SUCC;
        return DD_EOPEN;
    }
    
    return DD_EFAIL;

}

unsigned int cache_test_state(int test_state)
{
    unsigned int ret = DD_SUCC;

    switch (test_state) {
        case ABS_DIAG_CACHETEST_INPROCESS:
            D_printf("abs_diag cachetest is in process!\n");
            ret = DD_INPROC;
            break;
        case ABS_DIAG_CACHETEST_READY:
        case ABS_DIAG_CACHETEST_STOP_SUCC:
           ret = DD_SUCC;
           break;
        case ABS_DIAG_CACHETEST_STOP_FAIL:
            D_printf("abs_diag cachetest is failure to stop!\n");
           ret = DD_EFAIL;
           break;
        case ABS_DIAG_CACHETEST_STOP_NOMEM:
            D_printf("abs_diag cachetest stop for no memory!\n");
           ret = DD_ENOMEM;
           break;
        default:
           break;
    }
    return ret;
}

unsigned int test_cache_spill_tag(void)
{
    absdiag_struc abs;
    int cache_test_stat;
    unsigned int ret = ERR_SUCCESS;
    int i=0;

    memset((void *)&abs, 0, sizeof(abs));
    ret = verify_absdiag_dev(&abs);
    if (ret != ERR_SUCCESS) {
        memory_error_msg(ret);
        return ret;
    }

    do {
        i++;

        if(i >= 10)
            break;

        ret = cache_test(&abs);
        if (DD_EOPEN == ret) {
            break;
        } else if (DD_ENOMEM == ret || DD_INPROC == ret) {
            nsleep(1, 0, 0);
            continue;
        } else {
            ret = verify_absdiag_dev(&abs);
            if (ret != ERR_SUCCESS)
                return ret;
            cache_test_stat = abs.cash_test_stat;
            ret = cache_test_state(cache_test_stat);
            break;
        }
    } while (1);
   
    if (i == 10)
        ret = ERR_TIMEOUT_CACHE_TEST;

    return ret;
}
