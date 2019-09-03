/*!
*\file cache_randaddr.c
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
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <log.h>

extern int exit_flag;
extern int exit_on_error;

#define NO_COMPARISION		1


#include "jedi_comm.h"
#include "memory.h"
#include "cache_test.h"
#include "memory_err.h"

#define VERSION "0.1.1"

#define ERR_FAIL    1

#define JEDI_K2B(i) (i << 10)
#define JEDI_CACHE_BANK_LITE_SIZE   JEDI_K2B(8)
#define BANK    JEDI_CACHE_BANK_LITE_SIZE
#define CNT_PER_BANK    (BANK / sizeof(uintptr_t))

typedef uintptr_t addr_type;
typedef addr_type pat_type;
typedef ULV* pointer;


extern unsigned int rand_array[RAND_ARRAY_NUM][RAND_NUM_SIZE];

void init_random()
{
    unsigned int ticks;
    struct timeval tv;
    int fd;
    unsigned int ret = ERR_SUCCESS;

    gettimeofday(&tv, NULL);
    ticks = tv.tv_sec + tv.tv_usec;
    fd = open("/dev/urandom", O_RDONLY);
    if (fd > 0) {
        unsigned int r;
        int i;
        for (i=0; i<512; i++) {
            read(fd, &r, sizeof(r));
            ticks += r;
        }
        close(fd);
    } else {
        ret = -1;
        D_printf("Open /dev/urandom is failure\n");
    }
    srand(ticks);
}

unsigned int new_rand()
{
    int fd;
    unsigned int n=0;

    fd = open("/dev/urandom", O_RDONLY);
    if (fd>0) {
        read(fd, &n, sizeof(n));
    } 
    else {
        D_printf("Open /dev/urandom is failure\n");
    }
    close(fd);
    return n;
}


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

pointer mix_random(uintptr_t in)
{
    uintptr_t ret;

    in = in * 1221721 + 68314103;
    ret = in & 0xffff0000;
    in = in * 1221721 + 68314103;
    ret |= (in >> 16) & 0xffff;
    return (pointer)ret;
}


void randaddr_fillup(pointer test_buf, const UL cnt)
{
    pointer pa;
    UL i;

    for (i=0, pa=test_buf;
            i < cnt;
            i++, pa++) {
        *pa = (UL)mix_random((addr_type)pa);
  }
}

unsigned int randaddr_verify(pointer test_buf, UL test_cnt, int no_comp)
{
    pointer pa;
    UL pat;
    UL i;
    unsigned int ret = ERR_SUCCESS;

    for (i=0, pa = test_buf;
            i<test_cnt;
            i++, pa++) {
        pat = (UL)mix_random((addr_type)pa);
        if (no_comp == NO_COMPARISION) {
        }
        else {
        if (!(*pa == pat || *pa == ~pat)) {
            ret = ERR_FAIL;
            break;
        }
        }
    }
    return ret;
}



void randaddr_test(pointer test_buf, UL test_cnt, const UL cnt_per_bank)
{
    pointer pa;
    UL banks = 0;
    UL ibank = 0;
    UL irand = 0;
    UL array_p = 0;
    UL rand_val = 0;
    UL remain = 0;
    pointer test_p;
	int flag = 0;

    banks = test_cnt / cnt_per_bank;
    for (ibank=0, test_p = test_buf;
            ibank < banks;
            ibank++) {
        for (array_p=0; array_p<RAND_ARRAY_NUM; array_p++) {
            for (irand=0; irand<RAND_NUM_SIZE; irand++) {
                rand_val = (UL)((cnt_per_bank * (float)rand_array[array_p][irand])/(RAND_MAX+1.0));
                pa = test_p + rand_val;
                *pa = ~(*pa);
            }    
        }
        test_p += cnt_per_bank;
    }               
    
    remain = test_cnt - banks * (cnt_per_bank);
    lprintf(LOG_DEBUG, "The remain is 0x%lx", remain);
    for (array_p=0; array_p<RAND_ARRAY_NUM; array_p++) {
         if (flag == 1) {
                    lprintf(LOG_DEBUG, "Jump out of the external cycle, remain is 0x%lx", remain);
					break;
                    //goto the_end;
                }

        for (irand=0; irand<RAND_NUM_SIZE; irand++) {
            if ((RAND_ARRAY_NUM*array_p + irand) >= remain) {
                    lprintf(LOG_DEBUG, "Jump out of the internal cycle, remain is 0x%lx", remain);
					flag = 1;
					break;
                    //goto the_end;
                }
            rand_val = (UL)((remain * (float)rand_array[array_p][irand])/(RAND_MAX+1.0));
            pa = test_p + rand_val;
            *pa = ~(*pa);
        }
    }

the_end:
    return;
}

/*!
 *
 */
unsigned int test_cache_randaddr(ULV *p_start, UL block_bytes, int module_id, int no_comp)
{
    pointer test_buf;
    UL test_cnt;
    UL bank_size = 0;;
    unsigned int ret = ERR_SUCCESS;
    
    test_buf = p_start;
    test_cnt = ((block_bytes) / sizeof(UL));


    /* Memory bank size  */
    if (module_id == MEMORY_MODULE) //MEMORY_MODULE
        bank_size = MEM_BANK_SIZE;

    /* Cache bank size  */
    if (module_id == CPU_CACHE) //CPU_CACHE
        bank_size = MEM_BANK_SIZE;

    randaddr_fillup(test_buf, test_cnt);
    randaddr_test(test_buf, test_cnt, bank_size / sizeof(*test_buf));

    ret = randaddr_verify(test_buf, test_cnt, no_comp);
    if (ret != ERR_SUCCESS)
        ret = ERR_CACHE_RAND_DATA;

    return ret;
}
