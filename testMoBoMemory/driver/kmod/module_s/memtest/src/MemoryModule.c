 /*! 
 *\file fileops.c
 *
 * Main module for memory test.
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
#include <asm/mtrr.h>

#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/ioport.h>
//#include <asm/e820.h>
#include <asm/io.h>
#include <linux/numa.h>
#include <linux/ioctl.h>

#include "mmtest_intf.h"
//#define DEBUG
#define MODULE_NAME "memtest"

#define MEMORY_TEST  "MemoryTest"

MODULE_DESCRIPTION("Memory Test module");
MODULE_AUTHOR("Jackie (jackie.liu2@jabil.com)");
MODULE_LICENSE("GPL");

 
//unsigned int __invalid_size_argument_for_IOC;
extern int acpi_numa_init(void);

extern int generate_bootmem_region(unsigned long long memLimit);

extern int generate_node_map_from_e820_and_SRAT(void);

struct mye820map mye820;
__u64 memLimit=0;

extern struct mtrr_state_type mtrr_state;

static int mmtest_open (struct inode *inode, struct file *file) {
	printk("mmtest_open\n");
	return 0;
}

static int mmtest_release( struct inode *i, struct file *f)
{
    kfree(f->private_data);
    return 0;
}

static ssize_t mmtest_read( struct file *f, char  *buffer, size_t len, loff_t *off)
{
    return 0;
}

static ssize_t mmtest_write( struct file *f, const char *buffer, size_t len, loff_t *off)
{
    return 0;
}


static int mmtest_mmap(struct file *fp, struct vm_area_struct *vma) {
	int ret;
	unsigned int start_pfn = vma->vm_pgoff;
	unsigned int end_pfn = start_pfn + ((vma->vm_end - vma->vm_start) >> PAGE_SHIFT);
	int i;
        unsigned long vm_pgoff = vma->vm_pgoff;
        u64 phys_start, phys_end;

        unsigned long vm_start_original, vm_end_original;

        #ifdef DEBUG
              struct mm_struct *mm = vma->vm_mm;
              pgd_t *pgd;
              pud_t *pud;
              pmd_t *pmd;
              pte_t *ptep, pte;
        #endif
 
        phys_start = (u64)start_pfn << PAGE_SHIFT;
        phys_end = (u64)end_pfn << PAGE_SHIFT;

        vm_start_original = vma->vm_start;
        vm_end_original = vma->vm_end;

	printk("mmap pages [%lx-%lx]..., prot %lx, vma_start %lx, vma_end %lx\n", start_pfn, end_pfn, vma->vm_page_prot, vma->vm_start, vma->vm_end);
        #ifdef DEBUG
           printk("mmap pages: vma_flags=%lx\n", vma->vm_flags);
        #endif

        //vma->vm_flags |= VM_PFN_AT_MMAP;
	//for (i = start_pfn; i < end_pfn; i++) 
          {
                /*if ( !mtrr_state.enabled )
                   return 0xFF;*/ 
                for ( i = 0; i < MTRR_MAX_VAR_RANGES; i ++ ) {
                   u64 mtrr_start, mtrr_end, mask;
                   unsigned int j, map_start_pfn, map_end_pfn;

                   if (!( mtrr_state.var_ranges[i].mask_lo & (1 << 11)))
                       continue;
                   mtrr_start = (((u64)mtrr_state.var_ranges[i].base_hi) << 32)+(mtrr_state.var_ranges[i].base_lo & PAGE_MASK);
                   mask = (((u64)mtrr_state.var_ranges[i].mask_hi) << 32) +
		       (mtrr_state.var_ranges[i].mask_lo & PAGE_MASK);

                   for ( j = 12; j < 64; j ++ ) {
                      if( mask & ( (u64)1 << j ) )
                         break; 
                   }
                   mtrr_end = mtrr_start + ( (u64)1 << j );
                   
                   #ifdef DEBUG
                      printk("mtrr_start=%lx, mtrr_end=%lx\n", mtrr_start, mtrr_end);
                   #endif

                   if( ( phys_start >= mtrr_end ) || ( phys_end <= mtrr_start ) )
                      continue;
                   if( phys_start > mtrr_start )
                      map_start_pfn = phys_start >> PAGE_SHIFT;
                   else 
                      map_start_pfn = mtrr_start >> PAGE_SHIFT;

                   if( phys_end > mtrr_end )
                      map_end_pfn = mtrr_end >> PAGE_SHIFT;
                   else 
                      map_end_pfn = phys_end >> PAGE_SHIFT;
                   
                   vma->vm_start= vma->vm_start + ((u64)(map_start_pfn -start_pfn) << PAGE_SHIFT);
                   vma->vm_end= vma->vm_start + ((u64) (map_end_pfn - map_start_pfn ) << PAGE_SHIFT);
                   vma->vm_pgoff = map_start_pfn;
	           
                   #ifdef DEBUG
                      printk("Mapping VM start %lx, end %lx, start_pfn %lx, end_pfn %lx\n", vma->vm_start, vma->vm_end, map_start_pfn, map_end_pfn);
                   #endif

                   //do we need to check inconsistence between mtrr and e820/SRAT???
		   ret = remap_pfn_range(vma,
			                 vma->vm_start,
					 map_start_pfn,
					 (u64)(map_end_pfn - map_start_pfn) << PAGE_SHIFT,
					 vma->vm_page_prot);
                   vma->vm_start =  vm_start_original;
                   vma->vm_end = vm_end_original;
                   vma->vm_pgoff = vm_pgoff ;
 
                   if ( ret < 0 ) {
			printk("remap_pfn_range fails at pfn %x ret %d.\n", start_pfn, ret);
		    	return ret;
		   }
                }
                printk("mmap pages: vma_flags=%lx\n", vma->vm_flags);
	}
        
        printk("mmtest_mmap return.\n");
    #ifdef DEBUG
        pgd = pgd_offset(mm,  vma->vm_start );
        printk("mmtest_remap pgd=%lx\n", (unsigned long)pgd);
        pud = pud_offset(pgd, vma->vm_start );
        printk("mmtest_remap pud=%lx\n", (unsigned long)pud);               
        pmd = pmd_offset(pud, vma->vm_start );
        printk("mmtest_remap pmd=%lx\n", (unsigned long)pmd);
              
        ptep= pte_offset_kernel(pmd, vma->vm_start);
        printk("mmtest_remap ptep=%lx\n", (unsigned long)ptep);
        pte= *ptep;
        printk("mmtest_remap flag=%lx", pte);                 
    #endif
	return 0;
}
/*static int testMemory_mmap(struct file *fp, struct vm_area_struct *vma) {
	int ret;
	int start_pfn = vma->vm_pgoff;
	int end_pfn = start_pfn + ((vma->vm_end - vma->vm_start) >> PAGE_SHIFT);
	int i;
    
	#ifdef DEBUG
		printk("mmap pages [%d-%d]...\n", start_pfn, end_pfn);
	#endif
	for (i = start_pfn; i < end_pfn; i++) {
		if ((ret = remap_pfn_range(vma,
						vma->vm_start + (i - start_pfn) * PAGE_SIZE,
						i,
						PAGE_SIZE,
						vma->vm_page_prot)) < 0) {
			printk("remap_pfn_range fail at pfn %x, ret %d, prot=%lu\n", i, ret, vma->vm_page_prot.pgprot);
			return ret;
		}
	}
    #ifdef DEBUG
		printk("testMemory_mmap return.\n");
	#endif
	return 0;
}*/


