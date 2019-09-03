/*!
 * \file sys_i.h
 *
 * Includes helpers to get/set system parameters.
 *
 * \note
 * Naming *_i.h stands for it's an interface header.
 *
 * \author Pat Huang
 *
 * \version r0.1b01
 * \par ChangeLog:
 * \verbatim
 *  rev 0.1-
 * \endverbatim
 */

#ifndef SYS_I_H
#define SYS_I_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \return Identification code of a system.
 * \see sysconf(_SC_VERSION).
 */
extern long isys_isposix(void);

/*!
 * \return PageSize of a system.
 * \see sysconf(_SC_PAGE_SIZE).
 */
extern long isys_pagesize(void);

/*!
 * Binds current process or thread to a CPU.
 * \param cpu to bind to.
 * \return 0 if succ, otherwise JEDI_FAIL.
 * \see sched_setaffinity().
 */
extern int isys_cpu_bind(const unsigned short cpu);

/*!
 * Finds the pid of a process corresponding 
 *   to the input name.
 * \param name process name to be found.
 * \return pid if found, otherwise JEDI_FAIL.
 * \see /proc.
 */
extern long isys_process_find(const char *name);

#ifdef __cplusplus
} // extern c
#endif

#endif

