#define __SCHED_H__

#include "types.h"
#include "list.h"
#include "spinlock.h"

/* 进程状态定义 */
#define TASK_RUNNING            0
#define TASK_INTERRUPTIBLE      1
#define TASK_UNINTERRUPTIBLE    2
#define TASK_ZOMBIE             4
#define TASK_STOPPED            8
#define TASK_TRACED             16

/* 进程优先级定义 */
#define MAX_NICE                19
#define MIN_NICE                -20
#define NICE_WIDTH              (MAX_NICE - MIN_NICE + 1)
#define DEFAULT_PRIO            120
#define MAX_USER_RT_PRIO        100
#define MAX_RT_PRIO             MAX_USER_RT_PRIO
#define MAX_PRIO                (MAX_RT_PRIO + NICE_WIDTH)
#define DEFAULT_TIMESLICE       (100 * HZ / 1000)

/* 进程标志 */
#define PF_KTHREAD              0x00000001  /* 内核线程 */
#define PF_IDLE                 0x00000002  /* 空闲进程 */
#define PF_EXITING              0x00000004  /* 正在退出 */
#define PF_VCPU                 0x00000010  /* 虚拟CPU */
#define PF_WQ_WORKER            0x00000020  /* 工作队列工作者 */
#define PF_FORKNOEXEC           0x00000040  /* fork但未exec */
#define PF_MCE_PROCESS          0x00000080  /* 处理机器检查异常 */
#define PF_SUPERPRIV            0x00000100  /* 超级用户权限 */
#define PF_DUMPCORE             0x00000200  /* 转储核心 */
#define PF_SIGNALED             0x00000400  /* 被信号杀死 */
#define PF_MEMALLOC             0x00000800  /* 正在分配内存 */
#define PF_NPROC_EXCEEDED       0x00001000  /* 进程数量超过限制 */
#define PF_USED_MATH            0x00002000  /* 使用了FPU */
#define PF_USED_ASYNC           0x00004000  /* 使用了异步I/O */
#define PF_NOFREEZE             0x00008000  /* 不能被冻结 */
#define PF_FROZEN               0x00010000  /* 已冻结 */
#define PF_SUSPEND_TASK         0x00020000  /* 挂起任务 */

/* 克隆标志 */
#define CLONE_VM                0x00000100  /* 共享虚拟内存 */
#define CLONE_FS                0x00000200  /* 共享文件系统信息 */
#define CLONE_FILES             0x00000400  /* 共享文件描述符表 */
#define CLONE_SIGHAND           0x00000800  /* 共享信号处理程序 */
#define CLONE_PTRACE            0x00002000  /* 继续跟踪子进程 */
#define CLONE_VFORK             0x00004000  /* vfork标志 */
#define CLONE_PARENT            0x00008000  /* 与父进程同父 */
#define CLONE_THREAD            0x00010000  /* 同一线程组 */
#define CLONE_NEWNS             0x00020000  /* 新的命名空间 */
#define CLONE_SYSVSEM           0x00040000  /* 共享System V SEM_UNDO */
#define CLONE_SETTLS            0x00080000  /* 创建新的TLS */
#define CLONE_PARENT_SETTID     0x00100000  /* 设置父进程的TID */
#define CLONE_CHILD_CLEARTID    0x00200000  /* 清除子进程的TID */
#define CLONE_DETACHED          0x00400000  /* 分离的进程 */
#define CLONE_UNTRACED          0x00800000  /* 不能被跟踪 */
#define CLONE_CHILD_SETTID      0x01000000  /* 设置子进程的TID */
#define CLONE_NEWCGROUP         0x02000000  /* 新的cgroup命名空间 */
#define CLONE_NEWUTS            0x04000000  /* 新的UTS命名空间 */
#define CLONE_NEWIPC            0x08000000  /* 新的IPC命名空间 */
#define CLONE_NEWUSER           0x10000000  /* 新的用户命名空间 */
#define CLONE_NEWPID            0x20000000  /* 新的PID命名空间 */
#define CLONE_NEWNET            0x40000000  /* 新的网络命名空间 */
#define CLONE_IO                0x80000000  /* 共享I/O */

