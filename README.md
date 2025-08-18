# Linux 微内核操作系统
这是一个基于微内核架构的操作系统实现，遵循Linux微内核核心文档的设计原则。

# 总体设计原则（简要）

* 内核只做最小信任/最核心工作（地址空间、线程、IPC、低级资源管理、异常/中断）。
* 尽量把复杂逻辑放到用户态服务（文件系统、网络栈、驱动、策略）。
* 代码分层清晰：`arch/`、`kernel/`、`lib/`、`user/`、`tools/`、`docs/`。
* 强制接口契约（IPC/ABI），便于并行开发与语言互操作（C/Rust）。
* 小步快跑：先做 QEMU 可启动的 minimal kernel，再逐步展开子系统。

# 推荐项目结构（示例）

```
linux-microkernel/                 # 根仓库
├── .github/
│   ├── workflows/                 # CI (build/test/qemu)
│   └── ISSUE_TEMPLATE.md
├── arch/
│   └── x86_64/
│       ├── boot/                  # 启动、链接脚本、启动汇编
│       ├── mm/                    # 架构相关页表 helper
│       └── cpu/                   # cpu 特定代码
├── kernel/
│   ├── include/                   # 内核公共头 (kernel/*.h)
│   ├── core/                      # microkernel core（scheduler, ipc core）
│   │   ├── process/
│   │   ├── thread/
│   │   ├── sched/
│   │   └── ipc/
│   ├── mm/                        # 伙伴系统、虚拟内存抽象（不含arch）
│   ├── interrupt/                 # 中断/异常框架
│   ├── drivers/                   # 简单内核驱动骨架（打印、console）
│   └── Kconfig / Makefile         # 内核构建接口
├── lib/                           # 内核与用户态共用的轻量库 (rb-tree, lists)
├── user/                          # 用户态服务与示例程序
│   ├── services/
│   │   ├── init/                  # 初始化进程（启动服务）
│   │   ├── vfsd/                  # VFS 服务（用户态）
│   │   ├── netd/                  # 网络服务（用户态）
│   │   └── devd/                  # 用户态设备驱动（如 virtio）
│   ├── apps/                      # 用户程序示例 (shell, hello)
│   └── libs/                      # 用户库 (libc shim, ipc client)
├── tools/                         # 构建/模拟/测试脚本 (qemu helpers)
├── tests/                         # 单元、集成测试 (qemu-based)
│   ├── kernel-unit/
│   └── integration/
├── docs/
│   ├── design/                    # 设计文档 (IPC spec, ABI, sched)
│   ├── dev-setup.md
│   └── roadmap.md
├── examples/                       # 最小系统镜像 / demo
├── scripts/                        # 自动化脚本 (run-qemu, build-image)
├── .clang-format
├── Makefile                        # 顶层构建入口：make all/qemu/test/clean
├── README.md
├── CONTRIBUTING.md
└── LICENSE
```

# 每个目录的详细职责（精炼版）

* `arch/x86_64`: 启动代码、bootloader 接口、架构特定的页表/上下文切换实现、异常向量。
* `kernel/core`: microkernel 最小内核（线程/调度/IPC/资源分配/内核同步）。
* `kernel/mm`: 伙伴系统 / 内存分配器 / 虚拟内存管理的非架构部分。
* `kernel/interrupt`: 中断向量注册、IRQ 管理、异常处理框架。
* `kernel/drivers`: 只放能在内核低层运行且必须在内核态的驱动（例如 early console、低级中断控制）。其余放到 `user/services/devd`。
* `lib`: 常用数据结构（双向链表、红黑树）与无需大型依赖的算法。
* `user/services`: 将大功能拆成守护进程（VFS、网络、设备代理、权限管理），以实现「功能可以在用户空间安全运行」的原则。
* `tests`: QEMU-based integration tests，单元测试可借助 `unity` 或 `cmocka` 对 kernel core 的用户态模拟接口做测试（注意：内核态代码需要 cross-build & qemu-run 才能真实验证）。
* `docs/design`: 放 IPC 协议、消息格式、capability 模型、syscall ABI 文档（非常重要，接口一旦确定会影响大量代码）。

# 编译 / 构建建议（顶层 Makefile）

* 顶层 `Makefile` 负责：

  * 检查工具链（gcc/x86\_64-elf-gcc/nasm/qemu）
  * `make kernel`、`make user`、`make image`、`make qemu`、`make test`
* 建议单独的 `kernel/Makefile` 与 `user/Makefile`，支持交叉编译目标和 `DEBUG=1`。
* 保持构建产物放在 `out/`，不污染源树。

# IPC / ABI 设计建议（要写到 docs）

* **基本思想**： capability + 基于消息的 IPC，结合共享内存映射（grant/portal）以支持高速数据通道（网络、文件缓存）。
* **通道类型**：

  * Control messages（固定小结构，可靠，不阻塞）
  * Bulk shared memory（通过 grant/FD-like handle 映射页）
* **消息格式（示例）**：


* **同步模型**：

  * 同步调用（RPC-like，阻塞等待回复）
  * 异步通知（事件队列、epoll-like）
* **安全性**：每个端点携带 capability/token，内核仅检查 capability 是否允许该操作（no implicit global rights）。

# 用户态服务建议（语言 & 风格）

* 推荐：**核心内核**用 **C (限定 subset)** + 必要汇编；**用户态服务**优先使用C，对现有 C 库和工具链友好。
* 在 `user/libs/` 提供一个轻量 libc-shim（只实现必要 syscalls），方便用 C 编写服务。
* 为了降低集成成本，先用 C 实现 `init` 与 `vfsd` 的最简版本，再逐步扩展。

# 同步与锁（工程实践）

* Kernel 内部只允许简单、可验证的 sync 原语：自旋锁、简洁的优先级提升/死锁检测。
* 在 `kernel/core/sched` 添加 `CONFIG_TRACELOCK` 开关用于调试死锁路径。
* 在 `user/` 服务里使用更高级别的 Rust 的 `Mutex`/`RwLock`。

# 测试策略（必需）

* 单元测试（可在宿主机 run 的纯 C 函数，放 `tests/kernel-unit`）。
* 集成测试（QEMU 启动镜像并运行脚本，检查日志/退出码）。
* 回归测试：在 CI 上运行 `make qemu-test`，对关键路径（fork, mmap, ipc, vfs ops）做黑盒验证。

# CI（Github Actions）简要建议

* Workflow 分为：`build-kernel`、`build-user`、`qemu-integration`（QEMU headless 使用 `--nographic`，比较日志）。
* 缓存构建产物（ccache）。
* 在 PR 时触发：编译 + 测试套件（快速 smoke tests）。

# 文档与贡献（必备文件）

* `docs/design/ipc.md`：IPC 协议、例子、错误码。
* `docs/dev-setup.md`：交叉编译工具链安装、QEMU 启动说明、调试（gdbserver）。
* `CONTRIBUTING.md`：分支策略、PR 模板、代码风格检查（clang-format）。
* `CODE_OF_CONDUCT.md`：开源社区友好政策。
* `README.md`：可执行的入门步骤（如何在 QEMU 上启动最小镜像）。


- [ ] 基础内核框架
- [ ] 多级页表虚拟内存管理
- [ ] 完全公平调度器 (CFS)
- [ ] 伙伴系统内存分配
- [ ] 进程管理和上下文切换
- [ ] 基础系统调用接口
- [ ] 双向链表和红黑树数据结构
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


### 贡献

1. Fork本项目
2. 创建特性分支
3. 提交更改
4. 发送Pull Request


## 许可证

本项目采用MIT许可证，详见LICENSE文件。



这项目真烂(x
