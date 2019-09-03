/*!
 * \file abs_diag_thrd.h
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

#ifndef ABS_DIAG_THRD_H__
#define ABS_DIAG_THRD_H__

extern int abs_diag_thrd_init(void);
extern void abs_diag_thrd_clean(void);

extern int abs_diag_thrd_mcamce_start(const void *mm);
extern void abs_diag_thrd_mcamce_stop(void);

#endif

