#include <types.h>

#include "exam/syscalls.h"
#include "exam/tasks.h"

static inline void sys_counter(uint32_t *counter)
{
    asm volatile(
        "movl $1, %%eax \n"
        "movl %0, %%ebx \n"
        "int $0x80 \n"
        :
        : "r"(counter)
        : "eax", "ebx"
    );
}

__attribute__((used, section(".user1")))
void user1(void)
{
    volatile uint32_t *c = (volatile uint32_t *)SHARED_T1_VA;
    uint32_t x = 0;

    for (;;) {
        *c = x++;
    }
}

__attribute__((used, section(".user2")))
void user2(void)
{
    volatile uint32_t *c = (volatile uint32_t *)SHARED_T2_VA;

    for (;;) {
        sys_counter((uint32_t *)c);
    }
}
