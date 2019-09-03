/*!
 * \file mcamce_p4_i.h
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

#ifndef MCAMCE_P4_I_H__
#define MCAMCE_P4_I_H__

/* as supported by the P4/Xeon family */
typedef struct mcamce_p4_extmsrs_t mcamce_p4_extmsrs_t;

struct mcamce_p4_extmsrs_t {
	unsigned int eax;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;
	unsigned int esi;
	unsigned int edi;
	unsigned int ebp;
	unsigned int esp;
	unsigned int eflags;
	unsigned int eip;
	/* unsigned int *reserved[]; */
};

typedef struct mcamce_bank_cell_t mcamce_bank_cell_t;

struct mcamce_bank_cell_t {
	unsigned int hi;
	unsigned int lo;
};

typedef struct mcamce_bank_t mcamce_bank_t;

struct mcamce_bank_t {
	mcamce_bank_cell_t status;
	mcamce_bank_cell_t addr;
	mcamce_bank_cell_t info;
};

/* checking VAL (bit 63 of IA32_MCI_STATUS register) */
#define MCEVAL(b)	((b)->status.hi & (1<<31))

typedef struct mcamce_p4_t mcamce_p4_t;

struct mcamce_p4_t {
	mcamce_p4_extmsrs_t extmsrs;
	mcamce_bank_t *banks;
	/*mcamce_bank_cell_t mcgst;*/
	/*short cpu;*/
	unsigned short ext_cnt;
	unsigned short banks_cnt;
};

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif


