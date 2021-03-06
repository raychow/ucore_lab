# Lab 2

## 练习 1: 实现 first-fit 连续物理内存分配算法

1. `default_init`

    没有变动。

2. `default_init_memmap`

    没有变动。

3. `default_alloc_pages`

    为了在分配页面后保持页面的有序性，如果对于空闲列表中的一项，被分配后仍有剩余页面，则将这些剩余页面作为新的一项，插入到来列表项的位置。

4. `default_free_pages`

    为了在分配页面后保持页面的有序性，将释放页面后的列表项插入到正确的位置。

## 练习 2: 实现寻找虚拟地址对应的页表项

* 在 `get_pte` 函数中：

    1. 寻找一级页表项，给定一级页表基地址为 `pgdir`，线性地址为 `la`，则 `PDX(la)` 为一级页表项索引， `pgdir + PDX(la)` 为一级页表项位置；

    2. 检查二级页表是否存在，即一级页表项的 PTE_P 是否置位，如果不存在，如果需要 `create` 则进入步骤 3，否则返回 NULL；

    3. 如果需要创建二级页表，首先使用 `alloc_page` 在物理内存分配一个新的页面，用于存放二级页表，并设置好这个页面的引用次数，将此页面内容全部置 0，并将物理地址与属性登记在一级页表项；

    4. 最后，返回二级页表项地址，这个地址应该是线性地址。

* 如何 ucore 执行过程中访问内存，出现了页访问异常，请问硬件要做哪些事情？

    访问内存时出现页访问异常，硬件应该产生缺页中断，首先将中断号与错误信息压入堆栈，再保存现场，转交中断处理程序处理。

* 如何 ucore 的缺页服务例程在执行过程中访问内存，出现了页访问异常，请问硬件要做哪些事情？

    通常情况下是不会出现这种双重中断的问题的，出现了的话就说明内核写的有问题了。在 [Intel® 64 and IA-32 Architectures](http://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-system-programming-manual-325384.pdf) 的 6-28 中，明确指出在处理缺页异常时再次发生缺页异常，会导致发生 Double Fault 异常，只能保存程序的上下文信息并强制关闭程序。

    另参见: [Page fault in Interrupt contex](http://stackoverflow.com/questions/4848457/page-fault-in-interrupt-context)

## 练习 3: 释放某虚地址所在的页并取消对应二级页表项的映射

* 在 `page_remove_pte` 函数中：

    1. 检查页表项是否存在，如果存在才进行移除；

    2. 通过页表项内容找到页面地址，减少一次页面引用，如果页面引用为 0，释放这个页面；

    3. 清除页表项内容，无效快表内容。

* 数据结构 `Page` 的全局变量（其实是一个数组） 的每一项与页表中的页目录项和页表项有无对应关系？ 如果有，其对应关系是啥？

    有对应关系，页目录项或页表项的高 20 位实际上存放的是 `pages` 数组的索引，也就是页面物理地址的高 20 位。

* 如果希望虚拟地址与物理地址相等，则需要如何修改 lab2，完成此事？

    在 `get_pte` 调用 `alloc_page` 时，指定分配与虚拟地址相同的物理地址，就能实现。

## 扩展练习 Challenge: buddy system (伙伴系统)分配算法

参考了[伙伴分配器的一个极简实现](http://coolshell.cn/articles/10427.html)这篇文章实现了伙伴系统。

为了方便实现，这个伙伴系统的实现具有如下限制：

1. ~~只能管理某一块内存（通常是第一块），因为伙伴系统是对一块连续内存的管理。如果要管理所有内存，则还需在使用伙伴系统之前实现某种用于选择内存块的算法。~~更新：已经能够管理所有内存块。

2. ~~为了节省内存使用，`Page` 结构中 `page_link` 被用于存放 `longest` 数据，因为这个成员在伙伴系统中是没有使用的。而 `property` 成员并未使用。~~更新：使用幂来表示 `longest`，使得占用的空间减少四分之三，因此 `longest` 数据可以存放于 `property` 中。
在最新的实现中，`Page` 结构的 `property` 被赋予两种用途，内存块的第一个 `Page`结构的 `property` 的低 8 位表示了内存块大小的幂，而更高位则表示了 longest 数据。也就是说，分配树节点 0 的 `longest` 存放于第一个 `Page` 结构的 `property` 的第 8 到第 15 位，以此类推。

3. 回收内存的函数只能指定分配内存函数返回的页面作为参数，而不是任意一个已被分配的页面，这是为了高效实现内存分配与回收算法限制的。进一步的，此函数亦无法指定要回收内存的大小，因为回收的一定是分配出去的一整块内存。

在 `kern/mm/buddy_pmm.h` 中，声明了伙伴内存管理器 `buddy_pmm_manager`,只要将 `init_pmm_manager()` 中的 pmm_manager 改成 `buddy_pmm_manager` 就能使用伙伴内存管理器。

此外，在 `libs/defs.h` 中加入了几个辅助宏定义，`is_power_of_2` 判断一个数是否为 2 的整数次幂，`previous_power_of_2` 返回小于等于一个数的最接近 2 的整数次幂的数字，`previous_power_of_2` 返回小于等于一个数的最接近 2 的整数次幂的数字，`next_power_of_2` 返回大于等于一个数的最接近 2 的整数次幂的数字，而 `max` 返回两个数中较大的数字。

主要的实现在 `kern/mm/buddy_pmm.c` 中，包含如下全局变量:

1. `free_area`

    记录了所有可用的内存块以及剩余空间总和。

2. `buddy_pmm_manager`

    伙伴内存管理器。

包含如下函数或宏定义:

1. `parent`

    返回分配树中某个节点的父节点。

2. `left_leaf`

    返回分配树中某个节点的左孩子节点。

3. `right_leaf`

    返回分配树中某个节点的右孩子节点。

4. `buddy_log2`

    快速求解 log2，返回一个 `uint8_t`。如果原数字为 0，则返回 0xFF。

5. `buddy_pow2`

    快速求解 pow2。如果接受一个 0xFF，则返回 0。

6. `get_block_size`

    获得被伙伴系统管理的内存块的总大小。

7. `set_block_size`

    设置被伙伴系统管理的内存块的总大小。

8. `get_longest`

    接受 `Page` 的基址与一个分配树节点的编号，返回此分配树节点的值，即此节点下的最大内存大小。
将 `longest` 值的幂存放于 `Page` 的 `property` 中，一个 `property` 可以存放四个值。

9. `set_longest`

    设置分配树某个节点的值。

10. `buddy_init`

    初始化函数，给 `buddy_base` 与 `buddy_size` 赋初值。

11. `buddy_init_memmap`

    初始化内存映射与分配树，但只在第一次调用时工作。

12. `buddy_alloc_pages`

    页面分配函数，具体流程与参考的文章类似，主要有两处不同：
    
    1. 并不要求分配的大小必须是 2 的整数次幂，而是会自动将大小扩张到 2 的整数次幂。
    2. 在分配树中向下寻找可用内存时，原文中是左孩子优先，也就是只要左孩子的大小满足要求，就分配左孩子；在本实现中，被改为较小的孩子优先，这样会更好地利用小块内存。

13. `buddy_free_pages`

    页面回收函数，只能指定分配内存函数返回的页面作为参数，亦无法指定要回收内存的大小。

14. `buddy_nr_free_pages`

    用于查询可用页面数，实际返回的是最大可用页面数。

15. `default_check`

    检查函数。某些特性是由于总是优先分配较小的孩子节点而具备的。