/* 内存描述符结构 */
struct mm_struct {
    struct list_head mmap_list;     /* VMA链表 */
    struct rb_root mm_rb;           /* VMA红黑树 */
    u32 map_count;                  /* VMA数量 */
    spinlock_t page_table_lock;     /* 页表锁 */

    /* 地址空间布局 */
    ulong mmap_base;                /* mmap区域起始地址 */
    ulong mmap_legacy_base;         /* 传统mmap基址 */
    ulong task_size;                /* 任务地址空间大小 */
    ulong highest_vm_end;           /* 最高虚拟地址 */

    /* 页目录 */
    phys_addr_t pgd;                /* 页全局目录物理地址 */

    /* 统计信息 */
    ulong total_vm;                 /* 总虚拟内存页数 */
    ulong locked_vm;                /* 锁定的虚拟内存页数 */
    ulong pinned_vm;                /* 固定的虚拟内存页数 */
    ulong shared_vm;                /* 共享的虚拟内存页数 */
    ulong exec_vm;                  /* 可执行的虚拟内存页数 */
    ulong stack_vm;                 /* 栈虚拟内存页数 */
    ulong data_vm;                  /* 数据段虚拟内存页数 */

    /* 特殊地址 */
    ulong start_code, end_code;     /* 代码段边界 */
    ulong start_data, end_data;     /* 数据段边界 */
    ulong start_brk, brk;           /* 堆边界 */
    ulong start_stack;              /* 栈起始地址 */
    ulong arg_start, arg_end;       /* 参数区边界 */
    ulong env_start, env_end;       /* 环境变量区边界 */

    /* 引用计数 */
    u32 mm_users;                   /* 使用此mm的进程数 */
    u32 mm_count;                   /* 引用计数 */

    /* 核心转储支持 */
    u32 core_state;                 /* 核心转储状态 */
    spinlock_t ioctx_lock;          /* 异步I/O上下文锁 */
};

/* 文件系统信息结构 */
struct fs_struct {
    int users;                      /* 用户计数 */
    spinlock_t lock;                /* 保护锁 */

    struct path root;               /* 根目录 */
    struct path pwd;                /* 当前工作目录 */

    mode_t umask;                   /* 文件创建掩码 */
};

/* 文件描述符表结构 */
struct files_struct {
    u32 count;                      /* 引用计数 */
    spinlock_t file_lock;           /* 文件锁 */

    struct file **fdt;              /* 文件描述符表 */
    u32 max_fds;                    /* 最大文件描述符数 */
    u32 next_fd;                    /* 下一个可用fd */

    struct file *fd_array[32];      /* 内置文件描述符数组 */
};

/* 信号处理结构 */
struct signal_struct {
    u32 count;                      /* 引用计数 */
    u32 live;                       /* 活跃线程数 */

    /* 信号处理程序 */
    struct k_sigaction action[64];  /* 信号处理程序数组 */
    spinlock_t siglock;             /* 信号锁 */

    /* 进程组和会话 */
    pid_t pgrp;                     /* 进程组ID */
    pid_t session;                  /* 会话ID */

    /* 统计信息 */
    u64 utime;                      /* 用户态时间 */
    u64 stime;                      /* 内核态时间 */
    u64 cutime;                     /* 子进程用户态时间 */
    u64 cstime;                     /* 子进程内核态时间 */

    /* 资源限制 */
    struct rlimit rlim[16];         /* 资源限制数组 */
};

/* 调度实体 */
struct sched_entity {
    struct load_weight load;        /* 负载权重 */
    struct rb_node run_node;        /* 运行队列红黑树节点 */
    struct list_head group_node;    /* 组调度链表节点 */

    u64 exec_start;                 /* 开始执行时间 */
    u64 sum_exec_runtime;           /* 累计执行时间 */
    u64 vruntime;                   /* 虚拟运行时间 */
    u64 prev_sum_exec_runtime;      /* 上次累计执行时间 */

    u64 nr_migrations;              /* 迁移次数 */

