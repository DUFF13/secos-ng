#include <debug.h>
#include <string.h>
#include <types.h>
#include <cr.h>
#include <segmem.h>

#include "exam/tasks.h"
#include "exam/paging.h"

static task_t g_tasks[2];
static uint32_t g_cur = 0;

static tss_t TSS;

extern void user1(void);
extern void user2(void);

static void *volatile keep_user_syms[] = { (void *)user1, (void *)user2 };


typedef struct irq_frame {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t gs, fs, es, ds;
    uint32_t int_no, err;
    uint32_t eip, cs, eflags, useresp, ss;
} irq_frame_t;

static void write_seg_desc(seg_desc_t *d, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags)
{
    uint8_t *b = (uint8_t *)d;
    memset(d, 0, sizeof(*d));

    b[0] = (uint8_t)(limit & 0xFFu);
    b[1] = (uint8_t)((limit >> 8) & 0xFFu);

    b[2] = (uint8_t)(base & 0xFFu);
    b[3] = (uint8_t)((base >> 8) & 0xFFu);
    b[4] = (uint8_t)((base >> 16) & 0xFFu);

    b[5] = access;

    b[6] = (uint8_t)(((limit >> 16) & 0x0Fu) | (flags & 0xF0u));
    b[7] = (uint8_t)((base >> 24) & 0xFFu);
}

static void write_tss_desc(seg_desc_t *d, uint32_t base, uint32_t limit)
{
    write_seg_desc(d, base, limit, 0x89, 0x00);
}

void exam_tss_init(void)
{
    gdt_reg_t gdtr;
    get_gdtr(gdtr);

    seg_desc_t *gdt = (seg_desc_t *)gdtr.addr;

    uint32_t flat_limit = 0xFFFFFu;
    uint8_t flags_4k_32 = 0xC0;

    write_seg_desc(&gdt[3], 0u, flat_limit, 0xFA, flags_4k_32);
    write_seg_desc(&gdt[4], 0u, flat_limit, 0xF2, flags_4k_32);

    memset(&TSS, 0, sizeof(TSS));
    TSS.s0.ss = KRN_DS_SEL;

    write_tss_desc(&gdt[EXAM_TSS_IDX], (uint32_t)&TSS, (uint32_t)(sizeof(tss_t) - 1u));

    uint16_t needed_limit = (uint16_t)(((EXAM_TSS_IDX + 1) * sizeof(seg_desc_t)) - 1);
    if (gdtr.limit < needed_limit) {
        gdtr.limit = needed_limit;
        set_gdtr(gdtr);
    }

    set_tr(EXAM_TSS_SEL);

    debug("exam: tss ok (idx=%u sel=0x%x)\n", EXAM_TSS_IDX, (unsigned)EXAM_TSS_SEL);
}

static void build_initial_frame(task_t *t, uint32_t entry_eip)
{
    uint32_t *sp = (uint32_t *)t->kstack_top;

    *(--sp) = (uint32_t)USR_DS_SEL;        /* ss */
    *(--sp) = (uint32_t)t->ustack_top;     /* useresp */
    *(--sp) = 0x202u;                      /* eflags */
    *(--sp) = (uint32_t)USR_CS_SEL;        /* cs */
    *(--sp) = entry_eip;                   /* eip */

    *(--sp) = 0u;                          /* err */
    *(--sp) = 0u;                          /* int_no */

    *(--sp) = (uint32_t)USR_DS_SEL;        /* ds */
    *(--sp) = (uint32_t)USR_DS_SEL;        /* es */
    *(--sp) = (uint32_t)USR_DS_SEL;        /* fs */
    *(--sp) = (uint32_t)USR_DS_SEL;        /* gs */

    *(--sp) = 0u;                          /* eax */
    *(--sp) = 0u;                          /* ecx */
    *(--sp) = 0u;                          /* edx */
    *(--sp) = 0u;                          /* ebx */
    *(--sp) = 0u;                          /* esp (dummy, skipped by POP_ALL) */
    *(--sp) = 0u;                          /* ebp */
    *(--sp) = 0u;                          /* esi */
    *(--sp) = 0u;                          /* edi */

    t->kstack_top = (uint32_t)sp;
}

static void activate_task(uint32_t idx)
{
    task_t *t = &g_tasks[idx];
    set_cr3((uint32_t)t->pgd);
    TSS.s0.esp = t->kstack_top;
}

void exam_tasks_init(void)
{
    g_tasks[0].pgd = exam_pgd_t1();
    g_tasks[0].kstack_top = KSTACK1_PHYS + KSTACK_SZ;
    g_tasks[0].ustack_top = USTACK1_PHYS + USTACK_SZ;
    g_tasks[0].shared_va = SHARED_T1_VA;

    g_tasks[1].pgd = exam_pgd_t2();
    g_tasks[1].kstack_top = KSTACK2_PHYS + KSTACK_SZ;
    g_tasks[1].ustack_top = USTACK2_PHYS + USTACK_SZ;
    g_tasks[1].shared_va = SHARED_T2_VA;

    build_initial_frame(&g_tasks[0], T1_PHYS_BASE);
    build_initial_frame(&g_tasks[1], T2_PHYS_BASE);

    g_cur = 0;
    activate_task(g_cur);

    debug("exam: tasks ok (cur=%u)\n", g_cur);
    debug("ENTRY planned: t1=0x%x t2=0x%x\n", T1_PHYS_BASE, T2_PHYS_BASE);

}

uint32_t exam_tasks_current_cr3(void)
{
    return (uint32_t)g_tasks[g_cur].pgd;
}

void exam_enter_first_task(void)
{
    activate_task(g_cur);
    exam_switch_iret(g_tasks[g_cur].kstack_top);
    debug("CR3 cur=0x%x\n", exam_tasks_current_cr3());

}

void exam_task_switch_from_irq(void *frame)
{
    irq_frame_t *f = (irq_frame_t *)frame;

    if ((f->cs & 3u) != 3u) {
        return;
    }

    g_tasks[g_cur].kstack_top = (uint32_t)frame;

    g_cur ^= 1u;

    activate_task(g_cur);

    exam_switch_iret(g_tasks[g_cur].kstack_top);
}
