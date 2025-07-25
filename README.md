# Linux 微内核操作系统

这是一个基于微内核架构的操作系统实现，遵循Linux微内核核心文档的设计原则。

## 核心设计原则

**任何可以在用户空间安全运行的功能，都必须在用户空间运行。**

内核本身只提供最基础、最核心的服务，充当进程间安全通信和资源隔离的媒介。

## 架构概述

### 微内核核心职责

- **进程隔离 (Process Isolation)**: 为每个进程提供独立的虚拟地址空间
- **进程间通信 (IPC)**: 提供高效、安全的进程间通信机制
- **基础线程管理**: 线程的创建、销毁和调度
- **基础资源管理**: CPU时间、物理内存等核心资源的分配

### 主要子系统

1. **进程管理**
   - 完全公平调度器 (CFS)
   - 进程生命周期管理
   - 上下文切换
   - 多级反馈队列

2. **内存管理**
   - 伙伴系统 (Buddy System)
   - 虚拟内存管理
   - 页表管理
   - 内存回收机制

3. **虚拟文件系统 (VFS)**
   - 统一的文件系统接口
   - 多级缓存机制
   - 目录项缓存

4. **网络协议栈**
   - 分层协议处理
   - 套接字接口
   - 数据包处理

5. **设备驱动与模块**
   - 用户态驱动支持
   - 内核模块机制
   - 设备抽象层

## 目录结构

```
Kernal/
├── arch/                   # 架构相关代码
│   └── x86_64/            # x86_64架构支持
│       ├── boot.S         # 引导代码
│       ├── kernel.ld      # 链接脚本
│       ├── interrupt.S    # 中断处理
│       └── switch.S       # 上下文切换
├── include/               # 头文件
│   ├── types.h           # 基本类型定义
│   ├── sched.h           # 调度器相关
│   ├── mm.h              # 内存管理
│   ├── list.h            # 链表操作
│   └── spinlock.h        # 同步原语
├── src/                  # 源代码
│   ├── kernel/           # 内核核心
│   │   ├── main.c        # 内核入口
│   │   ├── sched.c       # 调度器
│   │   └── sched_fair.c  # CFS调度器
│   ├── mm/               # 内存管理
│   │   └── buddy.c       # 伙伴系统
│   ├── fs/               # 文件系统
│   ├── net/              # 网络子系统
│   ├── ipc/              # 进程间通信
│   └── drivers/          # 设备驱动
├── Makefile              # 构建脚本
├── LICENSE               # 许可证
└── README.md            # 本文件
```

## 特性

### 已实现

- [x] 基础内核框架
- [x] 多级页表虚拟内存管理
- [x] 完全公平调度器 (CFS)
- [x] 伙伴系统内存分配
- [x] 进程管理和上下文切换
- [x] 基础系统调用接口
- [x] 双向链表和红黑树数据结构


### 计划实现

- [ ] 完整的VFS实现
- [ ] 网络协议栈
- [ ] 进程间通信机制 (IPC)
- [ ] 用户态驱动支持
- [ ] 自旋锁和读写锁
- [ ] 内核模块加载
- [ ] 中断和异常处理框架
- [ ] 信号机制
- [ ] 文件系统支持
- [ ] 多处理器支持 (SMP)

## 构建和运行

### 依赖

- GCC (支持x86_64)
- NASM (汇编器)
- Binutils (链接器等)
- QEMU (用于测试)

### 构建

```bash
# 检查工具链
make check-tools

# 构建内核(这边建议在debain系下构建)
make all

# 清理构建文件
make clean
```

### 运行

```bash
# 在QEMU中运行
make qemu

# 调试运行
make debug

```

### 测试

```bash
# 运行内核测试
make test

# 代码统计
make stats

# 静态分析
make analyze
```

## 系统调用

当前支持的系统调用：

- `getpid()` - 获取进程ID
- `fork()` - 创建子进程
- `exit()` - 退出进程
- `wait4()` - 等待子进程
- `sched_yield()` - 让出CPU
- `brk()` - 调整堆大小
- `mmap()` - 内存映射
- `munmap()` - 解除内存映射
- `sysinfo()` - 系统信息
- `uname()` - 系统名称信息

## 调度器

### 完全公平调度器 (CFS)

- 基于虚拟运行时间 (vruntime)
- 使用红黑树维护就绪队列
- 支持Nice值优先级调整(其实还没写完这个)
- 负载平衡机制

### 调度策略

- `SCHED_NORMAL` - 普通进程
- `SCHED_IDLE` - 空闲进程
- `SCHED_BATCH` - 批处理进程

## 内存管理

### 伙伴系统

- 支持最大11阶 (4MB) 连续内存分配
- 自动内存碎片整理
- 多种迁移类型支持
- 每CPU页面缓存

### 虚拟内存

- 四级页表 (PML4, PDPT, PDT, PT)
- 写时复制 (COW)
- 内存映射文件
- 交换空间支持

## 同步机制

### 自旋锁

- 基本自旋锁
- 读写锁
- 中断安全版本
- 死锁检测

### 等待队列

- 可中断等待
- 不可中断等待
- 超时等待

## 开发指南

### 编码规范

- 使用GNU C99标准
- 遵循Linux内核编码风格
- 每个函数都要有注释
- 使用有意义的变量名

### 调试

```bash
# 生成反汇编
make disasm

# 生成符号表
make symbols

# 检查内核大小
make size
```

### 贡献

1. Fork本项目
2. 创建特性分支
3. 提交更改
4. 发送Pull Request

## 性能特点

- 微内核架构，核心功能精简
- 高效的CFS调度器
- 优化的内存管理
- 最小化系统调用开销
- 良好的缓存友好性

## 限制和已知问题

1. 当前只支持x86_64架构
2. 单处理器支持（SMP正在开发）
3. 有限的设备驱动支持
4. 网络功能未完全实现
5. 文件系统支持有限
6. 自旋锁和读写锁写的很糟糕
···

## 许可证

本项目采用MIT许可证，详见LICENSE文件。



这项目真烂(x