    /* 调度统计 */
    u64 start_runtime;              /* 开始运行时间 */
    u64 avg_overlap;                /* 平均重叠时间 */
    u64 avg_wakeup;                 /* 平均唤醒时间 */
    u64 avg_running;                /* 平均运行时间 */

    s64 sum_sleep_runtime;          /* 累计睡眠时间 */
    s64 sum_block_runtime;          /* 累计阻塞时间 */
    s64 exec_max;                   /* 最大执行时间 */
    s64 slice_max;                  /* 最大时间片 */

    u64 nr_migrations_cold;         /* 冷迁移次数 */
    u64 nr_failed_migrations_affine; /* 失败的亲和性迁移次数 */
    u64 nr_failed_migrations_running; /* 运行时失败的迁移次数 */
    u64 nr_failed_migrations_hot;   /* 热迁移失败次数 */
    u64 nr_forced_migrations;       /* 强制迁移次数 */

    u64 nr_wakeups;                 /* 唤醒次数 */
    u64 nr_wakeups_sync;            /* 同步唤醒次数 */
    u64 nr_wakeups_migrate;         /* 迁移唤醒次数 */
    u64 nr_wakeups_local;           /* 本地唤醒次数 */
    u64 nr_wakeups_remote;          /* 远程唤醒次数 */
    u64 nr_wakeups_affine;          /* 亲和性唤醒次数 */
    u64 nr_wakeups_affine_attempts; /* 亲和性唤醒尝试次数 */
    u64 nr_wakeups_passive;         /* 被动唤醒次数 */
    u64 nr_wakeups_idle;            /* 空闲唤醒次数 */
};

/* 负载权重 */
struct load_weight {
    u32 weight;                     /* 权重 */
    u32 inv_weight;                 /* 反向权重 */
};

/* 资源限制 */
struct rlimit {
    u64 rlim_cur;                   /* 当前限制 */
    u64 rlim_max;                   /* 最大限制 */
};

/* 信号处理 */
struct k_sigaction {
    void (*sa_handler)(int);        /* 信号处理函数 */
    u64 sa_flags;                   /* 标志 */
    void (*sa_restorer)(void);      /* 恢复函数 */
    u64 sa_mask;                    /* 信号掩码 */
};

/* 路径结构 */
struct path {
    struct vfsmount *mnt;           /* 挂载点 */
    struct dentry *dentry;          /* 目录项 */
};

/* 任务结构 - 进程控制块 */
struct task_struct {
    volatile long state;            /* 进程状态 */
    int exit_state;                 /* 退出状态 */
    int exit_code;                  /* 退出代码 */
    int exit_signal;                /* 退出信号 */

    u32 flags;                      /* 进程标志 */
    u32 ptrace;                     /* ptrace标志 */

    /* 进程标识 */
    pid_t pid;                      /* 进程ID */
    pid_t tgid;                     /* 线程组ID */
    pid_t ppid;                     /* 父进程ID */
    pid_t pgrp;                     /* 进程组ID */
    pid_t session;                  /* 会话ID */

    /* 用户标识 */
    uid_t uid;                      /* 用户ID */
    gid_t gid;                      /* 组ID */
    uid_t euid;                     /* 有效用户ID */
    gid_t egid;                     /* 有效组ID */
    uid_t suid;                     /* 保存的用户ID */
    gid_t sgid;                     /* 保存的组ID */
    uid_t fsuid;                    /* 文件系统用户ID */
    gid_t fsgid;                    /* 文件系统组ID */

    /* 调度相关 */
    int prio;                       /* 动态优先级 */
    int static_prio;                /* 静态优先级 */
    int normal_prio;                /* 正常优先级 */
    struct sched_entity se;         /* 调度实体 */
    struct sched_rt_entity rt;      /* 实时调度实体 */

    u32 policy;                     /* 调度策略 */
    u64 se_vruntime;               /* 虚拟运行时间 */
    u64 se_sum_exec_runtime;       /* 累计执行时间 */

    /* CPU亲和性 */
    u64 cpus_allowed;               /* 允许的CPU掩码 */
    u32 nr_cpus_allowed;            /* 允许的CPU数量 */

