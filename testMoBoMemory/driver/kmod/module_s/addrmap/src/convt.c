/*!
 * \file convt.c
 * 
 * \author Pat Huang
 *
 * Implementing address mapping.
 *
 * \version r0.1b03
 * rev 0.1-
 * \par ChangeLog:
 * \verbatim
 *  rev 0.1-
 *    b02, dont use find_task_xxx, use current-> directly.
 *    b03, 2009sep09, remove any returned hiest 0x80.
 * \endverbatim
 */

#include "convt.h"
#include "addrmap.h"
#include "addrmap_ctl_i.h"

unsigned long addrmap_convt(const unsigned long in)
{
	struct task_struct *task;
	struct mm_struct *mm;
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;
	unsigned long ret = 0;
	pid_t pid;

	task = current;
	pid = task->tgid;
	
	/*printk(KLL_DEBG""ADDRMAP_DEV": convt in addr 0x%p\n", (void*)in);

	printk(KLL_DEBG""ADDRMAP_DEV": PAGE_OFFSET = 0x%p\n", (void*)PAGE_OFFSET);
	printk(KLL_DEBG""ADDRMAP_DEV": PGDIR_SHIFT = 0x%p\n", (void*)PGDIR_SHIFT);
	printk(KLL_DEBG""ADDRMAP_DEV": PUD_SHIFT = 0x%p\n", (void*)PUD_SHIFT);
	printk(KLL_DEBG""ADDRMAP_DEV": PMD_SHIFT = 0x%p\n", (void*)PMD_SHIFT);
	printk(KLL_DEBG""ADDRMAP_DEV": PAGE_SHIFT = 0x%p\n", (void*)PAGE_SHIFT);
	
	printk(KLL_DEBG""ADDRMAP_DEV": PTRS_PER_PGD = 0x%p\n", (void*)PTRS_PER_PGD);
	printk(KLL_DEBG""ADDRMAP_DEV": PTRS_PER_PUD = 0x%p\n", (void*)PTRS_PER_PUD);
	printk(KLL_DEBG""ADDRMAP_DEV": PTRS_PER_PMD = 0x%p\n", (void*)PTRS_PER_PMD);
	printk(KLL_DEBG""ADDRMAP_DEV": PTRS_PER_PTE = 0x%p\n", (void*)PTRS_PER_PTE);

	printk(KLL_DEBG""ADDRMAP_DEV": PAGE_MASK = 0x%p\n", (void*)PAGE_MASK);
	printk(KLL_DEBG""ADDRMAP_DEV": pid = %ld\n", (long)pid);
	*/

	/*task = find_task_by_pid(pid);
	if (NULL == task) {
		printk(KERN_CRIT""ADDRMAP_DEV": task of pid %ld not found\n", (long)pid);
		return ret;
	}*/
	mm = get_task_mm(task);
	//printk(KLL_DEBG""ADDRMAP_DEV": pgd of task = 0x%p\n", mm->pgd);
	if (!find_vma(mm, in)) {
		printk(KERN_CRIT""ADDRMAP_DEV": address 0x%p not available\n", (void*)in);
		return ret;
	}
	pgd = pgd_offset(mm, in);
	//printk(KLL_DEBG""ADDRMAP_DEV": pgd = 0x%p\n", pgd);
	//printk(KLL_DEBG""ADDRMAP_DEV": pgd_val = 0x%p\n", (void*)pgd_val(*pgd));
	if (pgd_none(*pgd)) {
		printk(KERN_CRIT""ADDRMAP_DEV": not mapped in this pgd\n");
		return ret;
	}
	pud = pud_offset(pgd, in);
	//printk(KLL_DEBG""ADDRMAP_DEV": pud = 0x%p\n", pud);
	//printk(KLL_DEBG""ADDRMAP_DEV": pud_val = 0x%p\n", (void*)pud_val(*pud));
	if (pud_none(*pud)) {
		printk(KERN_CRIT""ADDRMAP_DEV": not mapped in this pud\n");
		return ret;
	}
	pmd = pmd_offset(pud, in);
	//printk(KLL_DEBG""ADDRMAP_DEV": pmd = 0x%p\n", pmd);
	//printk(KLL_DEBG""ADDRMAP_DEV": pmd_val = 0x%p\n", (void*)pmd_val(*pmd));
	if (pmd_none(*pmd)) {
		printk(KERN_CRIT""ADDRMAP_DEV": not mapped in this pmd\n");
		return ret;
	}
	pte = pte_offset_kernel(pmd, in);
	//printk(KLL_DEBG""ADDRMAP_DEV": pte = 0x%p\n", pte);
	//printk(KLL_DEBG""ADDRMAP_DEV": pte_val = 0x%p\n", (void*)pte_val(*pte));
	if (pte_none(*pte)) {
		printk(KERN_CRIT""ADDRMAP_DEV": not mapped in this pte\n");
		return ret;
	}
	if (!pte_present(*pte)) {
		printk(KERN_CRIT""ADDRMAP_DEV": this pte is not in RAM\n");
		return ret;
	}
	ret = (pte_val(*pte) & PAGE_MASK) | (in & ~PAGE_MASK);
#if JEDI_BITS > 32
	/* Bug, mask any hiest 0x80. */
	ret &= ~(0x80UL << 56);
#endif

	return ret;
}

