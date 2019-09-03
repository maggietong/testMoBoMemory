/*!
 * \file mecc_procfs.c
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

#include "mecc_procfs.h"
#include "mcamce.h"
#include "abs_diag.h"
#include "abs_diagdev_ctl_i.h"

static DECLARE_MUTEX(g_sem);

static struct proc_dir_entry *g_proc = NULL;

static unsigned long g_cnt = 0;

static ssize_t f_on_read(struct file *p, char __user *buf, size_t len, loff_t *offset)
{
	char msg[32];
	ssize_t ret = -EFAULT;
	size_t sz;

	/*printk(KLL_INFO""ABS_DIAGDEV": %s\n", 
		__FUNCTION__
		);
	*/
	ret = sprintf(msg,
		"1\n%lu\n",
		g_cnt
		);
	if (*offset >= ret) {
		/* EOF */
		ret = 0;
		return ret;
	}
	
	if (down_interruptible(&g_sem)) {
		ret = -ERESTARTSYS;
		return ret;
	} 
	ret = sprintf(msg,
		"%d\n%lu\n",
		mcamce_enabled() ? 1 : 0,
		g_cnt
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
	up(&g_sem);
	
	return ret;
}

static ssize_t f_on_write(struct file *p, const char __user *buf, size_t len, loff_t *offset)
{
	ssize_t ret = -EFAULT;

	/*printk(KLL_INFO""ABS_DIAGDEV": %s\n", 
		__FUNCTION__
		);
	printk(KLL_DEBG""ABS_DIAGDEV" %s len %lu offset %lu\n",
		__FUNCTION__,
		(unsigned long)len,
		(unsigned long)*offset
		);
	*/
	ret = len;
	if (down_interruptible(&g_sem)) {
		ret = -ERESTARTSYS;
		return ret;
	} 
	g_cnt = 0;
	up(&g_sem);

	return ret;
}

static int f_on_open(struct inode *inode, struct file *file)
{
	int ret = -EBUSY;

	/*printk(KLL_INFO""ABS_DIAGDEV": %s\n", 
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

	/*printk(KLL_INFO""ABS_DIAGDEV": %s\n", 
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

int mecc_procfs_init(void)
{
	int ret = -ENOMEM;

	if (NULL == g_proc) {
		g_proc = create_proc_entry(ABS_DIAG_MECC_PROC_FILENAME, 0644, NULL);
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

void mecc_procfs_clean(void)
{
	/*printk(KLL_INFO""ABS_DIAGDEV": %s, g_proc %p\n", 
		__FUNCTION__,
		g_proc
		);
	*/
	if (NULL != g_proc) {
		remove_proc_entry(ABS_DIAG_MECC_PROC_FILENAME, NULL);
		g_proc = NULL;
	}
}

void mecc_procfs_add(void)
{
	if (g_cnt < (~(unsigned long)0)) {
		g_cnt ++;
	} else {
		g_cnt = 0;
	}
}

