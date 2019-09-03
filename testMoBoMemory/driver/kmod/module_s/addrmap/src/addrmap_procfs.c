/*!
 * \file addrmap_procfs.c
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

#include "addrmap_procfs.h"
#include "convt.h"
#include "addrmap.h"
#include "addrmap_ctl_i.h"

static DECLARE_MUTEX(g_sem);

static struct proc_dir_entry *g_proc = NULL;

static unsigned long g_va = 0;

static ssize_t f_on_read(struct file *p, char __user *buf, size_t len, loff_t *offset)
{
	char msg[32 * 4];
	ssize_t ret = -EFAULT;
	size_t sz;
	unsigned long pa;

	//printk(KLL_DEBG""ADDRMAP_DEV": on_read in addr 0x%p\n", (void*)g_va);
	if (down_interruptible(&g_sem)) {
		ret = -ERESTARTSYS;
		return ret;
	} 
	pa = addrmap_convt(g_va);
	ret = sprintf(msg,
		ADDRMAP_VERSION"\n0x%p\n0x%p\n0x%p\n",
		(void*)g_va,
		(void*)pa,
		//(void*)(pa > 0 ? (*(unsigned long*)((char*)pa + PAGE_OFFSET)) : 0)
		(void*)0
		);
	if (*offset >= ret) {
		/* EOF */
		ret = 0;
		up(&g_sem);
		return ret;
	}
	
	sz = ret - *offset;
	if (sz > len) {
		sz = len;
	}
	ret = copy_to_user(buf, msg + *offset, sz);
	if (0 == ret) {
		*offset += sz;
		ret = sz;
	} 
	up(&g_sem);
	
	return ret;
}

static ssize_t f_on_write(struct file *p, const char __user *buf, size_t len, loff_t *offset)
{
	char msg[32];
	ssize_t ret = -EFAULT;
	size_t sz;

	sz = len;
	if (sz > sizeof(msg)) {
		sz = sizeof msg;
	}
	if (down_interruptible(&g_sem)) {
		ret = -ERESTARTSYS;
		return ret;
	} 
	ret = copy_from_user(msg, buf, sz);
	if (0 == ret) {
		char *str;
		ret = sz;
		msg[sz] = '\0';
		str = strrchr(msg, '\n');
		if (NULL != str) {
			*str = '\0';
		}
		g_va = simple_strtoul(msg, NULL, 16);
	} 
	//printk(KLL_DEBG""ADDRMAP_DEV": on_write in addr 0x%p\n", (void*)g_va);
	up(&g_sem);

	return ret;
}

static int f_on_open(struct inode *inode, struct file *file)
{
	int ret = -EBUSY;

	try_module_get(THIS_MODULE);
	ret = 0;

	return ret;
}

static int f_on_release(struct inode *inode, struct file *file)
{
	int ret = -EFAULT;

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

int addrmap_procfs_init(void)
{
	int ret = -ENOMEM;

	if (NULL == g_proc) {
		g_proc = create_proc_entry(ADDRMAP_PROC_FILENAME, 0644, NULL);
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

void addrmap_procfs_clean(void)
{
	if (NULL != g_proc) {
		remove_proc_entry(ADDRMAP_PROC_FILENAME, NULL);
		g_proc = NULL;
	}
}

