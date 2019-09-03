 /*! 
 *\file fileops.c
 *
 * Numa related operations: data parse from SRAT, construct usable nodes information from SRAT and e820
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

#include <acpi/acpi.h>
#include <acpi/actypes.h>
#include <acpi/platform/aclinux.h>
#include <acpi/acpixf.h>
#include <linux/nodemask.h>
#include <linux/acpi.h>
#include <asm/numa.h>
#include <asm/e820.h>
#include <asm/proto.h>
#include <acpi/acpi_numa.h>
#include <linux/sort.h>
#include "mmtest_intf.h"

#define MAX_ACTIVE_REGIONS 256
#define MAX_BOOTMEM_REGIONS 64

#define MMTEST_RO	0
#define MMTEST_RW	1

typedef struct bootmem_region {
	unsigned long start_pfn;
	unsigned long end_pfn;
	int nid;
	int type;
	struct list_head list;
} bootmem_region_t;

int acpi_numa_init(void);
int acpi_check_table_existence(char *id);
static int acpi_table_parse_srat(enum acpi_srat_type id,
		      acpi_table_entry_handler handler, unsigned int max_entries);
static int acpi_parse_processor_affinity(struct acpi_subtable_header * header,
			      const unsigned long end);
static int acpi_parse_memory_affinity(struct acpi_subtable_header * header,
			   const unsigned long end);
int __acpi_table_parse_entries(char *id,
			     unsigned long table_size,
			     int entry_id,
			     acpi_table_entry_handler handler,
			     unsigned int max_entries);
void acpi_numa_processor_affinity_init(struct acpi_srat_cpu_affinity *pa);
void acpi_numa_memory_affinity_init(struct acpi_srat_mem_affinity *ma);
static int conflicting_memblks(unsigned long start, unsigned long end);
static int setup_node(int pxm);
void __acpi_map_pxm_to_node(int pxm, int node);
int acpi_map_pxm_to_node(int pxm);
int node_to_pxm(int node);
void add_active_range(unsigned int nid, unsigned long start_pfn,
				  unsigned long end_pfn);

//Functions from e820_kernel.c				  
extern void e820_register_active_regions(int nid, unsigned long start_pfn,
				 unsigned long last_pfn);
extern int e820_find_active_region(const struct e820entry *ei,
				  unsigned long start_pfn,
				  unsigned long last_pfn,
				  unsigned long *ei_startpfn,
				  unsigned long *ei_endpfn);

static nodemask_t nodes_found_map = NODE_MASK_NONE;

/* maps to convert between proximity domain and logical node ID */
static int pxm_to_node_map[MAX_PXM_DOMAINS]
				= { [0 ... MAX_PXM_DOMAINS - 1] = NID_INVAL };
static int node_to_pxm_map[MAX_NUMNODES]
				= { [0 ... MAX_NUMNODES - 1] = PXM_INVAL };
				
static struct bootnode nodes[MAX_NUMNODES];
static int num_node_memblks;
static struct bootnode node_memblk_range[NR_NODE_MEMBLKS];
static int memblk_nodeid[NR_NODE_MEMBLKS];

static nodemask_t nodes_parsed;

bootmem_region_t bootmem_regions[MAX_BOOTMEM_REGIONS];
int bregion_num;

extern struct mye820map mye820;

struct node_active_region  early_node_map[MAX_ACTIVE_REGIONS];

static int nr_nodemap_entries;

int acpi_numa_init(void)
{
	/* SRAT: Static Resource Affinity Table */
	if (acpi_check_table_existence(ACPI_SIG_SRAT)) {
	
		acpi_table_parse_srat(ACPI_SRAT_TYPE_CPU_AFFINITY,
				      acpi_parse_processor_affinity, NR_CPUS);
					  
		acpi_table_parse_srat(ACPI_SRAT_TYPE_MEMORY_AFFINITY,
				      acpi_parse_memory_affinity,
				      NR_NODE_MEMBLKS);
	}
	
	return 0;
}

