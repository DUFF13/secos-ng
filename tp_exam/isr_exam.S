.code32
.section .text
.global irq0_isr
.global syscall_isr
.global exam_switch_iret

.extern exam_task_switch_from_irq
.extern syscall_0x80_handler

.macro PUSH_ALL
    push %eax
    push %ecx
    push %edx
    push %ebx
    push %esp
    push %ebp
    push %esi
    push %edi
.endm

.macro POP_ALL
    pop %edi
    pop %esi
    pop %ebp
    add $4, %esp
    pop %ebx
    pop %edx
    pop %ecx
    pop %eax
.endm

irq0_isr:
    push $0
    push $32

    push %ds
    push %es
    push %fs
    push %gs

    PUSH_ALL

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es

    mov %esp, %eax
    push %eax
    call exam_task_switch_from_irq
    add $4, %esp

    movb $0x20, %al
    outb %al, $0x20

    POP_ALL

    pop %gs
    pop %fs
    pop %es
    pop %ds

    add $8, %esp
    iret

syscall_isr:
    push $0
    push $128

    push %ds
    push %es
    push %fs
    push %gs

    PUSH_ALL

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es

    mov %esp, %eax
    push %eax
    call syscall_0x80_handler
    add $4, %esp

    POP_ALL

    pop %gs
    pop %fs
    pop %es
    pop %ds

    add $8, %esp
    iret

exam_switch_iret:
    mov 4(%esp), %esp

    POP_ALL

    pop %gs
    pop %fs
    pop %es
    pop %ds

    add $8, %esp
    iret
