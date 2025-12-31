#!/usr/bin/make -f

isr_exam.o: isr_exam.S
	$(CC) -m32 -c -o $@ $<


include ../utils/config.mk

LDSCRIPT = ./linker_exam.lds
INCLUDE += -I./include

objects += tp.o tasks.o paging_tasks.o syscalls.o user_tasks.o isr_exam.o

include ../utils/rules.mk

#-include $(dependencies)
