#ifndef __KERN_MM_SWAP_ECLOCK_H__
#define __KERN_MM_SWAP_ECLOCK_H__

#include <swap.h>

typedef struct {
    list_entry_t pra_list_head;
    list_entry_t *hand;
} eclock_private_t;

extern struct swap_manager swap_manager_eclock;

#endif