    /* 进程关系 */
    struct task_struct *real_parent; /* 真实父进程 */
    struct task_struct *parent;     /* 父进程 */
    struct list_head children;      /* 子进程链表 */
    struct list_head sibling;       /* 兄弟进程链表 */
    struct task_struct *group_leader; /* 线程组领导者 */

    /* 链表节点 */
    struct list_head tasks;         /* 全局任务链表 */
    struct rb_node run_node;        /* 运行队列红黑树节点 */

    /* 内存管理 */
    struct mm_struct *mm;           /* 内存描述符 */
    struct mm_struct *active_mm;    /* 活跃内存描述符 */

    /* 文件系统 */
    struct fs_struct *fs;           /* 文件系统信息 */
    struct files_struct *files;     /* 文件描述符表 */

    /* 信号处理 */
    struct signal_struct *signal;   /* 信号处理 */
    struct sighand_struct *sighand; /* 信号处理程序 */
    sigset_t blocked;               /* 被阻塞的信号 */
    sigset_t real_blocked;          /* 真实被阻塞的信号 */
    sigset_t saved_sigmask;         /* 保存的信号掩码 */
    struct sigpending pending;      /* 待处理的信号 */

    /* 执行上下文 */
    void *stack;                    /* 内核栈 */
    u64 stack_size;                 /* 栈大小 */

    /* 寄存器状态 */
    struct pt_regs *regs;           /* 寄存器状态 */

    /* 时间统计 */
    u64 utime;                      /* 用户态时间 */
    u64 stime;                      /* 内核态时间 */
    u64 gtime;                      /* 客户态时间 */
    u64 start_time;                 /* 开始时间 */
    u64 real_start_time;            /* 真实开始时间 */

    /* 内存使用统计 */
    ulong min_flt;                  /* 次要页错误 */
    ulong maj_flt;                  /* 主要页错误 */
    ulong nvcsw;                    /* 主动上下文切换 */
    ulong nivcsw;                   /* 被动上下文切换 */

    /* 进程名称 */
    char comm[16];                  /* 进程名称 */

    /* 退出处理 */
    int exit_code;                  /* 退出代码 */
    int exit_signal;                /* 退出信号 */

    /* 内核线程相关 */
    int (*thread_fn)(void *data);   /* 线程函数 */
    void *thread_data;              /* 线程数据 */

    /* 等待队列 */
    wait_queue_head_t *wait_chldexit; /* 等待子进程退出 */

    /* 命名空间 */
    struct nsproxy *nsproxy;        /* 命名空间代理 */

    /* 审计 */
    u32 audit_context;              /* 审计上下文 */

    /* 性能事件 */
    struct perf_event_context *perf_event_ctxp; /* 性能事件上下文 */

    /* 组调度 */
    struct task_group *sched_task_group; /* 调度任务组 */

    /* 控制组 */
    struct cgroup_subsys_state *cgroups; /* 控制组 */

    /* 工作队列 */
    struct list_head ptraced;       /* 被跟踪的进程 */
    struct list_head ptrace_entry;  /* 跟踪条目 */

    /* 锁 */
    spinlock_t alloc_lock;          /* 分配锁 */

    /* 其他 */
    u32 personality;                /* 个性 */
    u32 did_exec:1;                 /* 已执行exec */
    u32 in_execve:1;                /* 正在execve */
    u32 in_iowait:1;                /* 等待I/O */
    u32 no_new_privs:1;             /* 无新权限 */
    u32 sched_reset_on_fork:1;      /* fork时重置调度 */
    u32 sched_contributes_to_load:1; /* 贡献到负载 */
    u32 sched_migrated:1;           /* 已迁移 */

    /* 抢占计数 */
    u32 preempt_count;              /* 抢占计数 */

    /* 待处理的工作 */
    struct restart_block restart_block; /* 重启块 */

    /* 插件钩子 */
    void *security;                 /* 安全模块私有数据 */

    /* 审计 */
    struct audit_context *audit_context; /* 审计上下文 */

    /* 跟踪 */
    unsigned long trace;            /* 跟踪标志 */
    unsigned long trace_recursion;  /* 跟踪递归 */

    /* RCU */
    struct rcu_head rcu;            /* RCU头 */