static int mmtest_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    #ifdef DEBUG
       int count;
	#endif
    int bregion_num;
	region_t region;
    int idx;
/*	switch ( cmd )*/ {
	//case GET_BREGION_NUM:
              if( cmd == GET_BREGION_NUM ) {
		bregion_num = get_bregion_num();
		if (copy_to_user((int *)arg, &bregion_num, sizeof(int)))
			return -EFAULT;
		//break;
               } else if ( cmd == GET_BREGION ) {
	//case GET_BREGION:
		if (copy_from_user(&region, (region_t *)arg, sizeof(region_t)))
			return -EFAULT;
		idx = region.index;
		get_bregion(idx, &region.start_pfn, &region.end_pfn, &region.nid, &region.type);
		if (copy_to_user((region_t *)arg, &region, sizeof(region_t)))
			return -EFAULT;
                } else if ( cmd == MEMORY_SET_E820_INFO ) {
		//break;
//	case MEMORY_SET_E820_INFO:
	    if (copy_from_user(&mye820, (char *)arg, sizeof(struct mye820map)) )
		    return -EFAULT;
        #ifdef DEBUG
			for ( count = 0; count <mye820.nr_map; count ++ )
	        	printk("Entry %d, addr=%lx, size=%lx, type=%x\n", count, mye820.map[count].addr, mye820.map[count].size, mye820.map[count].type);
		#endif
	    generate_node_map_from_e820_and_SRAT(); } else if ( MEMORY_SET_MEM_INFO == cmd ) {
//		//break;
	//case MEMORY_SET_MEM_INFO:
	    if(copy_from_user(&memLimit, (char *)arg, sizeof(unsigned long long)))
		    return -EFAULT;
	    #ifdef DEBUG
			printk("memLimit=%lx\n", memLimit);
		#endif
	    generate_bootmem_region(memLimit); } else if ( cmd == FLUSH_CACHE ) {
		//break;
	//case FLUSH_CACHE:
		wbinvd(); } else {
		//break;
	//default:
                printk("Unkown command from usr space %lx\n", cmd);
		return -EINVAL;
        }   
    }
    #ifdef DEBUG   
    	printk(KERN_INFO "testMemory driver: successful ioctrl cmd: 0x%x.\n", cmd);
	#endif	
    return 0;
}


