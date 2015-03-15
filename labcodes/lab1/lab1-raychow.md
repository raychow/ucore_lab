# Lab 1

## 练习 1: 理解通过 make 生成执行文件的过程

1. 操作系统镜像文件 ucore.img 是如何一步一步生成的？
> 1. gcc 编译并链接 kernel;
> 2. gcc 编译并链接 bootloader;
> 3. dd 创建了一个 5,120,000 字节的 ucore.img，并将 bootloader 与 kernel 的内容复制进去。

2. 一个被系统认为是符合规范的硬盘主引导扇区的特征是什么？
> 从 tools/sign.c 的签名方式来看，主引导扇区的最后两个字节应该是 0x55AA。

## 练习 2: 使用 qemu 执行并调试 lab1 中的软件

1. 使用 make lab1-mon 进入调试，观察到 gdb 在 0x7c00 处的断点停了下来；
2. 为了更好地观察汇编代码，键入 `layout asm` 进入汇编视图；
3. 使用 `ni` 或者 `si` 单步运行汇编代码；
4. 由于 gdb 中显示的是反汇编得到的代码，所以与 bootasm.S 中有所不同。例如，`jnz seta20.1` 反汇编为 `jne 0x7c0a`，`jnz` 被替换为同义指令 `jne`，`seta20.1` 被替换为真实地址。
5. 为了在设置段表的地方停止，键入 `b *0x7c1e`，并按 `c` 继续执行，gdb 将在程序运行到 0x7c1e 时中断。

## 练习 3: 分析 bootloader 进入保护模式的过程

1. 系统启动后，BIOS 将硬盘的第一个扇区读到 0x7c00 处并开始执行；
2. 禁止了中断，并将一些寄存器置零；
3. 使能 A20 地址线：
    1. 读取 0x64 地址，这是 8042 键盘控制器的状态寄存器，一直循环读取直到 input register 为空，即第 1 位为0；
    2. 向 0x64 地址写入 0xd1，即通知控制寄存器希望操作 8042 的 P2 端口，以使能 A20 地址线；
    3. 循环直到 input register 为空；
    4. 向 0x60 地址写入 0xdf，即使能 A20 地址线。
4. `lgdt gdtdesc` 初始化了 GDT 表，表的定义在 `gdtdesc` 段中给出，包含了代码段与数据段；
5. 将 `%cr0` 的第 1 位置为 1，进入保护模式。

## 练习 4: 分析 bootloader 加载 ELF 格式的 OS 的过程

1. bootloader 首先读取 ELF header，但实际上读取了 8 个扇区，远远超过了 ELF header 的大小，应该是把之后要用的 program section header 都读进来了；
2. 检查 ELF header 的第一个字节是否为 0x464C457FU，如果不是就意味着有问题；
3. program section header 在 ELF header 起始位置的后 `e_phoff` 处，并且一共有 `e_phnum`个，进入循环开始读取每一个程序段：
    * `ph->p_va` 是在内存中的逻辑地址，`ph->p_memsz` 是程序段大小，`ph->p_offset` 是从硬盘中读取的位置；
    * 由于只能对整个扇区进行读取操作，所以如果 `p_offset` 不是扇区大小的整数倍，`readseg` 中会从扇区开始读取，因此会向内存的目标位置之前多写入一些东西。当然，也可能在之后多写入。
4. 全数加载完毕后，转到 ELF header 的 `e_entry` 位置开始运行系统。