    /* 延迟计数 */
    struct task_delay_info *delays; /* 延迟信息 */

    /* 故障注入 */
    struct fault_attr *make_it_fail; /* 故障注入 */

    /* 页错误处理 */
    int make_it_fail;               /* 使其失败 */

    /* 目录通知 */
    struct list_head *scm_work_list; /* SCM工作列表 */

    /* 栈金丝雀 */
    unsigned long stack_canary;     /* 栈金丝雀 */

    /* 原子计数 */
    atomic_t usage;                 /* 使用计数 */
    atomic_t live;                  /* 活跃计数 */

    /* 等待队列 */
    wait_queue_head_t wait_chldexit; /* 等待子进程退出 */

    /* 通知链 */
    struct atomic_notifier_head *task_exit_notifier; /* 任务退出通知 */

    /* 进程时间 */
    struct timespec start_time;     /* 开始时间 */
    struct timespec real_start_time; /* 真实开始时间 */

    /* 调度统计 */
    struct sched_info sched_info;   /* 调度信息 */

    /* 任务统计 */
    struct task_stats *stats;       /* 任务统计 */

    /* 锁统计 */
    struct lock_class_key *lock_class_key; /* 锁类键 */

    /* 内存策略 */
    struct mempolicy *mempolicy;    /* 内存策略 */
    short il_next;                  /* 下一个交错节点 */
    short pref_node_fork;           /* fork时的首选节点 */

    /* 块I/O */
    struct bio_list *bio_list;      /* 生物列表 */
    struct blk_plug *plug;          /* 块插件 */

    /* 互斥锁 */
    struct mutex_waiter *blocked_on; /* 阻塞在的互斥锁 */

    /* 延迟 */
    struct task_delay_info *delays; /* 延迟信息 */

    /* 跟踪 */
    struct tracer *tracer;          /* 跟踪器 */

    /* 页面 */
    struct page_frag task_frag;     /* 任务片段 */

    /* 接收队列 */
    struct sk_buff_head sk_receive_queue; /* 套接字接收队列 */

    /* 发送队列 */
    struct sk_buff_head sk_write_queue; /* 套接字写队列 */

    /* 错误队列 */
    struct sk_buff_head sk_error_queue; /* 套接字错误队列 */

    /* 套接字 */
    struct socket *socket;          /* 套接字 */

    /* 网络命名空间 */
    struct net *net_ns;             /* 网络命名空间 */

    /* 最后使用的CPU */
    int last_cpu;                   /* 最后使用的CPU */

    /* 唤醒CPU */
    int wake_cpu;                   /* 唤醒CPU */

    /* 迁移禁用 */
    int migrate_disable;            /* 迁移禁用 */

    /* RCU读取锁计数 */
    int rcu_read_lock_nesting;      /* RCU读取锁嵌套 */

    /* RCU读取解锁特殊 */
    char rcu_read_unlock_special;   /* RCU读取解锁特殊 */

    /* RCU节点 */
    struct rcu_node *rcu_blocked_node; /* RCU阻塞节点 */

    /* RCU任务 */
    struct list_head rcu_tasks_holdout_list; /* RCU任务持有列表 */

    /* RCU任务持有 */
    int rcu_tasks_holdout;          /* RCU任务持有 */

    /* RCU任务空闲CPU */
    int rcu_tasks_idle_cpu;         /* RCU任务空闲CPU */

    /* 原子使用计数 */
    atomic_t usage;                 /* 使用计数 */

    /* 引用计数 */
    struct kref kref;               /* 内核引用 */
};

/* 实时调度实体 */
struct sched_rt_entity {
    struct list_head run_list;      /* 运行列表 */
    unsigned long timeout;          /* 超时 */
    unsigned long watchdog_stamp;   /* 看门狗时间戳 */
    unsigned int time_slice;        /* 时间片 */

    struct sched_rt_entity *back;   /* 后向指针 */
    struct sched_rt_entity *parent; /* 父指针 */
    struct rt_rq *rt_rq;            /* 实时运行队列 */
    struct rt_rq *my_q;             /* 我的队列 */
};

