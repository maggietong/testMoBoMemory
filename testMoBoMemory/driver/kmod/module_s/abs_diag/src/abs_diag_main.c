/*!
 * \file abs_diag_main.c
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

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "abs_diagdev_procfs.h"
#include "mecc_procfs.h"
#include "cecc_procfs.h"
#include "cachetest_procfs.h"
#include "abs_diag.h"
#include "abs_diagdev_ctl_i.h"
#include "abs_diag_thrd.h"
#include "mcamce.h"

/*#define ABS_DIAGDEV_MINOR 		0

static dev_t g_devno = MKDEV(0,0);

static int alloc_major(void)
{
	int ret;

	ret = alloc_chrdev_region(&g_devno, ABS_DIAGDEV_MINOR, 1, ABS_DIAGDEV);
	if (ret < 0) {
		printk(KERN_CRIT""ABS_DIAGDEV": %s cannot get major %d\n",
			__FUNCTION__,
			MAJOR(g_devno)
			);
	} else {
		printk(KLL_INFO""ABS_DIAGDEV": %s major %d\n",
			__FUNCTION__,
			MAJOR(g_devno)
			);
	}

	return ret;
}

dev_t abs_diag_devno(void)
{
	if (0 == MAJOR(g_devno)) {
		alloc_major();
	}
	return g_devno;
}

void abs_diag_devno_close(void)
{
	if (0 != MAJOR(g_devno)) {
		unregister_chrdev_region(g_devno, 1);
		g_devno = MKDEV(0,0);
	}
}
*/

static int __init abs_diag_init(void)
{
	int ret = -EFAULT;

	ret = abs_diagdev_procfs_init();
	ret = mecc_procfs_init();
	ret = cecc_procfs_init();
	ret = cachetest_procfs_init();
	ret = mcamce_init();
	ret = abs_diag_thrd_init();
	ret = mcamce_thrd_start();

	return ret;
}

static void __exit abs_diag_exit(void)
{
	mcamce_thrd_stop();
	abs_diag_thrd_clean();
	mcamce_clean();
	cachetest_procfs_clean();
	cecc_procfs_clean();
	mecc_procfs_clean();
	abs_diagdev_procfs_clean();
}

module_init(abs_diag_init);
module_exit(abs_diag_exit);
MODULE_LICENSE("GPL");

