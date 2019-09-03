/*!
*\file memory.c
* 
* Define memory module test function
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
* Nov. 19, 2009    Change coding stytle from c++ to linux c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/mman.h> /* mlock  */
#include <pthread.h>
#include <signal.h>
#include <errno.h>

#include "jedi_comm.h"
#include "sys_i.h"
#include "absdiag_dev.h"
#include "memory_err.h"
#include "memory.h"
#include "test.h"
#include "cache_test.h"
//#include "vir2phy.h"


const struct tseq tseq[MAX_ITEM_NUM] = {
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
	{1,  8,   1, 0, "[Bit fade test, 90 min, 2 patterns]   "},
	{0,  0,   0, 0, NULL}
};

#define DMILOG  "/tmp/dmimem.log"
#define MLOCK_BLOCK (1*8*1024*1024)

/*
 * Define the global variables
 */

thread_info_struc thread_info[MAX_CORE_NUM];
thread_info_struc thread_info_a;
int mem_block_num = 0;
int core_num = 0;
UL test_total_bytes = 0;


/*
 *  The Pattern size is RAND_ARRAY_NUM * RAND_NUM_SIZE = (8 * 1024) * (1 * 1024) = 8M positions
 *  The pattern memory size is 8M position * sizeof (unsigned int) = 64 M bytes
 *  The algorithm test block should be rank aligment, so should be more than 0X20000 = 128 k
 *  The test range should be more than 8 M bytes
 *
 * */

unsigned int rand_array[RAND_ARRAY_NUM][RAND_NUM_SIZE];



/*!
 * Get memory information for /proc/meminfo
 * \param
 * \param
 * \return  0      success
 *          Not 0  return error code
 */
unsigned int get_mem_info(char *string, unsigned long *ret_value)
{
    unsigned int ret = ERR_SUCCESS;
    FILE *stream;
    char command_str[LEN_256] = "";
    char temp_string[LEN_256] = "";
    char temp_file[LEN_256] = "/tmp/memorytest_a.txt";
    
    if (access(temp_file, F_OK) == 0)
        remove(temp_file);

    sprintf(command_str, "cat /proc/meminfo | grep %s | awk '{print $2}' > %s"
            , string, temp_file);
    system(command_str);

    stream = fopen(temp_file, "r");
    if (NULL == stream){
        ret = ERR_OPEN_TEMP_FILE;
        sprintf(error_info, "Function %s, Line %d", __FUNCTION__, __LINE__);
        return ret;
    }
    if (NULL == fgets(temp_string, sizeof(temp_string), stream)){
        ret = ERR_READ_TEMP_FILE;
        sprintf(error_info, "Function %s, Line %d", __FUNCTION__, __LINE__);
        fclose(stream);
        return ret;
    }
    fclose(stream);

    if (access(temp_file, F_OK) == 0)
        remove(temp_file);
    *ret_value = atoi(temp_string)/1024;
    return ret;
}

/*!
 * initilize module
 * \param   module_id   the id of test module
 * \param   item_id     the id of test item
 * \param   param       the point of parameter
 * \return  0  success
 *          Not 0 is the error code
 */
