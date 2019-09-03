/*!
 * \file addrmap_main.c
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

#include "addrmap.h"
#include "addrmap_procfs.h"

static int __init addrmap_init(void)
{
	int ret = -EFAULT;

	ret = addrmap_procfs_init();

	return ret;
}

static void __exit addrmap_exit(void)
{
	addrmap_procfs_clean();
}

module_init(addrmap_init);
module_exit(addrmap_exit);
MODULE_LICENSE("GPL");

