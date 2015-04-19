# Lab 4

## 练习 1: 分配并初始化一个进程控制块

在 `alloc_proc` 函数中初始化了 `proc_struct`，设置进程状态为 `PROC_UNINIT`，`pid` 为 -1，`cr3` 为 `boot_cr3`，其它均置 0。

* 请说明 `proc_struct` 中 `struct context context` 和 `struct trapframe *tf` 成员变量含义和在本实验中的作用是什么？

    `context` 保存了进程切换时的上下文，其中是一些寄存器的值；

    `tf` 是中断帧的指针，总是指向内核栈的某个位置，在从用户态与内核态相互转换或中断嵌套时有用。

## 练习 2: 为新创建的内核线程分配资源

在 `do_fork` 函数中，进行了如下修改：

1. 使用 `alloc_proc` 函数创建一个新的进程结构，如果创建失败，跳转到 `fork_out`；
2. 使用 `setup_kstack` 分配进程的内核栈，如果分配失败，跳转到 `bad_fork_cleanup_proc`；
3. 使用 `copy_mm` 复制或共享进程的内存管理结构，如果复制失败，跳转到 `bad_fork_cleanup_kstack`；
4. 使用 `copy_thread` 设置新进程结构中的中断帧与执行上下文；
5. 使用 `get_pid` 函数得到进程的 PID；
6. 使用 `hash_proc` 将进程结构插入哈希表中；
7. 将进程结构插入 `proc_list` 中，再将 `nr_process` 自增 1；
8. 设置进程为 RUNNABLE 状态；
9. 返回 PID。

在第 5 步到第 7 步中，涉及到进程列表的操作，为了保证操作的原子性，暂时关闭中断。

* 请说明 ucore 是否做到给每个新 fork 的线程一个唯一的 id？请说明你的分析和理由。

`get_pid` 函数负责了 PID 的产生，其中基本思想是将 `last_pid` 限制在一个区间中，所有进程的 PID 应该均不存在于此区间中，所以每个线程都有一个唯一 id。
