/*!
 * \file abs_diagdev_procfs.c
 *
 * \note
 * The local implementing code of this module.
 *
 * \author Pat Huang
 *
 * \version r0.1b01
 * \par ChangeLog:
 * \verbatim
 *  rev 0.1-
 * \endverbatim
 */


/*CR0*/
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,20)
#include <asm/processor-flags.h>
#endif

#include "abs_diagdev_procfs.h"
#include "abs_diag.h"
#include "abs_diagdev_ctl_i.h"

#ifndef X86_CR0_CD
#define X86_CR0_CD	(0x0001UL << 29)
#define X86_CR0_NW	(0x0001UL << 28)
#endif

static DECLARE_MUTEX(g_sem);
static spinlock_t g_lock = SPIN_LOCK_UNLOCKED;

static struct proc_dir_entry *g_proc = NULL;

static ssize_t f_on_read(struct file *p, char __user *buf, size_t len, loff_t *offset)
{
	char msg[32];
	ssize_t ret = -EFAULT;
	size_t sz;
        unsigned long cr0;
	int on;

	/*printk(KLL_INFO""ABS_DIAGDEV": %s\n", 
		__FUNCTION__
		);
	*/
	ret = sprintf(msg,
		ABS_DIAGDEV_VERSION"\n%d\n",
		1
		);
	/*printk(KLL_DEBG""ABS_DIAGDEV" %s len %lu offset %lu ret %ld\n",
		__FUNCTION__,
		(unsigned long)len,
		(unsigned long)*offset,
		(long)ret
		);
	*/
	if (*offset >= ret) {
		/* EOF */
		ret = 0;
		return ret;
	}
	
	if (down_interruptible(&g_sem)) {
		ret = -ERESTARTSYS;
		return ret;
	} 
	cr0 = read_cr0();
	up(&g_sem);
	/*printk(KLL_DEBG""ABS_DIAGDEV" %s cr0 0x%p\n",
		__FUNCTION__,
		(void*)cr0
		);
	*/
	on = 0;
	/*if (sizeof(long) > 4) {
		asm volatile("movl %%cr0,%0\n\t" :"=r" (cr0));
	} else {
		asm volatile("movq %%cr0,%0" : "=r" (cr0));
	}*/
	if (cr0 & X86_CR0_CD) {
		/*regardless of the NW flag if CD is set*/
		on = 0;
	} else if (!(cr0 & X86_CR0_NW)) {
		on = 1;
	}
	ret = sprintf(msg,
		ABS_DIAGDEV_VERSION"\n%d\n",
		on	
		);
	sz = ret - *offset;
	if (sz > len) {
		sz = len;
	}
	ret = copy_to_user(buf, msg + *offset, sz);
	if (0 == ret) {
		*offset += sz;
		ret = sz;
	} 
	
	return ret;
}

static ssize_t f_on_write(struct file *p, const char __user *buf, size_t len, loff_t *offset)
{
	char msg[8];
	ssize_t ret = -EFAULT;
	size_t sz;
	unsigned long cr0;
	int in;
/*
	printk(KLL_INFO""ABS_DIAGDEV": %s\n", 
		__FUNCTION__
		);
	printk(KLL_DEBG""ABS_DIAGDEV" %s len %lu offset %lu\n",
		__FUNCTION__,
		(unsigned long)len,
		(unsigned long)*offset
		);
*/
	sz = len;
	if (sz > sizeof(msg)) {
		sz = sizeof msg;
	}
	ret = copy_from_user(msg, buf, sz);
	if (0 == ret) {
		ret = sz;
		msg[sz] = '\0';
		if (down_interruptible(&g_sem)) {
			ret = -ERESTARTSYS;
			return ret;
		} 
		cr0 = read_cr0();
/*
		printk(KLL_DEBG""ABS_DIAGDEV" %s cr0 0x%p msg [%s]\n",
			__FUNCTION__,
			(void*)cr0,
			msg
			);
*/
		in = *msg - '0';
		/*if (sizeof(long) > 4) {
			asm volatile("movl %%cr0,%0\n\t" :"=r" (cr0));
		} else {
			asm volatile("movq %%cr0,%0" : "=r" (cr0));
		}
		*/
		if (!in) {
			/*  Enter the no-fill (CD=1, NW=0) cache mode and flush caches. */
			cr0 |= X86_CR0_CD;
			cr0 &= ~X86_CR0_NW;
			spin_lock(&g_lock);
			write_cr0(cr0);  /* set CD flag */
			spin_unlock(&g_lock);
		} else {
			cr0 &= ~X86_CR0_CD;
			cr0 &= ~X86_CR0_NW;
			spin_lock(&g_lock);
			write_cr0(cr0);  /* set CD flag */
			spin_unlock(&g_lock);
		}
		cr0 = read_cr0();
/*
		printk(KLL_DEBG""ABS_DIAGDEV" %s readback cr0 0x%p\n",
			__FUNCTION__,
			(void*)cr0
			);
*/
		up(&g_sem);
		wbinvd();
	} 

	return ret;
}

static int f_on_open(struct inode *inode, struct file *file)
{
	int ret = -EBUSY;
/*
	printk(KLL_INFO""ABS_DIAGDEV": %s\n", 
		__FUNCTION__
		);
*/
	try_module_get(THIS_MODULE);
	ret = 0;

	return ret;
}

static int f_on_release(struct inode *inode, struct file *file)
{
	int ret = -EFAULT;
/*
	printk(KLL_INFO""ABS_DIAGDEV": %s\n", 
		__FUNCTION__
		);
*/
	module_put(THIS_MODULE);

	ret = 0;

	return ret;
}

static struct file_operations g_fops = {
	.owner = THIS_MODULE,
	.open = f_on_open,
	.release = f_on_release,
	.read = f_on_read,
	.write = f_on_write
};

int abs_diagdev_procfs_init(void)
{
	int ret = -ENOMEM;

	if (NULL == g_proc) {
		g_proc = create_proc_entry(ABS_DIAGDEV_PROC_FILENAME, 0644, NULL);
		if (NULL != g_proc) {
			g_proc->owner = THIS_MODULE;
			g_proc->proc_fops = &g_fops;
			g_proc->mode = S_IFREG | S_IRUGO | S_IWUSR;
			ret = 0;
		}
	} else {
		ret = 0;
	}
	
	return ret;
}

void abs_diagdev_procfs_clean(void)
{
/*	printk(KLL_INFO""ABS_DIAGDEV": %s, g_proc %p\n", 
		__FUNCTION__,
		g_proc
		);
*/
	if (NULL != g_proc) {
		remove_proc_entry(ABS_DIAGDEV_PROC_FILENAME, NULL);
		g_proc = NULL;
	}
}


