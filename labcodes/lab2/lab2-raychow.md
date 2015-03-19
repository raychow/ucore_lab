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
