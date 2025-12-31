#include <debug.h>
#include <string.h>
#include <types.h>
#include <pagemem.h>

#include "exam/paging.h"
#include "exam/tasks.h"

static pde32_t *const pgd_k = (pde32_t *)EXAM_PGD_KERNEL_PHYS;
static pde32_t *const pgd_1 = (pde32_t *)EXAM_PGD_T1_PHYS;
static pde32_t *const pgd_2 = (pde32_t *)EXAM_PGD_T2_PHYS;

static uint32_t pt_alloc_cursor = EXAM_PTB_KERNEL_BASE_PHYS;

static uint32_t *alloc_pt_raw(void)
{
    uint32_t *pt_raw = (uint32_t *)pt_alloc_cursor;
    pt_alloc_cursor += 0x1000u;
    memset(pt_raw, 0, 0x1000);
    return pt_raw;
}

static void map_page(pde32_t *pgd, uint32_t va, uint32_t pa, uint32_t flags)
{
    uint32_t pdi = (va >> 22) & 0x3FFu;
    uint32_t pti = (va >> 12) & 0x3FFu;

    uint32_t *pgd_raw = (uint32_t *)pgd;

    if ((pgd_raw[pdi] & PG_P) == 0u) {
        uint32_t *pt_raw = alloc_pt_raw();
        pgd_raw[pdi] = ((uint32_t)pt_raw) | PG_P | PG_RW | (flags & PG_USR);
    }

    uint32_t *pt_raw = (uint32_t *)(pgd_raw[pdi] & 0xFFFFF000u);
    pt_raw[pti] = (pa & 0xFFFFF000u) | PG_P | (flags & (PG_RW | PG_USR));
}

static void map_range(pde32_t *pgd, uint32_t va, uint32_t pa, uint32_t sz, uint32_t flags)
{
    for (uint32_t off = 0; off < sz; off += 0x1000u) {
        map_page(pgd, va + off, pa + off, flags);
    }
}

static void build_kernel_map(void)
{
    memset(pgd_k, 0, 0x1000);

    pt_alloc_cursor = EXAM_PTB_KERNEL_BASE_PHYS;

    map_range(pgd_k, 0x00000000u, 0x00000000u, 0x00400000u, PG_RW);
}

static void build_task_map(pde32_t *pgd, uint32_t user_phys_base, uint32_t shared_va)
{
    memset(pgd, 0, 0x1000);

    memcpy(pgd, pgd_k, 0x1000);

    map_range(pgd, user_phys_base, user_phys_base, USER_REGION_SZ, PG_RW | PG_USR);

    map_range(pgd, shared_va, SHARED_PHYS, SHARED_SZ, PG_RW | PG_USR);
}

void exam_paging_init(void)
{
    build_kernel_map();

    build_task_map(pgd_1, T1_PHYS_BASE, SHARED_T1_VA);
    build_task_map(pgd_2, T2_PHYS_BASE, SHARED_T2_VA);

    debug("exam: paging ok (pgd k=0x%p t1=0x%p t2=0x%p)\n", pgd_k, pgd_1, pgd_2);
}

pde32_t *exam_pgd_kernel(void) { return pgd_k; }
pde32_t *exam_pgd_t1(void) { return pgd_1; }
pde32_t *exam_pgd_t2(void) { return pgd_2; }
