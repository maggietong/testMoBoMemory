 /*! 
 *\file e820_kernel.c
 *
 * Kernel mode e820 operations.
 *
 * Author: Jackie Liu
 *
 * Copyright (c) 2012 Jabil Circuit.
 *
 * This source code and any compilation or derivative thereof is the sole
 * property of Jabil Circuit and is provided pursuant to a Software License
 * Agreement. This code is the proprietary information of Jabil Circuit and
 * is confidential in nature. Its use and dissemination by any party other
 * than Jabil Circuit is strictly limited by the confidential information
 * provisions of Software License Agreement referenced above.
 
 *\par ChangeLog:
 * July 25, 2012      The initial version#include <asm/types.h>
  */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/gfp.h>
#include <linux/device.h>

#include <linux/types.h>
#include <linux/fs.h>

//#include <linux/uaccess.h>

#include <linux/mm.h>

#include <asm/mmzone.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/slab.h>

#include <linux/ioport.h>
#include <asm/e820.h>
#include <asm/io.h>
#include <linux/numa.h>
#include <asm/proto.h>
#include "mmtest_intf.h"

extern struct mye820map mye820;
extern struct node_active_region  early_node_map[];

extern void add_active_range(unsigned int nid, unsigned long start_pfn,
						unsigned long end_pfn);
int e820_find_active_region(const struct e820entry *ei,
				  unsigned long start_pfn,
				  unsigned long last_pfn,
				  unsigned long *ei_startpfn,
				  unsigned long *ei_endpfn);

/* Walk the e820 map and register active regions within a node */
void e820_register_active_regions(int nid, unsigned long start_pfn,
					 unsigned long last_pfn)
{
	unsigned long ei_startpfn;
	unsigned long ei_endpfn;
	int i;

	for (i = 0; i < mye820.nr_map; i++)
		if (e820_find_active_region(&mye820.map[i],
					    start_pfn, last_pfn,
					    &ei_startpfn, &ei_endpfn))
			add_active_range(nid, ei_startpfn, ei_endpfn);
}

/*
 * Finds an active region in the address range from start_pfn to last_pfn and
 * returns its range in ei_startpfn and ei_endpfn for the e820 entry.
 */
int e820_find_active_region(const struct e820entry *ei,
				  unsigned long start_pfn,
				  unsigned long last_pfn,
				  unsigned long *ei_startpfn,
				  unsigned long *ei_endpfn)
{
	u64 align = PAGE_SIZE;

	*ei_startpfn = round_up(ei->addr, align) >> PAGE_SHIFT;
	*ei_endpfn = round_down(ei->addr + ei->size, align) >> PAGE_SHIFT;

	/* Skip map entries smaller than a page */
	if (*ei_startpfn >= *ei_endpfn)
		return 0;

	/* Skip if map is outside the node */
	if (ei->type != E820_RAM || *ei_endpfn <= start_pfn ||
				    *ei_startpfn >= last_pfn)
		return 0;

	/* Check for overlaps */
	if (*ei_startpfn < start_pfn)
		*ei_startpfn = start_pfn;
	if (*ei_endpfn > last_pfn)
		*ei_endpfn = last_pfn;

	return 1;
}