static struct file_operations mmtest_fops = {
    .read = mmtest_read,
    .write = mmtest_write,
    .open = mmtest_open,
    .release = mmtest_release,
    .unlocked_ioctl = mmtest_ioctl,
    .mmap  = mmtest_mmap,
    .owner = THIS_MODULE,
};

#define MMTEST_MAX_DEVS	1
struct cdev *mmtest_cdev;
struct class *mmtest_class;
static int mmtest_major;
resource_size_t start_add = 0x30000000;

static int __init mmtest_init_module(void)
{
    int err = 0;
	struct device *dev;
    dev_t devid;	
	/*create owner class*/
	mmtest_class = class_create(THIS_MODULE, MODULE_NAME);
	if ( IS_ERR(mmtest_class) ) {
		err = PTR_ERR(mmtest_class);
		mmtest_class = 0;
		printk("mmtest create class failed!\n");
		return err;
	}
	
	err = alloc_chrdev_region(&devid, 0, MMTEST_MAX_DEVS, MODULE_NAME);
	if (err) {
		printk("%s, alloc_chrdev_region failed.\n", __FUNCTION__);
		goto destroy_mmtest_class;
	}
	
	mmtest_major = MAJOR(devid);
		
	/*create the class node for the test app accessing this driver*/
	dev = device_create(mmtest_class,
				NULL, MKDEV(mmtest_major, 0), NULL,
				MODULE_NAME);
	if (IS_ERR(dev)) {
		printk("%s, device_create failed.\n", __FUNCTION__);
		err = -EFAULT;
		goto unregister_chrdev;
	}
	
	mmtest_cdev = cdev_alloc();
	if ( !mmtest_cdev ) {
		printk(KERN_ERR ": Could not allocate cdev\n");
		err = -ENOMEM;
		goto unregister_chrdev;
    }
	mmtest_cdev->owner = THIS_MODULE;
	cdev_init(mmtest_cdev, &mmtest_fops);
	err = cdev_add(mmtest_cdev, MKDEV(mmtest_major, 0), MMTEST_MAX_DEVS);
	
	if (err) {
        cdev_del(mmtest_cdev);
        mmtest_cdev = NULL;		
		goto unregister_chrdev;
	}
	
    printk( KERN_INFO "Module memtest init\n");
    
    #ifdef DEBUG
    printk( KERN_INFO "size (e820) = %lx, size (e820[1])=%lx\n", sizeof(struct mye820map), sizeof(struct mye820map[1])); 
    printk( KERN_INFO "size E820_X_MAX=%lx\n", E820_X_MAX);
    #endif

    acpi_numa_init();
	goto done;
unregister_chrdev:
	unregister_chrdev_region(MKDEV(mmtest_major, 0), MMTEST_MAX_DEVS);
destroy_mmtest_class:
	class_destroy(mmtest_class);
    mmtest_class = 0;
done:
    return err;
}


static void __exit mmtest_exit_module(void)
{ 
    if ( mmtest_class ) {
		device_destroy(mmtest_class, MKDEV(mmtest_major, 0));
		class_destroy(mmtest_class); 
	}
	
	if ( mmtest_cdev )
		cdev_del(mmtest_cdev);

    printk( KERN_INFO "Module memtest exit\n");
}

module_init(mmtest_init_module);
module_exit(mmtest_exit_module);