unsigned int init_module(int module_id, void *param, int length)
{
    unsigned int ret = ERR_SUCCESS;
    UL i = 0;
    UL j = 0;
    UL cpu_id = 0;
    mem_info mem = {0, 0};
    UL test_block_bytes = 0;
    ULV *memory_p = NULL;
    test_parm *p;

    p = (test_parm *)param;

    /* Initial the golbal variables   */
    core_num = 0;
    mem_block_num = 0;
    test_total_bytes = 0;

    memset((void *)&thread_info, 0, MAX_CORE_NUM*sizeof(thread_info_struc));
    memset((void *)&thread_info_a, 0, sizeof(thread_info_struc));
    memset((void *)&mem, 0, sizeof(mem_info));
    memset((void *)&error_info, 0, sizeof(error_info));

#if 0
    if (-1 == system("modprobe -f abs_diag")) {
        printf("Fail to load abs_diag module\n");
    }
    if (-1 == system("modprobe -f addrmap")) {
        printf("Fail to load addrmap module\n");
    }

    if (-1 == system("dmidecode >" DMILOG)) {
        printf("Fail to system call dmidecode.\n");
    }
#endif

    if ((module_id != MEMORY_MODULE) && (module_id != CPU_CACHE) &&(module_id != CACHE_MEMORY)) {
        ret = ERR_MODULE_ID;
        memory_error_msg(ret);
        return ret;
    }

    ret = get_mem_info("MemTotal:", &(mem.total));
    if (ret != ERR_SUCCESS) {
        memory_error_msg(ret);
        return ret;
    }
    ret = get_mem_info("MemFree:", &(mem.free));
    if (ret != ERR_SUCCESS) {
        memory_error_msg(ret);
        return ret;
    }
    if ((mem.total == 0) ||(mem.free == 0)) {
        ret = ERR_GET_MEMORY_INFO;
        memory_error_msg(ret);
        return ret;
    }
    
    /* If mem.total<256M bytes, or mem.free < 128M bytes, 
     * it is too few memory space */
    if ((mem.total < 256) || (mem.free < 64)) {
        ret = ERR_NO_ENOUGH_MEM_SPACE;
        memory_error_msg(ret);
        return ret;
    }
    /* reserve 64M memory for testing program  */
    mem.free = mem.free * 0.98; //0.02 for testing

    core_num = sysconf(_SC_NPROCESSORS_CONF);
    if ((core_num == 0)||(core_num > MAX_CORE_NUM)) {
        ret = ERR_CORE_NUM;
        sprintf(error_info, "Functin %s, Line %d", __FUNCTION__, __LINE__);
        memory_error_msg(ret);
        return ret;
    }

    if((p->cpu != NO_SPECIFY_CPU)&&(p->cpu >= core_num)) {
        ret = ERR_CORE_NUM;
        memory_error_msg(ret);
        return ret;
    }

    if (module_id == MEMORY_MODULE) {
        test_total_bytes = (mem.free << 20);  //UL
    }
    else if (module_id == CPU_CACHE) {
        if (p->cpu == NO_SPECIFY_CPU)
            test_total_bytes = (64 << 20) * core_num;
        else
            test_total_bytes = (64 << 20);
/* split 128M memory for test cpu_cache  */
    } else if (module_id == CACHE_MEMORY) {
	if (p->cpu == NO_SPECIFY_CPU)
            test_total_bytes = (16 << 20) * core_num;
        else
            test_total_bytes = (16 << 20);
    }
 
    mem_block_num = 0;

    if (p->cpu == NO_SPECIFY_CPU) {
        test_block_bytes = test_total_bytes / core_num;
        for (i=0; i<core_num; i++) {
            thread_info[i].block_bytes = test_block_bytes;
            thread_info[i].module_id = module_id;
        
            memory_p = (ULV *)malloc(test_block_bytes);
            if (memory_p == NULL) {
                ret = ERR_MALLOC_SPACE;
                sprintf(error_info, "Functin %s, Line %d, System errno:%s", 
                            __FUNCTION__, __LINE__, strerror(errno));
                break;
            }
            mem_block_num++;

            cpu_id = i;
            thread_info[i].cpu = cpu_id;
            thread_info[i].block = memory_p;
        }

        for (i=0; i<mem_block_num; i++) {
            j = 0;
            while ((j*MLOCK_BLOCK) < test_block_bytes) { 
                if (j < (test_block_bytes / MLOCK_BLOCK))
                    ret = mlock((void *)(thread_info[i].block + j*(MLOCK_BLOCK/sizeof(UL))), MLOCK_BLOCK);
                else
                    ret = mlock((void *)(thread_info[i].block + j*(MLOCK_BLOCK/sizeof(UL))), test_block_bytes % MLOCK_BLOCK);
                if (ret != ERR_SUCCESS) {
                    ret = ERR_MLOCK_SPACE;
                    sprintf(error_info, "Functin %s, Line %d, System errno:%s", 
                            __FUNCTION__, __LINE__, strerror(errno));
                    break;
                }
                j++;
            }
        }
    }

    if (p->cpu != NO_SPECIFY_CPU) {
        test_block_bytes = test_total_bytes;
            thread_info_a.cpu = p->cpu;
            thread_info_a.block_bytes = test_block_bytes; 
            thread_info_a.module_id = module_id;

        memory_p = (ULV *)malloc(test_block_bytes);
        if (memory_p == NULL) {
            ret = ERR_MALLOC_SPACE;
            sprintf(error_info, "Functin %s, Line %d, System errno:%s", 
                        __FUNCTION__, __LINE__, strerror(errno));
            memory_error_msg(ret);
            return ret;
        }
        thread_info_a.block = memory_p;
        /*When mlock a large of memory space, mlock system call will be failure. So, add the for-loop to mlock a free
         * memory block */
        j = 0;
        while ((j*MLOCK_BLOCK) < test_block_bytes) { 
            if (j < (test_block_bytes / MLOCK_BLOCK))
                ret = mlock((void *)(thread_info_a.block + j*(MLOCK_BLOCK/sizeof(UL))), MLOCK_BLOCK);
            else
                ret = mlock((void *)(thread_info_a.block + j*(MLOCK_BLOCK/sizeof(UL))), test_block_bytes % MLOCK_BLOCK);
            if (ret != ERR_SUCCESS) {
                ret = ERR_MLOCK_SPACE;
                sprintf(error_info, "Functin %s, Line %d, System errno:%s", 
                            __FUNCTION__, __LINE__, strerror(errno));
                break;
            }
            j++;
        }

#if 0
        if (ret == ERR_SUCCESS) {
            ret = page_list_create(thread_info_a.block, thread_info_a.block_bytes);

            conu_phyblock_list_create();

            conu_phyblock_cleanup();

            page_list_cleanup();

        }
#endif
        
    }
    return ret;
}

