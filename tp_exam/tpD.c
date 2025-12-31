#include <cr.h>
#include <debug.h>
#include <pagemem.h>
#include <segmem.h>

#include "pagination.h"
#include "utils.h"

seg_desc_t GDT[6];
tss_t TSS;
#define VIRTUAL_KERNEL_STACK 0xC0000000
#define PHY_KERNEL_STACK 0x201000

#define c0_idx 1
#define d0_idx 2
#define c3_idx 3
#define d3_idx 4
#define ts_idx 5

#define c0_sel gdt_krn_seg_sel(c0_idx)
#define d0_sel gdt_krn_seg_sel(d0_idx)
#define c3_sel gdt_usr_seg_sel(c3_idx)
#define d3_sel gdt_usr_seg_sel(d3_idx)
#define ts_sel gdt_krn_seg_sel(ts_idx)

#define gdt_flat_dsc(_dSc_, _pVl_, _tYp_)                                      \
  ({                                                                           \
    (_dSc_)->raw = 0;                                                          \
    (_dSc_)->limit_1 = 0xffff;                                                 \
    (_dSc_)->limit_2 = 0xf;                                                    \
    (_dSc_)->type = _tYp_;                                                     \
    (_dSc_)->dpl = _pVl_;                                                      \
    (_dSc_)->d = 1;                                                            \
    (_dSc_)->g = 1;                                                            \
    (_dSc_)->s = 1;                                                            \
    (_dSc_)->p = 1;                                                            \
  })

#define tss_dsc(_dSc_, _tSs_)                                                  \
  ({                                                                           \
    raw32_t addr = {.raw = _tSs_};                                             \
    (_dSc_)->raw = sizeof(tss_t);                                              \
    (_dSc_)->base_1 = addr.wlow;                                               \
    (_dSc_)->base_2 = addr._whigh.blow;                                        \
    (_dSc_)->base_3 = addr._whigh.bhigh;                                       \
    (_dSc_)->type = SEG_DESC_SYS_TSS_AVL_32;                                   \
    (_dSc_)->p = 1;                                                            \
  })

#define c0_dsc(_d) gdt_flat_dsc(_d, 0, SEG_DESC_CODE_XR)
#define d0_dsc(_d) gdt_flat_dsc(_d, 0, SEG_DESC_DATA_RW)
#define c3_dsc(_d) gdt_flat_dsc(_d, 3, SEG_DESC_CODE_XR)
#define d3_dsc(_d) gdt_flat_dsc(_d, 3, SEG_DESC_DATA_RW)

void init_tss() {

  debug("=======Setting up TSS=======\n");
  // Task State Segment descriptor
  initialize_segment_descriptor(&GDT[ts_idx], (offset_t)&TSS, sizeof(tss_t),
                                SEG_DESC_SYS_TSS_AVL_32, 0b00, 0, 0, 0, 0, 1);
  memset(&TSS, 0, sizeof(tss_t));
  TSS.s0.ss = make_seg_sel(2, 0, 0);
  TSS.s0.esp = (uint32_t)(PHY_KERNEL_STACK);
  short tr = make_seg_sel(5, 0, 0);
  set_tr(tr);
  // TSS.ss = make_seg_sel(5, 0, 3); // Ring 3 stack segment
  debug("=======Setting up TSS=======\n");
  display_tss(&TSS);
}
void init_gdt() {
  gdt_reg_t gdtr;

  GDT[0].raw = 0ULL;

  c0_dsc(&GDT[c0_idx]);
  d0_dsc(&GDT[d0_idx]);
  c3_dsc(&GDT[c3_idx]);
  d3_dsc(&GDT[d3_idx]);

  gdtr.desc = GDT;
  gdtr.limit = sizeof(GDT) - 1;
  set_gdtr(gdtr);

  set_cs(c0_sel);

  debug("cs\n");
  print_selector_content(c0_sel);

  set_ss(d0_sel);
  set_ds(d0_sel);
  set_es(d0_sel);
  set_fs(d0_sel);
  set_gs(d0_sel);

  debug("ss, ds, es, fs, gs\n");
  print_selector_content(d0_sel);

  init_tss();
  print_gdt_content(gdtr);

  debug("======Finished Setting up GDT ======\n");
}

