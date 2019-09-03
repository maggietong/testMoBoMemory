/*!
 * \file cachetest.c
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

#include "cachetest.h"
#include "abs_diagdev_ctl_i.h"
#include "abs_diag.h"

#define UNIT_CNT	((unsigned long)2)
#define TEST_SIZE	(UNIT_CNT * JEDI_UNIT_SIZE)
#define PGCNT		(TEST_SIZE / PAGE_SIZE)	
/*per page cacheline cnt */
#define CLCNT		(PAGE_SIZE / JEDI_CLSZ)
#define CNT_PER_PAGE	(PAGE_SIZE / sizeof(long))
#define CNT_PER_CLINE	(JEDI_CLSZ / sizeof(long))
#define PAT		0x80808080UL

static int g_stat = ABS_DIAG_CACHETEST_READY;

static unsigned long *g_buf[PGCNT];

static int tmemsetup(void)
{
	unsigned long i;
	int ret;

	ret = 0;
	for (i = 0; i < PGCNT; i++) {
		g_buf[i] = (unsigned long*)kmalloc(PAGE_SIZE, GFP_ATOMIC);
		if (NULL == g_buf[i]) {
			ret = -ENOMEM;
			break;
		}
	}

	return ret;
}

static void tmemclean(void)
{
	unsigned long i;
	for (i = 0; i < PGCNT; i++) {
        if (NULL != g_buf[i])
		    kfree(g_buf[i]);
	}
}

static inline int do_test(const unsigned long pat)
{
	unsigned long *pa;
	unsigned long i;
	unsigned long ip;
	int ret = 0;
	unsigned long cnt = 100;

	/*printk(KLL_DEBG""ABS_DIAGDEV" %s: pat 0x%p\n",
		__FUNCTION__,
		(void*)pat
		);
	*/
	while (0 == ret && --cnt > 0) {
		for (ip = 0; ip < PGCNT; ip++) {
			pa = g_buf[ip];
			for (i = 0; i < CNT_PER_PAGE; i++, pa++) {
				*pa = pat;
			}
		}

		wbinvd();

		for (ip = 0; ip < PGCNT; ip++) {
			pa = g_buf[ip];
			for (i = 0; i < CLCNT; i++, pa += CNT_PER_CLINE) {
				prefetch(pa);
			}
		}

		wbinvd();

		for (ip = 0; 0 == ret && ip < PGCNT; ip++) {
			pa = g_buf[ip];
			for (i = 0; i < CNT_PER_PAGE; i++, pa++) {
				if (*pa != pat) {
					ret = -1;
					break;
				}
			}
		}
	}

	return ret;
}

int cachetest(void)
{
	unsigned long pat;
	int ret = -EBUSY;

	cachetest_stat();

	if (ABS_DIAG_CACHETEST_READY == g_stat) {
		g_stat = ABS_DIAG_CACHETEST_INPROCESS;
		ret = tmemsetup();
		if (0 == ret) {
			pat = PAT;
#if JEDI_BITS > 32
			pat |= PAT << 32;
#endif
			ret = do_test(pat);
			if (0 == ret) {
				pat = ~pat;
				ret = do_test(pat);
			}
			tmemclean();
			if (0 == ret) {
				g_stat = ABS_DIAG_CACHETEST_STOP_SUCC;
			} else {
				g_stat = ABS_DIAG_CACHETEST_STOP_FAIL;
			}
		} else {
			g_stat = ABS_DIAG_CACHETEST_STOP_NOMEM;
			ret = -ENOMEM;
		}
	} 

	return ret;
}

char cachetest_stat(void)
{
	int ret = g_stat;

	if (ABS_DIAG_CACHETEST_STOP_SUCC == g_stat 
		|| ABS_DIAG_CACHETEST_STOP_FAIL == g_stat
		|| ABS_DIAG_CACHETEST_STOP_NOMEM == g_stat
	) {
		g_stat = ABS_DIAG_CACHETEST_READY;
	}

	return (char)ret;
}

