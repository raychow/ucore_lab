# Lab 1

## 练习 1: 理解通过 make 生成执行文件的过程

1. 操作系统镜像文件 ucore.img 是如何一步一步生成的？
> 1. gcc 编译并链接 kernel;
> 2. gcc 编译并链接 bootloader;
> 3. dd 创建了一个 5,120,000 字节的 ucore.img，并将 bootloader 与 kernel 的内容复制进去。

2. 一个被系统认为是符合规范的硬盘主引导扇区的特征是什么？
> 从 tools/sign.c 的签名方式来看，主引导扇区的最后两个字节应该是 0x55AA。
