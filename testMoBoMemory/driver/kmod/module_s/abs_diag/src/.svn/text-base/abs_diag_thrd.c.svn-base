/*!
 * \file abs_diag_thrd.c
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

#include "kthrd.h"
#include "mcamce.h"
#include "abs_diag.h"
#include "abs_diagdev_ctl_i.h"

typedef struct ktdata_t {
	void *mcamce;
	long mcamce_refcnt;
	char term;
	char exit;
}ktdata_t;

static kthrd_t g_thrd;
static ktdata_t g_ktdata;

/******************************/

static inline int dt_mcamce_is_running(const ktdata_t *p)
{
	return p->mcamce_refcnt > 0;
}

static inline void dt_mcamce_run(ktdata_t *p)
{
	p->mcamce_refcnt ++;
}

static inline void dt_mcamce_shutdown(ktdata_t *p)
{
	if (p->mcamce_refcnt > 0)
	{
		p->mcamce_refcnt --;
	}
}

static inline int dt_thrd_is_running(const ktdata_t *p)
{
	return 0 == p->exit;
}

static inline void dt_thrd_run(ktdata_t *p)
{
	p->exit = 0;
}

static inline void dt_thrd_shutdown(ktdata_t *p)
{
	p->mcamce_refcnt = 0;
	p->exit = 1;
}

/*****************************/

static int dt_thrd_start(void)
{
	int ret = -EFAULT;
	kthrd_t *thrd;
	ktdata_t *kt;

	thrd = &g_thrd;
	kt = (ktdata_t*)thrd->ktdata;
	if ( !dt_thrd_is_running(kt) ) {
		kt->term = 0;
		ret = kthrd_start(thrd);
	} else {
		ret = 0;
	}
	
	return ret;
}

static void dt_thrd_stop(void)
{
	kthrd_t *thrd;
	ktdata_t *kt;
	int cnt;

	thrd = &g_thrd;
	kt = (ktdata_t*)thrd->ktdata;
	if (dt_thrd_is_running(kt)) {
		kthrd_stop(thrd);
		cnt = 0;
		while ( dt_thrd_is_running(kt) ) {
			msleep(KTHRD_DFLT_SLEEP);
			if (++cnt > 1000) {
				break;
			}
		}
	}
	printk(KLL_INFO""ABS_DIAGDEV": %s, thrd exit.\n", 
		__FUNCTION__
		);
}

static void dt_on_timer(ktdata_t *p)
{
	if (NULL != p->mcamce) {
		mcamce_on_timer(p->mcamce);
	}
}

static void f_on_run(kthrd_t *p)
{
	ktdata_t *kt;

	kt = (ktdata_t*)p->ktdata;

	dt_thrd_run(kt);
	while ( !(kt->term) ) {
		dt_on_timer(kt);
		msleep(KTHRD_DFLT_SLEEP);
	}
	dt_thrd_shutdown(kt);
}

static void f_on_stop(kthrd_t *p)
{
	ktdata_t *kt;

	kt = (ktdata_t*)p->ktdata;
	kt->term = 1;
}

/*****************************/

int abs_diag_thrd_init(void)
{
	int ret = -EFAULT;
	kthrd_t *thrd;
	ktdata_t *kt;
	
	kt = &g_ktdata;
	memset(kt, 0, sizeof *kt);
	dt_thrd_shutdown(kt);
	thrd = &g_thrd;
	kthrd_init(thrd);
	thrd->on_run = f_on_run;
	thrd->on_stop = f_on_stop;
	thrd->ktdata = kt;
	
	ret = dt_thrd_start();

	return ret;
}

void abs_diag_thrd_clean(void)
{
	kthrd_t *thrd;
	ktdata_t *kt;

	thrd = &g_thrd;
	kt = (ktdata_t*)thrd->ktdata;
	dt_thrd_stop();
	memset(kt, 0, sizeof *kt);
}

int abs_diag_thrd_mcamce_start(const void *mm)
{
	kthrd_t *thrd;
	ktdata_t *kt;

	thrd = &g_thrd;
	kt = (ktdata_t*)thrd->ktdata;

	kt->mcamce = (void*)mm;
	
	dt_mcamce_run(kt);

	return 0;
}

void abs_diag_thrd_mcamce_stop(void)
{
	kthrd_t *thrd;
	ktdata_t *kt;

	thrd = &g_thrd;
	kt = (ktdata_t*)thrd->ktdata;
	
	dt_mcamce_shutdown(kt);
	kt->mcamce = NULL;
}


