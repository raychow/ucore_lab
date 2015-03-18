# Lab 2

## 练习 1: 实现 first-fit 连续物理内存分配算法

1. default_init

    没有变动。

2. default_init_memmap

    没有变动。

3. default_alloc_pages

    为了在分配页面后保持页面的有序性，如果对于空闲列表中的一项，被分配后仍有剩余页面，则将这些剩余页面作为新的一项，插入到来列表项的位置。

4. default_free_pages

    为了在分配页面后保持页面的有序性，将释放页面后的列表项插入到正确的位置。