/* 信号处理程序 */
struct sighand_struct {
    atomic_t count;                 /* 引用计数 */
    struct k_sigaction action[64];  /* 信号处理程序 */
    spinlock_t siglock;             /* 信号锁 */
    wait_queue_head_t signalfd_wqh; /* 信号文件描述符等待队列 */
};

/* 待处理信号 */
struct sigpending {
    struct list_head list;          /* 信号链表 */
    sigset_t signal;                /* 信号集 */
};

/* 信号集 */
typedef struct {
    unsigned long sig[2];
} sigset_t;

/* 寄存器状态 */
struct pt_regs {
    unsigned long r15;
    unsigned long r14;
    unsigned long r13;
    unsigned long r12;
    unsigned long rbp;
    unsigned long rbx;
    unsigned long r11;
    unsigned long r10;
    unsigned long r9;
    unsigned long r8;
    unsigned long rax;
    unsigned long rcx;
    unsigned long rdx;
    unsigned long rsi;
    unsigned long rdi;
    unsigned long orig_rax;
    unsigned long rip;
    unsigned long cs;
    unsigned long eflags;
    unsigned long rsp;
    unsigned long ss;
};

/* 重启块 */
struct restart_block {
    long (*fn)(struct restart_block *);
    union {
        struct {
            u32 *uaddr;
            u32 val;
            u32 flags;
            u32 bitset;
            u64 time;
        } futex;
        struct {
            clockid_t clockid;
            struct timespec *rmtp;
            struct timespec __user *compat_rmtp;
            u64 expires;
        } nanosleep;
        struct {
            struct pollfd __user *ufds;
            int nfds;
            int has_timeout;
            unsigned long tv_sec;
            unsigned long tv_nsec;
        } poll;
    };
};

/* 调度信息 */
struct sched_info {
    unsigned long pcount;           /* 调度次数 */
    unsigned long long run_delay;   /* 运行延迟 */
    unsigned long long last_arrival; /* 最后到达时间 */
    unsigned long long last_queued;  /* 最后入队时间 */
    unsigned long pcnt;             /* 处理器次数 */
};

/* 任务统计 */
struct task_stats {
    u64 ac_etime;                   /* 累计时间 */
    u64 ac_utime;                   /* 用户时间 */
    u64 ac_stime;                   /* 系统时间 */
    u64 ac_minflt;                  /* 次要页错误 */
    u64 ac_majflt;                  /* 主要页错误 */
    u64 coremem;                    /* 核心内存 */
    u64 virtmem;                    /* 虚拟内存 */
    u64 hiwater_rss;                /* 高水位RSS */
    u64 hiwater_vm;                 /* 高水位VM */
    u64 read_char;                  /* 读取字符数 */
    u64 write_char;                 /* 写入字符数 */
    u64 read_syscalls;              /* 读取系统调用数 */
    u64 write_syscalls;             /* 写入系统调用数 */
    u64 read_bytes;                 /* 读取字节数 */
    u64 write_bytes;                /* 写入字节数 */
    u64 cancelled_write_bytes;      /* 取消写入字节数 */
    u64 nvcsw;                      /* 主动上下文切换 */
    u64 nivcsw;                     /* 被动上下文切换 */
    u64 ac_utimescaled;             /* 缩放用户时间 */
    u64 ac_stimescaled;             /* 缩放系统时间 */
    u64 cpu_count;                  /* CPU计数 */
    u64 cpu_delay_total;            /* CPU延迟总计 */
    u64 blkio_count;                /* 块I/O计数 */
    u64 blkio_delay_total;          /* 块I/O延迟总计 */
    u64 swapin_count;               /* 换入计数 */
    u64 swapin_delay_total;         /* 换入延迟总计 */
    u64 cpu_run_real_total;         /* CPU真实运行总计 */
    u64 cpu_run_virtual_total;      /* CPU虚拟运行总计 */
    u64 ac_comm[32];                /* 命令名称 */
    u64 ac_exe_dev;                 /* 可执行设备 */
    u64 ac_exe_inode;               /* 可执行索引节点 */
    u64 wpcopy_count;               /* 写时复制计数 */
    u64 wpcopy_delay_total;         /* 写时复制延迟总计 */
};

