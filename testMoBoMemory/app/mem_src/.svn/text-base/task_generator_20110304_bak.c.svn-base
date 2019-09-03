/*!
*\file task_generator.c
* 
*\brief  Get the physical memory layout information with memory region attributes and then split memory ranges into several test slices according to the test policy inputted by user
*
*\author Maggie Tong
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

#include "jedi_comm.h"
#include "log.h"
#include "memory.h"
#include "list.h"
#include "mem_err.h"
#include "task_generator.h"
    
test_config_struc test_config;

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
    ULL range_size = 0;
    ULL slice_num = 0;
    int core_num = 0;
    test_parm *p;
    int spec_cpu = 0;
    int **table[NODE_COUNT];
    ULL n = 0;
    ULL step_size = 0;
    test_slice *tmp_slice;

    p = (test_parm *)parm;
    core_num = sysconf(_SC_NPROCESSORS_CONF);
    if ((core_num == 0)||(core_num > CORE_COUNT)) {
        lprintf(LOG_ERR, "The read core count is over the supporting MAX_CORE_NUM");
        return ERR_CORE_NUM;
    }
    /* Set the cpu_core allocation about Node, cpuid  */
    core_num = 8;
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
        p->mode = SPECIFIC;
    }
    switch (p->mode) {
        case LOCAL:
            lprintf(LOG_INFO, "Hello, it is LOCAL mode");
            table[0] = create_LOCAL_table(core_num/2);
            table[1] = create_REMOTE_table(core_num/2);

            break;
        case REMOTE:
            table[0] = create_REMOTE_table(core_num/2);
            table[1] = create_LOCAL_table(core_num/2);
            break;

        case ALL:
//            table = create_LOCAL_table(core_num);
            break;

        case SPECIFIC:
            table[0] = create_SPECIFIC_table(core_num/2, spec_cpu);
            table[1] = create_SPECIFIC_table(core_num/2, spec_cpu);
            break;
        default:
            break;
    }

    for (i=0; i<NODE_COUNT; i++) {
        for (j=0; j<SOCKET_COUNT; j++) {
            list_for_each(list_a, &(test_config.node[i].socket[j].RW_list)) {
                tmp_RW = list_entry(list_a, RW_range, range_list);
                INIT_LIST_HEAD(&tmp_RW->RW_slice);
                lprintf(LOG_INFO, "The node is %d", i);
                lprintf(LOG_INFO, "RW_range: the start is %lx, the end is %lx", tmp_RW->start, tmp_RW->end);
                range_size = tmp_RW->end - tmp_RW->start;
                slice_num = range_size/BLOCK_SIZE + 1;
                step_size = BLOCK_SIZE;
                if (slice_num > (core_num/2)) {
                    slice_num = (core_num/2);
                    step_size = range_size/(core_num/2);
                }
                /* According to the test policy, split the test range into several test slices*/
                for (n=0; n<slice_num; n++) {
                    tmp_slice = (test_slice *)malloc(sizeof(test_slice));
                    INIT_LIST_HEAD(&tmp_slice->slice_list);
                    tmp_slice->start = tmp_RW->start + n*step_size;
                    if (tmp_RW->end >= (tmp_slice->start + step_size))
                        tmp_slice->end = tmp_slice->start + step_size; 
                    else
                        tmp_slice->end = tmp_RW->end;
                    tmp_slice->cpuid = table[i][slice_num-1][n];
                    list_add(&(tmp_slice->slice_list), &(tmp_RW->RW_slice));

                }
                list_for_each(list_b, &(tmp_RW->RW_slice)) {
                    tmp_slice = list_entry(list_b, test_slice, slice_list);
                    lprintf(LOG_DEBUG, "test_slice: the start is %lx, the end is %lx, the cpuid is cpu%d", tmp_slice->start, tmp_slice->end, tmp_slice->cpuid);
                }
                
            }
        }
    }
    return ret;
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
    FILE *fp = NULL;
    int region_num = 0;
    int i = 0;
    int j = 0;
    int node_n = 0;
    int socket_n = 0;
    unsigned ret = ERR_SUCCESS;

    region_t region[10] = {
        {0,10,300,'w',0},
        {1,400,800,'r',0},
        {2,900,1000,'w',1},
        {3,1100,2000,'w',1},
        {4,2100,2500,'r',0},
        {5,2600,3000,'w',1},
        {6,4000,8800000,'w',0},
        {7,9000000,10000000,'w',1},
        {8,11000000,12000000,'w',0},
        {9,13000000,15000000,'r',1},
    };
    ULL offset = 0;
    ULL len = 0;
    ULV *addr = NULL;

    range_struc range;
    RW_range *tmp_RW;
    RD_range *tmp_RD;
    struct list_head *list;
    test_parm *p;
    mem_info mem;

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
        lprintf(LOG_ERR, "Fail to get MemTotal from /proc");
        return ERR_GET_MEM_INFO;
    }

    if ((p->start) >= (p->end)){
        lprintf(LOG_ERR, "The specified start address is bigger than the end address.");
        return ERR_INPUT_ADDRESS;
    }

    if ((p->start)>(mem.total * 1024 *1024)) {
        lprintf(LOG_ERR, "The start address is over the maximum physical address");
        return ERR_INPUT_ADDRESS;
    }
    
    /* p->end couldn't bigger than the maximum physical address  */

