/*!
*\file mem_controller.c
* 
*\brief  Memory test controller is designed to conduct test procedure according to CPU and memory configuration.
*
*\author Maggie Tong
*\author Jackie Liu
*
* Copyright(c) 2011 Jabil Circuit.
*
* This source code and any compilation or derivative thereof is the sole property of 
* Jabil Circuit and is provided pursuant to a Software License Agreement. This code 
* is the proprietary information of Jabil Circuit and is confidential in nature. 
* It's use and dissemination by any party other than Jabil Circuit is strictly 
* limited by the confidential information provisions of Software License Agreement 
* referenced above.
*
*\par ChangeLog:
* Feb 11, 2011 :   Create this file
*/

/*
 * INCLUDE FILES
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
//#include <linux/autoconf.h>

#include "jedi_comm.h"
#include "log.h"
#include "memory.h"
#include "list.h"
#include "mem_err.h"
#include "task_generator.h"
#include "mem_controller.h"
#include "mem_test.h"
#include "cache_test.h"
#include "sys_i.h"
#include "mce_extern.h"

typedef void (*sighandler_t)(int);

#ifndef CONFIG_STRICT_DEVMEM
   #define CONFIG_STRICT_DEVMEM 0
#endif

const struct tseq tseq[ITEM_COUNT+3] = {
	{0,  5,   4, 0, "[Address test, walking ones, no cache]"},
	{1,  6,   4, 0, "[Address test, own address]           "},
        {1,  6,   4, 0, "[Random Address test]                 "},
	{1,  0,   4, 0, "[Moving inversions, ones & zeros]     "},
	{1,  1,   2, 0, "[Moving inversions, 8 bit pattern]    "},
	{1, 10,  50, 0, "[Moving inversions, random pattern]   "},
	{1,  7,  80, 0, "[Block move]                          "},
	{1,  2,   2, 0, "[Moving inversions, long pattern]     "},
	{1,  9,  30, 0, "[Random number sequence]              "},
        {1, 11,   6, 0, "[Modulo 20, ones&zeros pattern]       "},
	{1,  8,   2, 0, "[March C test]"},
	{1,  8,   1, 0, "[Bit fade test, 90 min, 2 patterns]   "},
	{0,  0,   0, 0, NULL}
};

/* 
 *Random array is RAND_ARRAY_NUM * RAND_NUM_SIZE * sizeof(unsigned int)
 * If RAND_ARRAY_NUM = 1 * 1024, RAND_NUM_SIZE = 1 * 1024, sizeof(unsigned int) = 8
 * The pattern size is 1 * 1024 * 1 * 1024 = 8M positions
 * The memory size for the pattern is 1M * 8 = 8M
 * The random test bank = 8M*BYTE =8M/sizeof(UL) = 1M count position
 * */
/*
 * For algorithm test, the minimum block size is 128K because the memory bank size is 128K
 * The  test range should be more than 8M bytes 
 *
 */ 
unsigned int rand_array[RAND_ARRAY_NUM][RAND_NUM_SIZE];
static int exit_on_error = 0;
static int termination_flag = 0;
static int mce_flag = 0;
time_t kill_time;
unsigned int ret_flag = ERR_SUCCESS;

static int record_errors = 0;
static int msr_errors = 0;
extern test_config_struc test_config;
extern int mmtest_fd;
extern int RD_empty;
extern int RW_empty;

extern void release_munmap_close(void);
/*!
 * \brief   initilize module
 * \param   module_id   the id of test module
 * \param   param       the head pointer of parameters
 * \param   length      the length of parameters
 * \return  0           success
 *          nonzero     return the error code
 */
extern unsigned int init_module(int module_id, void *param, int length);

void release_list_resource(void);

int glob_test = 6;

static pid_t pida = 0;
static pid_t pidb = 0;

void sig_ctrlC_exit(int signo)
{
        //lprintf(LOG_INFO, "Received Ctrl-C signal\n");
        //printf("Received Ctrl-C signal, quit! PID=%d\n", getpid());
        
        if (pida != 0) {
             if (kill(pida, SIGABRT) != 0)
                 lprintf(LOG_ERROR, "Kill MCE check child failed\n");
                 //return;
        }

        if( pidb != 0 ) {
	    if (kill(pidb, SIGABRT) != 0)
                 lprintf(LOG_ERROR, "Kill MCE check child failed\n");
                //return;
        }
        
        release_munmap_close();
        release_list_resource();
        exit(EINTR);       
}