static int cmp_node_active_region(const void *a, const void *b)
{
	struct node_active_region *arange = (struct node_active_region *)a;
	struct node_active_region *brange = (struct node_active_region *)b;

	/* Done this way to avoid overflows */
	if (arange->start_pfn > brange->start_pfn)
		return 1;
	if (arange->start_pfn < brange->start_pfn)
		return -1;
	return 0;
}
int generate_node_map_from_e820_and_SRAT(void)
{
   int i;
   #ifdef DEBUG
      printk("num node memblks=%d\n", num_node_memblks);
   #endif
   for ( i = 0; i < num_node_memblks; i ++ ) {
      e820_register_active_regions(memblk_nodeid[i], node_memblk_range[i].start >> PAGE_SHIFT, node_memblk_range[i].end >> PAGE_SHIFT);
   }
   //debug for sort function
   /*early_node_map[100]=early_node_map[0];
   early_node_map[0]=early_node_map[1];
   early_node_map[1]=early_node_map[100];*/
   if ( num_node_memblks == 0) {
      for ( i = 0; i < mye820.nr_map; i ++) {
	     e820_register_active_regions(0, mye820.map[i].addr >> PAGE_SHIFT, (mye820.map[i].addr + mye820.map[i].size) >> PAGE_SHIFT);
	  }
   }
   
   sort(early_node_map, (size_t)nr_nodemap_entries, sizeof(struct node_active_region), cmp_node_active_region, NULL);
  
   #ifdef DEBUG
      for ( i = 0; i <  nr_nodemap_entries; i ++ ) {
         printk("nodeid=%d, start_pfn=%lx, end_pfn=%lx\n", early_node_map[i].nid, early_node_map[i].start_pfn, early_node_map[i].end_pfn);
      }
   #endif
   return 0;
}

int generate_bootmem_region(unsigned long long memLimit)
{
   //initial check needed here, will add later

   int i, count;
   unsigned long memLimit_pfn = memLimit >> PAGE_SHIFT;
   //for debug
   //memLimit_pfn=0x7e5b7; 

   for ( i = 0, count = 0; i <  nr_nodemap_entries; i ++ ) {
      if( (early_node_map[i].end_pfn <= memLimit_pfn) ||  (early_node_map[i].start_pfn >= memLimit_pfn) ) {
	     if ( count >= MAX_BOOTMEM_REGIONS ) {
		    printk("Too many regions\n"); 
		    bregion_num = MAX_BOOTMEM_REGIONS;
			return -1;
	     }

	     bootmem_regions[count].start_pfn= early_node_map[i].start_pfn;
	     bootmem_regions[count].end_pfn= early_node_map[i].end_pfn;
	     bootmem_regions[count].nid= early_node_map[i].nid;
		 if ( early_node_map[i].end_pfn <= memLimit_pfn ) {
     		 bootmem_regions[count].type= MMTEST_RO;
	     } else {
     		 bootmem_regions[count].type= MMTEST_RW;
		 }
		 count ++;
	  } else {
	     if ( count >= MAX_BOOTMEM_REGIONS ) {
		    printk("Too many regions\n");
		    bregion_num = MAX_BOOTMEM_REGIONS;
		    return -1;
	     }

		 bootmem_regions[count].start_pfn=early_node_map[i].start_pfn;
		 bootmem_regions[count].end_pfn= memLimit_pfn;
	     bootmem_regions[count].nid= early_node_map[i].nid;
		 bootmem_regions[count].type= MMTEST_RO;
		 count ++;

	     if ( count >= MAX_BOOTMEM_REGIONS ) {
		    printk("Too many regions\n"); 
		    bregion_num = MAX_BOOTMEM_REGIONS;
		    return -1;
	     }
		 
         bootmem_regions[count].start_pfn = memLimit_pfn;
		 bootmem_regions[count].end_pfn= early_node_map[i].end_pfn;
	     bootmem_regions[count].nid= early_node_map[i].nid;
		 bootmem_regions[count].type= MMTEST_RW;
		 count ++;
	  }
   }
   
   bregion_num = count;
   #ifdef DEBUG
      printk("bootmem_region=%d:\n", count);
      for ( i = 0; i < count; i ++)
         printk("Entry %d, start_pfn=%lx, end_pfn=%lx, nid=%d, type=%d\n", i, bootmem_regions[i].start_pfn, bootmem_regions[i].end_pfn, bootmem_regions[i].nid, bootmem_regions[i].type);
   #endif
   return 0;
}

