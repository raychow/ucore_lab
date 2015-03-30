#include <pmm.h>
#include <list.h>
#include <string.h>
#include <buddy_pmm.h>

// Only use one memory block now.

#define parent(n) (((n) - 1) / 2)
#define left_leaf(n) ((n) * 2 + 1)
#define right_leaf(n) ((n) * 2 + 2)

struct Page *buddy_base;
unsigned int buddy_size;

static inline unsigned int get_longest(struct Page *bb, unsigned int i) {
    unsigned int __i = i >> 1;
    if (i & 1) {
        return (unsigned int)bb[__i].page_link.next;
    } else {
        return (unsigned int)bb[__i].page_link.prev;
    }
}

static inline void set_longest(struct Page *bb,
        unsigned int i, unsigned int longset) {
    unsigned int __i = i >> 1;
    if (i & 1) {
        bb[__i].page_link.next = (struct list_entry *)longset;
    } else {
        bb[__i].page_link.prev = (struct list_entry *)longset;
    }
}

static void buddy_init(void) {
    buddy_base = NULL;
    buddy_size = 0;
}

static void buddy_init_memmap(struct Page *base, size_t n) {
    assert(n > 0);
    if (buddy_base) {
        return;
    }
    buddy_base = base;
    buddy_size = previous_power_of_2(n);
    unsigned int node_size = buddy_size << 1;
    int i = 0;
    for (; i < 2 * buddy_size - 1; ++i) {
        struct Page *p = buddy_base + (i >> 1);
        assert((i & 1) || PageReserved(p));
        p->flags = p->property = 0;
        set_page_ref(p, 0);
        if (is_power_of_2(i + 1)) {
            node_size >>= 1;
        }
        if (i & 1) {
            p->page_link.next = (struct list_entry *)node_size;
        } else {
            p->page_link.prev = (struct list_entry *)node_size;
        }
    }
}

static struct Page *
buddy_alloc_pages(size_t n) {
    assert(buddy_base && n > 0);
    if (!is_power_of_2(n)) {
        n = next_power_of_2(n);
    }
    if ((size_t)buddy_base->page_link.prev < n) {
        return NULL;
    }

    unsigned int index = 0;
    unsigned int node_size;
    for (node_size = buddy_size; node_size != n; node_size /= 2) {
        unsigned int left_longest = get_longest(buddy_base, left_leaf(index));
        unsigned int right_longest = get_longest(buddy_base, right_leaf(index));
        if ((left_longest <= right_longest && left_longest >= n)
                || right_longest < n) {
            index = left_leaf(index);
        } else {
            index = right_leaf(index);
        }
    }

    set_longest(buddy_base, index, 0);
    unsigned int offset = (index + 1) * node_size - buddy_size;

    while (index) {
        index = parent(index);
        set_longest(buddy_base, index, max(
                get_longest(buddy_base, left_leaf(index)),
                get_longest(buddy_base, right_leaf(index))));
    }

    return buddy_base + offset;
}

static void buddy_free_pages(struct Page *base, size_t n) {
    unsigned int offset = base - buddy_base;
    assert(buddy_base && offset < buddy_size);
    unsigned int node_size = 1;
    unsigned int index = offset + buddy_size - 1;

    for (; get_longest(buddy_base, index); index = parent(index)) {
        node_size *= 2;
        if (0 == index) {
            return;
        }
    }

    set_longest(buddy_base, index, node_size);

    unsigned int left_longest;
    unsigned int right_longest;
    while (index) {
        index = parent(index);
        node_size *= 2;

        left_longest = get_longest(buddy_base, left_leaf(index));
        right_longest = get_longest(buddy_base, right_leaf(index));

        if (left_longest + right_longest == node_size) {
            set_longest(buddy_base, index, node_size);
        } else {
            set_longest(buddy_base, index, max(left_longest, right_longest));
        }
    }
}

static size_t buddy_nr_free_pages(void) {
    return (size_t)buddy_base->page_link.prev;
}

static void default_check(void) {
    assert(buddy_base);
    assert(buddy_size > 0);
    struct Page *p0 = alloc_pages(1);
    struct Page *p1 = alloc_pages(2);
    struct Page *p2 = alloc_pages(3);
    struct Page *p3 = alloc_pages(4);
    assert(NULL != p0);
    assert(p0 + 2 == p1);
    assert(p0 + 4 == p2);
    assert(p0 + 8 == p3);

    free_page(p1);
    p1 = alloc_pages(2);
    assert(p0 + 2 == p1);

    free_page(p0);
    p0 = alloc_pages(2);
    assert(p0 + 2 == p1);

    free_page(p0);
    p0 = alloc_pages(8192);
    assert(NULL != p0);

    free_page(p1);
    p1 = alloc_pages(4097);
    assert(NULL == p1);

    free_page(p0);
    free_page(p2);
    free_page(p3);

    int i;
    struct Page *prev_addr = NULL;
    for (i = 0; i < buddy_size; ++i) {
        struct Page *addr = alloc_page();
        assert(NULL != addr && (addr - 1 == prev_addr || NULL == prev_addr));
        prev_addr = addr;
    }
    assert(NULL == alloc_page());
    for (i = 0; i < buddy_size; ++i) {
        free_page(buddy_base + i);
    }
    assert((unsigned int)buddy_base->page_link.prev == buddy_size);
}

const struct pmm_manager buddy_pmm_manager = { .name = "buddy_pmm_manager",
        .init = buddy_init, .init_memmap = buddy_init_memmap, .alloc_pages =
                buddy_alloc_pages, .free_pages = buddy_free_pages,
        .nr_free_pages = buddy_nr_free_pages, .check = default_check, };

