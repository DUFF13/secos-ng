/* GPLv2 (c) Airbus */
OUTPUT_FORMAT("elf32-i386","elf32-i386","elf32-i386");
OUTPUT_ARCH("i386")

ENTRY(entry)

PHDRS
{
   phboot   PT_LOAD FLAGS (7);
   phstack  PT_LOAD FLAGS (6);
   phsetup  PT_LOAD FLAGS (7);
   phuser1  PT_LOAD FLAGS (7);
   phuser2  PT_LOAD FLAGS (7);
}

SECTIONS
{
   . = 0x300000;
   .mbh      : { KEEP(*(.mbh)) . = ALIGN(4);     } : phboot
   .stack    : { KEEP(*(.stack))                 } : phstack

   __kernel_start__ = .;

   .idt_jmp  : { KEEP(*(.idt_jmp))               } : phsetup
   .text     : { *(.text) *(.text.*)             } : phsetup
   .rodata   : { *(.rodata) *(.rodata.*)         } : phsetup
   .data     : { *(.data) *(.data.*)             } : phsetup
   .bss      : { *(.bss COMMON) *(.bss.*)        } : phsetup
   /DISCARD/ : { *(.note* .indent .comment)      } : phsetup

   __kernel_end__ = .;

   . = 0x00400000;
   .user1 : ALIGN(4096)
   {
      KEEP(*(.user1))
   } : phuser1

   . = 0x00800000;
   .user2 : ALIGN(4096)
   {
      KEEP(*(.user2))
   } : phuser2
}