//not for MADT as it has several instances
int acpi_check_table_existence(char *id)
{
	struct acpi_table_header *table = NULL;
	acpi_get_table(id, 0, &table);
	if (table) {
		return 1;
	} else
		return 0;
}


static int acpi_table_parse_srat(enum acpi_srat_type id,
		      acpi_table_entry_handler handler, unsigned int max_entries)
{
	return __acpi_table_parse_entries(ACPI_SIG_SRAT,
					    sizeof(struct acpi_table_srat), id,
					    handler, max_entries);
}

static int acpi_parse_processor_affinity(struct acpi_subtable_header * header,
			      const unsigned long end)
{
	struct acpi_srat_cpu_affinity *processor_affinity;

	processor_affinity = (struct acpi_srat_cpu_affinity *)header;
	if (!processor_affinity)
		return -EINVAL;

	//acpi_table_print_srat_entry(header);

	/* let architecture-dependent part to do it */
	acpi_numa_processor_affinity_init(processor_affinity);

	return 0;
}

static int acpi_parse_memory_affinity(struct acpi_subtable_header * header,
			   const unsigned long end)
{
	struct acpi_srat_mem_affinity *memory_affinity;

	memory_affinity = (struct acpi_srat_mem_affinity *)header;
	if (!memory_affinity)
		return -EINVAL;

	//acpi_table_print_srat_entry(header);

	/* let architecture-dependent part to do it */
	acpi_numa_memory_affinity_init(memory_affinity);

	return 0;
}

int __acpi_table_parse_entries(char *id,
			     unsigned long table_size,
			     int entry_id,
			     acpi_table_entry_handler handler,
			     unsigned int max_entries)
{
	struct acpi_table_header *table_header = NULL;
	struct acpi_subtable_header *entry;
	unsigned int count = 0;
	unsigned long table_end;

	if (!handler)
		return -EINVAL;

	/*if (strncmp(id, ACPI_SIG_MADT, 4) == 0)
		acpi_get_table(id, acpi_apic_instance, &table_header);
	else*/
		acpi_get_table(id, 0, &table_header);

	if (!table_header) {
		printk(KERN_WARNING "%4.4s not present\n", id);
		return -ENODEV;
	}

	table_end = (unsigned long)table_header + table_header->length;

	/* Parse all entries looking for a match. */

	entry = (struct acpi_subtable_header *)
	    ((unsigned long)table_header + table_size);

	while (((unsigned long)entry) + sizeof(struct acpi_subtable_header) <
	       table_end) {
		if (entry->type == entry_id
		    && (!max_entries || count++ < max_entries))
			if (handler(entry, table_end))
				return -EINVAL;

		entry = (struct acpi_subtable_header *)
		    ((unsigned long)entry + entry->length);
	}
	if (max_entries && count > max_entries) {
		printk(KERN_WARNING "[%4.4s:0x%02x] ignored %i entries of "
		       "%i found\n", id, entry_id, count - max_entries, count);
	}

	return count;
}

