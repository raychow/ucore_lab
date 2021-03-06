# Lab 3

## 练习 1: 给未被映射的地址映射上物理页

* 实现过程

    1. 寻找引起缺页异常的线性地址对应的页表项；
    2. 如果页表项无内容，表示此线性地址未映射，则为此地址进行映射。

* 请描述页目录项(Page Director Entry)和页表(Page Table Entry)中组成部分对 ucore 实现页替换算法的潜在用处。

    * 页目录项的 PTE_P 表示了此页目录项是否存在，如果不存在则需要创建；
    * 页表项的 PTE_P 有如下几种用途：
        1. 如果设置了 PTE_P，表示页面存在内存中；
        2. 如果未设置 PTE_P，且高 24 位为 0，表示页面尚未创建，需要创建页面；
        3. 如果未设置 PTE_P，且高 24 位不为 0，表示页面已被换出到页表项高 24 位地址的硬盘扇区。

* 如果 ucore 的缺页服务例程在执行过程中访问内存，出现了页访问异常，请问硬件要做哪些事情？

    1. CPU 在当前内核栈保存被打断的程序现场即依次压入当前被打断程序使用的 EFLAGS, CS, EIP, errorCode；
    2. 由于页访问异常的中断号是 0xE，CPU 把异常中断号 0xE 对应的中断服务例程的地址（vectors.S 中的标号 vector14 处）加载到 CS 和 EIP 寄存器中，开始执行中断服务例程。

## 练习 2: 补充完成基于 FIFO 的页面替换算法

* 实现过程

    1. `do_pgfault` 函数:
        
        补充完成了换入页面的逻辑，首先使用 `swap_in` 换入页面，并使用 `page_insert` 插入页面，最后标记页面为可交换的并更新 `Page` 的 `pra_vaddr`。
        
    2. `_fifo_map_swappable` 函数:
        
        只需要将页面插入到 `mm->sm_priv` 的结尾即可。
        
    3. `_fifo_swap_out_victim` 函数:
    
        移除 `mm->sm_priv` 头部的列表项，并返回这个页表项对应的页面。

* 如果要在 ucore 上实现 "extended clock 页替换算法" 请给你的设计方案，现有的 swap_manager 框架是否足以支持在 ucore 中实现此算法？如果是，请给你的设计方案。如果不是，请给出你的新的扩展和基此扩展的设计方案。并需要回答如下问题

    * 需要被换出的页的特征是什么？

    可以在现有框架实现此算法，被换出的页面是访问位与修改位均为 0 的页面。
    
    * 在 ucore 中如何判断具有这样特征的页？

    在读取页面时，页表项的 PTE_A 位被设置；在修改页面时，页表项的 PTE_D 位被设置。则 PTE_A 位表示页面被访问，PTE_D 位表示页面被修改。
    
    * 何时进行换入和换出操作？

    在缺页时进行换入换出。

## 扩展练习 Challenge: 实现识别 dirty bit 的 extended clock 页替换算法

此 extended clock 算法，根据 PTE 中的 `PTE_A` 与 `PTE_D` 位来判断页面状态，并进行页替换。

在 `kern/mm/swap_eclock.h` 中，有 `eclock_private_t` 的定义和 swap 算法管理器 `swap_manager_eclock` 的声明，`eclock_private_t` 是 extended clock 算法在 `mm_struct` 的 `sm_priv` 中存放数据的类型。

`eclock_private_t` 的完整定义如下：

    typedef struct {
        list_entry_t pra_list_head; // 被 extended clock 算法管理的页面链表
        list_entry_t *hand;         // 链表表盘指针，实际上指向的是上一个要操作 / 访问的位置
    } eclock_private_t;

主要的实现在 `kern/mm/swap_eclock.c` 中，包含如下函数：

1. `_eclock_init_mm`

    初始化 `mm_struct` 数据结构。

2. `_eclock_map_swappable`

    将页面标记为可交换的。由于 `hand` 指针指向了插入位置，只要把页面插入到 `hand` 指针之后的位置，并更新 `hand` 指针即可。

3. `_eclock_swap_out_victim`

    页面换出策略。从 `hand` 指针的下一个位置开始无限循环，获得页面对应的页表项，进行如下三个操作中的某一个操作：
    
    1. 如果页表项的 `PTE_A` 位被设置，清除该位，并使相应快表无效。
    2. 如果 `PTE_A` 位没有设置，但设置了 `PTE_D` 位，清除该位，并使相应快表无效。需要注意的是，此处默认换出页面时一定写回硬盘。
    3. 如果以上条件均不满足，说明找到了被换出的页面，结束循环。

    将选择的页面从页面链表中移除，并更新 `hand` 指针。

4. `_eclock_check_swap`
    
    检查函数。

其它函数略去不提。

此外，为了保证能从 `Page` 结构获得正确的线性地址，将 `pgdir_alloc_page` 函数中对 `page->pra_vaddr` 的更新移动到调用 `swap_map_swappable` 之前，并在 `do_pgfault` 函数中调用 `swap_map_swappable` 之前插入 `page->pra_vaddr = addr;`。

实现 extended clock 算法时遇到的一个大坑是，一定要在每次更新页表项之后将相应缓存无效化，否则 CPU 访问的页表项与内存中不一致，导致算法的换出逻辑行为诡异。
