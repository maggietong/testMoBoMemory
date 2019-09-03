/*!
 * \file kthrd.c
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

#include "abs_diag.h"
#include "kthrd.h"

#ifdef KTHRD_COMP
static DEFINE_SPINLOCK(g_lck_create);
/* 
 * Thread stopping is done by setthing this var: lock serializes
 * multiple kthread_stop calls. 
 */
static DEFINE_MUTEX(g_lck_stop);

typedef struct kthrd_stop_info_t {
	struct task_struct *k;
	int err;
	struct completion done_stop;
} kthrd_stop_info_t;

static kthrd_stop_info_t g_stop_info;

static int kthrd_should_stop(void)
{
	return g_stop_info.k == current;
}

typedef struct handle_t {
	struct task_struct *thrd;
	struct completion done_create;
	struct completion started;
} handle_t;

#endif

int kthrd_init(kthrd_t *p)
{
	int ret = -EFAULT;
#ifdef KTHRD_COMP
	handle_t *handle;
#endif

	memset(p, 0, sizeof *p);
#ifdef KTHRD_COMP
	p->handle = (handle_t*)kmalloc(sizeof *handle, GPF_ATOMIC);
	if (NULL != p->handle) {
		handle = (handle_t*)p->handle;
		jedi_bzero(handle, sizeof *handle);
		init_completion(&handle->started);
		init_completion(&handle->done_create);

		spin_lock(&g_lck_create);
		/* add node to list */
		spin_unlock(&g_lck_create);
#endif
		ret = 0;
#ifdef KTHRD_COMP
	}
#endif
	
	return ret;
}

void kthrd_clean(kthrd_t *p)
{
#ifdef KTHRD_COMP
	if (NULL != p->handle) {
		kfree(p->handle);
	}
#endif
	memset(p, 0, sizeof *p);
}

static int run_proc(void *arg)
{
	kthrd_t *thrd;
#ifdef KTHRD_COMP
	handle_t *handle;
#endif

	thrd = (kthrd_t*)arg;
#ifdef KTHRD_COMP
	handle = (handle_t*)thrd->handle;
#endif
	
#ifdef KTHRD_COMP
	/* OK, tell user we're spawned, wait for stop or wakeup */
	__set_current_state(TASK_UNINTERRUPTIBLE);
	complete(&handle->started);
	schedule();

	if (!kthrd_should_stop()) {
#endif
		if (NULL != thrd->on_run) {
			thrd->on_run(thrd);
		}
#ifdef KTHRD_COMP
	}

	/* It might have exited on its own, w/o kthread_stop.  Check. */
	if (kthrd_should_stop()) {
		complete(&g_stop_info.done_stop);
	}
#endif

	return 0;
}

int kthrd_start(kthrd_t *p)
{
	int ret = -EFAULT;
#ifdef KTHRD_COMP
	handle_t *handle;
#endif

#ifdef KTHRD_COMP
	spin_lock(&g_lck_create);
	/* get node from list */
	spin_unlock(&g_lck_create);
	
	handle = (handle_t*)p->handle;
#endif
	p->pid = kernel_thread(run_proc, p, CLONE_KERNEL);

	if (p->pid > 0) {
#ifdef KTHRD_COMP
		wait_for_completion(&handle->started);
		/* read_lock(&tasklist_lock); */
		handle->thrd = find_task_by_pid(p->pid);
		/* read_unlock(&tasklist_lock); */
#endif
		ret = 0;
	}
#ifdef KTHRD_COMP
	else {
		handle->thrd = ERR_PTR(p->pid);
	}
	complete(&handle->done_create);

	spin_lock(&g_lck_create);
	/* leave node list */
	spin_unlock(&g_lck_create);
#endif
	
	return ret;
}

void kthrd_stop(kthrd_t *p)
{
#ifdef KTHRD_COMP
	handle_t *handle;
#endif

	if (p->pid > 0) {
#ifdef KTHRD_COMP
		mutex_lock(&g_lck_stop);

		handle = (handle_t*)p->handle;

		/* It could exit after stop_info.k set, but before wake_up_process. */
		/*get_task_struct(handle->thrd); */

		/* Must init completion *before* thread sees kthread_stop_info.k */
		init_completion(&g_stop_info.done_stop);
		smp_wmb();

		/* Now set kthread_should_stop() to true, and wake it up. */
		g_stop_info.k = handle->thrd;
		wake_up_process(handle->thrd);
		/*put_task_struct(handle->thrd); */
#endif
		if (NULL != p->on_stop ) {
			p->on_stop(p);
		}
		p->pid = 0;
#ifdef KTHRD_COMP
		wait_for_completion(&g_stop_info.done_stop);
		g_stop_info.k = NULL;
		handle->thrd = NULL;
		mutex_unlock(&g_lck_stop);
#endif
	}
}


