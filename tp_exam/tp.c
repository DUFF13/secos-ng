#include <debug.h>
#include <intr.h>
#include <pic.h>
#include <io.h>
#include <types.h>
#include <cr.h>

#include "exam/paging.h"
#include "exam/tasks.h"
#include "exam/syscalls.h"

extern void irq0_isr(void);
extern void syscall_isr(void);

static void pit_set_hz(uint32_t hz)
{
    uint32_t divisor = 1193182u / hz;

    outb(0x36, 0x43);
    outb((uint8_t)(divisor & 0xFFu), 0x40);
    outb((uint8_t)((divisor >> 8) & 0xFFu), 0x40);
}

static void pic_unmask_irq0(void)
{
    uint8_t mask = inb(0x21);
    mask &= (uint8_t)~1u;
    outb(mask, 0x21);
}

static void idt_install_gate(uint8_t vec, void (*isr)(void), uint8_t dpl)
{
    idt_reg_t idtr;
    get_idtr(idtr);

    int_desc_t *idt = (int_desc_t *)idtr.addr;

    build_int_desc(&idt[vec], KRN_CS_SEL, (offset_t)isr);
    idt[vec].dpl = dpl;
}

void tp(void)
{
    debug("=== EXAM: start ===\n");

    intr_init();
    exam_tss_init();

    idt_install_gate(32, irq0_isr, 0);
    idt_install_gate(0x80, syscall_isr, 3);

    pic_init();
    pit_set_hz(100);
    pic_unmask_irq0();

    exam_paging_init();
    exam_tasks_init();

    set_cr3(exam_tasks_current_cr3());
    set_cr0(get_cr0() | 0x80000000u);

    asm volatile("sti");

    debug("=== EXAM: enter task1 ===\n");
    exam_enter_first_task();

    for (;;) {
        asm volatile("hlt");
    }
}
