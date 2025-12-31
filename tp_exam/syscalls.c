#include <debug.h>
#include <types.h>

#include "exam/syscalls.h"

typedef struct irq_frame {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t gs, fs, es, ds;
    uint32_t int_no, err;
    uint32_t eip, cs, eflags, useresp, ss;
} irq_frame_t;

void syscall_0x80_handler(void *frame)
{
    irq_frame_t *f = (irq_frame_t *)frame;

    if (f->eax == EXAM_SYSCALL_COUNTER) {
        uint32_t v = *(uint32_t *)f->ebx;
        debug("counter=%u\n", v);
        return;
    }

    debug("syscall: unknown %u\n", f->eax);
}
