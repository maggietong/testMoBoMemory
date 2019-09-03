#ifndef __MMTEST_INTF__
#define __MMTEST_INTF__
#include <linux/types.h>
#include <asm/e820.h>

typedef struct region {
	int index;
	unsigned long start_pfn;
	unsigned long end_pfn;
	int node;
	int type;
} region_t;


struct mye820map {
       __u32 nr_map;
       struct e820entry map[E820MAX];
};

#define MMTEST_MAGIC	'M'
#define GET_BREGION_NUM	_IOR(MMTEST_MAGIC, 1, int)
#define GET_BREGION	_IOWR(MMTEST_MAGIC, 2, region_t)
#define MEMORY_SET_E820_INFO _IOWR(MMTEST_MAGIC, 3, struct e820map)
#define MEMORY_SET_MEM_INFO _IOWR(MMTEST_MAGIC, 4, unsigned long long) 
#define FLUSH_CACHE	_IO(MMTEST_MAGIC, 5)

extern int get_bregion_num(void);
extern int get_bregion(int index, unsigned long *start_pfn, unsigned long *end_pfn, int *nid, int *type);
extern int set_bregion(int index, unsigned long start_pfn, unsigned long end_pfn, int nid, int type);

#endif //__MMTEST_INTF__
