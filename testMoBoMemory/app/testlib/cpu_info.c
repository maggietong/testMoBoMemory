/*!
*\file cpu_info.c
* 
* Get cpu information
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
* Nov. 15, 2009    Use CPUID instruction to detect cpu information
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#include "jedi_comm.h"
#include "memory_err.h"

#define VERSION "0.1.1"

/*!
 *\def CONSTANT
 *define INTEL_CPU as 0X01
 */ 
#define IS_INTEL_CPU	0x01

/*!
 *\def CONSTANT
 *define IS_UNKNOW_CPU as 0X00
 */ 
#define IS_UNKNOW_CPU	0x00

/*!
 *  The function check the processor whether supports CPUID instruction
 *
 *\param	input  - None
 *\param	output - None
 *\return   	0  - support CPUID instruction
 *          	-1 - not support
 */
int cpu_support_CPUID(void)
{
    int original_val = 0;
    int change_val = 0;

#if __WORDSIZE == 64
	__asm__ __volatile(
                      "pushF\n\t"
                      "popq %%rax\n\t"
                      "movl %%eax, %%ecx\n\t"
                      "xor $0x200000, %%eax\n\t"
                      "pushq %%rax\n\t"
                      "popF\n\t"
                      "pushF\n\t"
                      "popq %%rax\n\t"
                      :"=a"(change_val), "=c"(original_val)
                      :
        );

#else
	__asm__ __volatile(
                      "pushF\n\t"
                      "popl %%eax\n\t"
                      "movl %%eax, %%ecx\n\t"
                      "xor $0x200000, %%eax\n\t"
                      "pushl %%eax\n\t"
                      "popF\n\t"
                      "pushF\n\t"
                      "popl %%eax\n\t"
                      :"=a"(change_val), "=c"(original_val)
                      :
        );

#endif
    if(change_val == original_val)
        return -1;        
    else
        return 0;
}

/*
 * get the processor manufacture
 *
 *\param  	input  - None 
 *\param	Output - None 
 *\return       1      - Genuine Intel Processor;
                0      - others
 */
unsigned int get_cpu_manufacture(void)
{
    unsigned int venid_b = 0;
    unsigned int venid_d = 0;
    unsigned int venid_c = 0;

#if __WORDSIZE == 64

            __asm__ __volatile__(
                         /* call cpuid with eax = 0  */
			  "xorl %%eax, %%eax\n\t"
                         "cpuid\n\t"

                        : "=b" (venid_b), "=d" (venid_d), "=c" (venid_c)
                        :
                        : "%eax"
            );
#else
            __asm__ __volatile__(
                         /* call cpuid with eax = 0  */
                        "pushl %%ebx \n\t"
			            "xorl %%eax, %%eax\n\t"
                        "cpuid     \n\t"
			            "popl  %%ebx \n\t"	
			
                        : "=d" (venid_d), "=c" (venid_c)
            		    :            
                        : "%eax"
            );


	 __asm__ __volatile__(
                         /* call cpuid with eax = 0 */
                        "pushl %%ebx \n\t"
                        "xorl %%eax, %%eax\n\t"
                        "cpuid\n\t"
                      
                        "movl %%ebx, %%ecx   \n\t"
                        "popl  %%ebx \n\t"
                        
                        : "=c" (venid_b)
                        :
                        : "%eax"
	);

#endif

    /* ebx--uneG, edx--Ieni, ecx--letn.(GenuineIntel) */
    if((venid_b == 0x756e6547) && (venid_d == 0x49656e69) && (venid_c == 0x6c65746e)){
        return IS_INTEL_CPU;        
    }
    else
        return IS_UNKNOW_CPU;        
}

/*!
 * The function get the CPU main frequency
 *
 *\param	input  - None 
 *\param	output - mainfeq
 *\return	0      - success
 * 		    errorcode - fail
 */     
unsigned int cpu_main_frequency(unsigned long main_freq)
{
    static unsigned long frequency=0;
    unsigned long st_low = 0;
    unsigned long st_high = 0;
    unsigned long  end_low = 0;
    unsigned long  end_high = 0;
    int loops = 0;
    
    if(cpu_support_CPUID() == -1){
	    return ERR_CPU_NOT_SUPPORT_CPUID;
    }
	
    for(loops=0; loops<50; loops++){    
        struct timeval tv1,tv2;
	struct timezone tz1,tz2;
        gettimeofday (&tv1 , &tz1);
	__asm__ __volatile__(
    				"rdtsc		\n\t"
	    			:"=a" (st_low),"=d" (st_high)
	    			:
	);
        usleep(100);
	__asm__ __volatile__(
    				"rdtsc\n\t" 
				:"=a" (end_low), "=d" (end_high)
				:
	);
	gettimeofday(&tv2,&tz2);
	end_low-=st_low;
	tv2.tv_usec-=tv1.tv_usec;
	end_low/=tv2.tv_usec;
        frequency += end_low;
    }
    frequency=frequency/50;
    main_freq = frequency;
    D_printf("the main frequency is %u Mhz\n", (int)frequency);
    return ERR_SUCCESS;
}


