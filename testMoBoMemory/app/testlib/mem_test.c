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
* Nov. 29, 2009    Change coding stytle from c++ to linux c
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h> /* uintptr_t */

#include "jedi_comm.h"
#include "memory.h"
#include "memory_err.h"
#include "log.h"

#define VERSION "0.1.1"

extern int exit_flag;
extern int exit_on_error;

#define HAS_COMPARISION  1
#define TEST_FROM_UP_TO_DOWN 0
#define TEST_FROM_DWON_TO_UP 1

#define BLOCK_SIZE_BYTES  (64*1024*1024) /* every block size is less than 4 G Bytes */
#define BLOCK_SIZE  (BLOCK_SIZE_BYTES/sizeof(UL)) /* every block size  */

#define JEDI_MODX 20 

#define ERR_FAIL    1
//#define MEM_BANK_1    (1*1024*1024) /* bank size is RANK/8  */
//#define MEM_CNT_PER_BANK (MEM_BANK_1/sizeof(UL *))
#define SPINSZ 0x2000000
#define BAILR  if(0) return(0);
#define MOD_SZ   20
#define UL_SIZE (sizeof(UL)*8)

#define RAND_C UINTPTR_C

typedef ULV* pointer;
typedef struct prandr_t prandr_t;

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
    uintptr_t ret = 0;

#if __WORDSIZE == 32
    p->s[0] = a * (p->s[0] & 0xFFFF) + (p->s[0] >> 16);
    p->s[1] = b * (p->s[1] & 0xFFFF) + (p->s[1] >> 16);
    ret = (p->s[0] << 16) + (p->s[1] & 0xFFFF);

#elif __WORDSIZE == 64
    p->s[0] = a * (p->s[0] & (UL)(0xFFFF)) + (p->s[0] >> 16);
    p->s[1] = b * (p->s[1] & (UL)(0xFFFF)) + (p->s[1] >> 16);
    ret |= (uintptr_t)((p->s[0] << 16) + (p->s[1] & (UL)(0xFFFF))) << 32;

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
    sprintf(error_info, "Function %s, ErrosMsg(addr %p, good %lx, bad %lx)",__FUNCTION__, adr, good, bad);

}

/*
 * Test all of memory using a "moving inversions" algorithm using the
 * pattern in p1 and it's complement in p2.
 */
unsigned int movinv1(ULV *start_p, UL cnt, int iter, UL p1, UL p2, int no_comp)
{
	int i, j, done;
	ULV *pe;
	ULV len;
	ULV *start; 
    ULV *end;
    UL segs = 1;
    ULV *p;
    UL bad = 0;
	
    unsigned int ret = ERR_SUCCESS;

	/* Display the current pattern */
//        hprint(LINE_PAT, COL_PAT, p1);

	/* Initialize memory with the initial pattern.  */
	for (j=0; j<segs; j++) {
		start = start_p;
		end = start_p + cnt;
		pe = start;
		p = start;
		done = 0;
		do {
			/* Check for overflow */
			if (pe + SPINSZ > pe) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			len = pe - p;
			if (p == pe ) {
				break;
			}
/* Original C code replaced with hand tuned assembly code*/
 			for (; p < pe; p++) {
 				*p = p1;
 			}
 
#if 0
			asm __volatile__ (
				"rep\n\t" \
				"stosl\n\t"
				: "=D" (p)
				: "c" (len), "0" (p), "a" (p1)
			);
#endif
//			do_tick();
		} while (!done);
	}

	/* Do moving inversions test. Check for initial pattern and then
	 * write the complement for each memory location. Test from bottom
	 * up and then from the top down.  */
	for (i=0; i<iter; i++) {
		for (j=0; j<segs; j++) {
			start = start_p;
			end = start_p + cnt;
			pe = start;
			p = start;
			done = 0;
			do {
				/* Check for overflow */
				if (pe + SPINSZ > pe) {
					pe += SPINSZ;
				} else {
					pe = end;
				}
				if (pe >= end) {
					pe = end;
					done++;
				}
				if (p == pe ) {
					break;
				}
/* Original C code replaced with hand tuned assembly code*/
                    for (; p < pe; p++) {
                        if (no_comp != HAS_COMPARISION) {
                        }
                        else {
 					    if ((bad=*p) != p1) {
 						    lprintf(LOG_ERR, "Address %p, Good 0x%lx, Bad 0x%lx", (ulong*)p, p1, bad);
                            ret = ERR_FAIL;
 					    }
                        }
 					    *p = p2;
 				    }
            
			//	do_tick();
			} while (!done);
		}
		for (j=segs-1; j>=0; j--) {
			start = start_p;
			end = start_p + cnt;
			pe = end -1;
			p = end -1;
			done = 0;
			do {
				/* Check for underflow */
				if (pe - (UL)SPINSZ < pe) {
					pe -= (UL)SPINSZ;
				} else {
					pe = start;
				}
				if (pe <= start) {
					pe = start;
					done++;
				}
				if (p == pe ) {
					break;
				}
/* Original C code replaced with hand tuned assembly code*/
 				do {
 					if (no_comp != HAS_COMPARISION) {
                    }
                    else {
                    if ((bad=*p) != p2) {
 						lprintf(LOG_ERR, "Address %p, Good 0x%lx, Bad 0x%lx", (ulong*)p, p2, bad);
 					    ret = ERR_FAIL;
                    }
                    }
					*p = p1;
				} while (p-- > pe);
				
			} while (!done);
		}
#if 0
	int tmp_num = 0;

		lprintf(LOG_DEBUG, "Algorithm 3, address is [0x%lx, 0x%lx], p=%lx \n", start, end, p);
/*-----------------*/
		printf("The address is [0x%lx, 0x%lx], p=%lx\n", start, end, p);
		printf("p1=%lx, p2=%lx\n", p1, p2);
	for (p=start, tmp_num=0; p<end; p++, tmp_num++)
	{
		printf("0x%lx  ", *(p));
		if ((tmp_num%16) == 0)
			printf("\n");
	}
		printf("\n");

/*-----------------*/
#endif

	}	
	
    return ret;
}


/*!
 * Memory address walking ones test
 * \param point -   the start memory position to be tested.
 * \param cnt   -   the covered counts
 * \return  0      success
 *          Not 0  return error code
 */
