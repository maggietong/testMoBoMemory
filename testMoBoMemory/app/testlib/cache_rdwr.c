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
#include <time.h>

#include "jedi_comm.h"
#include "memory.h"
#include "memory_err.h"

#define VERSION "0.1.1"

#define ERR_FAIL    1

#define JEDI_M2B(i) (((UL)(i)) << 20)
#define JEDI_L3CACHE_SIZE   JEDI_M2B(8)
#define JEDI_UNIT_SIZE  JEDI_L3CACHE_SIZE

#define MKPAT(n) (~((UL)(n)))
#define STEP     ((UL)JEDI_UNIT_SIZE << 1)

#define CNT_PER_STEP (STEP / sizeof (UL))

typedef uintptr_t addr_type;
typedef addr_type pat_type;
typedef ULV* pointer;


/*
 *!
 *
 * If you want to generate a random integer between 1 and 10, you should always
 * do it by using high-order bits, as in
 * j = 1 + (int) (10.0 * (rand() / (RAND_MAX + 1.0)));
 * 
 * and never by anything resembling
 *  j = 1 + (rand() % 10);
 *  (which uses lower-order bits)
 */
 unsigned int prand_btw32(const unsigned int begin, const unsigned int end)
{
    return begin + (unsigned int)((end * 1.0 * rand()) / (RAND_MAX + 1.0));
}

static inline void fillup(pointer p, const int cnt)
{
    int i;
    pointer pa;

    for  (i=0, pa = p; i<cnt; i++, pa++) {
        *pa = MKPAT(pa);
    }
}

unsigned int do_test(pointer p, const int cnt)
{
    pointer pa;
    int i;
    unsigned ret = ERR_SUCCESS;
    int read = 0;

    for (i=0, pa= p;
            i<cnt;
            i++, pa++) {
        read = prand_btw32(0, 512) ;
        read = read % 2;
        if (read) {
            if (*pa != MKPAT(pa)) {
                ret = ERR_FAIL;
                break;
            }
        } else {
            *pa = MKPAT(pa);
        }
    }
    return ret;
}

/*!
 *
 */
unsigned int test_cache_rdwr(ULV *p_start, UL block_bytes)
{
    ULV *test_step;
    UL test_cnt;
    UL step;
    UL steps;
    unsigned int ret = ERR_SUCCESS;
    
    test_cnt = (block_bytes) / sizeof(UL);
    steps = test_cnt / CNT_PER_STEP;

    fillup(p_start, test_cnt);
    
    for (step=0, test_step = p_start;
            step < steps;
            step++, test_step += CNT_PER_STEP) 
    {
        if (do_test(test_step, CNT_PER_STEP)) {
            ret = ERR_FAIL;
            break;
        }
    }
    if (ret != ERR_SUCCESS)
        ret = ERR_CACHE_RDWR;

    return ret;
}
