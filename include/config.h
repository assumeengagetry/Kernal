#ifndef __CONFIG_H__
#define __CONFIG_H__

/* 内核配置选项 */

/* 版本信息 */
#define KERNEL_VERSION_MAJOR    0
#define KERNEL_VERSION_MINOR    1
#define KERNEL_VERSION_PATCH    0
#define KERNEL_VERSION_STRING   "0.1.0"
#define KERNEL_NAME             "MicroKernel"
#define KERNEL_RELEASE          "MicroKernel 0.1.0"

/* 架构配置 */
#define CONFIG_X86_64           1
#define CONFIG_64BIT            1
#define CONFIG_SMP              0       /* 单处理器模式 */
#define CONFIG_PREEMPT          1       /* 抢占式调度 */

/* 内存管理配置 */
#define CONFIG_MMU              1       /* 内存管理单元 */
#define CONFIG_HIGHMEM          1       /* 高端内存支持 */
#define CONFIG_SWAP             0       /* 交换空间 */
#define CONFIG_SLAB             1       /* Slab分配器 */
#define CONFIG_SLUB             0       /* SLUB分配器 */
#define CONFIG_SLOB             0       /* SLOB分配器 */

/* 调度器配置 */
#define CONFIG_SCHED_DEBUG      1       /* 调度器调试 */
#define CONFIG_FAIR_GROUP_SCHED 0       /* 公平组调度 */
#define CONFIG_RT_GROUP_SCHED   0       /* 实时组调度 */
#define CONFIG_CGROUP_SCHED     0       /* 控制组调度 */

/* 文件系统配置 */
#define CONFIG_VFS              1       /* 虚拟文件系统 */
#define CONFIG_EXT2_FS          0       /* EXT2文件系统 */
#define CONFIG_EXT3_FS          0       /* EXT3文件系统 */
#define CONFIG_EXT4_FS          0       /* EXT4文件系统 */
#define CONFIG_PROC_FS          1       /* /proc文件系统 */
#define CONFIG_SYSFS            1       /* /sys文件系统 */
#define CONFIG_TMPFS            1       /* 临时文件系统 */
#define CONFIG_DEVTMPFS         1       /* 设备临时文件系统 */

/* 网络配置 */
#define CONFIG_NET              1       /* 网络支持 */
#define CONFIG_INET             1       /* IPv4支持 */
#define CONFIG_IPV6             0       /* IPv6支持 */
#define CONFIG_UNIX             1       /* Unix域套接字 */
#define CONFIG_PACKET           1       /* 包套接字 */
#define CONFIG_NETLINK          1       /* Netlink套接字 */

/* 设备驱动配置 */
#define CONFIG_BLOCK            1       /* 块设备支持 */
#define CONFIG_CHAR_DEVICES     1       /* 字符设备支持 */
#define CONFIG_SERIAL_8250      1       /* 8250串口驱动 */
#define CONFIG_KEYBOARD         1       /* 键盘驱动 */
#define CONFIG_MOUSE            1       /* 鼠标驱动 */
#define CONFIG_VGA_CONSOLE      1       /* VGA控制台 */

/* 调试配置 */
#define CONFIG_DEBUG_KERNEL     1       /* 内核调试 */
#define CONFIG_DEBUG_SLAB       1       /* Slab调试 */
#define CONFIG_DEBUG_SPINLOCK   1       /* 自旋锁调试 */
#define CONFIG_DEBUG_MUTEXES    1       /* 互斥锁调试 */
#define CONFIG_DEBUG_PAGEALLOC  1       /* 页面分配调试 */
#define CONFIG_DEBUG_INFO       1       /* 调试信息 */
#define CONFIG_FRAME_POINTER    1       /* 帧指针 */
#define CONFIG_STACK_TRACER     1       /* 栈跟踪 */

/* 安全配置 */
#define CONFIG_SECURITY         0       /* 安全框架 */
#define CONFIG_SECURITY_SELINUX 0       /* SELinux */
#define CONFIG_SECURITY_APPARMOR 0      /* AppArmor */
#define CONFIG_SECURITY_SMACK   0       /* Smack */

/* 电源管理配置 */
#define CONFIG_PM               0       /* 电源管理 */
#define CONFIG_ACPI             0       /* ACPI支持 */
#define CONFIG_APM              0       /* APM支持 */

