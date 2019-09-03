/*!
*\file task_generator.c
* 
*\brief  Get the physical memory layout information with memory region attributes and then split memory ranges into several test slices according to the test policy inputted by user
*
*\author Maggie Tong
*\author Jackie Liu jackie.liu2@jabil.com
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
* Feb 15, 2011 :   Create this file
*/

/*
 * INCLUDE FILES
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>

#include "jedi_comm.h"
#include "log.h"
#include "memory.h"
#include "list.h"
#include "mem_err.h"
#include "task_generator.h"
#include "mce_extern.h"

//#define DEBUG
test_config_struc test_config;

/*  Driver mmtest device file fd  */
int mmtest_fd = 0;   
int RD_empty = 0;
int RW_empty = 0;

typedef void (*sighandler_t)(int);

extern int do_mce_check(int flag, int exitonerror, int *msr_errors, int *record_errors);

extern void release_list_resource(void);

extern int e820AndCmdlineInitialization(void);

void release_munmap_close(void);

extern void sig_ctrlC_exit(int signo);

extern sighandler_t signal_a(int signo, sighandler_t func);

UL pfn_start_roundup(UL a, UL b)
{
    return (a & ~b); 

}

UL pfn_end_roundup(UL a, UL b)
{
    return ((a + b) & ~b);
}

#if 0
UL pfn_roundup(UL a, UL b)
{
    if (a%b == 0)
        return a;
    else
        return (a + (b - a%b));
}
#endif


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
        lperror(LOG_ERR, "Fail to open %s file", temp_file);
        return errno;
    }
    if (NULL == fgets(temp_string, sizeof(temp_string), stream)){
        lperror(LOG_ERR, "Fail to read %s file", temp_file);
        fclose(stream);
        return errno;
    }
    fclose(stream);

    if (access(temp_file, F_OK) == 0)
        remove(temp_file);
    *ret_value = atoi(temp_string)/1024;
    return ret;
}

/*!
 * \brief create LOCAL test mode table
 *
 */
int **create_LOCAL_table(int num)
{
    int **p;
    int i = 0;
    int j= 0;

    /* create the two dimemsional table (num * num/2)  */
    p = (int **)malloc(num*sizeof(int *));
    for (i=0; i<num; i++) {
        p[i] = (int *)malloc(num*sizeof(int));
        memset((void *)p[i], -1, num*sizeof(int));
    }

    for (i=0; i<num; i++) {
        for (j=0; j<num; j++) {
            if (i>=j) {
                p[i][j] = j;
            }
        }
    }     
     
    return p;
}

int **create_REMOTE_table(int num)
{
    int **p;
    int i = 0;
    int j = 0;

    p = (int **)malloc(num*sizeof(int *));
    for (i=0; i<num; i++) {
        p[i] = (int *)malloc(num*sizeof(int));
        memset((void *)p[i], -1, num*sizeof(int));
    }

    for (i=0; i<num; i++) {
        for (j=0; j<num; j++) {
            if (i>=j)
                p[i][j] = j + num;
        }
    }
    return p;
}

int **create_SPECIFIC_table(int num, int spec_id)
{
    int **p;
    int i = 0;
    int j = 0;

    p = (int **)malloc(num*sizeof(int *));
    for (i=0; i<num; i++) {
        p[i] = (int *)malloc(num * sizeof(int));
        memset((void *)p[i], -1, num*sizeof(int));
    }

    for (i=0; i<num; i++) {
        for (j=0; j<num; j++)
            p[i][j] = spec_id;
    }
    return p;
}

/*!
 *\brief  Generate task list based on the test mode
 *
*/