/* Callback for Proximity Domain -> LAPIC mapping */
void acpi_numa_processor_affinity_init(struct acpi_srat_cpu_affinity *pa)
{
	int pxm, node;

	/*if (srat_disabled())
		return;*/
	if (pa->header.length != sizeof(struct acpi_srat_cpu_affinity)) {
		//bad_srat();
		printk("Bad SRAT\n");
		return;
	}
	if ((pa->flags & ACPI_SRAT_CPU_ENABLED) == 0)
		return;
	pxm = pa->proximity_domain_lo;
	node = setup_node(pxm);
	if (node < 0) {
		printk(KERN_ERR "SRAT: Too many proximity domains %x\n", pxm);
		//bad_srat();
		return;
	}

/*	if (get_uv_system_type() >= UV_X2APIC)
		apic_id = (pa->apic_id << 8) | pa->local_sapic_eid;
	else
		apic_id = pa->apic_id;
	apicid_to_node[apic_id] = node;
	acpi_numa = 1;
	printk(KERN_INFO "SRAT: PXM %u -> APIC %u -> Node %u\n",
	       pxm, apic_id, node);*/
}


/* Callback for parsing of the Proximity Domain <-> Memory Area mappings */
void acpi_numa_memory_affinity_init(struct acpi_srat_mem_affinity *ma)
{
	struct bootnode *nd;
	unsigned long start, end;
	int node, pxm;
	int i;

	/*if (srat_disabled())
		return;*/
	if (ma->header.length != sizeof(struct acpi_srat_mem_affinity)) {
		//bad_srat();
		return;
	}
	if ((ma->flags & ACPI_SRAT_MEM_ENABLED) == 0)
		return;

	/*if ((ma->flags & ACPI_SRAT_MEM_HOT_PLUGGABLE) && !save_add_info())
		return;*/
	start = ma->base_address;
	end = start + ma->length;
	pxm = ma->proximity_domain;
	node = setup_node(pxm);
	if (node < 0) {
		printk(KERN_ERR "SRAT: Too many proximity domains.\n");
		//bad_srat();
		return;
	}
	i = conflicting_memblks(start, end);
	if (i == node) {
		printk(KERN_WARNING
		"SRAT: Warning: PXM %d (%lx-%lx) overlaps with itself (%Lx-%Lx)\n",
			pxm, start, end, nodes[i].start, nodes[i].end);
	} else if (i >= 0) {
		printk(KERN_ERR
		       "SRAT: PXM %d (%lx-%lx) overlaps with PXM %d (%Lx-%Lx)\n",
		       pxm, start, end, node_to_pxm(i),
			nodes[i].start, nodes[i].end);
		//bad_srat();
		return;
	}
	nd = &nodes[node];
	//oldnode = *nd;
	if (!node_test_and_set(node, nodes_parsed)) {
		nd->start = start;
		nd->end = end;
	} else {
		if (start < nd->start)
			nd->start = start;
		if (nd->end < end)
			nd->end = end;
	}
    #ifdef DEBUG
		printk(KERN_INFO "SRAT: Node %u PXM %u %lx-%lx\n", node, pxm,
	       start, end);
    #endif
	//e820_register_active_regions(node, start >> PAGE_SHIFT,
				    // end >> PAGE_SHIFT);
	//push_node_boundaries(node, nd->start >> PAGE_SHIFT,
						//nd->end >> PAGE_SHIFT);

	/*if ((ma->flags & ACPI_SRAT_MEM_HOT_PLUGGABLE) &&
	    (reserve_hotadd(node, start, end) < 0)) {
		// Ignore hotadd region. Undo damage 
		printk(KERN_NOTICE "SRAT: Hotplug region ignored\n");
		*nd = oldnode;
		if ((nd->start | nd->end) == 0)
			node_clear(node, nodes_parsed);
	}*/

	node_memblk_range[num_node_memblks].start = start;
	node_memblk_range[num_node_memblks].end = end;
	memblk_nodeid[num_node_memblks] = node;
	num_node_memblks++;
}

static int conflicting_memblks(unsigned long start, unsigned long end)
{
	int i;
	for (i = 0; i < num_node_memblks; i++) {
		struct bootnode *nd = &node_memblk_range[i];
		if (nd->start == nd->end)
			continue;
		if (nd->end > start && nd->start < end)
			return memblk_nodeid[i];
		if (nd->end == end && nd->start == start)
			return memblk_nodeid[i];
	}
	return -1;
}

