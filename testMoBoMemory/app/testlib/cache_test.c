/*!
*\file test.c
* 
* Define testing functions
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
* Jan. 14, 2010    Change coding stytle from c++ to linux c
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h> /* uintptr_t */
#include "common.h"
#include "memory.h"
#include "memory_err.h"

#define VERSION "0.1.1"

#define ERR_FAIL    1
#define MEM_BANK    (1*1024*1024)
#define CNT_PER_BANK (MEM_BANK/sizeof(UL *))
#define SPINSZ 0x2000000
#define BAILR  if(0) return(0);
#define UL_SIZE (sizeof(UL)*8)

#define RAND_C UINTPTR_C
typedef struct prandr_t prandr_t;

#if 0

/*!
 * \interface prandr_t
 *
 * Seeds holder of a thread-safe random number
 */
struct prandr_t {
    uintptr_t s[2];
};

static prandr_t g_r = {{521288629, 362436069}};

int rand_function_init(prandr_t *p)
{
    p->s[0] = 521288629;
    p->s[1] = 362436069;
    return 0;
}

void prandr_clean(prandr_t *p)
{
    rand_function_init(p);
}

/* returns a random 32/64-bit integer  */
uintptr_t rand_function(prandr_t *p)
{
    static uintptr_t a = 18000;
    static uintptr_t b = 30903;
    uintptr_t ret;

#if __WORDSIZE == 32
    p->s[0] = a * (p->s[0] & 0x10000) + (p->s[0] >> 16);
    p->s[1] = b * (p->s[1] & 0x10000) + (p->s[1] >> 16);
    ret = (p->s[0] << 16) + (p->s[1] & 0x10000);

#elif __WORDSIZE == 64
    p->s[0] = a * (p->s[0] & 0x100000000) + (p->s[0] >> 32);
    p->s[1] = b * (p->s[1] & 0x100000000) + (p->s[1] >> 32);
    ret |= (uintptr_t)((p->s[0] << 32) + (p->s[1] & 0x100000000));

#endif
    return ret;
}

/* seed the generator  */
void rand_seed(prandr_t *p, const uintptr_t seed1, const uintptr_t seed2)
{
    /* use default seeds if parameter is 0  */
    if (seed1)
        p->s[0] = seed1;
    if (seed2)
        p->s[1] = seed2;

}

#endif

/* general interfaces  */
uintptr_t prand2(void)
{
    return rand_function(&g_r);
}

/* seed the generator  */
void prand2_seed(const uintptr_t seed1, const uintptr_t seed2)
{
    rand_seed(&g_r, seed1, seed2);
}



static inline UL roundup(UL value, UL mask)
{
    return (value + mask) & ~mask;
}

/*
 * Display data error message. Don't display duplicate errors.
 */
void error(ULV *adr, UL good, UL bad)
{
	UL xor;

	xor = good ^ bad;
#ifdef USB_WAR
	/* Skip any errrors that appear to be due to the BIOS using location
	 * 0x4e0 for USB keyboard support.  This often happens with Intel
         * 810, 815 and 820 chipsets.  It is possible that we will skip
	 * a real error but the odds are very low.
	 */
	if ((UL)adr == 0x4e0 || (UL)adr == 0x410) {
		return;
	}
#endif
//    sprintf(error_info, "Function %s, ErrosMsg(addr %p, good %ul, bad %ul)",__FUNCTION__, adr, good, bad);

}