extern int do_mce_check(int flag, int exitonerror, int *msr_errors, int *record_errors);

/* Signal handler for MCE_EVENT  */
static void sig_mce_exit(int);

static void sig_mce_exit(int signo)
{
    if (signo == SIGUSR1) {
//        lprintf(LOG_INFO, "Received MCE_EVENT signal\n");
        mce_flag = 1;
        
        if (pidb != 0) {
        if (kill(pidb, SIGUSR2) != 0)
                return;
        }

        if (exit_on_error == EXIT_ON_ERROR) {
           termination_flag = 1;
        }
    }
    else {
    }
    return;
}

/*!
 * \brief   release test_config RW_lists, RD_lists and slice_lists lists resource
 */
void release_list_resource(void)
{
    int i = 0;
    int j = 0;
    struct list_head *pos1;
    struct list_head *pos2;
    struct list_head *node1;
    struct list_head *node2;
    RW_range *tmp_RW;
    RD_range *tmp_RD;
    test_slice *tmp_slice;

    for (i=0; i<NODE_COUNT; i++) {
        for (j=0; j<SOCKET_COUNT; j++) {
           //avoid the problem if this is called from Ctrl-C handling while RW_list is not setup yet, also it has problem if the system only has one Node, will be fixed
            if(test_config.node[i].socket[j].RW_list.next != NULL ) {
                list_for_each_safe(pos1, node1, &(test_config.node[i].socket[j].RW_list)) {
                    tmp_RW = list_entry(pos1, RW_range, range_list);
                    list_for_each_safe(pos2, node2, &(tmp_RW->RW_slice)) {
                        tmp_slice = list_entry(pos2, test_slice, slice_list);
                        list_del(&(tmp_slice->slice_list));
                        free(tmp_slice);
                    }
                    //printf("Jackie here\n"); 
                    list_del(&(tmp_RW->range_list));
                    free(tmp_RW);
                }
                       
            }
           
            list_for_each_safe(pos1, node1, &(test_config.node[i].socket[j].RD_list)) {
                tmp_RD = list_entry(pos1, RD_range, range_list);
                list_del(&(tmp_RD->range_list));
                free(tmp_RD);
            }
        }
    }
}

sighandler_t signal_a(int signo, sighandler_t func)
{
    struct sigaction act;
    struct sigaction oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;
#endif
    }
    else {
#ifdef SA_RESTART
        act.sa_flags |= SA_RESTART;
#endif
    }
    if (sigaction(signo, &act, &oact) <0 )
        return(SIG_ERR);
    return(oact.sa_handler);
}

int del_signal(int sig)
{
   sigset_t mask;

   sigemptyset(&mask);
   
   if (sigprocmask(SIG_BLOCK, NULL, &mask) < 0)
       return -1;

   sigdelset(&mask, sig);

   if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0)
       return -1;

   return ERR_SUCCESS;

}


/*!
 *  Mask signal to this process
 *
 *
*/ 
int mask_signal(int sig)
{
   sigset_t mask;

   sigemptyset(&mask);
   
   if (sigprocmask(SIG_BLOCK, NULL, &mask) < 0)
       return -1;

   sigaddset(&mask, sig);

   if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0)
       return -1;

   return ERR_SUCCESS;

}


/*!
 * Set thread signal
 *
 */
void thread_signal_set(void)
{
    sigset_t new_set;
    sigset_t old_set;

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);

    sigemptyset(&new_set);
    sigemptyset(&old_set);
    sigaddset(&new_set, SIGHUP);
//    sigaddset(&new_set, SIGINT);
    sigaddset(&new_set, SIGQUIT);
    sigaddset(&new_set, SIGTERM);
    sigaddset(&new_set, SIGUSR1);
//    sigaddset(&new_set, SIGUSR2);
    sigaddset(&new_set, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &new_set, &old_set);
}

/*!
 *The entry of test every test items function
 *\param thread_info -  the struct of thread information
 */
