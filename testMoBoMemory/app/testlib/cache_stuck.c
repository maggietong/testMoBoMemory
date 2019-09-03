/*!
*\file test.c
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
* Jan. 20, 2010    Change coding stytle from c++ to linux c
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h> /* uintptr_t */

#include "jedi_comm.h"
#include "memory.h"
#include "memory_err.h"

#define VERSION "0.1.1"

#define ERR_FAIL    1

#define JEDI_K2B(i) (((UL)(i)) << 10)
#define JEDI_CACHE_BANK_LITE_SIZE JEDI_K2B(8)
#define BANK ((UL)JEDI_CACHE_BANK_LITE_SIZE)
#define CNT_PER_BANK (BANK / sizeof(UL *))
#define CNT_HALF        (CNT_PER_BANK >> 1)

typedef uintptr_t addr_type;
typedef addr_type pat_type;
typedef ULV* pointer;

static inline void fillup(pointer p, const int idx, const UL pat, const UL pad)
{
    if (idx > 0) {
        p[idx - 1] = pad;
    }
    p[idx] = pat;
}


/* test_counts is the count of verification units  */
static int verify(pointer p, const int test_counts, const int idx, const UL pat, const UL pad)
{
    pointer ppat;
    int i = 0;
    UL vpat;
    UL vpad;
    unsigned int ret = ERR_SUCCESS;
    /* CNT-HALF change to 256   */
    for (i=0; i<test_counts; i++) {
        ppat = p + i;
        vpat = p[i];
        vpad = p[i + CNT_HALF];

        /* determining  */
        if (i != idx) {
            /* not a pattern location */
            if (i + CNT_HALF == idx) {
                /* another one is the pattern location  */
                vpat ^= vpad;
                vpad ^= vpat;
                vpat ^= vpad;
            } else {
                /* none  */
                ppat = NULL;
                
                /* merge into one  */
                vpad |= vpat;
            }
        }
        
        if (NULL == ppat) {
            if (pad != vpad) {
                ret = ERR_FAIL;
                break;
            }
        }else {
            if (pat != vpat || (pad != vpad)) {
                ret = ERR_FAIL;
                break;
            }
        }
    }
    return ret;
}

/*!
 *
 */
unsigned int test_cache_stuck(ULV *p_start, UL block_bytes)
{
    int banks = 0;
    int ibank = 0;
    int i = 0;
    ULV *pbank;
    UL pat = 0;
    unsigned int ret = ERR_SUCCESS;

//    printf("The CNT_PER_BANK is %x\n", CNT_PER_BANK);

//    printf("The BANK is %x\n", BANK);

    banks = ((block_bytes)/sizeof(UL)) / CNT_PER_BANK;
    pbank = p_start;
    memset((void *)p_start, 0x00, block_bytes);

    for (ibank = 0; 
        ibank < banks && 0 == ret;
        ibank++, pbank += CNT_PER_BANK) {
        
        for (i=0; i<CNT_PER_BANK && 0 == ret; i++) {
            for (pat = 0x01; 0 != pat; pat <<=1) {
                fillup((pointer)pbank, i, pat, 0x0);
                /* Below 16 is the verification positions  */
                if (verify((pointer)pbank, 16, i, pat, (UL)(0x0))) {
                ret = ERR_FAIL;
                break;
                }
            }
        }
    }
    if ( ret != ERR_SUCCESS) 
        return ret;

    pbank = p_start;
    memset((void *)p_start, 0xff, block_bytes);
    for (ibank = 0;
            ibank < banks && 0 == ret;
            ibank++, pbank += CNT_PER_BANK
        ) {
        for (i=0; i<CNT_PER_BANK && 0 == ret; i++) {
            for (pat = 0x01; 0 != pat; pat <<= 1) {
                pat = ~pat;
                fillup((pointer)pbank, i, pat, ~((UL)(0x0)));
                /* Below 16 is the verification positions  */
                if (verify((pointer)pbank, 16, i, pat, ~((UL)(0x0)))) {
                    ret = ERR_FAIL;
                    break;
                }
                pat = ~pat;
            }
        }
    }
    if (ret != ERR_SUCCESS)
        ret = ERR_CACHE_STUCK;

    return ret;
}