/* 等待队列头 */
typedef struct wait_queue_head {
    spinlock_t lock;                /* 锁 */
    struct list_head task_list;     /* 任务列表 */
} wait_queue_head_t;

/* 等待队列条目 */
typedef struct wait_queue_entry {
    unsigned int flags;             /* 标志 */
    void *private;                  /* 私有数据 */
    int (*func)(struct wait_queue_entry *, unsigned, int, void *); /* 函数 */
    struct list_head entry;         /* 条目 */
} wait_queue_entry_t;

/* 当前进程宏 */
extern struct task_struct *current;
#define current get_current()

/* 进程状态检查宏 */
#define task_is_running(task)       ((task)->state == TASK_RUNNING)
#define task_is_interruptible(task) ((task)->state == TASK_INTERRUPTIBLE)
#define task_is_uninterruptible(task) ((task)->state == TASK_UNINTERRUPTIBLE)
#define task_is_zombie(task)        ((task)->state == TASK_ZOMBIE)
#define task_is_stopped(task)       ((task)->state == TASK_STOPPED)
#define task_is_traced(task)        ((task)->state == TASK_TRACED)

/* 进程管理函数声明 */
extern struct task_struct *get_current(void);
extern void schedule(void);
extern void yield(void);
extern long do_fork(unsigned long clone_flags, unsigned long stack_start,
                   unsigned long stack_size, int *parent_tidptr,
                   int *child_tidptr);
extern void do_exit(long code) __noreturn;
extern void do_group_exit(int exit_code) __noreturn;
extern long do_wait(struct wait_opts *wo);
extern long do_waitpid(pid_t pid, int *stat_addr, int options);
extern int do_execve(const char *filename, const char *const argv[],
                    const char *const envp[]);
extern int do_execveat(int fd, const char *filename,
                      const char *const argv[], const char *const envp[],
                      int flags);

/* 调度器函数声明 */
extern void sched_init(void);
extern void scheduler_tick(void);
extern void wakeup_process(struct task_struct *p);
extern void wake_up_new_task(struct task_struct *p);
extern void activate_task(struct rq *rq, struct task_struct *p, int flags);
extern void deactivate_task(struct rq *rq, struct task_struct *p, int flags);
extern void check_preempt_curr(struct rq *rq, struct task_struct *p, int flags);
extern struct task_struct *pick_next_task(struct rq *rq, struct task_struct *prev);
extern void put_prev_task(struct rq *rq, struct task_struct *prev);
extern void set_next_task(struct rq *rq, struct task_struct *next);
extern void resched_curr(struct rq *rq);
extern void resched_cpu(int cpu);
extern int can_migrate_task(struct task_struct *p, struct lb_env *env);
extern struct task_struct *load_balance(struct rq *this_rq, int idle,
                                       struct rq *busiest, unsigned long *nr_moved);

/* 任务创建和销毁 */
extern struct task_struct *alloc_task_struct(void);
extern void free_task_struct(struct task_struct *tsk);
extern void __put_task_struct(struct task_struct *tsk);
extern void put_task_struct(struct task_struct *tsk);
extern struct task_struct *dup_task_struct(struct task_struct *orig);

/* 进程组和会话 */
extern void change_pid(struct task_struct *task, enum pid_type type, struct pid *pid);
extern void attach_pid(struct task_struct *task, enum pid_type type);
extern void detach_pid(struct task_struct *task, enum pid_type type);
extern void transfer_pid(struct task_struct *old, struct task_struct *new,
                        enum pid_type type);

/* 调度类 */
extern const struct sched_class stop_sched_class;
extern const struct sched_class dl_sched_class;
extern const struct sched_class rt_sched_class;
extern const struct sched_class fair_sched_class;
extern const struct sched_class idle_sched_class;

/* 运行队列 */
struct rq {
    spinlock_t lock;                /* 运行队列锁 */
    unsigned int nr_running;        /* 运行任务数 */
    unsigned int nr_numa_running;   /* NUMA运行任务数 */
    unsigned int nr_preferred_running; /* 首选运行任务数 */