void *run_test_slice(thread_info_struc *thread)
{
    unsigned int ret = ERR_SUCCESS;
    pthread_t tid = 0;
    UL c_iter = 0;

    tid = pthread_self();

    ret = isys_cpu_bind(thread->cpuid);
    if (ret != ERR_SUCCESS)
    {
        lperror(LOG_ERR, "Failure to bind the cpu-core");
        return (void *)ERR_BIND_CPU;
    }

    thread_signal_set();
    c_iter = tseq[thread->item_id].iter;
    
    if (thread->module_id == MEMORY_MODULE) {
        switch (thread->item_id) {
            case MEM_ADD_WALK_ONES:         /*test #0  */
                ret = test_addr_walk_ones(thread->block, thread->block_bytes, thread->no_comp);
                break;
            case MEM_ADD_OWN_ADDRESS:       /*test #1 */
                ret = test_addr_walk_own(thread->block, thread->block_bytes, thread->no_comp);
                break;
            case MEM_RANDOM_ADDRESS:        /*test #2 */
                ret = test_cache_randaddr(thread->block, thread->block_bytes, thread->module_id, thread->no_comp);
                break;
            case MEM_MOVE_INVER_ONES_ZERO:  /*test #3 */
                ret = test_movinv_10(thread->block, thread->block_bytes, c_iter, thread->no_comp);
                break;
            case MEM_MOVE_INVER_8BIT:       /*test #4 */
                ret = test_movinv_8bit(thread->block, thread->block_bytes, c_iter, thread->no_comp);
                break;
            case MEM_MOVE_INVER_RANDOM:     /*test #5 */
                ret = test_movinv_random(thread->block, thread->block_bytes, c_iter, thread->no_comp);
                break;
            case MEM_BLOCK_MOVE:            /*test #6  */
                ret = test_block_move(thread->block, thread->block_bytes, c_iter, thread->no_comp);
                break;
            case MEM_MOVE_INVER_32BIT:      /*test #7  */
                ret = test_movinv_long(thread->block, thread->block_bytes, c_iter, thread->no_comp);
                break;
            case MEM_RANDOM_NUM_SEQUENCE:   /*test #8  */
                ret = test_random_num_sequence(thread->block, thread->block_bytes, thread->no_comp);
                break;
            case MEM_MODULO:                /*test #9  */
                ret = test_mem_modulo(thread->block, thread->block_bytes, c_iter, thread->no_comp);
                break;
            case MEM_MARCH_C_TEST:          /*test #10  */
 		//printf("running march test on CPU %d, start %lx, size %lx\n", thread->cpuid, thread->block, thread->block_bytes);
                ret = test_march_c(thread->block, thread->block_bytes, c_iter, thread->no_comp);
		break;
            case MEM_BIT_FADE_TEST:        /*test #11  */
                ret = test_bit_fade(thread->block, thread->block_bytes, thread->no_comp);
                break;
            default:
                ret = ERR_TEST_ITEM;
                break;
        }
    }
#if 0
    else if (thread->module_id == CPU_CACHE) {
        switch (thread->item_id) {
            case CPU_CACHE_WALKING:          /*test #0  */
                ret = test_cache_walking(thread->block, thread->block_bytes);
                break;
            case CPU_CACHE_STUCK:            /*test #1 */
                ret = test_cache_stuck(thread->block, thread->block_bytes);
                break;
            case CPU_CACHE_RDWR:             /*test #2 */
                ret = test_cache_rdwr(thread->block, thread->block_bytes);
                break;
            case CPU_CACHE_RANDOM_DATA:      /*test #3 */
                ret = test_cache_randdata(thread->block, thread->block_bytes);
                break;
            case CPU_CACHE_RANDOM_ADDR:      /*test #4 */
                ret = test_cache_randaddr(thread->block, thread->block_bytes, thread->module_id);
                break;
#if 0
            case CPU_CACHE_SPILL:            /*test #5 */
                if (NULL != thread->block) {
                    munlock((void *)(thread->block), thread->block_bytes);
                    free((void *)thread->block);
                    thread->block = NULL;
                }
                ret = test_cache_spill_tag();
                break;
#endif
            case CPU_CACHE_SPILL:
            case CPU_CACHE_TAG:
                if (NULL != thread->block) {
                    munlock((void *)(thread->block), thread->block_bytes);
                    free((void *)thread->block);
                    thread->block = NULL;
                } 
                ret = test_cache_spill_tag();
               break;

            default:
                ret = ERR_TEST_ITEM;
                break;
        }
    }
    else if (thread->module_id == CACHE_MEMORY) {
        switch (thread->item_id) {
            case CACHE_MEM_MODULO:
                c_iter = 2;
                ret = cache_test_mem_modulo(thread->block, thread->block_bytes, c_iter);
                break;
            default:
                ret = ERR_TEST_ITEM;
                break;
        }
    }

#endif
    return (void *)ret;
}