/********************************/
//    if (((mem.total*1024*1024)<=(p->end)) && ((mem.total*1024*1024)>=p->start)) {
//        p->end = mem.total*1024*1024;
//    }
/************************************/
    /* Initial the global data and lists */
    for (i=0; i<NODE_COUNT; i++) {
        for (j=0; j<SOCKET_COUNT; j++) {
            init_RD_RW_list(&(test_config.node[i].socket[j]));
        }
    }


    /* Get memroy regions information depending on Driver mem_test  */
#if 0
    fd = open(DEV_NAME, O_RDWR|O_SYNC);
    if (-1 == fd) {
        lperror(LOG_ERR, "Fail to open %s file", DEV_NAME);
        return errno; 
    }
    if (ERR_SUCCESS != ioctl(fd, GET_BREGION_NUM, &region_num)) {
        lperror(LOG_ERR, "Fail to ioctl system call");
        return errno;
    }
    lprintf(LOG_DEBUG, "Region number is %d.", region_num) ;
#endif

    region_num = 10;
    for (i=region_num-1; i>=0 ; i--) {
        region[i].index = i;
#if 0
        if (ERR_SUCCESS != ioctl(fd, GET_BREGION, &region)) {
            lperror(LOG_ERR, "Fail to ioctl system call");
            return errno;
        }
#endif
        offset = region[i].start_pfn * PAGE_SIZE;
        len = (region[i].end_pfn - region[i].start_pfn) * PAGE_SIZE;
        if (len <=0 ) {
            lprintf(LOG_ERR, "The error region length (less than 0)");
            ret = ERR_INPUT_ADDRESS;
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
        if ((offset+len) < p->start)
            continue;
        else if ((offset<=p->start)&&(p->start<=(offset+len))&&((offset+len)<=p->end)) {
            range.start = p->start;
            range.end = offset + len;
        } else if ((offset>p->start)&&((offset+len)<p->end)) {
            range.start = offset;
            range.end = offset + len;
        } else if ((offset<p->start)&&((offset+len)>p->end)){
            range.start = p->start;
            range.end = p->end;
        } else if ((p->start<=offset)&&(offset<=p->end) && ((offset + len) >= p->end)) {
            range.start = offset;
            range.end = p->end;
        } else {
            continue;
        }        
            
        lprintf(LOG_DEBUG, "Region %d, start_pfn %d, end_pfn %d.", i, region[i].start_pfn, region[i].end_pfn);
        lprintf(LOG_DEBUG, "    The offset is %lx, the size is %lx", offset, len);
        
        node_n = region[i].node;
        socket_n = SOCKET_COUNT - 1;
        if (region[i].type == 'w') {
            lprintf(LOG_DEBUG, "The region type is Read/Write");
#if 0
            addr = (ULV *)mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, fd, offset);
            if (addr == (unsigned long)(void *)-1) {
                lperror(LOG_ERR, "Fail to mmap system call");
                return errno;
            }
#endif
            lprintf(LOG_DEBUG, "Addr is %lx", addr);
            
            /* Add to RW_list  */
            tmp_RW = (RW_range *)malloc(sizeof(RW_range));
            INIT_LIST_HEAD(&tmp_RW->range_list);
            tmp_RW->start = range.start;
            tmp_RW->end = range.end;
            list_add(&(tmp_RW->range_list), &(test_config.node[node_n].socket[socket_n].RW_list));

            } else {
                if (region[i].type == 'r') {
                lprintf(LOG_DEBUG, "The region type is Read-only");

                /* Add to RD_list */
                tmp_RD = (RD_range *)malloc(sizeof(RD_range));
                INIT_LIST_HEAD(&tmp_RD->range_list);
                tmp_RD->start = range.start;
                tmp_RD->end = range.end;
                list_add(&(tmp_RD->range_list), &(test_config.node[node_n].socket[socket_n].RD_list));

                } else {
                lperror(LOG_ERR, "The region type is incorrect");
                return errno;
                }
            }
        }

        for (i=0; i<NODE_COUNT; i++) {
            for (j=0; j<SOCKET_COUNT; j++) {
                list_for_each(list, &(test_config.node[i].socket[j].RW_list)) {
                    tmp_RW = list_entry(list, RW_range, range_list);
                    lprintf(LOG_INFO, "The node is %d", i);
                    lprintf(LOG_INFO, "RW_range: the start is %lx, the end is %lx", tmp_RW->start, tmp_RW->end);
                }
                list_for_each(list, &(test_config.node[i].socket[j].RD_list)) {
                    tmp_RD = list_entry(list, RD_range, range_list);
                    lprintf(LOG_INFO, "RD_range: the start is %lx, the end is %lx", tmp_RD->start, tmp_RD->end);
                }
            }
            lprintf(LOG_INFO, "\n");
        }

    /* According to the test policy, split the test range into several test slices*/

    ret = test_policy((void *)parm);


    return ERR_SUCCESS;
}
