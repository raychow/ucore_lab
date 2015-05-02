# Lab 6

## 练习 1: 使用 Round Robin 调度算法

1. 请理解并分析 `sched_class` 中各个函数指针的用法，并结合 Round Robin 调度算法描述 ucore 的调度执行过程

    * 在 `sched_class` 中，包含以下函数指针：
    
        1. `init`
            初始化运行队列。
        
        2. `enqueue`
            将指定进程加入就绪队列，这个进程应该处于 `PROC_RUNNABLE` 状态。
        
        3. `dequeue`
            将指定进程从就绪队列中移除，这个进程通常是由 `pick_next` 选择的。
        
        4. `pick_next`
            在就绪队列中选择一个进程。
        
        5. `proc_tick`
            在时钟中断触发时调用，用于减少进程的剩余时间片，以及设置待调度状态。
    
    * 结合 Round Robin 调度算法描述 ucore 的调度执行过程
    
        1. 在系统启动时，使用 `sched_init()` 来初始化调度器；
        2. 在唤醒进程或将剥夺运行中进程的时间片时，`sched_class_enqueue()` 被调用，将进程加入就绪队列的尾部；
        3. 在调度时使用 `sched_class_pick_next()` 选择就绪队列头部的进程；
        4. 选择一个进程后，使用 `sched_class_dequeue()` 将进程从就绪队列中移除；
        5. 在处理时钟中断时，`sched_class_proc_tick()` 被调用，进程的剩余时间片减一，如果成为 0，就将进程标记为待调度的状态，在中断处理结束时进程将被调度。

2. 简要说明如何设计实现“多级反馈队列调度算法”

    多级反馈队列调度算法能够使得高优先级的作业得到响应，又能使短作业迅速完成。为了方便实现，在 `proc_struct` 中加入一个整型量 `run_level`，记录了下一次应该插入到的队列级数。具体到每个函数的设计为：
    
    1. `init`
        初始化各级优先级队列。
    
    2. `enqueue`
        将进程插入到 `run_level` 级别的队列尾部。
    
    3. `deque`
        将进程从所在队列中移除，并将 `run_level` 加一。
    
    4. `pick_next`
        从优先级最高的队列开始依次向低优先级队列遍历，直到找到一个进程。
    
    5. `proc_tick`
        减少进程的剩余时间片，如果时间片成为 0，就将进程标记为待调度的状态。

## 练习2: 实现 Stride Scheduling 调度算法

在 Stride Scheduling 的 default_sched.c 中，完成了以下函数：

    1. `stride_init`
        初始化运行队列。
    
    2. `stride_enqueue`
        使用 `skew_heap_insert` 将进程插入就绪队列中并维持斜堆的性质。之后，更新进程的可用时间片。
    
    3. `stride_dequeue`
        使用 `skew_heap_remove` 将进程从就绪队列中移除并维持斜堆的性质。
    
    4. `stride_pick_next`
        由于是一个最小堆，斜堆中的第一个元素一定是 stride 最小的，因此直接选中。之后，更新选中进程的 stride。
    
    5. `stride_proc_tick`
        减少进程的剩余时间片，如果剩余 0，则标记为待调度状态。