/*!
 * close memory module
 * \param   module_id   the id of test module
 * \param   item_id     the id of test item
 * \param   param       the point of parameter
 * \return  0  success
 *          Not 0 is the error code
 */
unsigned int close_module(int module_id, void *param, int length)
{
    unsigned int ret = ERR_SUCCESS;
    int i = 0;
    test_parm *p;

    p = (test_parm *)param;
    if (p->cpu == NO_SPECIFY_CPU) {
        for (i=0; i<mem_block_num; i++) {
            if (NULL != thread_info[i].block) {
                munlock((void *)thread_info[i].block, thread_info[i].block_bytes);
                free((void *)thread_info[i].block);
                thread_info[i].block = NULL;
            }
        }
    }
    if (p->cpu != NO_SPECIFY_CPU) {
        if (NULL != thread_info_a.block) {
            munlock((void *)(thread_info_a.block), test_total_bytes);
            free((void *)thread_info_a.block);
            thread_info_a.block = NULL;
        }
    }

    return ret;
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
    sigaddset(&new_set, SIGINT);
    sigaddset(&new_set, SIGQUIT);
    sigaddset(&new_set, SIGTERM);
    sigaddset(&new_set, SIGUSR1);
    sigaddset(&new_set, SIGUSR2);
    sigaddset(&new_set, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &new_set, &old_set);
}

/*!
 *The entry of test every test items function
 *\param thread_info -  the struct of thread information
 */
unsigned int test_function(thread_info_struc *thread)
{
    unsigned int ret = ERR_SUCCESS;
    pthread_t tid = 0;
    UL c_iter = 0;
    absdiag_struc abs;
    
    memset((void *)&abs, 0, sizeof(abs));

    tid = pthread_self();

        ret = isys_cpu_bind(thread->cpu);
        if (ret != ERR_SUCCESS)
            return ERR_BIND_CPU;
 
        if ((thread->module_id == CPU_CACHE) || (thread->module_id == CACHE_MEMORY)) {
         ret = verify_absdiag_dev(&abs);
        if (ret != ERR_SUCCESS){
            return ret;
        }
        if (abs.cashing == 0) {
            ret = ERR_TURNOFF_CACHE;
            return ret;
        }
    }

    thread_signal_set();
    c_iter = tseq[thread->item_id-1].iter;
    
    if (thread->module_id == MEMORY_MODULE) {
        switch (thread->item_id) {
            case MEM_ADD_WALK_ONES:         /*test #0  */
                ret = test_addr_walk_ones(thread->block, thread->block_bytes);
                break;
            case MEM_ADD_OWN_ADDRESS:       /*test #1 */
                ret = test_addr_walk_own(thread->block, thread->block_bytes);
                break;
            case MEM_RANDOM_ADDRESS:        /*test #2 */
                ret = test_cache_randaddr(thread->block, thread->block_bytes, thread->module_id);
                break;
            case MEM_MOVE_INVER_ONES_ZERO:  /*test #3 */
                ret = test_movinv_10(thread->block, thread->block_bytes, c_iter);
                break;
            case MEM_MOVE_INVER_8BIT:       /*test #4 */
                ret = test_movinv_8bit(thread->block, thread->block_bytes, c_iter);
                break;
            case MEM_MOVE_INVER_RANDOM:     /*test #5 */
                ret = test_movinv_random(thread->block, thread->block_bytes, c_iter);
                break;
            case MEM_BLOCK_MOVE:            /*test #6  */
                ret = test_block_move(thread->block, thread->block_bytes, c_iter);
                break;
            case MEM_MOVE_INVER_32BIT:      /*test #7  */
                ret = test_movinv_long(thread->block, thread->block_bytes, c_iter);
                break;
            case MEM_RANDOM_NUM_SEQUENCE:   /*test #8  */
                ret = test_random_num_sequence(thread->block, thread->block_bytes);
                break;
            case MEM_MODULO:                /*test #9  */
                ret = test_mem_modulo(thread->block, thread->block_bytes, c_iter);
                break;
            case MEM_BIT_FADE_TEST:         /*test #10  */
                ret = test_bit_fade(thread->block, thread->block_bytes);
                break;
            default:
                ret = ERR_TEST_ITEM_NUM;
                break;
        }
    }
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
                ret = ERR_TEST_ITEM_NUM;
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
                ret = ERR_TEST_ITEM_NUM;
                break;
        }
    }

    return ret;
}

