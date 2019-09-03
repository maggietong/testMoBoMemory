/*!
*\file task_generator.h
* 
* Define task_generator sub-module data structures
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
* Version history
* Feb. 16, 2011    Create this file
*/

#ifndef TASK_GENERATOR_H
#define TASK_GENERATOR_H

/*
 struct list_head {
    struct list_head *prev;
    struct list_head *next;
 }
 */

#include "mmtest_intf.h"
#define MMTEST_DEV_NAME "/dev/memtest"

typedef struct _range_struc_ {
    UL start;
    UL end;
}range_struc;

typedef struct _test_slice_ {
    struct list_head slice_list;
    UL start;
    UL end;
    UL phy_start;
    int cpuid;
}test_slice;

typedef struct _RD_range_ {
    struct list_head range_list;
    UL start;
    UL end;
}RD_range;

typedef struct _RW_range_ {
    struct list_head range_list;
    UL start;
    UL end;
	UL phy_start;
	UL phy_end;
    struct list_head RW_slice;
}RW_range;

typedef struct _socket_info_ {
    int cpuid[CORE_COUNT/2];
    struct list_head RD_list;
    struct list_head RW_list;
}socket_info;

typedef struct _node_info_ {
    socket_info socket[SOCKET_COUNT];
}node_info;

typedef struct _test_config_struc_ {
    int test_item[ITEM_COUNT*2];
    UL start;
    UL end;
    int test_mode;
    node_info node[NODE_COUNT];
}test_config_struc;

typedef struct _slice_info_ {
    UL start;
    UL end;
    int cpuid;
}slice_info;

typedef struct _RW_range_info_ {
    slice_info sinfo[HALF_CORE_COUNT];
    int slice_num;
    UL start;
    UL end;
}RW_range_info;

typedef struct _info_struc_ {
    RW_range_info rinfo[RANGE_COUNT];
    int range_num;
}info_struc;

#endif /* TASK_GENERATOR_H */
