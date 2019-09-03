/*!
*\file cache_test.h
* 
* test function header file
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
* Dec. 29, 2009    Create this file
*/

#ifndef CACHE_TEST_H
#define CACHE_TEST_H


#define RAND_ARRAY_NUM      (1*128) //32
#define RAND_NUM_SIZE       (1*1024) //16
#define MEM_BANK_SIZE       (1*1024*1024)
#define RAND_PATTERN        "/tmp/rand_pattern"

unsigned int test_cache_walking(ULV *p_start, UL block_bytes);
unsigned int test_cache_stuck(ULV *p_start, UL block_bytes);

unsigned int test_cache_rdwr(ULV *p_start, UL block_bytes);

unsigned int test_cache_randdata(ULV *p_start, UL block_bytes);
unsigned int test_cache_randaddr(ULV *p_start, UL block_bytes, int module_id, int no_comp);

unsigned int test_cache_spill_tag(void);

void init_random();

unsigned int new_rand();


#endif /* TEST_H */