/* 虚拟化配置 */
#define CONFIG_VIRTUALIZATION   0       /* 虚拟化支持 */
#define CONFIG_KVM              0       /* KVM支持 */
#define CONFIG_XEN              0       /* Xen支持 */

/* 容器配置 */
#define CONFIG_NAMESPACES       0       /* 命名空间 */
#define CONFIG_CGROUPS          0       /* 控制组 */
#define CONFIG_CHECKPOINT_RESTORE 0     /* 检查点恢复 */

/* 性能配置 */
#define CONFIG_PROFILING        1       /* 性能分析 */
#define CONFIG_PERF_EVENTS      1       /* 性能事件 */
#define CONFIG_TRACING          1       /* 跟踪系统 */
#define CONFIG_FTRACE           1       /* 函数跟踪 */

/* 加密配置 */
#define CONFIG_CRYPTO           0       /* 加密支持 */
#define CONFIG_CRYPTO_AES       0       /* AES加密 */
#define CONFIG_CRYPTO_SHA1      0       /* SHA1哈希 */
#define CONFIG_CRYPTO_SHA256    0       /* SHA256哈希 */

/* 压缩配置 */
#define CONFIG_ZLIB_INFLATE     0       /* ZLIB解压 */
#define CONFIG_ZLIB_DEFLATE     0       /* ZLIB压缩 */
#define CONFIG_LZO_COMPRESS     0       /* LZO压缩 */
#define CONFIG_LZO_DECOMPRESS   0       /* LZO解压 */

/* 系统限制 */
#define CONFIG_BASE_FULL        1       /* 完整基础功能 */
#define CONFIG_EMBEDDED         0       /* 嵌入式系统 */
#define CONFIG_EXPERIMENTAL     1       /* 实验性功能 */
#define CONFIG_BROKEN           0       /* 损坏的功能 */

/* 硬件特性 */
#define CONFIG_X86_MCE          0       /* 机器检查异常 */
#define CONFIG_X86_THERMAL      0       /* 热管理 */
#define CONFIG_MICROCODE        0       /* 微代码更新 */
#define CONFIG_X86_MSR          1       /* MSR设备 */
#define CONFIG_X86_CPUID        1       /* CPUID设备 */

/* 时钟和定时器 */
#define CONFIG_TICK_ONESHOT     1       /* 单次时钟 */
#define CONFIG_NO_HZ            0       /* 无时钟滴答 */
#define CONFIG_HIGH_RES_TIMERS  1       /* 高分辨率定时器 */
#define CONFIG_GENERIC_CLOCKEVENTS 1    /* 通用时钟事件 */

/* 中断配置 */
#define CONFIG_GENERIC_HARDIRQS 1       /* 通用硬中断 */
#define CONFIG_SPARSE_IRQ       0       /* 稀疏中断 */
#define CONFIG_NUMA_IRQ_DESC    0       /* NUMA中断描述符 */

/* NUMA配置 */
#define CONFIG_NUMA             0       /* NUMA支持 */
#define CONFIG_NODES_SHIFT      0       /* NUMA节点位移 */

/* 热插拔配置 */
#define CONFIG_HOTPLUG_CPU      0       /* CPU热插拔 */
#define CONFIG_MEMORY_HOTPLUG   0       /* 内存热插拔 */
#define CONFIG_MEMORY_HOTREMOVE 0       /* 内存热移除 */

/* 模块配置 */
#define CONFIG_MODULES          1       /* 模块支持 */
#define CONFIG_MODULE_UNLOAD    1       /* 模块卸载 */
#define CONFIG_MODULE_FORCE_UNLOAD 0    /* 强制模块卸载 */
#define CONFIG_MODVERSIONS      0       /* 模块版本 */
#define CONFIG_MODULE_SRCVERSION_ALL 0  /* 模块源版本 */

/* 内核常量 */
#define NR_CPUS                 1       /* CPU数量 */
#define THREAD_SIZE             16384   /* 线程栈大小 */
#define PAGE_SIZE               4096    /* 页面大小 */
#define PAGE_SHIFT              12      /* 页面位移 */
#define MAX_ORDER               11      /* 最大分配阶数 */
#define FORK_PREEMPT_COUNT      2       /* Fork抢占计数 */
#define HZ                      1000    /* 时钟频率 */
#define USER_HZ                 100     /* 用户时钟频率 */
#define CLOCKS_PER_SEC          1000000 /* 每秒时钟数 */