#define PGD_ADDR_KERNEL 0x1000000
#define PT_BASE_KERNEL 0x1001000

#define PGD_ADDR_PROCESS1 0x1500000
#define PT_BASE_PROCESS1 0x1501000

#define PGD_ADDR_PROCESS2 0x1a00000
#define PT_BASE_PROCESS2 0x1a01000

void init_paging() {
  pde32_t *pgd_kernel = (pde32_t *)PGD_ADDR_KERNEL;
  pde32_t *pgd_process1 = (pde32_t *)PGD_ADDR_PROCESS1;
  pde32_t *pgd_process2 = (pde32_t *)PGD_ADDR_PROCESS2;

  memset((void *)pgd_kernel, 0, PAGE_SIZE);
  memset((void *)pgd_process1, 0, PAGE_SIZE);
  memset((void *)pgd_process2, 0, PAGE_SIZE);

  // Map Kernel space 0x0-0x2fffff
  map_addresses(pgd_kernel, (uint32_t)PT_BASE_KERNEL, 0x0, 0x0, 0x400000,
                PG_KRN | PG_RW);

  // Map Kernel space of page tables 0x1000000-0x2000000
  map_addresses(pgd_kernel, (uint32_t)PT_BASE_KERNEL, 0x1000000, 0x1000000,
                0x1000000, PG_KRN | PG_RW);

  // Map Process 1 space 0x300000-0x3fffff
  map_addresses(pgd_process1, (uint32_t)PT_BASE_PROCESS1, 0x400000, 0x400000,
                0x100000, PG_USR | PG_RW);

  // Map Process 2 space 0x400000-0x4fffff
  map_addresses(pgd_process2, (uint32_t)PT_BASE_PROCESS2, 0x500000, 0x500000,
                0x100000, PG_USR | PG_RW);

  // Map shared memory 0x500000 - 0x501000
  map_addresses(pgd_process1, (uint32_t)PT_BASE_PROCESS1, 0x600000, 0x600000,
                0x1000, PG_USR | PG_RW);
  map_addresses(pgd_process2, (uint32_t)PT_BASE_PROCESS2, 0x700000, 0x600000,
                0x1000, PG_USR);

  // Map Process kernel stacks
  map_addresses(pgd_process1, (uint32_t)PT_BASE_PROCESS1, VIRTUAL_KERNEL_STACK,
                0x200000, 0x1000, PG_KRN | PG_RW);
  map_addresses(pgd_process2, (uint32_t)PT_BASE_PROCESS2, VIRTUAL_KERNEL_STACK,
                0x201000, 0x1000, PG_KRN | PG_RW);

  set_cr3((uint32_t)pgd_kernel);
  uint32_t cr0 = get_cr0();
  set_cr0(cr0 | CR0_PG);
  analyze_page_mapping(pgd_process2);
}

__attribute__((section(".process1"))) void process1() {
  while (1) {
  };
}

__attribute__((section(".process2"))) void process2() {
  while (1) {
  };
}

#define PROCESS1_STACK_ADDR 0x4fffff
#define PROCESS2_STACK_ADDR 0x5fffff

void ring3_transition() {
  asm volatile("push %0 \n" // ss
               "push %1 \n" // esp pour du ring 3 !
               "pushf   \n" // eflags
               "push %2 \n" // cs
               "push %3 \n" // eip
               "iret" ::"i"(d3_sel),
               "i"(PROCESS1_STACK_ADDR), "i"(c3_sel), "r"(&process1));
}

void tp() {
  init_gdt();
  init_paging();
   // ring3_transition();
}
