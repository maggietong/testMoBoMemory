/*!
 * \file addrmap.h
 *
 * \note
 * The local implementing header of this module.
 *
 * \author Pat Huang
 *
 * \version r0.1b01
 * \par ChangeLog:
 * \verbatim
 *  rev 0.1-
 * \endverbatim
 */

#ifndef ADDRMAP_H__
#define ADDRMAP_H__

/* Linux Kernel  */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/completion.h>
#include <linux/sched.h>
#include <linux/mutex.h>

/* kmalloc  */
#include <linux/slab.h>
#include <linux/prefetch.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/signal.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <asm/processor.h>

/* getuser putuser  */
#include <asm/uaccess.h>
#include <linux/version.h>
#include <asm/processor-flags.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <asm/system.h>

/* user space */
#define JEDI_CLSZ   64
#define JEDI_BITS   64

#define JEDI_M2B(i) (((unsigned long)(i)) << 20)
#define JEDI_L3CACHE_SIZE   JEDI_M2B(8)
#define JEDI_UNIT_SIZE  JEDI_L3CACHE_SIZE



#ifdef DEBUG
#define KLL_INFO	KERN_ALERT 
#define KLL_DEBG	KERN_ALERT 
#else
#define KLL_INFO	KERN_INFO
#define KLL_DEBG	KERN_DEBUG
#endif

#endif