static int setup_node(int pxm)
{
	return acpi_map_pxm_to_node(pxm);
}

void __acpi_map_pxm_to_node(int pxm, int node)
{
	pxm_to_node_map[pxm] = node;
	node_to_pxm_map[node] = pxm;
}

int acpi_map_pxm_to_node(int pxm)
{
	int node = pxm_to_node_map[pxm];

	if (node < 0){
		if (nodes_weight(nodes_found_map) >= MAX_NUMNODES)
			return NID_INVAL;
		node = first_unset_node(nodes_found_map);
		__acpi_map_pxm_to_node(pxm, node);
		node_set(node, nodes_found_map);
	}

	return node;
}

/*
 * Finds an active region in the address range from start_pfn to last_pfn and
 * returns its range in ei_startpfn and ei_endpfn for the e820 entry.
 */


/**
 * add_active_range - Register a range of PFNs backed by physical memory
 * @nid: The node ID the range resides on
 * @start_pfn: The start PFN of the available physical memory
 * @end_pfn: The end PFN of the available physical memory
 *
 * These ranges are stored in an early_node_map[] and later used by
 * free_area_init_nodes() to calculate zone sizes and holes. If the
 * range spans a memory hole, it is up to the architecture to ensure
 * the memory is not freed by the bootmem allocator. If possible
 * the range being registered will be merged with existing ranges.
 */
void add_active_range(unsigned int nid, unsigned long start_pfn,
						unsigned long end_pfn)
{
	int i;

	/*mminit_dprintk(MMINIT_TRACE, "memory_register",
			"Entering add_active_range(%d, %#lx, %#lx) "
			"%d entries of %d used\n",
			nid, start_pfn, end_pfn,
			nr_nodemap_entries, MAX_ACTIVE_REGIONS);

	mminit_validate_memmodel_limits(&start_pfn, &end_pfn);*/

	/* Merge with existing active regions if possible */
	for (i = 0; i < nr_nodemap_entries; i++) {
		if (early_node_map[i].nid != nid)
			continue;

		/* Skip if an existing region covers this new one */
		if (start_pfn >= early_node_map[i].start_pfn &&
				end_pfn <= early_node_map[i].end_pfn)
			return;

		/* Merge forward if suitable */
		if (start_pfn <= early_node_map[i].end_pfn &&
				end_pfn > early_node_map[i].end_pfn) {
			early_node_map[i].end_pfn = end_pfn;
			return;
		}

		/* Merge backward if suitable */
		if (start_pfn < early_node_map[i].end_pfn &&
				end_pfn >= early_node_map[i].start_pfn) {
			early_node_map[i].start_pfn = start_pfn;
			return;
		}
	}

	/* Check that early_node_map is large enough */
	if (i >= MAX_ACTIVE_REGIONS) {
		printk(KERN_CRIT "More than %d memory regions, truncating\n",
							MAX_ACTIVE_REGIONS);
		return;
	}

	early_node_map[i].nid = nid;
	early_node_map[i].start_pfn = start_pfn;
	early_node_map[i].end_pfn = end_pfn;
	nr_nodemap_entries = i + 1;
}
int node_to_pxm(int node)
{
	if (node < 0)
			return PXM_INVAL;
	return node_to_pxm_map[node];
}

int get_bregion_num(void)
{
	return bregion_num;
}

int get_bregion(int bregion_idx, unsigned long *start_pfn, unsigned long *end_pfn, int *nid, int *type)
{
	if (bregion_idx >= bregion_num) {
		return -1;
	}
	
	*start_pfn = bootmem_regions[bregion_idx].start_pfn;
	*end_pfn = bootmem_regions[bregion_idx].end_pfn;
	*nid = bootmem_regions[bregion_idx].nid;
	*type = bootmem_regions[bregion_idx].type;
	return 0;
}

