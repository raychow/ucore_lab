#include <pmm.h>
#include <list.h>
#include <string.h>
#include <buddy_pmm.h>

#define parent(n) (((n) - 1) / 2)
#define left_leaf(n) ((n) * 2 + 1)
#define right_leaf(n) ((n) * 2 + 2)

free_area_t free_area;

#define free_list (free_area.free_list)
#define nr_free (free_area.nr_free)

static inline uint8_t buddy_log2(unsigned int n) {
    unsigned int exp;
    if (0 == n) {
        exp = 0xFF;
    } else {
        exp = 0;
        while (n >>= 1) {
            ++exp;
        }
    }
    return exp;
}

static inline unsigned int buddy_pow2(uint8_t exp) {
    return 0xFF == exp ? 0 : 1 << exp;
}

static inline unsigned int get_block_size(struct Page *block_base) {
    return buddy_pow2(((uint8_t *)&block_base->property)[0]);
}

static inline void set_block_size(
        struct Page *block_base, unsigned int length) {
    ((uint8_t *)&block_base->property)[0] = buddy_log2(length);
}

static inline unsigned int get_longest(struct Page *base, unsigned int i) {
    ++i;
    uint8_t exp = ((uint8_t *)&base[i >> 2].property)[i & 0b11];
    return buddy_pow2(exp);
}

static inline void set_longest(struct Page *base,
        unsigned int i, unsigned int longest) {
    ++i;
    ((uint8_t *)&base[i >> 2].property)[i & 0b11] = buddy_log2(longest);
}

static void buddy_init(void) {
    list_init(&free_list);
}

static void buddy_init_memmap(struct Page *base, size_t n) {
    assert(n > 0);
    struct Page *p;
    for (p = base; p != base + n; ++p) {
        assert(PageReserved(p));
        p->flags = p->property = 0;
        set_page_ref(p, 0);
        SetPageProperty(p);
    }
    set_block_size(base, n);
    unsigned int block_size = get_block_size(base);
    unsigned int node_size = block_size << 1;
    int i;
    for (i = 0; i < 2 * block_size - 1; ++i) {
        if (is_power_of_2(i + 1)) {
            node_size >>= 1;
        }
        set_longest(base, i, node_size);
    }
    nr_free += block_size;
    list_add(&free_list, &(base->page_link));
}

static struct Page *
buddy_alloc_pages(size_t n) {
    assert(n > 0);
    if (!is_power_of_2(n)) {
        n = next_power_of_2(n);
    }

    struct Page *block_base = NULL;
    unsigned int block_size;
    list_entry_t *le = &free_list;
    while ((le = list_next(le)) != &free_list) {
        struct Page *p = le2page(le, page_link);
        if (get_longest(p, 0) >= n) {
            block_size = get_block_size(p);
            block_base = p;
            break;
        }
    }
    if (NULL == block_base) {
        return NULL;
    }

    unsigned int index = 0;
    unsigned int node_size;
    for (node_size = block_size; node_size != n; node_size /= 2) {
        unsigned int left_longest = get_longest(block_base, left_leaf(index));
        unsigned int right_longest = get_longest(block_base, right_leaf(index));
        if ((left_longest <= right_longest && left_longest >= n)
                || right_longest < n) {
            index = left_leaf(index);
        } else {
            index = right_leaf(index);
        }
    }

    set_longest(block_base, index, 0);
    unsigned int offset = (index + 1) * node_size - block_size;

    while (index) {
        index = parent(index);
        set_longest(block_base, index, max(
                get_longest(block_base, left_leaf(index)),
                get_longest(block_base, right_leaf(index))));
    }

    nr_free -= node_size;

    return block_base + offset;
}

static void buddy_free_pages(struct Page *base, size_t n) {
    struct Page *block_base = NULL;
    unsigned int block_size;
    list_entry_t *le = &free_list;
    while ((le = list_next(le)) != &free_list) {
        struct Page *p = le2page(le, page_link);
        block_size = get_block_size(p);
        if (p <= base && p + block_size > base) {
            block_base = p;
            break;
        }
    }

    if (NULL == block_base) {
        return;
    }

    unsigned int node_size = 1;
    unsigned int index = base - block_base + block_size - 1;

    for (; get_longest(block_base, index); index = parent(index)) {
        node_size *= 2;
        if (0 == index) {
            return;
        }
    }

    set_longest(block_base, index, node_size);
    nr_free += node_size;

    unsigned int left_longest;
    unsigned int right_longest;
    while (index) {
        index = parent(index);
        node_size *= 2;

        left_longest = get_longest(block_base, left_leaf(index));
        right_longest = get_longest(block_base, right_leaf(index));

        if (left_longest + right_longest == node_size) {
            set_longest(block_base, index, node_size);
        } else {
            set_longest(block_base, index, max(left_longest, right_longest));
        }
    }
}

static size_t buddy_nr_free_pages(void) {
    return nr_free;
}

static void default_check(void) {
    list_entry_t *list_entry_head = list_next(&free_list);
    assert(list_entry_head != &free_list);
    struct Page *block_base = le2page(list_entry_head, page_link);
    unsigned int block_size = get_block_size(block_base);
    assert(is_power_of_2(block_size));

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
    free_page(p1);
    free_page(p2);
    free_page(p3);

    unsigned int all_blocks_size = nr_free;
    int i;
    struct Page *prev_addr;
    list_entry_t *le = &free_list;
    while ((le = list_next(le)) != &free_list) {
        block_base = le2page(le, page_link);
        block_size = get_block_size(block_base);
        prev_addr = NULL;
        for (i = 0; i < block_size; ++i) {
            struct Page *addr = alloc_page();
            assert(NULL != addr
                    && (addr - 1 == prev_addr || NULL == prev_addr));
            prev_addr = addr;
        }
        assert(get_longest(block_base, 0) == 0);
    }
    assert(NULL == alloc_page());
    assert(0 == nr_free);

    while ((le = list_next(le)) != &free_list) {
        block_base = le2page(le, page_link);
        block_size = get_block_size(block_base);
        for (i = 0; i < block_size; ++i) {
            free_page(block_base + i);
        }
        assert(get_longest(block_base, 0) == block_size);
    }
    assert(all_blocks_size == nr_free);
}

const struct pmm_manager buddy_pmm_manager = { .name = "buddy_pmm_manager",
        .init = buddy_init, .init_memmap = buddy_init_memmap, .alloc_pages =
                buddy_alloc_pages, .free_pages = buddy_free_pages,
        .nr_free_pages = buddy_nr_free_pages, .check = default_check, };