unsigned int read_only_test(test_parm *para)
{
    //int mem_fd;
    int i = 0;
    int j = 0;
    struct list_head *list;
    RD_range *tmp_RD;
    //off_t p = 0;
    //int length = 0;
    //char rdbuf[PAGE_SIZE] = {};
    unsigned int ret = ERR_SUCCESS;

    /*mem_fd = open(MEM_DEV_FILENAME, O_RDONLY);
    if (mem_fd < 0) {
        lperror(LOG_ERR, "Openning /dev/mem failed");
        return errno;
    }*/

    if( para->mode == SPECIFIC ) {
        ret = isys_cpu_bind(para->cpu);
        if (ret != ERR_SUCCESS) {
            lperror(LOG_ERR, "Failure to bind the cpu-core for read-only test");
            return ERR_BIND_CPU;
        }
    }

    for (i=0; i<NODE_COUNT; i++) {
        for (j=0; j<SOCKET_COUNT; j++) {
            if (list_empty(&(test_config.node[i].socket[j].RD_list))) {
                lprintf(LOG_INFO,"The node %d, socket %d, RD_range to be tested is empty", i, j);
            }
            else {
                list_for_each(list, &(test_config.node[i].socket[j].RD_list)) {
                UL addr, size=0, data;
                ULV *pAddr;
                tmp_RD = list_entry(list, RD_range, range_list);
	        
                addr = (UL)mmap(NULL, (tmp_RD->end-tmp_RD->start), PROT_READ, MAP_SHARED, mmtest_fd, tmp_RD->start);
                
               if (addr == -1) {
                 lperror(LOG_ERR, "Fail to mmap read only test memory");
                 close(mmtest_fd);
                 return errno;
               }
               pAddr = (ULV*)addr;
               do {
                 data = *(pAddr++);
                 size += sizeof(UL);
               } while( size < tmp_RD->end - tmp_RD->start );
               munmap( (void *)addr, tmp_RD->end - tmp_RD->start );
		/*if (lseek(mem_fd,  p, SEEK_SET) == -1) {
		   lperror(LOG_ERR, "lseek system call is failure");
		   return errno;
		}
                for (; p<(tmp_RD->end-PAGE_SIZE); p+=PAGE_SIZE) {
		    length = read(mmtest_fd, (void *)rdbuf, PAGE_SIZE);
                    if (length < 0) {
                        lperror(LOG_ERR, "Read system call is failure");
                        close(mem_fd);
                        return errno;
                    } else if (length != PAGE_SIZE) {
                        lperror(LOG_ERR, "The read length %x is incorrect", length);
                        close(mem_fd);
                        return ERR_READ_ONLY;
                    }
                }*/
            }

            }
        }
    }
    //close(mem_fd);

    return ret;
}

int item_n = 0;

static void USR2_signal_exit(int sig)
{
//    lperror(LOG_ERR, "Running test item %d, MCE Event occurs", test_config.test_item[item_n]);
//    lperror(LOG_ERR, "Running test item %d, MCE Event occurs", test_config.test_item[item_n]);
    if (exit_on_error == EXIT_ON_ERROR)
        exit(sig);
    
    return;
//    abort();
}


/*!
 * \brief   Read/write test for introducing various algorithm test from memtest86 
 * \param   param   the head pointer of parameters
 * \param   length  the length of parameters
 * \return  0       success
 *          nonzero return the error code
 */