    struct load_weight load;        /* 负载权重 */
    unsigned long nr_load_updates;  /* 负载更新次数 */
    u64 nr_switches;               /* 切换次数 */

    struct cfs_rq cfs;             /* CFS运行队列 */
    struct rt_rq rt;               /* RT运行队列 */
    struct dl_rq dl;               /* DL运行队列 */

    struct task_struct *curr;       /* 当前任务 */
    struct task_struct *idle;       /* 空闲任务 */
    struct task_struct *stop;       /* 停止任务 */

    unsigned long next_balance;     /* 下次负载均衡时间 */
    struct mm_struct *prev_mm;      /* 上一个内存描述符 */

    u64 clock;                     /* 时钟 */
    u64 clock_task;                /* 任务时钟 */

    atomic_t nr_iowait;            /* I/O等待数 */

    struct root_domain *rd;         /* 根域 */
    struct sched_domain *sd;        /* 调度域 */

    unsigned long cpu_capacity;     /* CPU容量 */
    unsigned long cpu_capacity_orig; /* 原始CPU容量 */

    struct callback_head *balance_callback; /* 负载均衡回调 */

    unsigned char idle_balance;     /* 空闲负载均衡 */

    int post_schedule;             /* 后调度 */

    int active_balance;            /* 主动负载均衡 */
    int push_cpu;                  /* 推送CPU */
    struct cpu_stop_work active_balance_work; /* 主动负载均衡工作 */

    int cpu;                       /* CPU编号 */
    int online;                    /* 在线状态 */

    struct list_head cfs_tasks;    /* CFS任务列表 */

    u64 rt_avg;                    /* RT平均值 */
    u64 age_stamp;                 /* 年龄戳 */
    u64 idle_stamp;                /* 空闲戳 */
    u64 avg_idle;                  /* 平均空闲时间 */

    u64 prev_irq_time;             /* 上一次中断时间 */
    u64 prev_steal_time;           /* 上一次被偷时间 */
    u64 prev_steal_time_rq;        /* 上一次被偷时间(运行队列) */

    int lru_count;                 /* LRU计数 */
    struct call_single_data nohz_csd; /* NOHZ单调用数据 */
    unsigned int nohz_tick_stopped; /* NOHZ时钟停止 */
    atomic_t nohz_flags;           /* NOHZ标志 */

    unsigned long last_load_update_tick; /* 最后负载更新时钟 */
    unsigned long last_blocked_load_update_tick; /* 最后阻塞负载更新时钟 */
    unsigned int has_blocked_load;  /* 有阻塞负载 */

    u64 nohz_stamp;                /* NOHZ时间戳 */
    unsigned long nohz_flags;       /* NOHZ标志 */

    unsigned long max_idle_balance_cost; /* 最大空闲负载均衡成本 */

    struct sched_avg avg_rt;        /* RT平均值 */
    struct sched_avg avg_dl;        /* DL平均值 */
    struct sched_avg avg_irq;       /* 中断平均值 */

    u64 idle_stamp;                /* 空闲戳 */
    u64 avg_idle;                  /* 平均空闲时间 */

    unsigned long wake_stamp;       /* 唤醒戳 */
    u64 wake_avg_idle;             /* 唤醒平均空闲时间 */

    int balance_cpu;               /* 负载均衡CPU */

    struct cpu_stop_work balance_work; /* 负载均衡工作 */

    struct rq_flags rf;            /* 运行队列标志 */

    unsigned int yf_flags;         /* YF标志 */

    u64 clock_skip_update;         /* 时钟跳过更新 */

    struct sched_avg avg_thermal;   /* 热平均值 */

    struct cpu_capacity_info capacity_info; /* CPU容量信息 */

    struct rq_flags rf;            /* 运行队列标志 */

    unsigned int ttwu_count;       /* 尝试唤醒计数 */
    unsigned int ttwu_local;       /* 本地唤醒计数 */

    struct hrtimer hrtick_timer;   /* 高分辨率时钟定时器 */
    ktime_t hrtick_time;           /* 高分辨率时钟时间 */

    struct sched_info rq_s
