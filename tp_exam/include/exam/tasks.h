#ifndef EXAM_TASKS_H
#define EXAM_TASKS_H

#include <types.h>
#include <pagemem.h>

#define EXAM_PAGE_SZ        0x1000

#define T1_PHYS_BASE        0x00400000u
#define T2_PHYS_BASE        0x00800000u
#define USER_REGION_SZ      0x00100000u

#define SHARED_PHYS         0x00A00000u
#define SHARED_SZ           EXAM_PAGE_SZ
#define SHARED_T1_VA        0x00600000u
#define SHARED_T2_VA        0x00700000u

#define KSTACK1_PHYS        0x00201000u
#define KSTACK2_PHYS        0x00202000u
#define KSTACK_SZ           EXAM_PAGE_SZ

#define USTACK1_PHYS        (T1_PHYS_BASE + 0x0000F000u)
#define USTACK2_PHYS        (T2_PHYS_BASE + 0x0000F000u)
#define USTACK_SZ           EXAM_PAGE_SZ

#define KRN_CS_SEL          0x08
#define KRN_DS_SEL          0x10
#define USR_CS_SEL          0x1B
#define USR_DS_SEL          0x23

#define EXAM_TSS_IDX        6
#define EXAM_TSS_SEL        0x30

typedef struct task {
    pde32_t *pgd;
    uint32_t kstack_top;
    uint32_t ustack_top;
    uint32_t shared_va;
} task_t;

void exam_switch_iret(uint32_t next_esp);

void exam_tss_init(void);
void exam_tasks_init(void);

uint32_t exam_tasks_current_cr3(void);
void exam_enter_first_task(void);

void exam_task_switch_from_irq(void *frame);

static inline void irq0_handler(void *frame) { exam_task_switch_from_irq(frame); }

#endif