unsigned int read_write_test(int no_comp)
{
    int i = 0;
    int j = 0;
    int k = 0;
    int n = 0;
    int m = 0;
    int thread_num = 0;
    int max_range = 0;
	struct list_head *list;
    struct list_head *list1;
    RW_range *tmp_RW;
    test_slice *tmp_slice;
    pthread_t thread[CORE_COUNT];
    info_struc istr[NODE_COUNT];  
    thread_info_struc thread_stru[CORE_COUNT];
 
    RW_range_info   rinfo_tmp;
    UL block0 = 0;
    UL block1 = 0;
   
    FILE *fp;
    char buf[RAND_NUM_SIZE*8*2];
    char *temp_p;
    struct stat f_stat;
    unsigned int ret = ERR_SUCCESS;
    unsigned int ret_val[CORE_COUNT] = {0};

    memset((void *)thread, 0, sizeof(pthread_t) * CORE_COUNT);
    memset((void *)istr, 0, sizeof(info_struc) * NODE_COUNT);
    memset((void *)thread_stru, 0, sizeof(thread_info_struc) * CORE_COUNT);
    memset((void *)&f_stat, 0, sizeof(struct stat)); 

    for (i=0, k=0; i<NODE_COUNT; i++, k=0) {
        for (j=0; j<SOCKET_COUNT; j++) {
            list_for_each(list, &(test_config.node[i].socket[j].RW_list)) {
                tmp_RW = list_entry(list, RW_range, range_list);
                istr[i].rinfo[k].start = tmp_RW->start;
                istr[i].rinfo[k].end = tmp_RW->end;
                n = 0;
                list_for_each(list1, &(tmp_RW->RW_slice)) {
                    tmp_slice = list_entry(list1, test_slice, slice_list);
                    istr[i].rinfo[k].sinfo[n].start = tmp_slice->start;
                    istr[i].rinfo[k].sinfo[n].end = tmp_slice->end;
                    istr[i].rinfo[k].sinfo[n].cpuid = tmp_slice->cpuid;
                    n++;
                    //printf("Jackie: istr number %d, rangeinfo %d, sinfo %d, start %lx, end %lx, cpuid %d\n", i, k, n, tmp_slice->start, tmp_slice->end, tmp_slice->cpuid);
                }
                istr[i].rinfo[k].slice_num = n;
                k++;
            }
        }
        istr[i].range_num = k;
    }

    /* realease all lists resource  */
    release_list_resource();

    for (n=0; test_config.test_item[n] != -1; n++) {
        
        if (test_config.test_item[n] == MEM_RANDOM_ADDRESS) {
            memset((void *)rand_array, 0, (sizeof(unsigned int))*RAND_ARRAY_NUM*RAND_NUM_SIZE);
            /* Generate the random array for random address testing */

            if (access(RAND_PATTERN, F_OK) == 0) {
                if (stat(RAND_PATTERN, &f_stat)== -1) {
                    lperror(LOG_ERR, "Fail to stat the random pattern file.");
                    return errno;
                }
                lprintf(LOG_DEBUG, "The RAND_PATTERN file size is %lx", f_stat.st_size);
                if ((long)f_stat.st_size < (sizeof(unsigned int) * RAND_ARRAY_NUM * RAND_NUM_SIZE)) {
                    lperror(LOG_ERR, "The RAND_PATTERN file is wrong, please delete it");
                    return  ERR_READ_RAND_PATTERN; 
                }
                fp = fopen(RAND_PATTERN, "r");
                if (fp == NULL) {
                    lperror(LOG_ERR, "Fail to open the random pattern file.");
                    return errno;
                }      
                for (i=0; i<RAND_ARRAY_NUM; i++) {
                    memset((void *)buf, 0 , sizeof(char) * RAND_NUM_SIZE*8*2);
                    if (NULL != fgets(buf, sizeof(buf), fp)) {
                        temp_p = strtok(buf, ",");
                        for (j=0; j<RAND_NUM_SIZE; j++) {
                            if (temp_p == NULL)
                                break;
                            rand_array[i][j] = atoi(temp_p);
                            temp_p = strtok('\0', ",");
                        }
                    } else {
                        lperror(LOG_ERR, "Fail to read the random pattern file.");
                        fclose(fp);
                        return errno;
                    }
                }
                fclose (fp);
            } else {
            
                init_random();
                fp = fopen(RAND_PATTERN, "w+");
                if (fp == NULL) {
                    lperror(LOG_ERR, "Fail to open the random pattern file.");
                    return errno;
                }

                for (i=0; i<RAND_ARRAY_NUM; i++) {
                    for (j=0; j<(RAND_NUM_SIZE); j++) {
                        rand_array[i][j] = (unsigned int)rand();
                        fprintf(fp, "%d,", rand_array[i][j]);
                    }
                        fprintf(fp, "\n");
                }
                fclose(fp);
            }   
        }
    }

	for (i=0; i<NODE_COUNT; i++) {
		//max_range = istr[i].range_num;
		if (max_range < istr[i].range_num)
			max_range = istr[i].range_num;
	}

	for (i=0; i<NODE_COUNT; i++) {
        for (n=0; n<istr[i].range_num - 1; n++) {
	    for (m=0; m<(istr[i].range_num - 1); m++) {
            block0 = istr[i].rinfo[m].end - istr[i].rinfo[m].start;
            block1 = istr[i].rinfo[m+1].end - istr[i].rinfo[m+1].start;
            if (block0 < block1) {
                memset((void *)&rinfo_tmp, 0, sizeof(RW_range_info));
                rinfo_tmp = istr[i].rinfo[m];
                memset((void *)&istr[i].rinfo[m], 0, sizeof(RW_range_info));
                istr[i].rinfo[m] = istr[i].rinfo[m+1];
                memset((void *)&istr[i].rinfo[m+1], 0, sizeof(RW_range_info));
                istr[i].rinfo[m+1] = rinfo_tmp;
            }
        }
    }
    }

    for (item_n=0; test_config.test_item[item_n] != -1; item_n++) {
	   for (j=0; j<max_range; j++) {
       		thread_num = 0;
       		for (i=0; i<NODE_COUNT; i++) {
		   if (j >= istr[i].range_num)
		      continue;
                   for (k=istr[i].rinfo[j].slice_num-1; k>=0; k--) {
                      thread_stru[k+thread_num].module_id = MEMORY_MODULE;
                      thread_stru[k+thread_num].no_comp = no_comp;
                      thread_stru[k+thread_num].item_id = test_config.test_item[item_n];
                      thread_stru[k+thread_num].block = (ULV *)(istr[i].rinfo[j].sinfo[k].start);
                      thread_stru[k+thread_num].block_bytes = istr[i].rinfo[j].sinfo[k].end - istr[i].rinfo[j].sinfo[k].start;
                      thread_stru[k+thread_num].cpuid = istr[i].rinfo[j].sinfo[k].cpuid;
                      if (pthread_create(&thread[k+thread_num], NULL, (void *)run_test_slice, (void *)&thread_stru[k+thread_num]) != 0) {
                         lperror(LOG_ERR, "pthread_create() is failure for creating thread %d", k+thread_num);
                         ret = ERR_CREATE_THREAD;
                         ret_val[k+thread_num] = ret;
           //            break;
                   }
                }
	        thread_num += istr[i].rinfo[j].slice_num;
           }
           for (m=0; m<thread_num; m++) {
               if (thread[m] != 0) {
                  if (pthread_join(thread[m], (void **)&ret_val[m]) != 0)
                     lperror(LOG_ERR, "Cannot join thread %ld from main thread", (unsigned long)thread[m]);
               }
        //       ret = ret_val[m];
			   //if (ret_val[m] != ERR_SUCCESS)
			//	   	lperror(LOG_ERR, "Thread %d, failure, return value is %lx", m, ret_val[m]);
           }
           for (m=0; m<thread_num; m++) {
	       //ret = ret_val[m];
	       if(ret_val[m] != ERR_SUCCESS) {
                  ret = ret_val[m];
                  //lperror(LOG_ERR, "Thread %d, failure, return value is %lx", m, ret_val[m]);
                  lperror(LOG_ERR, "Thread %d, Test item %d, return failure code %lx", m, test_config.test_item[item_n], ret);
	       }
           }
    	}
        if (ret == ERR_SUCCESS)
            lprintf(LOG_INFO, "Test item %d, successful", test_config.test_item[item_n]);
        else {
            //lperror(LOG_ERR, "Test item %d, Failure", test_config.test_item[item_n]);
	    //goto error_flag;
		}
	}

error_flag:

    /* munmap and close mmtest_fd to release resource  */
    /*for (i=0; i<NODE_COUNT; i++) {
        for (j=0; j<istr[i].range_num; j++) {
            if (istr[i].rinfo[j].start != 0) {
                munmap((void *)(istr[i].rinfo[j].start), (istr[i].rinfo[j].end-istr[i].rinfo[j].start));
            }
        }
    }*/
    release_munmap_close();
    release_list_resource();
    if(mmtest_fd >0)
        close(mmtest_fd);

    return ret;
}

