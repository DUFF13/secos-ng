#ifndef EXAM_SYSCALLS_H
#define EXAM_SYSCALLS_H

#include <types.h>

#define EXAM_SYSCALL_COUNTER  1u

void syscall_0x80_handler(void *frame);

#endif
