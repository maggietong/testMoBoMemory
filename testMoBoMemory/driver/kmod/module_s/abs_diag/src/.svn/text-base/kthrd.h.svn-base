/*!
 * \file kthrd.h
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

#ifndef KTHRD_H__
#define KTHRD_H__

#define KTHRD_DFLT_SLEEP	500	/* 500ms */

typedef struct kthrd_t kthrd_t;

struct kthrd_t {
	void *handle;
	void (*on_run)(kthrd_t *p);
	void (*on_stop)(kthrd_t *p);
	void *ktdata;
	int pid;
};

extern int kthrd_init(kthrd_t *p);
extern void kthrd_clean(kthrd_t *p);
extern int kthrd_start(kthrd_t *p);
extern void kthrd_stop(kthrd_t *p);

#endif

