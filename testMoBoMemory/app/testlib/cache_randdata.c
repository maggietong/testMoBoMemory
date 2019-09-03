/*!
*\file cache_randdata.c
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
* Jan. 26, 2010    Change coding stytle from c++ to linux c
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
#define PATCNT  18UL

typedef ULV* pointer;

/*! 18 patterns. In gpat/pat.c/cpp. */
/* regardless of little-/big-endian */
#if __WORDSIZE >  32
#define PATTERNS \
	0xFFFFFFFFFFFFFFFFUL, \
	0x0000000000000000UL, \
	0xF0F0F0F0F0F0F0F0UL, \
	0x0F0F0F0F0F0F0F0FUL, \
	0xAAAAAAAAAAAAAAAAUL, \
	0x5555555555555555UL, \
	0x8080808080808080UL, \
	0x4040404040404040UL, \
	0x2020202020202020UL, \
	0x1010101010101010UL, \
	0x0808080808080808UL, \
	0x0404040404040404UL, \
	0x0202020202020202UL, \
	0x0101010101010101UL, \
	0x1111111111111111UL, \
	0x2222222222222222UL, \
	0x4444444444444444UL, \
	0x8888888888888888UL
#else
#define PATTERNS \
	0xFFFFFFFFUL, \
	0x00000000UL, \
	0xF0F0F0F0UL, \
	0x0F0F0F0FUL, \
	0xAAAAAAAAUL, \
	0x55555555UL, \
	0x80808080UL, \
	0x40404040UL, \
	0x20202020UL, \
	0x10101010UL, \
	0x08080808UL, \
	0x04040404UL, \
	0x02020202UL, \
	0x01010101UL, \
	0x11111111UL, \
	0x22222222UL, \
	0x44444444UL, \
	0x88888888UL
#endif

static const uintptr_t g_pat[PATCNT] = {PATTERNS};
char *g_pat_pri = (char *)g_pat;

static inline uintptr_t jedi_rand(void)
{
    uintptr_t ret = (uintptr_t)rand();
#if __WORDSIZE == 64
    ret |= ((uintptr_t)rand() << 32);
#else
    /* Do nothing.  */
#endif
    return ret;
}

char *rand_function_a()
{
    unsigned long ret = 0;
    char *a;

    ret = jedi_rand();
    ret = ret % PATCNT;
    a = g_pat_pri;
    return (char *)(a + ret);

}

void randdata_fillup(UL test_len, pointer p_pa, pointer p_pb)
{
    pointer pa;
    pointer pb;
    UL i;
    UL cnt;

    cnt = test_len / sizeof(*pa);

    for (i=0, pa= p_pa, pb=p_pb;
            i < cnt;
            i++, pa++, pb++) {
        *pa = *(pointer)rand_function_a();
        *pb = *pa;
    }
}

int comp_long(UL test_len, pointer p_pa, pointer p_pb)
{
    pointer p1;
    pointer p2;
    UL i;
    UL cnt;
    int ret = ERR_SUCCESS;

    p1 = p_pa;
    p2 = p_pb;
    cnt = test_len / sizeof(*p1);

    for (i=0; i<cnt; 
            i++, p1++, p2++) {
        if (*p1 != *p2) {
            if (*p1 > *p2) {
                ret = 1;
            } else {
                ret = -1;
            }
            break;
        }
    }
    return ret;
}
/*!
 *
 */
unsigned int test_cache_randdata(ULV *p_start, UL block_bytes)
{
    pointer test_buf;
    UL test_cnt;
    pointer pa;
    pointer pb;
    UL len = 0;
    unsigned int ret = ERR_SUCCESS;
    
    test_buf = p_start;
    test_cnt = ((block_bytes) / sizeof(UL)) >> 1;

    pa = test_buf;
    pb = test_buf + test_cnt;
    len = test_cnt * sizeof(*test_buf);

    randdata_fillup(len, pa, pb);
    if (comp_long(len, pa, pb))
        ret = ERR_CACHE_RAND_DATA;

    return ret;
}