unsigned int pr_exit(int status)
{
    unsigned int ret = ERR_SUCCESS;
    
    if (WIFEXITED(status)) { 
        lprintf(LOG_INFO, "Test child normal termination");
        ret = WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status)) {
        lprintf(LOG_INFO, "Test child abnormal termination, signal number = %d", WTERMSIG(status));
        ret = WIFSIGNALED(status);
    }
    else if (WIFSTOPPED(status)) {
        lprintf(LOG_INFO, "Test child stopped, signal number = %d", WSTOPSIG(status));
        ret = WIFSTOPPED(status);
    }
    else {
        lprintf(LOG_INFO, "Don't know the test child exit status, status = %d", status);
        ret = status;
    }
    return ret;
}


unsigned int pr_exit_a(int status)
{
    unsigned int ret = ERR_SUCCESS;
    
    if (WIFEXITED(status)) { 
        lprintf(LOG_INFO, "MCE child normal termination");
        ret = WIFEXITED(status);
    }
    else if (WIFSIGNALED(status)) {
        lprintf(LOG_INFO, "MCE child abnormal termination, signal number = %d", WTERMSIG(status));
        ret = WIFSIGNALED(status);
    }
    else if (WIFSTOPPED(status)) {
        lprintf(LOG_INFO, "MCE child stopped, signal number = %d", WSTOPSIG(status));
        ret = WIFSTOPPED(status);
    }
    else {
        lprintf(LOG_INFO, "Don't know the MCE child exit status, status = %d", status);
        ret = status;
    }
    return ret;
}