unsigned int test_policy(void *parm)
{
    int i=0;
    int j=0;
    int k=0;
    unsigned int ret = ERR_SUCCESS;
    
    RW_range *tmp_RW;
    struct list_head *list_a;
    struct list_head *list_b;
    UL range_size = 0;
    UL slice_num = 0;
    UL core_num = 0;
    test_parm *p;
    int spec_cpu = 0;
    int **table[NODE_COUNT] = {0};
    UL n = 0;
    UL step_size = 0;
    UL remain = 0;
    test_slice *tmp_slice;

    p = (test_parm *)parm;
    core_num = sysconf(_SC_NPROCESSORS_CONF);
    if ((core_num == 0)||(core_num > CORE_COUNT)) {
        lperror(LOG_ERR, "The read core count is over the supporting MAX_CORE_NUM");
        return ERR_CORE_NUM;
    }
    /* Set the cpu_core allocation about Node, cpuid  */
// for testing
// core_num = 8;    
    for (i=0; i<NODE_COUNT; i++) {
        for (j=0; j<SOCKET_COUNT; j++) {
            memset((void *)(test_config.node[i].socket[j].cpuid), -1, sizeof(int)*(CORE_COUNT/2));
            for (k=0; k<(core_num/2); k++) {   
                test_config.node[i].socket[j].cpuid[k] = k + i*(core_num/2);
            }
        }
    }
    if (p->cpu != NO_SPECIFY_CPU) {
        spec_cpu = p->cpu;
        if (spec_cpu >= core_num) {
            lperror(LOG_ERR, "The specified cpu is over the system cpu-core count");
            return ERR_CORE_NUM;
        }

        p->mode = SPECIFIC;
    }
    switch (p->mode) {
        case LOCAL:
            //table[0] = create_LOCAL_table(core_num/NODE_COUNT);
            //table[1] = create_REMOTE_table(core_num/NODE_COUNT);

            //break;
        case REMOTE:
            //table[0] = create_REMOTE_table(core_num/NODE_COUNT);
            //table[1] = create_LOCAL_table(core_num/NODE_COUNT);
            table[0] = create_LOCAL_table(core_num/NODE_COUNT);
            break;

        case ALL:
//            table = create_LOCAL_table(core_num);
            break;

        case SPECIFIC:
            table[0] = create_SPECIFIC_table(core_num/NODE_COUNT, spec_cpu);
            //table[1] = create_SPECIFIC_table(core_num/2, spec_cpu);
            break;
        default:
            break;
    }

    for (i=0; i<NODE_COUNT; i++) {
        for (j=0; j<SOCKET_COUNT; j++) {
            list_for_each(list_a, &(test_config.node[i].socket[j].RW_list)) {
                tmp_RW = list_entry(list_a, RW_range, range_list);
                INIT_LIST_HEAD(&tmp_RW->RW_slice);
                range_size = tmp_RW->end - tmp_RW->start;
                if (p->mode == SPECIFIC) {
                    slice_num = 1;
                    step_size = range_size;
                } else {
                    slice_num = range_size/MIN_BLOCK_SIZE;
                    step_size = MIN_BLOCK_SIZE;
                    remain = range_size % MIN_BLOCK_SIZE;
                    if (slice_num > (core_num/NODE_COUNT)) {
                        slice_num = (core_num/NODE_COUNT);
                        /* Notice: the remainder areas is not been inlcuded in testing areas*/
                        step_size = range_size/(core_num/NODE_COUNT);
                        step_size = pfn_start_roundup(step_size, (MIN_BLOCK_SIZE-1));
                        remain = range_size - step_size * (core_num/NODE_COUNT);
                    }
                }
                /* According to the test policy, split the test range into several test slices*/
                for (n=0; n<slice_num; n++) {
                    tmp_slice = (test_slice *)malloc(sizeof(test_slice));
                    INIT_LIST_HEAD(&tmp_slice->slice_list);
                    tmp_slice->start = tmp_RW->start + n*step_size;
                    tmp_slice->end = tmp_slice->start + step_size;
                    tmp_slice->phy_start = tmp_RW->phy_start + n*step_size;
                    if (n == slice_num -1)
                        tmp_slice->end = tmp_slice->end + remain;
                    tmp_slice->cpuid = table[i][slice_num-1][n];
                    list_add(&(tmp_slice->slice_list), &(tmp_RW->RW_slice));
                }
                list_for_each(list_b, &(tmp_RW->RW_slice)) {
                    tmp_slice = list_entry(list_b, test_slice, slice_list);
                }
                
            }
        }
    }
    return ret;
}

