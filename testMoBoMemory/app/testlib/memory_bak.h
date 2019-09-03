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

#define CPU_CACHE           1
#define MEMORY_MODULE       2
#define CACHE_MEMORY        3

#define MAX_ITEM_NUM        12
#define MAX_CORE_NUM        12

//#define RAND_ARRAY_NUM      8*1024 //32
//#define RAND_NUM_SIZE       1*1024 //16
//#define MEM_BANK_SIZE       64*1024*1024
//#define RAND_PATTERN        "/tmp/rand_pattern"

#ifndef __ASSEMBLY__

#define MEM_ADD_WALK_ONES        1
#define MEM_ADD_OWN_ADDRESS      2
#define MEM_RANDOM_ADDRESS       3
#define MEM_MOVE_INVER_ONES_ZERO 4
#define MEM_MOVE_INVER_8BIT      5
#define MEM_MOVE_INVER_RANDOM    6
#define MEM_BLOCK_MOVE           7
#define MEM_MOVE_INVER_32BIT     8
#define MEM_RANDOM_NUM_SEQUENCE  9
#define MEM_MODULO               10
#define MEM_BIT_FADE_TEST        11

#define CPU_CACHE_WALKING        1          
#define CPU_CACHE_STUCK          2       
#define CPU_CACHE_RDWR           3              
#define CPU_CACHE_RANDOM_DATA    4
#define CPU_CACHE_RANDOM_ADDR    5
#define CPU_CACHE_SPILL          6
#define CPU_CACHE_TAG            7

#define CACHE_MEM_MODULO         1

#define NO_SPECIFY_CPU          100
#define NO_SPECIFY_ITEM         100
#define NO_SPECIFY_BLOCK_SIZE   8

/*
 * test parameters
 */
typedef struct _test_parm_ {
    int cpu;
    int test_item;
    int block_size;
}test_parm;

typedef struct _mem_info_{
    unsigned long total;  /*M byte*/
    unsigned long free;
}mem_info;

typedef struct _thread_info_struc_{
    unsigned short cpu;
    int item_id;
    int module_id;
    ULV *block ;
    UL block_bytes;
}thread_info_struc;

struct tseq {
    short cache;
    short pat;
    short iter;
    short errors;
    char *msg;
};


unsigned int init_module(int module_id, void *param, int length);
unsigned int module_test(int module_id, int item_id, void *param, int length);
unsigned int close_module(int module_id, void *param, int length);

#endif  
#endif /* MEMORY_H */