void print_test_status(void)
{
    lprintf(LOG_INFO, "Exit the test child");

}
    
/*!
 * \brief   module test function
 * \param   module_id   the id of test module
 * \param   param       the head pointer of parameters
 * \paran   length      the length of parameters
 * \return  0           success
 *          nonzero     return the error code
 */
unsigned int module_test(int module_id, void *parm, int length)
{
    int statloc = 0;
	test_parm *p;
    unsigned int ret = ERR_SUCCESS;
    sigset_t mask;
    

	p = (test_parm *)parm;


	if (p->exit_error == EXIT_ON_ERROR) {
		exit_on_error = EXIT_ON_ERROR;
	}
	ret = do_mce_check(GET_MCE, exit_on_error, &msr_errors, &record_errors);
	if (ret == 0) {
    //    lprintf(LOG_NOTICE, "      MSR        [%d]", msr_errors);
        lprintf(LOG_NOTICE, "Before testing, get MCE information");
        lprintf(LOG_NOTICE, "      recorded   [%d]", record_errors);
        record_errors = 0;
	} else {
		lprintf(LOG_ERR, "Fail to get MCE error messages");
		return ERR_MCE_MSG;
    }

    sigemptyset(&mask); 
    if (sigprocmask(SIG_BLOCK, NULL, &mask) < 0) {
        lperror(LOG_ERR, "sigprocmask system call is failure");
        return errno;
    }
    sigaddset(&mask, SIGHUP);
//    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGTERM);
    if (exit_on_error == NO_EXIT_ERROR) {
//        sigaddset(&mask, SIGUSR1);
//    	sigaddset(&mask, SIGUSR2);
    }
    sigaddset(&mask, SIGPIPE);
    
    if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
        lperror(LOG_ERR, "sigprocmask system call is failure");
        return errno;
    }
   
		if (signal_a(SIGUSR1, sig_mce_exit) == SIG_ERR) {
			lperror(LOG_ERR, "Sigaction system call is failure");
			return errno;
		}


        /* Read-only test  */
#if 0 	
		ret = read_only_test(p->exit_error);
		if (ret == ERR_SUCCESS) {
			lprintf(LOG_INFO, "Read-only test is successful");
		} else {
			lprintf(LOG_INFO, "Read-only test is failure");
			exit(ret);
		}

        ret = read_write_test();
		if (ret == ERR_SUCCESS) {
			lprintf(LOG_INFO, "Read/write algorithm test is successful");
		} else {
			lprintf(LOG_INFO, "Read/write algorithm test is failure");
			exit(ret);
		}