void release_munmap_close(void)
{
    int i=0;
    int j=0; 
    RW_range *tmp_RW;
    struct list_head *list;


        for (i=0; i<NODE_COUNT; i++) {
            for (j=0; j<SOCKET_COUNT; j++) {
                //avoid the problem if this is called from Ctrl-C handling while RW_list is not setup yet
                if(test_config.node[i].socket[j].RW_list.next != NULL ) {
                    list_for_each(list, &(test_config.node[i].socket[j].RW_list)) {
                        tmp_RW = list_entry(list, RW_range, range_list);
                        if (tmp_RW->start != 0) {
                            munmap((void *)tmp_RW->start, (tmp_RW->end)-(tmp_RW->start));
                        }
                    }
                }
            }
        }
        if (mmtest_fd > 0)
            close(mmtest_fd);

}

/*
 *   Define the RD_list, RW_list
 */

/*!
 *\brief init RD_list
 */
void init_RD_RW_list(socket_info *info)
{
    memset(info, 0, sizeof(socket_info));
    INIT_LIST_HEAD(&info->RD_list);
    INIT_LIST_HEAD(&info->RW_list);
}

/*!
 *\brief init RW_list
 */

/*!
 * \brief   initilize module
 * \param   module_id   the id of test module
 * \param   param       the head pointer of parameters
 * \param   length      the length of parameters
 * \return  0           success
 *          nonzero     return the error code
 */
