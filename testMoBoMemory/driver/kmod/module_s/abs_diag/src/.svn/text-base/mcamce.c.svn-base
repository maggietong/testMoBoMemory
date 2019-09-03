/*!
 * \file mcamce.c
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
/*kmalloc */
#include <linux/slab.h>
/*put/get user*/
#include <asm/uaccess.h>

#ifdef CONFIG_X86_64
#include <asm/system.h>
#endif
#include "mcamce.h"
#include "abs_diag_thrd.h"
#include "mecc_procfs.h"
#include "cecc_procfs.h"

/* P4/Xeon Extended MCE MSR retrieval */
static inline void extmsrs(mcamce_p4_t *p)
{
	unsigned int h;
	mcamce_p4_extmsrs_t *r;

	r = &p->extmsrs;

	rdmsr(MSR_IA32_MCG_EAX, r->eax, h);
	rdmsr(MSR_IA32_MCG_EBX, r->ebx, h);
	rdmsr(MSR_IA32_MCG_ECX, r->ecx, h);
	rdmsr(MSR_IA32_MCG_EDX, r->edx, h);
	rdmsr(MSR_IA32_MCG_ESI, r->esi, h);
	rdmsr(MSR_IA32_MCG_EDI, r->edi, h);
	rdmsr(MSR_IA32_MCG_EBP, r->ebp, h);
	rdmsr(MSR_IA32_MCG_ESP, r->esp, h);
	rdmsr(MSR_IA32_MCG_EFLAGS, r->eflags, h);
	rdmsr(MSR_IA32_MCG_EIP, r->eip, h);
}

static inline int mcamce_p4_init(mcamce_p4_t *p)
{
	size_t sz;
	unsigned int l;
	unsigned int h;
	int ret = 0;

	memset(p, 0, sizeof *p);
	/*p->cpu = (short)smp_processor_id();*/
	rdmsr(MSR_IA32_MCG_CAP, l, h);
	if (l & (1<<8)) { 
		/*controller presents */
		p->banks_cnt = (unsigned short)(l & 0xff);
		if (p->banks_cnt > 0) {
			sz = p->banks_cnt * sizeof(*p->banks);
			p->banks = (mcamce_bank_t*)kmalloc(sz, GFP_ATOMIC);
			if (NULL != p->banks) {
				memset(p->banks, 0, sz);
				/* Check for P4/Xeon extended MCE MSRs */
				rdmsr(MSR_IA32_MCG_CAP, l, h);
				if (l & (1<<9)) {
					/* MCG_EXT_P */
					p->ext_cnt = (unsigned short)((l >> 16) & 0xff);
					if (p->ext_cnt > 0) {
						extmsrs(p);
					}
				}
			} else {
				ret = -ENOMEM;
				p->banks_cnt = 0;
			}
		}
	}
	return ret;
}

static inline void mcamce_p4_clean(mcamce_p4_t *p)
{
	if ( NULL != p->banks ) {
		kfree(p->banks);
		p->banks_cnt = 0;
		p->banks = NULL;
	}
}

static inline int mcamce_check(mcamce_p4_t *p)
{
	int ret = 0;
	mcamce_bank_t *bank;
	int i;

	/*p->cpu = (short)smp_processor_id();*/
	if (p->banks_cnt > 0 /*&& p->ext_cnt > 0 */) {
		for (i = 0, bank = p->banks; i < p->banks_cnt; i++, bank++) {
			/*pooling */
			rdmsr(MSR_IA32_MC0_STATUS + i*4, bank->status.lo, bank->status.hi);
			if (MCEVAL(bank)) {
				/*valid, error detected, needs process */
				rdmsr(MSR_IA32_MC0_MISC + i*4, bank->info.lo, bank->info.hi);
				rdmsr(MSR_IA32_MC0_ADDR + i*4, bank->addr.lo, bank->addr.hi);
				if (bank->status.lo & 0xffff) {
					/* simple error, bits[15:0] */
					if (0x0100 == (bank->status.lo & 0xff00)) {
						/* is memory heirarchy */
						mecc_procfs_add();
					} else if (0x8800 == (bank->status.lo & 0xf800)) {
						/* is bus interconnect */
						/* maybe memio */
						/* but nehalem arch memory controller is built-in, ignore*/
					} else if (0x0000 == (bank->status.lo & 0xff00)) {
						/* is tlb */
						cecc_procfs_add();
					}
				} else if (1 == (bank->status.lo & (1<<16))) {
					/*is in tag parity */
					cecc_procfs_add();
				}
				ret = 1;
			}
		}
		if(ret) {
			/*rdmsr(MSR_IA32_MCG_STATUS, p->mcgst.lo, p->mcgst.hi); */
			extmsrs(p);
		}
	}

	return ret;
}

//////////////////////////////

int mcamce_enabled(void)
{
	int ret = 0;
	unsigned long cr4;

#ifdef CONFIG_X86_64
	cr4 = read_cr4();
#else
	cr4 = read_cr4_safe();
#endif

	ret = cr4 & X86_CR4_MCE;

	return ret;
}

//////////////////////////////

typedef struct ktdata_t {
	/*mcamce_p4_t saved;*/
	mcamce_p4_t p4;
}ktdata_t;

ktdata_t g_ktdata;

void mcamce_on_timer(void *mm)
{
	ktdata_t *kt;
	
	kt = (ktdata_t*)mm;
	if (mcamce_check(&kt->p4)) {
		/*if (0 == kt->saved.banks) {
			kt->saved = kt->p4;
		}*/
	}
}

int mcamce_init(void)
{
	ktdata_t *kt;
	int ret;

	kt = &g_ktdata;
	memset(kt, 0, sizeof *kt);
	ret = mcamce_p4_init(&kt->p4);

	return ret;
}

void mcamce_clean(void)
{
	ktdata_t *kt;

	kt = &g_ktdata;

	mcamce_p4_clean(&kt->p4);
	memset(kt, 0, sizeof *kt);
}

#if 0
int mcamce_query(mcamce_p4_t *p)
{
	int ret = -EFAULT;
	ktdata_t *kt;

	kt = &g_ktdata;
	
	/*if (kt->saved.banks_cnt > 0) {*/
	/*ret = (int)copy_to_user(p, &kt->saved, sizeof *p);*/
	ret = (int)copy_to_user(p, &kt->p4, sizeof *p);
	/*}*/

	return ret;
}

int mcamce_banks(mcamce_bank_t *p)
{
	int ret = -EFAULT;
	ktdata_t *kt;

	kt = &g_ktdata;
	
	/*if (kt->saved.banks_cnt > 0) { */
	/*ret = (int)copy_to_user(p, &kt->saved.banks, kt->saved.banks_cnt * sizeof(*p));*/
	ret = (int)copy_to_user(p, &kt->p4.banks, kt->p4.banks_cnt * sizeof(*p));
	/*}*/

	return ret;
}
#endif

int mcamce_thrd_start(void)
{
	abs_diag_thrd_mcamce_start(&g_ktdata);
	
	return 0;
}

void mcamce_thrd_stop(void)
{
	abs_diag_thrd_mcamce_stop();
}