#endif
    
    if ((pida = fork()) < 0) {
        lperror(LOG_ERR, "Fork() system call error");
        return errno;
    } else if (pida == 0) {
        // Child 0 
        // Start MCE child 
		ret = do_mce_check(MCE_LOOP, exit_on_error, &msr_errors, &record_errors);
		if (ret == 0) {
		lprintf(LOG_INFO, "Poll to get MCE error message, recorded error count is %d", record_errors);
		}
        exit(ret);
    } else if ((pidb = fork()) < 0) {
        lperror(LOG_ERR, "Fork() system call error");
        return errno;
    } else if (pidb == 0) {
        // Child 1 
        // Start test child 
		// to wait MCE child starting 
	
 
        if (signal_a(SIGUSR2, USR2_signal_exit) == SIG_ERR) {
		    lperror(LOG_ERR, "Sigaction system call is failure");
		    return errno;
	    }
        
        atexit(print_test_status);
        // Read-only test  
        if (RD_empty == 0) {
            lprintf(LOG_INFO, "The total RD ranges to be tested are empty, skip read-only test");
        } else {
		    //if( CONFIG_STRICT_DEVMEM == 0 )
                      {
                       ret = read_only_test(p);
		       if (ret == ERR_SUCCESS) {
			    lprintf(LOG_INFO, "Read-only test is finished , return 0");
		       } else {
			    lperror(LOG_ERR, "Read-only test is failure, return [%lx]", ret);
			    exit(ret);
		      }
                   }// else {
                      //lprintf(LOG_WARNING, "Warning: CONFIG_STRICT_DEVMEM defined, skip read only test");
                   //}
                                      
        }
       	// Read/Write test
        if (RW_empty == 0) {
            lprintf(LOG_INFO, "The total RW ranges to be tested are empty, skip read/write test");
        } else {
            ret = read_write_test(p->no_comp);
		    if (ret == ERR_SUCCESS) {
			    lprintf(LOG_INFO, "Read/write algorithm test is successful");
		    } else {
			    lperror(LOG_ERR, "Read/write algorithm test is failure, return [%lx]", ret);
		    }
        }
        exit (ret);
    
    } else { 
    	/* Parent  */ 
		/* Wait for algorithm test child 1   */


		if (waitpid(pidb, &statloc, 0) != pidb) {
        	lperror(LOG_ERR, "waitpid Child 1 for algorithm test");
        	return errno;
    	}
        
        ret_flag = pr_exit(statloc);
    
	
		if (kill(pida, SIGABRT) != 0) {
			lperror(LOG_ERR, "Killing MCE child is failure");
			return errno;
		}

		/* Wait for MCE check child 0  */
    	if (waitpid(pida, &statloc, 0) != pida) {
        	lperror(LOG_ERR, "waitpid Child 0 for MCE check");
        	return errno;
    	}
	
        pr_exit_a(statloc);
	}
    
    msr_errors = 0;
    record_errors = 0;
//	lprintf(LOG_NOTICE, "After testing, get MSR errors"); 
    ret = do_mce_check(GET_MSR_MCE, exit_on_error, &msr_errors, &record_errors);
	if (ret == 0) {
    //    lprintf(LOG_NOTICE, "      MSR        [%d]", msr_errors);
    } else {
		lperror(LOG_ERR, "Fail to get MSR MCE errors\n");
		return ERR_MCE_MSG;
    }

    if (ret != ERR_SUCCESS) {
        release_munmap_close();
    }
    
	lprintf(LOG_INFO, "Memory Testing is finished");
    
    if (termination_flag == 1) {
        lperror(LOG_ERR, "MCE errors occurs during memory test and terminate the test");
        return ERR_MCE_TERMINATE;
    }
    
//    lprintf(LOG_INFO, "MCE_EVENT_FLAG = %d\n", mce_flag);
    if (mce_flag == 1) {
        lperror(LOG_ERR, "MCE errors occurs during memory test, please check MCE information");
        return ERR_MCE_TERMINATE;
    }

    if (ret_flag != ERR_SUCCESS) {
        lperror(LOG_ERR, "The algorithm test is failure, return 0x%lx", ret_flag+0xA1000000);
        return (ret_flag+ 0xA1000000);
    }

    return ERR_SUCCESS;
}