unsigned int init_module(int module_id, void *parm, int length)
{
    int region_num = 0;
    int i = 0;
    int j = 0;
    int node_n = 0;
    int socket_n = 0;
    unsigned ret = ERR_SUCCESS;
    int msr_err = 0;
    int rec_err = 0;
    unsigned long tmp_a = 0;
    unsigned long tmp_b = 0;
    unsigned long max_phy_addr = 0;

    region_t region;
//        {0,10,300,'w',0},
//        {1,400,800,'r',0},
//        {2,900,1000,'w',1},
//        {3,1100,2000,'w',1},
//        {4,2100,2500,'r',0},
//        {5,2600,3000,'w',1},
//        {6,4000,8800000,'w',0},
//        {7,9000000,10000000,'w',1},
//        {8,11000000,12000000,'w',0},
//        {9,13000000,15000000,'r',1},
//
    region_t raw_reg[MAX_REGION] = {};
	int raw_num = 0;
    UL offset = 0;
    UL len = 0;
    ULV *addr = 0;

    range_struc range;
    RW_range *tmp_RW;
    RD_range *tmp_RD;
    struct list_head *list;
    test_parm *p;
    mem_info mem;
    
    #ifdef DEBUG
       printf("Enter init module\n");
    #endif

    if (signal_a(SIGINT, sig_ctrlC_exit) == SIG_ERR) {
	lperror(LOG_ERR, "Sigaction system call is failure");
	return errno;
    } 
    if ( e820AndCmdlineInitialization() != 0 )
       return -1;
    #ifdef DEBUG
       printf("Finish e820 and numa initialization\n");
    #endif
    p = (test_parm *)parm;
    memset((void *)&test_config, 0, sizeof(test_config_struc));
    memset((void *)test_config.test_item, -1, sizeof(int)*ITEM_COUNT * 2);
    memset((void *)&range, 0, sizeof(range_struc));

    /* Get the test items from user input parameters such as (-t 0,0~4,9~6,5)  */
    if (p->test_item[0][0] == -1) {
        for (j=0; j<ITEM_COUNT; j++) {
            test_config.test_item[j] = j;
        }
    } else {
        for (i=0, j=0; (p->test_item[i][0]) != -1; i++) {
            if ((p->test_item[i][1]) == -1) {
                test_config.test_item[j] = p->test_item[i][0];
                j++;
            
            } else {
                test_config.test_item[j] = p->test_item[i][0];
                j++;
                if (p->test_item[i][0] <= p->test_item[i][1]) {
                    while (test_config.test_item[j-1] < (p->test_item[i][1])) {
                        test_config.test_item[j] = test_config.test_item[j-1] + 1;
                        j++;
                    }
                } else {
                    while (test_config.test_item[j-1] > (p->test_item[i][1])) {
                        test_config.test_item[j] = test_config.test_item[j-1] - 1;
                        j++;
                    }
                }
            }   
        }
    }

    memset((void *)&mem, 0, sizeof(mem_info));
    ret = get_mem_info("MemTotal:", &(mem.total));
    if (ret != ERR_SUCCESS) {
        lperror(LOG_ERR, "Fail to get MemTotal from /proc");
        return ERR_GET_MEM_INFO;
    }

    ret = get_mem_info("MemFree:", &(mem.free));
    if (ret != ERR_SUCCESS) {
        lperror(LOG_ERR, "Fail to get MemFree from /proc");
        return ERR_GET_MEM_INFO;
    }
    //if page cache is enabled, this can lead to unexpected exit
    /*if (mem.free <= 100) {
        lperror(LOG_ERR, "The system free memory is less than 100M, and you couldn't run the memory test");
        return ERR_GET_MEM_INFO;
    }*/


    if ((p->end < p->start)){
        lperror(LOG_ERR, "The specified end is less than the start address.");
        return ERR_INPUT_ADDRESS;
    }

/*
    if ((p->start)>(mem.total * 1024 *1024)) {
        lperror(LOG_ERR, "The start address is over the systm physical address, please check");
        return ERR_INPUT_ADDRESS;
    }
*/

    /* p->end couldn't bigger than the maximum physical address  */

/********************************/
//    if (((mem.total*1024*1024)<=(p->end)) && ((mem.total*1024*1024)>=p->start)) {
//        p->end = mem.total*1024*1024;
//    }
/************************************/

	/* User want to list the memory layout information, so the test window is unlimited  */ 
	if (p->list_info == LIST_INFO) {
		p->start = 0;
		p->end = 0x1ffffffffff;
	}

    /* Initial the global data and lists */
    for (i=0; i<NODE_COUNT; i++) {
        for (j=0; j<SOCKET_COUNT; j++) {
            init_RD_RW_list(&(test_config.node[i].socket[j]));
        }
    }

    #ifdef DEBUG
       printf("Finish general initialization\n");
    #endif
    /* Get memroy regions information depending on Driver mem_test  */
//#if 0
    mmtest_fd = open(MMTEST_DEV_NAME, O_RDWR|O_SYNC);
    if (-1 == mmtest_fd) {
        lperror(LOG_ERR, "Fail to open %s file", MMTEST_DEV_NAME);
        return errno; 
    }
    if (ERR_SUCCESS != ioctl(mmtest_fd, GET_BREGION_NUM, &region_num)) {
        lperror(LOG_ERR, "Fail to ioctl system call");
        close(mmtest_fd);
        return errno;
    }
//#endif

//    region_num = 10;
    for (i=region_num-1; i>=0 ; i--) {
		memset((void *)&region, 0, sizeof(region_t));
        region.index = i;
//#if 0
        if (ERR_SUCCESS != ioctl(mmtest_fd, GET_BREGION, &region)) {
            lperror(LOG_ERR, "Fail to ioctl system call");
            close(mmtest_fd);
            return errno;
        }
//#endif
        offset = region.start_pfn * PAGE_SIZE;
        len = (region.end_pfn - region.start_pfn) * PAGE_SIZE;
        if (len <=0 ) {
            lperror(LOG_ERR, "The error region length (less than 0)");
            close(mmtest_fd);
            ret = ERR_INPUT_ADDRESS;
        }
        if ((offset + len) > max_phy_addr) {
            max_phy_addr = offset + len;
        }

       

/* 
 *\brief Combine the test window with user specified test range
 *  Supposing the test window is defined by start and end,
 *  The user specified test range is defined by offset and len, offset is the offset address, len is the length of test range.
 *  1) offset+len  < start
 *      skip because this range is not in the test window
 *  2) (offset <= sart)  && (start<=(offset+len)<=end)
 *      range.start = start;
 *      range.end = offset + len;
 *  3) if len <= (end-start) {
 *          if (offset > start) && ((offset+len)<end) {
 *              range.start = offset;
 *              range.end = offset + len;
 *          }
 *      } 
 *      else 
 *      {
 *          if (offset < start) && ((offset+len) > end)
 *              range.start = start;
 *              range.end = end;
 *      }
 *  4) (start<=offset<=end) && ((offset+len)>=end)
 *      range.start = offset;
 *      range.end = end;
 *  5) offset > end
 *      skip because this range is not in the test window
 * */

        memset ((void *)&range, 0, sizeof(range_struc));

	if (p->list_info != LIST_INFO) {
           if ((offset+len) < p->start){
//	      lprintf(LOG_ERR, "The end address is little than the minimum physical address");
//            return ERR_INPUT_ADDRESS;
	      continue;
	
	   } else if ((offset<=p->start)&&(p->start<=(offset+len))&&((offset+len)<=p->end))      {
              range.start = p->start;
              range.end = offset + len;
           } else if ((offset>p->start)&&((offset+len)<p->end)) {
              range.start = offset;
              range.end = offset + len;
           } else if ((offset<=p->start)&&((offset+len)>=p->end)){
              range.start = p->start;
              range.end = p->end;
           } else if ((p->start<=offset)&&(offset<=p->end) && ((offset + len) >= p->end))    {
              range.start = offset;
              range.end = p->end;
           } else if (offset > p->end) { 
	      continue;
           } 
        } else {
            range.start = offset;
            range.end = offset + len;
        }
            
/*         if ((region[i].end) < p->start)
            continue;
        else if ((region[i].start<=p->start)&&(p->start<=(region[i].end))&&((region[i].end)<=p->end)) {
            range.start = p->start;
            range.end = region[i].end;
        } else if ((region[i].start>p->start)&&((region[i].end)<p->end)) {
            range.start = region[i].start;
            range.end = region[i].end;
        } else if ((region[i].start<p->start)&&((region[i].end)>p->end)){
            range.start = p->start;
            range.end = p->end;
        } else if ((p->start<=region[i].start)&&(region[i].start<=p->end) && ((region[i].end) >= p->end)) {
            range.start = region[i].start;
            range.end = p->end;
        } else {
            continue;
        }
*/

        raw_reg[raw_num].start_pfn = range.start;
        raw_reg[raw_num].end_pfn = range.end;
		raw_num ++;
       
        if (range.start == range.end)
            continue;

        node_n = region.node;
        socket_n = SOCKET_COUNT - 1;
       	/********************/
//        if (i == 3)
//            region[i].type = READ_WRITE;
        /************************/
        if (region.type == READ_WRITE) {

//#if 0
            /* Add range.start and range.end check for PFN alginment  */
            tmp_a = range.start;
            tmp_b = range.end;
            range.start = pfn_start_roundup(range.start, 0xfff);
            range.end =   pfn_end_roundup(range.end, 0xfff);

            raw_reg[raw_num-1].start_pfn = range.start;
            raw_reg[raw_num-1].end_pfn = range.end;

	    addr = NULL;
            if (p->list_info != LIST_INFO) {
               if ((range.start != tmp_a) || (range.end != tmp_b)) {
                  lprintf(LOG_NOTICE, "Enlarge RW range for PAGE alignment"); 
                  lprintf(LOG_NOTICE, "      [0x%lx,0x%lx) --> [0x%lx,0x%lx)", tmp_a, tmp_b, range.start, range.end); 
               }
               if (range.end < (range.start + MIN_BLOCK_SIZE)){
                  lprintf(LOG_NOTICE, "Skip RW range less than the minimum (4K)");
                  lprintf(LOG_NOTICE, "      [0x%lx,0x%lx)", range.start, range.end);
                  continue;
               }
               #ifdef DEBUG
                  printf("Before mmap, start=%lx, end=%lx\n", range.start, range.end);
               #endif
               
	       addr = (ULV *)mmap(NULL, (range.end-range.start), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, mmtest_fd, range.start);
               if (addr == (void *)-1) {
                 lperror(LOG_ERR, "Fail to mmap system call");
                 close(mmtest_fd);
                 return errno;
               }
               #ifdef DEBUG
                  printf("After mmap!\n");
               #endif 
           }
//#endif
            
           /* Add to RW_list  */
           tmp_RW = (RW_range *)malloc(sizeof(RW_range));
           INIT_LIST_HEAD(&tmp_RW->range_list);
           tmp_RW->start = (UL)addr;
           tmp_RW->end = (UL)addr + (range.end-range.start);
	   tmp_RW->phy_start = range.start;
	   tmp_RW->phy_end = range.end;
           /* for a segment bug of -d cpu9, no initialization */
           INIT_LIST_HEAD(&tmp_RW->RW_slice);

           list_add(&(tmp_RW->range_list), &(test_config.node[node_n].socket[socket_n].RW_list));

        } else {
                if (region.type == READ_ONLY) {

                /* Add to RD_list */
                tmp_RD = (RD_range *)malloc(sizeof(RD_range));
                INIT_LIST_HEAD(&tmp_RD->range_list);
                tmp_RD->start = range.start;
                tmp_RD->end = range.end;
                list_add(&(tmp_RD->range_list), &(test_config.node[node_n].socket[socket_n].RD_list));

                } else {
                lperror(LOG_ERR, "The region type is incorrect");
                //munmap((void *)addr, len);
                close(mmtest_fd);
                return errno;
                }
            }
        }
	
        lprintf(LOG_INFO, "The max system address = 0x%lx", max_phy_addr);
        lprintf(LOG_INFO, "List non-testable ranges in the user specified test window");
	if (p->start < raw_reg[raw_num-1].start_pfn)
		lprintf(LOG_INFO, "      [0x%lx,0x%lx)", p->start, raw_reg[raw_num-1].start_pfn);
        for (i=raw_num-1; i>=0; i--) {
	   if (raw_reg[i-1].start_pfn > raw_reg[i].end_pfn) {
	       lprintf(LOG_INFO, "      [0x%lx,0x%lx)", raw_reg[i].end_pfn, raw_reg[i-1].start_pfn);
	   }
        }

        if ((p->end) > max_phy_addr)
            p->end = max_phy_addr;

	if (p->end > raw_reg[0].end_pfn)
	   lprintf(LOG_INFO, "      [0x%lx,0x%lx)", raw_reg[0].end_pfn, p->end);

	if (p->list_info == LIST_INFO) {
            fprintf(stdout, "List this system memory range information: \n");
	    for (i=0; i<NODE_COUNT; i++) {
                for (j=0; j<SOCKET_COUNT; j++) {
                    fprintf(stdout, "Node %d\n", i);
                    if (list_empty(&(test_config.node[i].socket[j].RW_list))) {
                        fprintf(stdout, "       RW_range: zero\n");
                    } else {
                        list_for_each(list, &(test_config.node[i].socket[j].RW_list)) {
                           tmp_RW = list_entry(list, RW_range, range_list);
                           fprintf(stdout, "       RW_range: [0x%lx,0x%lx)\n", tmp_RW->phy_start, tmp_RW->phy_end);
                        }
                    }
                    if (list_empty(&(test_config.node[i].socket[j].RD_list))) {
                        fprintf(stdout, "       RD_range: zero\n");
                    } else {
                        list_for_each(list, &(test_config.node[i].socket[j].RD_list)) {
                           tmp_RD = list_entry(list, RD_range, range_list);
                           fprintf(stdout, "       RD_range: [0x%lx,0x%lx)\n", tmp_RD->start, tmp_RD->end);
                        }
                    }
                }
            }
            fprintf(stdout, "\n       Total physical memory: %d GB\n", (max_phy_addr - 0xc0000000) /1024/1024/1024) ;
	    release_list_resource();
            exit(0);
        }
		
        RW_empty = 0;
        RD_empty = 0;
   //     lprintf(LOG_NOTICE, "--------------------------------------------------------");
        lprintf(LOG_NOTICE, "List testing memory ranges in the user specified test window");
	for (i=0; i<NODE_COUNT; i++) {
           for (j=0; j<SOCKET_COUNT; j++) {
               lprintf(LOG_NOTICE, "Node %d", i);
               if (list_empty(&(test_config.node[i].socket[j].RW_list))) {
                   lprintf(LOG_NOTICE, "      RW_range: zero", i, j);
               } else {
                    list_for_each(list, &(test_config.node[i].socket[j].RW_list)) {
                        tmp_RW = list_entry(list, RW_range, range_list);
                        if (tmp_RW->start != tmp_RW->end) {
                            lprintf(LOG_NOTICE, "      RW_range: [0x%lx,0x%lx)", tmp_RW->phy_start, tmp_RW->phy_end);
                            RW_empty++;
                        }
                    }
               }
               if (list_empty(&(test_config.node[i].socket[j].RD_list))) {
                   lprintf(LOG_NOTICE, "      RD_range: zero", i, j);
               } else {
                    list_for_each(list, &(test_config.node[i].socket[j].RD_list)) {
                        tmp_RD = list_entry(list, RD_range, range_list);
                        if (tmp_RD->start != tmp_RD->end) {
                            lprintf(LOG_NOTICE, "      RD_range: [0x%lx,0x%lx)", tmp_RD->start, tmp_RD->end);
                            RD_empty++;
                        }
                    }
               }
            }
        } 
  //      lprintf(LOG_NOTICE, "--------------------------------------------------------");
        
        if ((RD_empty == 0)&&(RW_empty == 0)) {
            lperror(LOG_ERR, "The testable range is empty, invalid parameter");
            return ERR_INPUT_ADDRESS; 
        }

		if (p->clear_ecc == CLEAR_ECC) {
			ret = do_mce_check(GET_MCE, 0, &msr_err, &rec_err);
			if (ret != 0) {
				lperror(LOG_ERR, "Fail to clear MCE messages\n");
				exit (ERR_MCE_MSG);
			} else {
				lprintf(LOG_INFO, "Clear recorded MCE messages\n");
				exit(0);
			}
		}


    /* According to the test policy, split the test range into several test slices*/

    ret = test_policy((void *)parm);

    if (ERR_SUCCESS != ret) {
        release_munmap_close();
        release_list_resource();
        return ERR_TEST_POLICY;
      }

    return ERR_SUCCESS;
}      

/*!
 * \brief   close the test module
 * \param   module_id   the test module id
 * \param   param       the head pointer oof parameters
 * \param   length      the length of parameters
 * \return  0           success
 *          nonzero     return the error code
 */
unsigned int close_module(int module_id, void *param, int length)
{
    
    release_munmap_close();
    release_list_resource();

    return ERR_SUCCESS;
}

