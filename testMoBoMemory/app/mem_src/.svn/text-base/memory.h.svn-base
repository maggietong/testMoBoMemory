/*!
*\file getmemecc.h
* 
* Get mother board memory ECC count
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
* Nov. 23, 2009    Create this file
*/

#ifndef MEMORY_H
#define MEMORY_H
#define SOCKET_COUNT        1 /* the socket count per cpu node  */ 
//#define NODE_COUNT          2 /* the node count of the computer system  */
#define NODE_COUNT          1 /* the node count of the computer system  */

#define ITEM_COUNT          11 /* the test item count */
#define CORE_COUNT          32 /* CPU CORE COUNT of the computer system */
#define MAX_REGION          20

#define HALF_CORE_COUNT     (CORE_COUNT/2) /* CPU CORE COUNT of the computer system */
#define RANGE_COUNT         12 /* Define the maxmium range count  */

#define MCE_EVENT   1
#define LIST_INFO	1
#define CLEAR_ECC	1

#define HAS_COMPARISION  1

#define EXIT_ON_ERROR	1
#define NO_EXIT_ERROR	0

#define EXIT_FLAG		1

/* Define the test mode */
#define LOCAL       0
#define REMOTE      1
#define SPECIFIC    2
#define ALL         3

#define READ_ONLY   0
#define READ_WRITE  1

#define PAGE_SHIFT 12
#define PAGE_SIZE (1UL << PAGE_SHIFT)

//#define PAGE_SIZE   (4096)
//#define 8M_SHIFT    23
//#define MIN_BLOCK_SIZE  (1UL << 20) // 1M
#define MIN_BLOCK_SIZE  (1UL << 12) // 1M

#ifndef __ASSEMBLY__

#define MEM_ADD_WALK_ONES        0
#define MEM_ADD_OWN_ADDRESS      1
#define MEM_RANDOM_ADDRESS       2
#define MEM_MOVE_INVER_ONES_ZERO 3
#define MEM_MOVE_INVER_8BIT      4
#define MEM_MOVE_INVER_RANDOM    5
#define MEM_BLOCK_MOVE           6
#define MEM_MOVE_INVER_32BIT     7
#define MEM_RANDOM_NUM_SEQUENCE  8
#define MEM_MODULO               9
#define MEM_MARCH_C_TEST         10
#define MEM_BIT_FADE_TEST        11

#define CPU_CACHE_WALKING        1          
#define CPU_CACHE_STUCK          2       
#define CPU_CACHE_RDWR           3              
#define CPU_CACHE_RANDOM_DATA    4
#define CPU_CACHE_RANDOM_ADDR    5
#define CPU_CACHE_SPILL          6
#define CPU_CACHE_TAG            7

#define CACHE_MEM_MODULO         1

#define NO_SPECIFY_CPU          -1 /* Use "^" ASCII to show that users don't specify the cpu number */  
#define NO_SPECIFY_ITEM         -1 /* same as above*/
#define NO_SPECIFY_ADDRESS      0x55AE /* same as above */

#define NO_SPECIFY_BLOCK_SIZE   -1

/*
 * test parameters
 */
typedef struct _test_parm_ {
    int cpu;
    int test_item[ITEM_COUNT][2];
    int block_size;
    UL start;
    UL end;
    int verbose;
    int exit_error;
    int mode;
    int list_info;
	int clear_ecc;
    int no_comp;
}test_parm;

typedef struct _mem_info_{
    unsigned long total;  /*M byte*/
    unsigned long free;
}mem_info;

typedef struct _thread_info_struc_{
    unsigned short cpuid;
    int item_id;
    int module_id;
    ULV *block ;
    UL block_bytes;
    int no_comp;
}thread_info_struc;

struct tseq {
    short cache;
    short pat;
    short iter;
    short errors;
    char *msg;
};

/*typedef struct _region_t_{
    int index;
    unsigned long start_pfn;
    unsigned long end_pfn;
    int node;
    int type;
}region_t;*/

unsigned int init_module(int module_id, void *param, int length);
unsigned int module_test(int module_id, void *param, int length);
unsigned int close_module(int module_id, void *param, int length);

#endif  
#endif /* MEMORY_H */