/* 内存布局 */
#define KERNEL_VIRTUAL_BASE     0xFFFF800000000000UL
#define USER_VIRTUAL_BASE       0x0000000000000000UL
#define USER_VIRTUAL_END        0x0000800000000000UL
#define VMALLOC_START           0xFFFF880000000000UL
#define VMALLOC_END             0xFFFFC80000000000UL
#define MODULES_VADDR           0xFFFFFFFF80000000UL
#define MODULES_END             0xFFFFFFFFC0000000UL

/* 内核栈 */
#define KERNEL_STACK_SIZE       THREAD_SIZE
#define IRQ_STACK_SIZE          16384

/* 调度常量 */
#define MAX_NICE                19
#define MIN_NICE                -20
#define NICE_WIDTH              (MAX_NICE - MIN_NICE + 1)
#define DEFAULT_PRIO            120
#define MAX_USER_RT_PRIO        100
#define MAX_RT_PRIO             MAX_USER_RT_PRIO
#define MAX_PRIO                (MAX_RT_PRIO + NICE_WIDTH)
#define NICE_0_LOAD             1024
#define NICE_0_SHIFT            10

/* 内存管理常量 */
#define MAX_NUMNODES            1
#define MAX_NR_ZONES            4
#define NR_LRU_LISTS            5
#define MIGRATE_TYPES           6
#define MIGRATE_PCPTYPES        3
#define PID_MAX                 32768
#define BITS_PER_LONG           64
#define BITS_PER_BYTE           8

/* 文件系统常量 */
#define NAME_MAX                255
#define PATH_MAX                4096
#define PIPE_BUF                4096
#define MAX_LINKS               32

/* 信号常量 */
#define NSIG                    64
#define SIGRTMIN                32
#define SIGRTMAX                (NSIG-1)

/* 网络常量 */
#define IFNAMSIZ                16
#define MAX_ADDR_LEN            32

/* 字符串化宏 */
#define __stringify(x)          #x
#define stringify(x)            __stringify(x)

/* 条件编译助手 */
#define IS_ENABLED(option)      (option)
#define IS_BUILTIN(option)      (option)
#define IS_MODULE(option)       0

/* 优化提示 */
#define likely(x)               __builtin_expect(!!(x), 1)
#define unlikely(x)             __builtin_expect(!!(x), 0)

/* 编译器属性 */
#define __init                  __attribute__((section(".init.text")))
#define __initdata              __attribute__((section(".init.data")))
#define __exit                  __attribute__((section(".exit.text")))
#define __exitdata              __attribute__((section(".exit.data")))
#define __devinit               __init
#define __devinitdata           __initdata
#define __devexit               __exit
#define __devexitdata           __exitdata

/* 调试宏 */
#if CONFIG_DEBUG_KERNEL
#define DEBUG 1
#else
#define DEBUG 0
#endif

/* 断言宏 */
#if CONFIG_DEBUG_KERNEL
#define KERN_DEBUG              "<7>"
#define pr_debug(fmt, ...)      printk(KERN_DEBUG fmt, ##__VA_ARGS__)
#else
#define pr_debug(fmt, ...)      do { } while (0)
#endif

/* 内核日志级别 */
#define KERN_EMERG              "<0>"
#define KERN_ALERT              "<1>"
#define KERN_CRIT               "<2>"
#define KERN_ERR                "<3>"
#define KERN_WARNING            "<4>"
#define KERN_NOTICE             "<5>"
#define KERN_INFO               "<6>"
#define KERN_DEBUG              "<7>"

/* 内存分配标志 */
#define GFP_KERNEL              0x01
#define GFP_ATOMIC              0x02
#define GFP_USER                0x04
#define GFP_NOWAIT              0x08
#define GFP_NOIO                0x10
#define GFP_NOFS                0x20
#define GFP_ZERO                0x40

/* 内核模块信息 */
#define MODULE_LICENSE(x)       static const char __module_license[] = x
#define MODULE_AUTHOR(x)        static const char __module_author[] = x
#define MODULE_DESCRIPTION(x)   static const char __module_description[] = x
#define MODULE_VERSION(x)       static const char __module_version[] = x
#define MODULE_ALIAS(x)         static const char __module_alias[] = x

#endif /* __CONFIG_H__ */