unsigned int addr_walk_ones(ULV *start, UL cnt, int no_comp)
{
//    int t = 0;
	UL i, j, k;
	ULV *pt;
	ULV *end;
	UL bad, mask, bank;
    ULV *p;
    UL segs = 1;
    UL p1 = 0;
    unsigned int ret = 0;

//#if 0
	/* Test the global address bits */
	for (j=0; j<2; j++) {
		/* Set pattern in our lowest multiple of 0x20000 */
		end = start + cnt;
/* Chnage from 128K to 4K roundup */
		p = (UL *)roundup((ulong)start, 0x0fff);
//		p = (UL *)roundup((ulong)start, 0x1ffff);
		*p = p1;	
		/* Now write pattern compliment */
		p1 = ~p1;
		for (i=0; i<100; i++) {
			mask = sizeof(UL);
			do {
				pt = (UL*)((UL)p | mask);
				if ((uintptr_t)pt == (uintptr_t)p) {
					mask = mask << 1;
					continue;
				}
				if ((uintptr_t)pt >= (uintptr_t)end) {
					break;
				}
				*pt = p1;
                if (no_comp != HAS_COMPARISION) {
                }
                else {
				if ((uintptr_t)(bad = *p) != (uintptr_t)~p1) {
                    ret = ERR_FAIL;
                    lprintf(LOG_ERR, "Function %s, ErrosMsg(addr 0x%p, good 0x%lx, bad 0x%lx)", __FUNCTION__, p, ~p1, bad);
					i = 1000;
				}
                }
				mask = mask << 1;
			} while(mask);
		}
        if (ret != 0)
            return ret;
	}
//#endif

#if 0
	int tmp_num = 0;

		lprintf(LOG_DEBUG, "Algorithm 0, address is [0x%lx, 0x%lx], p=%lx \n", start, end, p);
/*-----------------*/
		printf("The address is [0x%lx, 0x%lx], p=%lx\n", start, end, p);
	for (p=start, tmp_num=0; p<end; p++, tmp_num++)
	{
		printf("0x%lx  ", *(p));
		if ((tmp_num%16) == 0)
			printf("\n");
	}
		printf("\n");

/*-----------------*/
#endif

	/* Now check the address bits in each bank */
	/* If we have more than 8mb of memory then the bank size must be */
	/* bigger than 256k.  If so use 1mb for the bank size. */
//	if (v->pmap[v->msegs - 1].end > (0x800000 >> 12)) {
		bank = 0x100000;
//	} else {
//		bank = 0x40000;
//	}

    p1 = 0;
	for (k=0; k<2; k++) {
		for (j=0; j<segs; j++) {
			/* Force start address to be a multiple of 256k */
			p = (UL *)roundup((UL)start, bank - 1);
			while (p < end) {
				*p = p1;
				p1 = ~p1;
				for (i=0; i<50; i++) {
					mask = sizeof(UL);
					do {
						pt = (UL *)((UL)p | mask);
						if ((uintptr_t)pt == (uintptr_t)p) {
							mask = mask << 1;
							continue;
						}
						if ((uintptr_t)pt >= (uintptr_t)end) {
							break;
						}
						*pt = p1;
                        
                        if (no_comp != HAS_COMPARISION) {
                        }
                        else {
						if ((bad = *p) != ~p1) {
							ret = ERR_FAIL;
                            lprintf(LOG_ERR, "Function %s, ErrorMSG(addr %p, good %lx, bad %lx)", __FUNCTION__, p, ~p1, bad);
                            i = 200;
						}
                        }
						mask = mask << 1;
//						t++;
//						printf("T is %d, mask %lx  ", t, mask);
					} while(mask);
				}
				if ((uintptr_t)(p + bank) > (uintptr_t)p) {
					p += bank;
				} else {
					p = end;
				}
				p1 = ~p1;
//				printf("*************test 0****address %lx\n", p);
			}
		}

#if 0
//	int tmp_num = 0;

		lprintf(LOG_DEBUG, "Algorithm 0, address is [0x%lx, 0x%lx], p=%lx \n", start, end, p);
/*-----------------*/
		printf("The address is [0x%lx, 0x%lx], p=%lx\n", start, end, p);
	for (p=start, tmp_num=0; p<end; p++, tmp_num++)
	{
		printf("0x%lx  ", *(p));
		if ((tmp_num%16) == 0)
			printf("\n");
	}
		printf("\n");

/*-----------------*/
#endif

		if (ret != 0)
            return ret;
		p1 = ~p1;
	}


    return ret;
}

/*!
 * Fill up this bank by writing the corresponding address.
 * \parm p -    the start memory position of current bank to
 *              be tested
 */

#if 0
static inline void fillup(ULV *p)
{
    ULV *cur = p;
    UL i = 0;

    for (i=0, cur=p; i<MEM_CNT_PER_BANK; i++, cur++) {
        *cur = (UL)cur;
    }
}
#endif

static inline void fillup(ULV *p, UL cnt)
{
    ULV *cur = p;
    ULV *end = p;

    end = p + cnt;
    for (cur=p; cur<end; cur++) {
        *cur = (UL)cur;
    }
}

/*!
 * Verifies this bank
 * \param p -   the start memory position of
 *              current bank to be tested
 */
static unsigned int verify(ULV *p, UL cnt, int no_comp)
{
    ULV *cur = p;
    ULV *end = p;

    unsigned int ret = ERR_SUCCESS;

    end = p + cnt;
    for (cur=p; cur<end; cur++) {
        if (no_comp != HAS_COMPARISION) {
        }
        else {
        if (*cur != (UL)cur) {
            ret = ERR_FAIL;
            break;
        }
        }
    }
    return ret;
}

/*!
 *
 *
 */
unsigned int addr_walk_own(ULV *p_start, UL cnt, int no_comp)
{
    ULV *bank_p;
    UL banks = 0;
    UL bank = 0;
    unsigned int ret = ERR_SUCCESS;

    fillup(p_start, cnt);
    if (verify(p_start, cnt, no_comp) != 0) {
        ret = ERR_FAIL;
    }

#if 0
    banks = cnt / MEM_CNT_PER_BANK;
    for (bank=0, bank_p=p_start; bank<banks; bank++, bank_p += MEM_CNT_PER_BANK) {
        fillup(bank_p);
        if (verify(bank_p, no_comp) != 0) {
            ret = ERR_FAIL;
            break;
        }
    }
#endif

    return ret;
}

/*
 * Test memory using block moves 
 * Adapted from Robert Redelmeier's burnBX test
 */
