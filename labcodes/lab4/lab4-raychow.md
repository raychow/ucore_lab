# Lab 4

## 练习 1: 分配并初始化一个进程控制块

在 `alloc_proc` 函数中初始化了 `proc_struct`，设置进程状态为 `PROC_UNINIT`，`pid` 为 -1，`cr3` 为 `boot_cr3`，其它均置 0。

* 请说明 `proc_struct` 中 `struct context context` 和 `struct trapframe *tf` 成员变量含义和在本实验中的作用是什么？

    `context` 保存了进程切换时的上下文，其中是一些寄存器的值；

    `tf` 是中断帧的指针，总是指向内核栈的某个位置，在从用户态与内核态相互转换或中断嵌套时有用。