/*!
 * module test function
 * \param   module_id   the id of test module
 * \param   item_id     the id of test item
 * \param   param       the point of parameter
 * \return  0  success
 *          Not 0 is the error code
 */
unsigned int module_test(int module_id, int item_id, void *param, int length)
{

    int i = 0;
    int j = 0;
    unsigned int ret = ERR_SUCCESS;
    int thread_num = 0;
    pthread_t thread[MAX_CORE_NUM];
    pthread_t thread_a;
    unsigned int ret_val[MAX_CORE_NUM];
    unsigned int ret_val_a = 0;
    test_parm *p;
    FILE *fp;
    char buf[RAND_NUM_SIZE*10*2];
    char *temp_p;

    memset((void *)&thread, 0, sizeof(pthread_t));
    memset((void *)&ret_val, 0, sizeof(ret_val));

    p = (test_parm *)param;

    if ((item_id<0) || item_id>MAX_ITEM_NUM) {
        ret = ERR_ITEM_ID;
        memory_error_msg(ret);
        return ret;
    }
    
    if (((module_id == MEMORY_MODULE) && (item_id == MEM_RANDOM_ADDRESS)) || ((module_id == CPU_CACHE) && (item_id == CPU_CACHE_RANDOM_ADDR))) {
        memset((void *)rand_array, 0, (sizeof(unsigned int))*RAND_ARRAY_NUM*RAND_NUM_SIZE);
        /* Generate the random array for random address testing */

        if (access(RAND_PATTERN, F_OK) == 0) {
            fp = fopen(RAND_PATTERN, "r");
            if (fp == NULL) {
                ret = ERR_OPEN_TEMP_FILE;
                sprintf(error_info, "Fail to open the random pattern file. Function %s, Line %d", __FUNCTION__, __LINE__);
                memory_error_msg(ret);
                return ret;
            }      
            for (i=0; i<RAND_ARRAY_NUM; i++) {
                memset((void *)buf, 0 , sizeof(buf));
                if (NULL != fgets(buf, sizeof(buf), fp)) {
                    temp_p = strtok(buf, ",");
                    for (j=0; j<RAND_NUM_SIZE; j++) {
                        if (temp_p == NULL)
                            break;
                        rand_array[i][j] = atoi(temp_p);
                        temp_p = strtok('\0', ",");
                    }
                } else {
                    ret = ERR_READ_TEMP_FILE;
                    sprintf(error_info, "Fail to read the random pattern file. Function %s, Line %d", __FUNCTION__, __LINE__);
                    fclose(fp);
                memory_error_msg(ret);
                    return ret;
                }
            }
            fclose (fp);

        } else {
            
            init_random();
            fp = fopen(RAND_PATTERN, "w+");
            if (fp == NULL) {
                ret = ERR_OPEN_TEMP_FILE;
                sprintf(error_info, "Function %s, Line %d", __FUNCTION__, __LINE__);
                memory_error_msg(ret);
                return ret;
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

    if (p->cpu == NO_SPECIFY_CPU) {
        if (ret == ERR_SUCCESS) {
            for (i=0; i<core_num; i++) { 
                thread_info[i].item_id = item_id;
            }
            if ((module_id == CPU_CACHE) && ((item_id == CPU_CACHE_SPILL) || (item_id == CPU_CACHE_TAG))) { 
                for (i=0; i<core_num; i++) {
                    if (pthread_create(&thread[i], NULL, (void *)test_function, 
                            (void *)&thread_info[i]) != 0) {
                        ret = ERR_CREATE_THREAD;
                        sprintf(error_info, "Function %s, Line %d, System errno: %s", 
                            __FUNCTION__, __LINE__, strerror(errno));
                        break;
                    }
                    thread_num ++;
                    pthread_join(thread[i], (void **)&ret_val[i]);
                    ret = ret_val[j];
                    if (ret != ERR_SUCCESS) {
                        sprintf(error_info, "thread number is %d, test error", j);  
                        D_printf("Warning: thread number is %d, test error\n", j);
                    }
                }

            } else {
                for (i=0; i<core_num; i++) {
                    if (pthread_create(&thread[i], NULL, (void *)test_function, 
                            (void *)&thread_info[i]) != 0) {
                        ret = ERR_CREATE_THREAD;
                        sprintf(error_info, "Function %s, Line %d, System errno: %s", 
                            __FUNCTION__, __LINE__, strerror(errno));
                        break;
                    }
                    thread_num ++;
                }
                for (j=0; j<thread_num; j++) {
                    pthread_join(thread[j], (void **)&ret_val[j]);
                    ret = ret_val[j];
                    if (ret != ERR_SUCCESS) {
                        sprintf(error_info, "thread number is %d, test error", j);  
                        D_printf("Warning: thread number is %d, test error\n", j); 
                    }
                }

            }
        }
    }

    if (p->cpu != NO_SPECIFY_CPU) {
        if (ret == ERR_SUCCESS) {
            thread_info_a.item_id = item_id;
            if (pthread_create(&thread_a, NULL, (void *)test_function, 
                        (void *)&thread_info_a) != 0) {
                ret = ERR_CREATE_THREAD;
                sprintf(error_info, "Function %s, Line %d, System errno: %s", 
                        __FUNCTION__, __LINE__, strerror(errno));
                memory_error_msg(ret);
                return ret;
            }
            pthread_join(thread_a, (void **)&ret_val_a);
            ret = ret_val_a;
            if (ret != ERR_SUCCESS) {
                        sprintf(error_info, "thread number is %d, test error", j);  
                        D_printf("Warning: thread number is %d, test error\n", j);  
            }
        }
    } 
    
    if (ret_val_a != ERR_SUCCESS) {
        ret = ret_val_a;
    }

    for (i=0; i<MAX_CORE_NUM; i++) {
        if (ret_val[i] != ERR_SUCCESS)
            ret = ret_val[i];
    }
    if (ret != ERR_SUCCESS)
        memory_error_msg(ret);

    return ret;
}


/* Test the library  */
#if 0
int main(int argc, char *argv[])
{
    int i = 0;
    unsigned int ret = ERR_SUCCESS;
    unsigned int cli_ret = ERR_SUCCESS;
    time_t now;
    struct tm *timenow;
    const char *progname;
    test_parm test_p = {100, 100, NO_SPECIFY_BLOCK_SIZE};

    progname = (const char *)strrchr(argv[0], '/');
    progname = progname ? (progname + 1) : argv[0];


    ret = init_module(MEMORY_MODULE, &test_p, 0);
    if (ret == ERR_SUCCESS)
        printf("Initial module is pass\n");
    else
        printf("Fail to initial module\n");

    if (test_p.test_item == 100) {
        for (i=0; i<2; i++) {
            time(&now);
            timenow = localtime(&now);
#if __DEBUG__
            D_printf("                         %s", asctime(timenow));
#endif
            ret = module_test(MEMORY_MODULE, i+1, &test_p, 0);
            if (ret == ERR_SUCCESS)
                printf("Test item %d  is pass\n", i);
            else
                printf("Fail to test item %d \n", i);

        }    
    } 
    else {
       i = test_p.test_item;
        ret = module_test(MEMORY_MODULE, i+1, &test_p, 0);
             if (ret == ERR_SUCCESS)
                printf("Test item %d  is pass\n", i);
            else
                printf("Fail to test item %d \n", i);

   }

    ret = close_module(MEMORY_MODULE, &test_p, 0);
     if (ret == ERR_SUCCESS)
        printf("Close module is pass\n");
    else
        printf("Fail to close the module\n");

 
    fprintf(stdout, "\033[32mPASS\033[0m\n");
    return cli_ret;
}
#endif

