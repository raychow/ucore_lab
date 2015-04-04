#include <defs.h>
#include <x86.h>
#include <stdio.h>
#include <string.h>
#include <swap.h>
#include <swap_eclock.h>
#include <list.h>

eclock_private_t eclock_private;

static int
_eclock_init_mm(struct mm_struct *mm) {
    list_init(&eclock_private.pra_list_head);
    eclock_private.hand = &eclock_private.pra_list_head;
    mm->sm_priv = &eclock_private;
    return 0;
}

static int
_eclock_map_swappable(struct mm_struct *mm, uintptr_t addr, struct Page *page, int swap_in) {
    eclock_private_t *private = (eclock_private_t *)mm->sm_priv;
    assert(NULL != private);
    list_entry_t *hand = private->hand;
    list_entry_t *entry = &(page->pra_page_link);
    assert(NULL != hand && NULL != entry);
    list_add(hand, entry);
    private->hand = entry;
    return 0;
}

static int
_eclock_swap_out_victim(struct mm_struct *mm, struct Page ** ptr_page, int in_tick) {
    eclock_private_t *private = (eclock_private_t *)mm->sm_priv;
    assert(NULL != private);
    list_entry_t *hand = private->hand;
    assert(NULL != hand && hand != hand->next);
    assert(in_tick == 0);

    while (1) {
        if ((hand = hand->next) != &private->pra_list_head) {
            struct Page *p = le2page(hand, pra_page_link);
            pte_t *ptep = get_pte(mm->pgdir, p->pra_vaddr, 0);
            assert(NULL != ptep && (*ptep & PTE_P));
            if (*ptep & PTE_A) {
                *ptep &= ~PTE_A;
                tlb_invalidate(mm->pgdir, p->pra_vaddr);
            } else if (*ptep & PTE_D) {
                *ptep &= ~PTE_D;
                tlb_invalidate(mm->pgdir, p->pra_vaddr);
            } else {
                *ptr_page = p;
                break;
            }
        }
    }
    private->hand = hand->prev;
    list_del(hand);
    return 0;
}

static int
_eclock_check_swap(void) {
    cprintf("write Virt Page c in eclock_check_swap\n");
    *(unsigned char *)0x3000 = 0x0c;
    assert(4 == pgfault_num);
    cprintf("write Virt Page a in eclock_check_swap\n");
    *(unsigned char *)0x1000 = 0x0a;
    assert(4 == pgfault_num);
    cprintf("write Virt Page d in eclock_check_swap\n");
    *(unsigned char *)0x4000 = 0x0d;
    assert(4 == pgfault_num);
    cprintf("write Virt Page b in eclock_check_swap\n");
    *(unsigned char *)0x2000 = 0x0b;
    assert(4 == pgfault_num);
    cprintf("write Virt Page e in eclock_check_swap\n");
    *(unsigned char *)0x5000 = 0x0e;
    assert(5 == pgfault_num);
    cprintf("write Virt Page b in eclock_check_swap\n");
    *(unsigned char *)0x2000 = 0x0b;
    assert(5 == pgfault_num);
    cprintf("write Virt Page a in eclock_check_swap\n");
    *(unsigned char *)0x1000 = 0x0a;
    assert(6 == pgfault_num);
    cprintf("write Virt Page b in eclock_check_swap\n");
    *(unsigned char *)0x2000 = 0x0b;
    assert(6 == pgfault_num);
    cprintf("write Virt Page c in eclock_check_swap\n");
    *(unsigned char *)0x3000 = 0x0c;
    assert(7 == pgfault_num);
    cprintf("write Virt Page d in eclock_check_swap\n");
    *(unsigned char *)0x4000 = 0x0d;
    assert(8 == pgfault_num);

    cprintf("read Virt Page a in eclock_check_swap\n");
    volatile unsigned char t = *(unsigned char *)0x1000;
    assert(8 == pgfault_num);
    cprintf("read Virt Page b in eclock_check_swap\n");
    *(unsigned char *)0x2000 = 0x0b;
    assert(8 == pgfault_num);
    cprintf("write Virt Page c in eclock_check_swap\n");
    *(unsigned char *)0x3000 = 0x0c;
    assert(8 == pgfault_num);
    cprintf("write Virt Page d in eclock_check_swap\n");
    *(unsigned char *)0x4000 = 0x0d;
    assert(8 == pgfault_num);
    cprintf("write Virt Page e in eclock_check_swap\n");
    *(unsigned char *)0x5000 = 0x0e;
    assert(9 == pgfault_num);
    cprintf("read Virt Page b in eclock_check_swap\n");
    *(unsigned char *)0x2000 = 0x0b;
    assert(9 == pgfault_num);
    cprintf("write Virt Page a in eclock_check_swap\n");
    *(unsigned char *)0x1000 = 0x0a;
    assert(10 == pgfault_num);
    return 0;
}

static int
_eclock_init(void) {
    return 0;
}

static int
_eclock_set_unswappable(struct mm_struct *mm, uintptr_t addr) {
    return 0;
}

static int
_eclock_tick_event(struct mm_struct *mm) {
    return 0;
}

struct swap_manager swap_manager_eclock =
{
     .name            = "eclock swap manager",
     .init            = &_eclock_init,
     .init_mm         = &_eclock_init_mm,
     .tick_event      = &_eclock_tick_event,
     .map_swappable   = &_eclock_map_swappable,
     .set_unswappable = &_eclock_set_unswappable,
     .swap_out_victim = &_eclock_swap_out_victim,
     .check_swap      = &_eclock_check_swap,
};
