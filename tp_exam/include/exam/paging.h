#ifndef EXAM_PAGING_H
#define EXAM_PAGING_H

#include <types.h>
#include <pagemem.h>

/* Physical placement for paging structures (aligned on 4KB). */
#define EXAM_PGD_KERNEL_PHYS        0x00100000u
#define EXAM_PGD_T1_PHYS            0x00101000u
#define EXAM_PGD_T2_PHYS            0x00102000u

#define EXAM_PTB_KERNEL_BASE_PHYS   0x00103000u
#define EXAM_PTB_T1_BASE_PHYS       0x00107000u
#define EXAM_PTB_T2_BASE_PHYS       0x0010B000u

void exam_paging_init(void);

pde32_t *exam_pgd_kernel(void);
pde32_t *exam_pgd_t1(void);
pde32_t *exam_pgd_t2(void);

#endif