unsigned int block_move(ULV *start_p, UL cnt, int iter, int no_comp)
{
	UL i, j, done;
	UL len;
    UL segs = 1;
	ULV *p;
    ULV *pe;
    ULV *pp;
	ULV *start;
    ULV *end;
    unsigned int ret = ERR_SUCCESS;

//	lprintf(LOG_DEBUG, "The cnt = %lx \n", cnt);
//	cnt =  (((unsigned long)cnt) & ~(unsigned long)0x0f);
//	lprintf(LOG_DEBUG,"The roundup cnt = %lx \n", cnt);
	/* Initialize memory with the initial pattern.  */
	for (j=0; j<segs; j++) {
		start = start_p;
#ifdef USB_WAR
		/* We can't do the block move test on low memory because
		 * BIOS USB support clobbers location 0x410 and 0x4e0
		 */
//		if (start < 0x4f0) {
//			start = 0x4f0;
//		}
#endif
		end = start_p + cnt;
		pe = start;
		p = start;
		done = 0;
		do {
			/* Check for overflow */
			if (pe + SPINSZ*4 > pe) {
				pe += SPINSZ*4;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
			len  = ((UL)pe - (UL)p) / 64;
                
#if __WORDSIZE == 32
			asm __volatile__ (
				"jmp L100\n\t"

				".p2align 4,,7\n\t"
				"L100:\n\t"
				"movl %%eax, %%edx\n\t"
				"notl %%edx\n\t"
				"movl %%eax,0(%%edi)\n\t"
				"movl %%eax,4(%%edi)\n\t"
				"movl %%eax,8(%%edi)\n\t"
				"movl %%eax,12(%%edi)\n\t"
				"movl %%edx,16(%%edi)\n\t"
				"movl %%edx,20(%%edi)\n\t"
				"movl %%eax,24(%%edi)\n\t"
				"movl %%eax,28(%%edi)\n\t"
				"movl %%eax,32(%%edi)\n\t"
				"movl %%eax,36(%%edi)\n\t"
				"movl %%edx,40(%%edi)\n\t"
				"movl %%edx,44(%%edi)\n\t"
				"movl %%eax,48(%%edi)\n\t"
				"movl %%eax,52(%%edi)\n\t"
				"movl %%edx,56(%%edi)\n\t"
				"movl %%edx,60(%%edi)\n\t"
				"rcll $1, %%eax\n\t"
				"leal 64(%%edi), %%edi\n\t"
				"decl %%ecx\n\t"
				"jnz  L100\n\t"
				: "=D" (p)
				: "D" (p), "c" (len), "a" (1)
				: "edx"
			);
#elif __WORDSIZE == 64
            asm __volatile__ (
       			"jmp L100\n\t"
    
	    		".p2align 4,,7\n\t" 
		    	"L100:\n\t"
		    	"movq %%rax, %%rdx\n\t"
		    	"notq %%rdx\n\t"
		    	"movq %%rax,0(%%rdi)\n\t"
		    	"movq %%rax,8(%%rdi)\n\t"
		    	"movq %%rdx,16(%%rdi)\n\t"
		    	"movq %%rax,24(%%rdi)\n\t"
		    	"movq %%rax,32(%%rdi)\n\t"
		    	"movq %%rdx,40(%%rdi)\n\t"
		    	"movq %%rax,48(%%rdi)\n\t"
		    	"movq %%rdx,56(%%rdi)\n\t"
		    	"rclq $1, %%rax\n\t"
		    	"leaq 64(%%rdi), %%rdi\n\t"
		    	"decq %%rcx\n\t"
		    	"jnz  L100\n\t"
		    	: "=D" (p)
		    	: "D" (p), "c" (len), "a" (1)
		    	: "rdx"
			);
#endif
		} while (!done);
	}

#if 0
	int tmp_num = 0;

		lprintf(LOG_DEBUG, "--1 The address is [0x%lx, 0x%lx], len=%lx, p=%lx \n", start, pe,len, p);
/*-----------------*/
		printf("The address is [0x%lx, 0x%lx], len=%lx, p=%lx\n", start, pe,len, p);
	for (p=start, tmp_num=0; p<pe; p++, tmp_num++)
	{
		printf("0x%lx  ", *(p));
		if ((tmp_num%8) == 0)
			printf("\n");
	}
		printf("\n");

/*-----------------*/
#endif
	/* Now move the data around 
	 * First move the data up half of the segment size we are testing
	 * Then move the data to the original location + 32 bytes
	 */
	for (j=0; j<segs; j++) {
		start = start_p;
#ifdef USB_WAR
		/* We can't do the block move test on low memory beacuase
		 * BIOS USB support clobbers location 0x410 and 0x4e0
		 */
//		if (start < 0x4f0) {
//			start = 0x4f0;
//		}
#endif
		end = start_p + cnt;
		pe = start;
		p = start;
		done = 0;
		do {
			/* Check for overflow */
			if (pe + SPINSZ*4 > pe) {
				pe += SPINSZ*4;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
			pp = p + ((pe - p) / 2);
// should be 8			len  = ((UL)pe - (UL)p) / (sizeof(UL)*2);
			len  = ((UL)pe - (UL)p) / 8;
			for(i=0; i<iter; i++) {

#if __WORDSIZE == 32
				asm __volatile__ (
					"cld\n"
					"jmp L110\n\t"

					".p2align 4,,7\n\t"
					"L110:\n\t"
					"movl %1,%%edi\n\t"
					"movl %0,%%esi\n\t"
					"movl %2,%%ecx\n\t"
					"rep\n\t"
					"movsl\n\t"
					"movl %0,%%edi\n\t"
					"addl $32,%%edi\n\t"
					"movl %1,%%esi\n\t"
					"movl %2,%%ecx\n\t"
					"subl $8,%%ecx\n\t"
					"rep\n\t"
					"movsl\n\t"
					"movl %0,%%edi\n\t"
					"movl $8,%%ecx\n\t"
					"rep\n\t"
					"movsl\n\t"
					:: "g" (p), "g" (pp), "g" (len)
					: "edi", "esi", "ecx"
				);
#elif __WORDSIZE == 64
                asm __volatile__ (
		            "cld\n"
		            "jmp L110\n\t"

		            ".p2align 4,,7\n\t"
		            "L110:\n\t"
		            "movq %1,%%rdi\n\t"
		            "movq %0,%%rsi\n\t"
		            "movq %2,%%rcx\n\t"
		            "rep\n\t"
		            "movsl\n\t"
		            "movq %0,%%rdi\n\t"
		            "addq $32,%%rdi\n\t" 
		            "movq %1,%%rsi\n\t"
		            "movq %2,%%rcx\n\t"
		            "subq $8,%%rcx\n\t" 
		            "rep\n\t"
		            "movsl\n\t"
		            "movq %0,%%rdi\n\t"
		            "movq $8,%%rcx\n\t"
		            "rep\n\t"
		            "movsl\n\t"
		            :: "g" (p), "g" (pp), "g" (len)
		            : "rdi", "rsi", "rcx"
		        );
#endif
			}
			p = pe;

		} while (!done);
	}
		
#if 0
	lprintf(LOG_DEBUG, "---2 The address is [0x%lx, 0x%lx], len=0x%lx, p=0x%lx, pp=0x%lx \n", start, pe,len, p, pp);
/*-----------------*/
		printf("The address is [0x%lx, 0x%lx], len=%lx, p=%lx\n", start, pe,len, p);
	for (p=start,tmp_num=0; p<pe; p++, tmp_num++)
	{
		printf("0x%lx  ", *(p));
		if ((tmp_num%8) == 0)
			printf("\n");
	}
		printf("\n");

/*-----------------*/
#endif

	/* Now check the data 
	 * The error checking is rather crude.  We just check that the
	 * adjacent words are the same.
	 */
	for (j=0; j<segs; j++) {
		start = start_p;
#ifdef USB_WAR
		/* We can't do the block move test on low memory beacuase
		 * BIOS USB support clobbers location 0x4e0 and 0x410
		 */
//		if (start < 0x4f0) {
//			start = 0x4f0;
//		}
#endif
		end = start_p + cnt;
		pe = start;
		p = start;
		done = 0;
		do {
			/* Check for overflow */
			if (pe + SPINSZ*4 > pe) {
				pe += SPINSZ*4;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}

        if (no_comp != HAS_COMPARISION) {
        }
        else {
#if __WORDSIZE == 32
	if (p[0] != p[1] || p[0] != p[2] || p[0] != p[3]) {
		sprintf(error_info, "should be same pat @ 0:1:2:3, address %p", p);

		ret = ERR_FAIL;
	}
	if (p[4] != ~p[0] || p[5] != ~p[0]) {
		sprintf(error_info, "should be same pat @ 4:5, address %p", p);
		ret = ERR_FAIL;
	}
	if (p[6] != p[7] || p[6] != p[8] || p[6] != p[9]) {
		sprintf(error_info, "should be same pat @ 6:7:8:9, address %p", p);
		ret = ERR_FAIL;
	}
	if (p[10] != ~p[0] || p[11] != ~p[0]) {
		sprintf(error_info, "should be same pat @ 10:11, address %p", p);
		ret = ERR_FAIL;
	}
	if (p[0] != p[12] || p[0] != p[13]) {
		sprintf(error_info, "should be same pat @ 0:12:13, address %p", p);
		ret = ERR_FAIL;
	}
	if (p[14] != ~p[0] || p[15] != ~p[0]) {
		sprintf(error_info, "should be complement @ 14:15, address %p", p);
		ret = ERR_FAIL;
	}
#elif __WORDSIZE == 64

#if 0

/*-----------------*/
		lprintf(LOG_DEBUG, "---3 The address is [0x%lx, 0x%lx], len=%lx, p=%lx, cnt=%lx \n", start, pe, len, p, cnt);
	for (tmp_num=0; tmp_num<8; tmp_num++)
	{
		lprintf(LOG_DEBUG, "0x%lx  ", p[tmp_num]);
	}
		lprintf(LOG_DEBUG, "\n");

/*-----------------*/
#endif

#if 0
	if ((p[0] != p[1]) && (p[0] != ~p[1])) {
		sprintf(error_info, "should be same pat @ 0:1, address %p", p);
		ret = ERR_FAIL;
	}
	if ((p[2] != ~p[0]) && (p[2] != p[0])) {
		sprintf(error_info, "should be complement @ 2, address %p", p);
		ret = ERR_FAIL;
	}
	if ((p[0] != p[3]) && (p[0] != ~p[3])) {
		sprintf(error_info, "should be same pat @ 0:3:4, address %p", p);
		ret = ERR_FAIL;
	}
	if ((p[5] != ~p[0]) && (p[5] != p[0])) {
		sprintf(error_info, "should be complement @ 5, address %p", p);
		ret = ERR_FAIL;
	}
	if ((p[0] != p[6]) && (p[0] != ~p[6])) {
		sprintf(error_info, "should be same pat @ 0:6, address %p", p);
		ret = ERR_FAIL;
	}
	if ((p[7] != ~p[0])&&(p[7] != p[0])) {
		sprintf(error_info, "should be complement @ 7, address %p", p);
		ret = ERR_FAIL;
	}
#endif

	if (p[0] != p[1]) {
		sprintf(error_info, "should be same pat @ 0:1, address %p", p);
		ret = ERR_FAIL;
	}
	if (p[2] != ~p[0]) {
		sprintf(error_info, "should be complement @ 2, address %p", p);
		ret = ERR_FAIL;
	}
	if (p[0] != p[3] && p[0] != p[4]) {
		sprintf(error_info, "should be same pat @ 0:3:4, address %p", p);
		ret = ERR_FAIL;
	}
	if (p[5] != ~p[0]) {
		sprintf(error_info, "should be complement @ 5, address %p", p);
		ret = ERR_FAIL;
	}
	if (p[0] != p[6]) {
		sprintf(error_info, "should be same pat @ 0:6, address %p", p);
		ret = ERR_FAIL;
	}
	if (p[7] != ~p[0]) {
		sprintf(error_info, "should be complement @ 7, address %p", p);
		ret = ERR_FAIL;
	}
	}
#endif

#if 0
			asm __volatile__ (
				"jmp L120\n\t"

				".p2align 4,,7\n\t"
				"L120:\n\t"
				"movl (%%edi),%%ecx\n\t"
				"cmpl 4(%%edi),%%ecx\n\t"
				"jnz L121\n\t"

				"L122:\n\t"
				"addl $8,%%edi\n\t"
				"cmpl %%edx,%%edi\n\t"
				"jb L120\n"
				"jmp L123\n\t"

				"L121:\n\t"
				"pushl %%edx\n\t"
				"pushl 4(%%edi)\n\t"
				"pushl %%ecx\n\t"
				"pushl %%edi\n\t"
				"call error\n\t"
				"popl %%edi\n\t"
				"addl $8,%%esp\n\t"
				"popl %%edx\n\t"
				"jmp L122\n"
				"L123:\n\t"
				: "=D" (p)
				: "D" (p), "d" (pe)
				: "ecx"
			);
#endif
        if (ret == ERR_SUCCESS)
            p += 64 / sizeof(UL);
        else
            break;

		} while (!done);
	}
    return ret;
}

unsigned int movinv32(ULV *start_p, UL cnt, int iter, UL p1, UL lb, UL hb, UL sval, UL off, int no_comp)
{
//	int t = 0;
    int i, j, k=0, done;
	ULV *pe;
	ULV *start, *end;
	UL pat = 0;
    UL segs = 1;
    unsigned int ret = ERR_SUCCESS;
    ULV *p;
    UL bad = 0;
    UL p3;
    
    p3 = (sval << (UL_SIZE -1));

	/* Display the current pattern */
//	hprint(LINE_PAT, COL_PAT, p1);

	/* Initialize memory with the initial pattern.  */
	for (j=0; j<segs; j++) {
		start = start_p;
		end = start_p + cnt;
		pe = start;
		p = start;
		done = 0;
		k = off;
		pat = p1;
		do {
			/* Check for overflow */
			if (pe + SPINSZ > pe) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
			/* Do a SPINSZ section of memory */
 			while (p < pe) {
    			*p = pat;
 				if (++k >= UL_SIZE) {
 					pat = lb;
 					k = 0;
 				} else {
 					pat = pat << 1;
 					pat |= sval;
 				}
 				p++;
 			}
		} while (!done);
	}

#if 0
	int tmp_num = 0;

		lprintf(LOG_DEBUG, "--1 The address is [0x%lx, 0x%lx], p=%lx \n", start, pe, p);
/*-----------------*/
		printf("The address is [0x%lx, 0x%lx], p=%lx\n", start, pe, p);
	for (p=start, tmp_num=0; p<pe; p++, tmp_num++)
	{
		printf("0x%lx  ", *(p));
		if ((tmp_num%8) == 0)
			printf("\n");
	}
		printf("\n");

/*-----------------*/
#endif

	/* Do moving inversions test. Check for initial pattern and then
	 * write the complement for each memory location. Test from bottom
	 * up and then from the top down.  */
	for (i=0; i<iter; i++) {
		for (j=0; j<segs; j++) {
			start = start_p;
			end = start + cnt;
			pe = start;
			p = start;
			done = 0;
			k = off;
			pat = p1;
			do {
				/* Check for overflow */
				if (pe + SPINSZ > pe) {
					pe += SPINSZ;
				} else {
					pe = end;
				}
				if (pe >= end) {
					pe = end;
					done++;
				}
				if (p == pe ) {
					break;
				}
/* Original C code replaced with hand tuned assembly code */
 				while (p < pe) {
                    if (no_comp != HAS_COMPARISION) {
                    }
                    else {
 					if ((bad=*p) != pat) {
 						lprintf(LOG_ERR, "Address %p, Good %lx, Bad %lx", p, pat, bad);
                        ret = ERR_FAIL;
 					}
                    }
 					*p = ~pat;
 					if (++k >= UL_SIZE) {
 						pat = lb;
 						k = 0;
 					} else {
 						pat = pat << 1;
 						pat |= sval;
 					}
 					p++;
 				}
				if (ret != ERR_SUCCESS)
				return ret;
			} while (!done);
		}
#if 0
/*-----------------*/
		printf("The address is [0x%lx, 0x%lx], p=%lx\n", start, pe, p);
	for (p=start, tmp_num=0; p<pe; p++, tmp_num++)
	{
		printf("0x%lx  ", *(p));
		if ((tmp_num%8) == 0)
			printf("\n");
	}
		printf("\n");

/*-----------------*/
#endif

		/* Since we already adjusted k and the pattern this
		 * code backs both up one step
		 */
/* CDH start */
/* Original C code replaced with hand tuned assembly code */
 		pat = lb;
 		if ( 0 != (k = (k-1) & (UL_SIZE - 1)) ) {
 			pat = (pat << k);
 			if ( sval )
 			pat |= ((sval << k) - 1);
 		}
 		k++;
//		t++;
//       printf("T is %d, i is %d\n", t, i);
    /* CDH end */
		for (j=segs-1; j>=0; j--) {
			start = start_p;
			end = start + cnt;
			p = end -1;
			pe = end -1;
			done = 0;
			do {
				/* Check for underflow */
				if (pe - SPINSZ < pe) {
					pe -= SPINSZ;
				} else {
					pe = start;
				}
				if (pe <= start) {
					pe = start;
					done++;
				}
				if (p == pe ) {
					break;
				}
/* Original C code replaced with hand tuned assembly code i*/
 				do {
 					if (no_comp != HAS_COMPARISION) {
                    }
                    else {
                    if ((bad=*p) != ~pat) {
 						lprintf(LOG_ERR, "Adress %p, Good %lx, Bad %lx", p, ~pat, bad);
/* Test the absolut platform  */
                        ret = ERR_FAIL;
 					}
                    }
 					*p = pat;
 					if (--k <= 0) {
 						pat = hb;
 						k = UL_SIZE;
 					} else {
 						pat = pat >> 1;
 						pat |= p3; //p3
 					}
 				} while (p-- > pe);

				if (ret != ERR_SUCCESS)
                    return ret;
			} while (!done);
		}
	}
    return ret;
}


/*
 * Test all of memory using a "half moving inversions" algorithm using random
 * numbers and their complment as the data pattern. Since we are not able to
 * produce random numbers in reverse order testing is only done in the forward
 * direction.
 */
unsigned int movinvr(ULV *start_p, UL cnt, int no_comp)
{
	UL i, j, done;
    UL seed, seed1, seed2;
    UL sz;
	ULV *pe;
	ULV *start,*end;
	UL num;
    UL segs = 1;
    ULV *p;
    UL bad = 0;
    unsigned int ret = ERR_SUCCESS;
    prandr_t rnd;

	/* Initialize memory with initial sequence of random numbers.  */
#if 0
    if (v->rdtsc) {
		asm __volatile__ ("rdtsc":"=a" (seed1),"=d" (seed2));
	} else {
		seed1 = 521288629 + v->pass;
		seed2 = 362436069 - v->pass;
	}
#endif
        sz = cnt * sizeof(*p);
        rand_function_init(&rnd);

        seed = sz % 10000;
		seed1 = 521288629 + seed;

		seed2 = 362436069 - seed;
	/* Display the current seed */
//        hprint(LINE_PAT, COL_PAT, seed1);
	rand_seed(&rnd, seed1, seed2);
	for (j=0; j<segs; j++) {
		start = start_p;
		end = start + cnt;
		pe = start;
		p = start;
		done = 0;
		do {
			/* Check for overflow */
			if (pe + SPINSZ > pe) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
/* Original C code replaced with hand tuned assembly code */

			for (; p < pe; p++) {
				*p = rand_function(&rnd);
			}

#if 0

                        asm __volatile__ (
                                "jmp L200\n\t"
                                ".p2align 4,,7\n\t"
                                "L200:\n\t"
                                "call rand\n\t"
				"movl %%eax,(%%edi)\n\t"
                                "addl $4,%%edi\n\t"
                                "cmpl %%ebx,%%edi\n\t"
                                "jb L200\n\t"
                                : "=D" (p)
                                : "D" (p), "b" (pe)
				: "eax"
                        );
#endif
//			do_tick();
			BAILR
		} while (!done);
	}

	/* Do moving inversions test. Check for initial pattern and then
	 * write the complement for each memory location. Test from bottom
	 * up and then from the top down.  */
	for (i=0; i<2; i++) {
//	printf("i is %d\n", i);
	    rand_seed(&rnd, seed1, seed2);
		for (j=0; j<segs; j++) {
			start = start_p;
			end = start + cnt;
			pe = start;
			p = start;
			done = 0;
			do {
				/* Check for overflow */
				if (pe + SPINSZ > pe) {
					pe += SPINSZ;
				} else {
					pe = end;
				}
				if (pe >= end) {
					pe = end;
					done++;
				}
				if (p == pe ) {
					break;
				}
/* Original C code replaced with hand tuned assembly code */
				for (; p < pe; p++) {
					num = rand_function(&rnd);
					if (i) {
						num = ~num;
					}
                    if (no_comp != HAS_COMPARISION) {
                    }
                    else {
					if ((bad=*p) != num) {
						lprintf(LOG_ERR, "Address %p, Good %lx, Bad %lx", (ulong*)p, num, bad);
                        ret = ERR_FAIL;
                        return ret;
					}
                    }
					*p = ~num;
				}

				if (i) {
					num = ~(UL)0;
				} else {
					num = 0;
				}
#if 0
				asm __volatile__ (
					"jmp L26\n\t" \

					".p2align 4,,7\n\t" \
					"L26:\n\t" \
					"call rand\n\t"
					"xorl %%ebx,%%eax\n\t" \
					"movl (%%edi),%%ecx\n\t" \
					"cmpl %%eax,%%ecx\n\t" \
					"jne L23\n\t" \
					"L25:\n\t" \
					"movl $0xffffffff,%%edx\n\t" \
					"xorl %%edx,%%eax\n\t" \
					"movl %%eax,(%%edi)\n\t" \
					"addl $4,%%edi\n\t" \
					"cmpl %%esi,%%edi\n\t" \
					"jb L26\n\t" \
					"jmp L24\n" \

					"L23:\n\t" \
					"pushl %%esi\n\t" \
					"pushl %%ecx\n\t" \
					"pushl %%eax\n\t" \
					"pushl %%edi\n\t" \
					"call error\n\t" \
					"popl %%edi\n\t" \
					"popl %%eax\n\t" \
					"popl %%ecx\n\t" \
					"popl %%esi\n\t" \
					"jmp L25\n" \

					"L24:\n\t" \
					: "=D" (p)
					: "D" (p), "S" (pe), "b" (num)
					: "eax", "ecx", "edx"
				);
#endif
				BAILR
			} while (!done);
		}
	}
    prandr_clean(&rnd);
    return ret;
}


/*
 * Test all of memory using modulo X access pattern.
 */
unsigned int modtst(ULV *start_p, UL cnt, int iter, UL offset, UL p1, UL p2, int no_comp)
{
	UL j, k, l, done;
	ULV *pe;
	ULV *start, *end;
    UL segs = 1;
    ULV *p;
    UL bad;
    unsigned int ret = ERR_SUCCESS;


	/* Display the current pattern */
   //     hprint(LINE_PAT, COL_PAT-2, p1);
//	cprint(LINE_PAT, COL_PAT+6, "-");
  //      dprint(LINE_PAT, COL_PAT+7, offset, 2, 1);

	/* Write every nth location with pattern */
	for (j=0; j<segs; j++) {
		start = start_p;
		end = start + cnt;
		pe = start;
		p = start+offset;
		done = 0;
		do {
			/* Check for overflow */
			if (pe + SPINSZ > pe) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
/* Original C code replaced with hand tuned assembly code
 */
            for (; p < pe; p += MOD_SZ) {
 				*p = p1;
 			}
 
/*
			asm __volatile__ (
				"jmp L60\n\t" \
				".p2align 4,,7\n\t" \

				"L60:\n\t" \
				"movl %%eax,(%%edi)\n\t" \
				"addl $80,%%edi\n\t" \
				"cmpl %%edx,%%edi\n\t" \
				"jb L60\n\t" \
				: "=D" (p)
				: "D" (p), "d" (pe), "a" (p1)
			);
*/
//			do_tick();
			BAILR
		} while (!done);
	}

	/* Write the rest of memory "iter" times with the pattern complement */
	for (l=0; l<iter; l++) {
		for (j=0; j<segs; j++) {
			start = start_p;
			end = start + cnt;
			pe = start;
			p = start;
			done = 0;
			k = 0;
			do {
				/* Check for overflow */
				if (pe + SPINSZ > pe) {
					pe += SPINSZ;
				} else {
					pe = end;
				}
				if (pe >= end) {
					pe = end;
					done++;
				}
				if (p == pe ) {
					break;
				}
/* Original C code replaced with hand tuned assembly code 
*/               
                for (; p < pe; p++) {
 					if (k != offset) {
 						*p = p2;
 					}
 					if (++k > MOD_SZ-1) {
 						k = 0;
 					}
 				}

#if 0
				asm __volatile__ (
					"jmp L50\n\t" \
					".p2align 4,,7\n\t" \

					"L50:\n\t" \
					"cmpl %%ebx,%%ecx\n\t" \
					"je L52\n\t" \
					  "movl %%eax,(%%edi)\n\t" \
					"L52:\n\t" \
					"incl %%ebx\n\t" \
					"cmpl $19,%%ebx\n\t" \
					"jle L53\n\t" \
					  "xorl %%ebx,%%ebx\n\t" \
					"L53:\n\t" \
					"addl $4,%%edi\n\t" \
					"cmpl %%edx,%%edi\n\t" \
					"jb L50\n\t" \
					: "=D" (p), "=b" (k)
					: "D" (p), "d" (pe), "a" (p2),
						"b" (k), "c" (offset)
				);
#endif
//				do_tick();
				BAILR
			} while (!done);
		}
	}

	/* Now check every nth location */
	for (j=0; j<segs; j++) {
		start = start_p;
		end = start + cnt;
		pe = start;
		p = start+offset;
		done = 0;
		do {
			/* check for overflow  */
            if (pe + SPINSZ > pe) {
				pe += SPINSZ;
			} else {
				pe = end;
			}
			if (pe >= end) {
				pe = end;
				done++;
			}
			if (p == pe ) {
				break;
			}
/* Original C code replaced with hand tuned assembly code
 */
            for (; p < pe; p += MOD_SZ) {
 				if (no_comp != HAS_COMPARISION) {
                }
                else {
                if ((bad=*p) != p1) {
 					lprintf(LOG_ERR, "Address %p, Good %lx, Bad %lx", (ulong*)p, p1, bad);
                    ret = ERR_FAIL;
                    return ret;
 				}
                }
 			}
 
#if 0
			asm __volatile__ (
				"jmp L70\n\t" \
				".p2align 4,,7\n\t" \

				"L70:\n\t" \
				"movl (%%edi),%%ecx\n\t" \
				"cmpl %%eax,%%ecx\n\t" \
				"jne L71\n\t" \
				"L72:\n\t" \
				"addl $80,%%edi\n\t" \
				"cmpl %%edx,%%edi\n\t" \
				"jb L70\n\t" \
				"jmp L73\n\t" \

				"L71:\n\t" \
				"pushl %%edx\n\t"
				"pushl %%ecx\n\t"
				"pushl %%eax\n\t"
				"pushl %%edi\n\t"
				"call error\n\t"
				"popl %%edi\n\t"
				"popl %%eax\n\t"
				"popl %%ecx\n\t"
				"popl %%edx\n\t"
				"jmp L72\n"

				"L73:\n\t" \
				: "=D" (p)
				: "D" (p), "d" (pe), "a" (p1)
				: "ecx"
			);
#endif
//			do_tick();
			BAILR

		} while (!done);
	}
//        cprint(LINE_PAT, COL_PAT, "          ");
//
     return ret; 
}


/*
 * Test memory for bit fade.
 */
#define STIME 10  //5400
unsigned int bit_fade(ULV *start_p, UL cnt, int no_comp)
{
	UL j;
	ULV *pe;
	UL bad;
	ULV *start,*end;
    UL segs = 1;
    unsigned int ret = ERR_SUCCESS;
    UL p1 = 0;
    ULV *p;

//	test_ticks += (STIME * 2);
//	v->pass_ticks += (STIME * 2);

	/* Do -1 and 0 patterns */
	p1 = 0;
//	while (1) {

		/* Display the current pattern */
//		hprint(LINE_PAT, COL_PAT, p1);

		/* Initialize memory with the initial pattern.  */
		for (j=0; j<segs; j++) {
			start = start_p;
			end = start + cnt;
			pe = start;
			p = start;
			for (p=start; p<end; p++) {
				*p = p1;
			}
//			do_tick();
			BAILR
		}
		/* Snooze for 90 minutes */
//		sleep (STIME);

		/* Make sure that nothing changed while sleeping */
		for (j=0; j<segs; j++) {
			start = start_p;
			end = start + cnt;
			pe = start;
			p = start;
			for (p=start; p<end; p++) {
                if (no_comp != HAS_COMPARISION) {
                }
                else {
 				if ((bad=*p) != p1) {
					lprintf(LOG_ERR, "Address %p, Good %lx, Bad %lx", (ulong*)p, p1, bad);
                    ret = ERR_FAIL;
                    return ret;
				}
                }
			}
			BAILR
		}
		if (p1 == 0) {
			p1=-1;
		} 
    return ret;
}
/* 
 * March C fillup function
 * direction: 1, down to up; 0 up to down
 */
void march_c_fillup(ULV *start_p, UL cnt, UL pattern, unsigned int direction)
{
   ULV *p, end = start_p + cnt;
   if ( cnt == 0 )
      return;
	  
   if( direction == TEST_FROM_DWON_TO_UP) {
     p = start_p;
     for ( p = start_p; p < end; p ++) {
        *p = pattern;
     } 
   } else {
     p = start_p + cnt - 1;
     for ( ; p >= start_p; p --) {
        *p = pattern;
     } 
   }	 
}

/* 
 * March C check A and write B function
 * direction: 1, down to up; 0 up to down
 */
unsigned int march_c_Verify_A_Write_B(ULV *start_p, UL cnt, UL patternA, UL patternB, unsigned int direction, int bCompare )
{
   	ULV *p;
	UL bad;
	ULV *start,*end;
	unsigned int ret = ERR_SUCCESS;
    if ( cnt == 0 ) //should not have this problem
      return ERR_FAIL;
	  
    if( direction == TEST_FROM_DWON_TO_UP) {

		start = start_p;
		end = start + cnt;

		for (p=start; p<end; p++) {
			/*if (bCompare != HAS_COMPARISION) {
			}
			else*/ {
				if ((bad=*p) != patternA) {
					lprintf(LOG_ERR, "Address %p, Good %lx, Bad %lx", (ulong*)p, patternA, bad);
					ret = ERR_FAIL;
					return ret;
				}
			}
			*p = patternB;
		}
    } else {
		start = start_p;
		end = start + cnt;
        //start cannot be zero, otherwise it will be endless loop
		for (p=end-1; p>=start; p--) {
			/*if (bCompare != HAS_COMPARISION) {
			}
			else*/ {
				if ((bad=*p) != patternA) {
					lprintf(LOG_ERR, "Address %p, Good %lx, Bad %lx", (ulong*)p, patternA, bad);
					ret = ERR_FAIL;
					return ret;
				}
			}
		    *p = patternB;
		}
   }
   return ret;     
}
 
 
 /* 
 * March C Verify A function
 * direction: 1, down to up; 0 up to down
 */
unsigned int march_c_Verify_A(ULV *start_p, UL cnt, UL patternA, unsigned int direction, int bCompare )
{
   	ULV *p;
	UL bad;
	ULV *start,*end;
	unsigned int ret = ERR_SUCCESS;
    if ( cnt == 0 )
      return ERR_FAIL;
	  
    if( direction == TEST_FROM_DWON_TO_UP) {

		start = start_p;
		end = start + cnt;

		for (p=start; p<end; p++) {
			/*if (bCompare != HAS_COMPARISION) {
			}
			else*/ {
				if ((bad=*p) != patternA) {
					lprintf(LOG_ERR, "Address %p, Good %lx, Bad %lx", (ulong*)p, patternA, bad);
					ret = ERR_FAIL;
					return ret;
				}
			}
		}
    } else {
		start = start_p;
		end = start + cnt;
        //start cannot be zero, otherwise it will be endless loop
		for (p=end-1; p>=start; p--) {
			/*if (bCompare != HAS_COMPARISION) {
			}
			else*/ {
				if ((bad=*p) != patternA) {
					lprintf(LOG_ERR, "Address %p, Good %lx, Bad %lx", (ulong*)p, patternA, bad);
					ret = ERR_FAIL;
					return ret;
				}
			}
		}
   }
   return ret;   
}
/*
 * Test memory for march C.
 */
unsigned int march_c(ULV *start_p, UL cnt, int no_comp, unsigned long pattern0, int iter)
{
	UL j;
	ULV *pe;
	UL bad;
	ULV *start,*end;
    UL segs = 1;
    unsigned int ret = ERR_SUCCESS;
    UL p1 = pattern0;

//	test_ticks += (STIME * 2);
//	v->pass_ticks += (STIME * 2);

//	while (1) {

		/* Display the current pattern */
//		hprint(LINE_PAT, COL_PAT, p1);

		/* Initialize memory with the initial pattern.  */
	//W0
    march_c_fillup(start_p, cnt, p1, TEST_FROM_DWON_TO_UP);
	
	//DOWN TO UP
	//R0,W1
    for ( j = 0; j < iter; j ++ ) {
	ret=march_c_Verify_A_Write_B(start_p, cnt, p1, ~p1, TEST_FROM_DWON_TO_UP, HAS_COMPARISION );
        if( ret != ERR_SUCCESS )
           return ret;

	//R1,W0
	ret=march_c_Verify_A_Write_B(start_p, cnt, ~p1, p1, TEST_FROM_DWON_TO_UP, HAS_COMPARISION );
        if( ret != ERR_SUCCESS )
           return ret;

	//R0
	ret=march_c_Verify_A(start_p, cnt, p1, 1, HAS_COMPARISION );
        if( ret != ERR_SUCCESS )
           return ret;
	
	
        //UP TO DOWN
	//R0,W1
	ret=march_c_Verify_A_Write_B(start_p, cnt, p1, ~p1, TEST_FROM_UP_TO_DOWN, HAS_COMPARISION );
        if( ret != ERR_SUCCESS )
           return ret;

	//R1,W0
	ret=march_c_Verify_A_Write_B(start_p, cnt, ~p1, p1, TEST_FROM_UP_TO_DOWN, HAS_COMPARISION );
        if( ret != ERR_SUCCESS )
           return ret;

	//R0
	ret=march_c_Verify_A(start_p, cnt, p1, TEST_FROM_UP_TO_DOWN, HAS_COMPARISION );
    }
    return ret;
}


unsigned int mod20_verify(ULV *p_start, int cnt, UL p1)
{
    ULV *pb;
    ULV *pe;
    int step;
    unsigned int i;
    UL patc;
    unsigned int ret = ERR_SUCCESS;

    pe = p_start + cnt;
    patc = ~p1;

    for (step=0; (!ret) && step<cnt; step += JEDI_MODX) {
        pb = p_start + step;
        if (p1 == *pb) {
            pb ++;
            for (i=1; i<JEDI_MODX && pb<pe; i++) {
                if (patc != *pb) {
					error(pb, *pb, patc);
                    ret = ERR_FAIL;
                    break;
                }
                pb++;
            }
        } else {
			error(pb, *pb, patc);
            ret = ERR_FAIL;
            break;
        }
    }
    return ret;
}

void mod20_fillup(ULV *p_start, int cnt, UL p1)
{
    ULV *pb;
    ULV *pe;
    UL patc;
    int step;
    unsigned int i;

    pe = p_start + cnt;
    patc = ~p1;
    for (step=0; step<cnt; step += JEDI_MODX) {
        pb = p_start + step;
        *pb = p1;
        pb ++;
        for (i=1; i<JEDI_MODX && pb<pe; i++) {
            *pb = patc;
            pb ++;
        }
    }
}


unsigned int mod20run(ULV *p_start, int cnt, UL p1)
{
    unsigned int ret = ERR_SUCCESS;

    mod20_fillup(p_start, cnt, p1);
    if (mod20_verify(p_start, cnt, p1)) {
        ret = ERR_FAIL;
    }
    return ret;
}


/*!
 *
 */
unsigned int test_addr_walk_ones(ULV *p_start, UL block_bytes, int no_comp)
{
//    int block_num = 0;
//    unsigned int i = 0;
    unsigned int ret = ERR_SUCCESS;

    /* every block is BLOCK_SIZE * sizeof(UL) bytes */ 
//    block_num = (block_bytes/sizeof(UL))/(BLOCK_SIZE);
//
#if 0
    for (i=0; i<block_num; i++) {
       ret = addr_walk_ones(p_start+(i*BLOCK_SIZE), BLOCK_SIZE);
        if (ret != ERR_SUCCESS) {
            ret = ERR_ADDR_WALK_ONES;
            return ret;
       }
    }
#endif
    ret = addr_walk_ones(p_start, block_bytes/(sizeof(UL)), no_comp);

    if (ret != ERR_SUCCESS) {
        ret = ERR_ADDR_WALK_ONES;
        return ret;
    }

    return ret;
}

/*!
 *
 */
unsigned int test_addr_walk_own(ULV *p_start, UL block_bytes, int no_comp)
{
    unsigned int ret = ERR_SUCCESS;
	
	ret = addr_walk_own(p_start, (block_bytes)/sizeof(UL), no_comp);

	if (ret != ERR_SUCCESS) {
		ret = ERR_ADDR_WALK_OWN;
		return ret;
	}

    return ret;
}

#if 0
/*!
 *
 */
unsigned int test_random_address(ULV *p_start, UL block_bytes)
{
    int block_num = 0;
    int i = 0;
    unsigned int ret = ERR_SUCCESS;

    /* every block is BLOCK_SIZE * sizeof(UL) Bytes */ 
    block_num = (block_bytes/sizeof(UL))/(BLOCK_SIZE);
   
    for (i=0; i<block_num; i++) {
        ret = random_address(p_start+(i*BLOCK_SIZE), BLOCK_SIZE);
        if (ret != ERR_SUCCESS) {
            ret = ERR_RANDOM_ADDRESS;
            return ret;
        }
    }
    return ret;
}
#endif

/*!
 *
 */
unsigned int test_movinv_10(ULV *p_start, UL block_bytes, int iter, int no_comp)
{
    unsigned int ret = ERR_SUCCESS;
    UL p1 = 0;
    UL p2 = 0;

        p1 = 0;
        p2 = ~p1;
        ret = movinv1(p_start, block_bytes/sizeof(UL), iter, p1, p2, no_comp);
        if (ret != ERR_SUCCESS) {
            ret = ERR_MOVINV_10;
            return ret;
        }

        /* Switch patterns */
        p2 = p1;
        p1 = ~p2;
        ret = movinv1(p_start, block_bytes/sizeof(UL), iter, p1, p2, no_comp);
        if (ret != ERR_SUCCESS) {
            ret = ERR_MOVINV_10;
            lprintf(LOG_ERR, "Functin %s, Line %d is failure", __FUNCTION__, __LINE__);
            return ret;
        }
    return ret;

}

/*!
 *
 */
unsigned int test_movinv_8bit(ULV *p_start, UL block_bytes, int iter, int no_comp)
{
//    unsigned int block_num = 0;
//    int i = 0;
    int j = 0;
    unsigned int ret = ERR_SUCCESS;
    UL p0 = 0;
    UL p1 = 0;
    UL p2 = 0;

    /* every block is BLOCK_SIZE * sizeof(UL) */ 
//    block_num = (block_bytes/sizeof(UL))/(BLOCK_SIZE);
   
  //  D_printf("The iter is %d, block_num is %d\n", iter);
//    for (i=0; i<block_num; i++) {
        p0 = 0x80;
        for (j=0; j<8; j++, p0=p0>>1) {
#if __WORDSIZE == 32
            p1 = p0 | (p0<<8) | (p0<<16) | (p0<<24);
#elif __WORDSIZE == 64
            p1 = p0 | (p0<<8) | (p0<<16) | (p0<<24) | (p0<<32)| (p0<<40) | (p0<<48) | (p0 <<56);
#endif
            p2 = ~p1;
            ret = movinv1(p_start, block_bytes/sizeof(UL), iter, p1, p2, no_comp);
           if (ret != ERR_SUCCESS) {
            ret = ERR_MOVINV_8BIT;
            lprintf(LOG_ERR, "Functin %s, Line %d is failure", __FUNCTION__, __LINE__);
            return ret;
            }   

            /* Switch patterns */
            p2 = p1;
            p1 = ~p2;
            ret = movinv1(p_start, block_bytes/sizeof(UL), iter, p1, p2, no_comp);
        	if (ret != ERR_SUCCESS) {
            ret = ERR_MOVINV_8BIT;
            lprintf(LOG_ERR, "Functin %s, Line %d is failure", __FUNCTION__, __LINE__);
            return ret;
            }
        }
//    }
    return ret;

}

/*!
 *
 */
unsigned int test_movinv_random(ULV *p_start, UL block_bytes, int iter, int no_comp)
{
//    int block_num = 0;
//    unsigned int i = 0;
    int j = 0;
    unsigned int ret = ERR_SUCCESS;
    UL p1 = 0;
    UL p2 = 0;
    prandr_t r;

    /* every block is BLOCK_SIZE * sizeof(UL) */ 
//    block_num = (block_bytes/sizeof(UL))/(BLOCK_SIZE);
   
    rand_function_init(&r);
//    D_printf("The iter is %d, block_num is %d\n", iter);
//    for (i=0; i<block_num; i++) {
        for (j=0; j<iter; j++) {
            p1 = (UL) rand_function(&r);
            p2 = ~p1;
            ret = movinv1(p_start, block_bytes/sizeof(UL), 2, p1, p2, no_comp);
           	if (ret != ERR_SUCCESS) {
            ret = ERR_MOVINV_RANDOM;
            lprintf (LOG_ERR, "Functin %s, Line %d is failure", __FUNCTION__, __LINE__);
            return ret;
            }   
        }
//    }
    return ret;
}


/*!
 *
 */
unsigned int test_block_move(ULV *p_start, UL block_bytes, int iter, int no_comp)
{
//    int block_num = 0;
//    unsigned int i = 0;
    unsigned int ret = ERR_SUCCESS;

    /* every block is BLOCK_SIZE * sizeof(UL) */ 
//    block_num = (block_bytes/sizeof(UL))/(BLOCK_SIZE);
   
//    D_printf("The iter is %d, block_num is %d\n", iter, block_num);
//    for (i=0; i<block_num; i++) {
        ret = block_move(p_start, block_bytes/sizeof(UL), iter, no_comp);
   		if (ret != ERR_SUCCESS) {
            ret = ERR_BLOCK_MOVE;
            lprintf(LOG_ERR, "Functin %s, Line %d is failure", __FUNCTION__, __LINE__);
            return ret;
        }    
//    }
    return ret;
}

/*!
 *
 */
unsigned int test_movinv_long(ULV *p_start, UL block_bytes, int iter, int no_comp)
{
//    int block_num = 0;
//    unsigned int i = 0;
    int j = 0;
    UL p1 = 0;
    unsigned int ret = ERR_SUCCESS;

    /* every block is BLOCK_SIZE * sizeof(UL) */ 
//    block_num = (block_bytes/sizeof(UL))/(BLOCK_SIZE);
   
//   D_printf("The iter is %d, block_num is %d\n", iter, block_num);
//    for (i=0; i<block_num; i++) {
        for (j=0, p1=1; p1; p1=p1<<1, j++) {  //p1
//			printf("p1 is %lx\n", p1);
#if __WORDSIZE == 32
//            ret = movinv32(p_start+(i*BLOCK_SIZE), BLOCK_SIZE, iter, p1, 1, 0x80000000, 0, j);
            ret = movinv32(p_start, block_bytes/sizeof(UL), iter, p1, 1, 0x80000000, 0, j, no_comp);
#elif __WORDSIZE == 64
//            ret = movinv32(p_start+(i*BLOCK_SIZE), BLOCK_SIZE, iter, p1, 1, 0x8000000000000000, 0, j);
            ret = movinv32(p_start, block_bytes/sizeof(UL), iter, p1, 1, 0x8000000000000000, 0, j, no_comp);
#endif
           if (ret != ERR_SUCCESS) {
            ret = ERR_MOVINV_32BIT;
            lprintf(LOG_ERR, "Functin %s, Line %d, testing is failure", __FUNCTION__, __LINE__);
            return ret;
            } 
#if __WORDSIZE == 32
//            ret = movinv32(p_start+(i*BLOCK_SIZE), BLOCK_SIZE, iter, ~p1, 0xfffffffe, 0x7fffffff, 1, j);
            ret = movinv32(p_start, block_bytes/sizeof(UL), iter, ~p1, 0xfffffffe, 0x7fffffff, 1, j, no_comp);
#elif __WORDSIZE == 64
//            ret = movinv32(p_start+(i*BLOCK_SIZE), BLOCK_SIZE, iter, ~p1, 0xfffffffffffffffe, 0x7fffffffffffffff, 1, j);
            ret = movinv32(p_start, block_bytes/sizeof(UL), iter, ~p1, 0xfffffffffffffffe, 0x7fffffffffffffff, 1, j, no_comp);
#endif
    		if (ret != ERR_SUCCESS) {
             ret = ERR_MOVINV_32BIT;
            lprintf(LOG_ERR, "Functin %s, Line %d, testing is failure", __FUNCTION__, __LINE__);
            return ret;
            }  
        }   
//    }
    return ret;
}

/*!
 *
 */
unsigned int test_random_num_sequence(ULV *p_start, UL block_bytes, int no_comp)
{
//    int block_num = 0;
//    unsigned int i = 0;
    unsigned int ret = ERR_SUCCESS;

    /* every block is BLOCK_SIZE * sizeof(UL) */ 
//    block_num = (block_bytes/sizeof(UL))/(BLOCK_SIZE);
   
//    D_printf("Test random num_seq block_num is %d\n", block_num);
//    for (i=0; i<block_num; i++) {
            ret = movinvr(p_start, block_bytes/sizeof(UL), no_comp);
           	if (ret != ERR_SUCCESS) {
            ret = ERR_RANDOM_NUM_SEQ;
            lprintf(LOG_ERR, "Functin %s, Line %d, testing is failure", __FUNCTION__, __LINE__);
            return ret;
            }    
//    }
    return ret;
}

/*!
 *
 */
unsigned int cache_test_mem_modulo(ULV *p_start, UL block_bytes, int iter)
{
    int block_num = 0;
    int j = 0;
    UL p1 = 0;
    UL p2 = 0;
    unsigned int ret = ERR_SUCCESS;

    /* every block is BLOCK_SIZE * sizeof(UL) */ 
    block_num = (block_bytes/sizeof(UL))/(BLOCK_SIZE);
   
        for (j=0; j<iter; j++) {
            for (p1=0x01; 0!=p1; p1<<=1) {
                p2 = ~p1;
//                D_printf("Sliding pattern %lx and complement %lx\n", p1, p2);
                ret = mod20run(p_start, block_bytes/sizeof(UL), p1);
                if (ret != ERR_SUCCESS) {
                ret = ERR_MODULO;
                sprintf(error_info, "Functin %s, Line %d", __FUNCTION__, __LINE__);
                return ret;
                }
            }
                /* Switch patterns  */
            for (p1=0x01; 0!=p1; p1<<=1) {
                p2 = p1;
//                D_printf("Sliding pattern %lx and complement %lx\n", ~p1, p2);
                ret = mod20run(p_start, block_bytes/sizeof(UL), ~p1);

                if (ret != ERR_SUCCESS) {
                ret = ERR_MODULO;
                sprintf(error_info, "Functin %s, Line %d", __FUNCTION__, __LINE__);
                return ret;
                }     
            }
        }
    
    return ret;
}



/*!
 *
 */
unsigned int test_mem_modulo(ULV *p_start, UL block_bytes, int iter, int no_comp)
{
//    int block_num = 0;
//    unsigned int i = 0;
    int j = 0;
    int k = 0;
    UL p1 = 0;
    UL p2 = 0;
    unsigned int ret = ERR_SUCCESS;
    prandr_t r;

    /* every block is BLOCK_SIZE * sizeof(UL) */ 
//    block_num = (block_bytes/sizeof(UL))/(BLOCK_SIZE);
   
    rand_function_init(&r);
//    for (i=0; i<block_num; i++) {
        for (j=0; j<iter; j++) {
            p1 = (UL) rand_function(&r);
            for (k=0; k<MOD_SZ; k++) {
//				printf("K is %d, j is %d\n", k, j);
                p2 = ~p1;
//                ret = modtst(p_start+(i*BLOCK_SIZE), BLOCK_SIZE, 2, k, p1, p2);
                ret = modtst(p_start, block_bytes/sizeof(UL), 2, k, p1, p2, no_comp);
               if (ret != ERR_SUCCESS) {
                ret = ERR_MODULO;
                lprintf(LOG_ERR, "Functin %s, Line %d, testing is failure", __FUNCTION__, __LINE__);
                return ret;
                }    
                /* Switch patterns  */
                p2 = p1;
                p1 = ~p2;
//                ret = modtst(p_start+(i*BLOCK_SIZE), BLOCK_SIZE, 2, k, p1, p2);
                ret = modtst(p_start, block_bytes/sizeof(UL), 2, k, p1, p2, no_comp);

                if (ret != ERR_SUCCESS) {
                ret = ERR_MODULO;
                lprintf(LOG_ERR, "Functin %s, Line %d, testing is failure", __FUNCTION__, __LINE__);
                return ret;
                }     
            }
        }
//    }
    return ret;
}

/*!
 *
 */
unsigned int test_bit_fade(ULV *p_start, UL block_bytes, int no_comp)
{
//    int block_num = 0;
//    unsigned int i = 0;
    unsigned int ret = ERR_SUCCESS;

    /* every block is BLOCK_SIZE * sizeof(UL) Bytes */ 
//    block_num = (block_bytes/sizeof(UL))/(BLOCK_SIZE);
   
//    for (i=0; i<block_num; i++) {
//        ret = bit_fade(p_start+(i*BLOCK_SIZE), BLOCK_SIZE);
        ret = bit_fade(p_start, block_bytes/sizeof(UL), no_comp);
        if (ret != ERR_SUCCESS) {
            ret = ERR_BIT_FADE;
            return ret;
        }
//    }
    return ret;
}

/*!
 *
 */
unsigned int test_march_c(ULV *p_start, UL block_bytes,int iter, int no_comp) 
{
//    int block_num = 0;
//    unsigned int i = 0;
    unsigned int ret = ERR_SUCCESS;
	unsigned long pattern0=0x0;

    /* every block is BLOCK_SIZE * sizeof(UL) Bytes */ 
//    block_num = (block_bytes/sizeof(UL))/(BLOCK_SIZE);
   
//    for (i=0; i<block_num; i++) {
//        ret = bit_fade(p_start+(i*BLOCK_SIZE), BLOCK_SIZE);
        ret = march_c(p_start, block_bytes/sizeof(UL), no_comp, pattern0, iter);
        if (ret != ERR_SUCCESS) {
            ret = ERR_MARCH_C;
            return ret;
        }
//    }
    return ret;
}


