/*!
 * \file cachetest_procfs.c
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

#include "cachetest_procfs.h"
#include "cachetest.h"
#include "abs_diag.h"
#include "abs_diagdev_ctl_i.h"

static struct proc_dir_entry *g_proc = NULL;

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
		"%c\n",
		cachetest_stat() + '0'
		);
	if (*offset >= ret) {
		/* EOF */
		ret = 0;
		return ret;
	}
	
	ret = sprintf(msg,
		"%c\n",
		cachetest_stat() + '0'
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
	cachetest();

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

int cachetest_procfs_init(void)
{
	int ret = -ENOMEM;

	if (NULL == g_proc) {
		g_proc = create_proc_entry(ABS_DIAG_CACHETEST_PROC_FILENAME, 0644, NULL);
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

void cachetest_procfs_clean(void)
{
	/*printk(KLL_INFO""ABS_DIAGDEV": %s, g_proc %p\n", 
		__FUNCTION__,
		g_proc
		);
	*/
	if (NULL != g_proc) {
		remove_proc_entry(ABS_DIAG_CACHETEST_PROC_FILENAME, NULL);
		g_proc = NULL;
	}
}

