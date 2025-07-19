#define SCHED_H

#include "types.h"
#include "list.h"
#include "config.h"

#define TASK_RUNNING            0
#define TASK_INTERRUPTIBLE      1
#define TASK_UNINTERRUPRIBLE    2
#define TASK_ZOMBIE             4
#define TASK_STOPPED            8
#define TASK_TRACED            16


#define MAX_NICE    19
#define MIN_NICE    -20
#define NICE_WIDTH    40
#define DEFAULT_PRIO    120
#define MAX_USER_RT_PRIO  100
#define MAX_RT_PRIO    100
#define MAX_PRIO    140
#define NICE_0_LOAD    1024
#define NICE_0_SHIFT    10







#define PF_KTHREAD                   0x00000001
#define PF_IDLE                      0x00000002
#define PF_EXITING                   0x00000004
#define PF_EXITPIDONE                0x00000008
#define PF_VCPU                      0x00000010
#define PF_WQ_WORKER                 0x00000020
#define PF_FORKNOEXEC                0x00000040
#define PF_MCE_PROCESS               0x00000080
#define PF_SUPERPRIV                 0x00000100
#define PF_DUMPCORE                  0x00000200
#define PF_SIGNALED                  0x00000400
#define PF_MEMALLOC                  0x00000800
#define PF_NPROC_EXCEEDED            0x00001000
#define PF_USED_MATH                 0x00002000
#define PF_USED_ASYNC                0x00004000
#define PF_NOFREEZE                  0x00008000
#define PF_FROZEN                    0x00010000
#define PF_FREEZER_SKIP              0x00020000


#define CLONE_VM                     0x00000100
#define CLONE_FS                     0x00000200
#define CLONE_FILES                  0x00000400
#define CLONE_SIGHAND                0x00000800
#define CLONE_PTRACE                 0x00002000
#define CLONE_VFORK                  0x00004000
#define CLONE_PARENT                 0x00008000
#define CLONE_THREAD                 0x00010000
#define CLONE_NEWNS                  0x00020000
#define CLONE_SYSVSEM                0x00040000
#define CLONE_SETTLS                 0x00080000
#define CLONE_PARENT_SETTID          0x00100000
#define CLONE_CHILD_CLEARTID         0x00200000
#define CLONE_DETACHED               0x00400000
#define CLONE_UNTRACED               0x00800000
#define CLONE_CHILD_SETTID           0x01000000
#define CLONE_NEWCGROUP              0x02000000
#define CLONE_NEWUTS                 0x04000000
#define CLONE_NEWIPC                 0x08000000
#define CLONE_NEWUSER                0x10000000
#define CLONE_NEWPID                 0x20000000
#define CLONE_NEWNET                 0x40000000
#define CLONE_IO                     0x80000000



struct mm_struct{
    struct list_head mmmp_list;
    struct rb_root mm_rb;
    u32 map_count;
    spinlock_t page_table_lock;


    ulong mmap_base;
    ulong mmap_lengcy_base;
    ulong tasks_size;
    ulong highest_vm_ended;



    phys_addr_t pgd;

    ulong total_vm;
    ulong locaked_vm;
    ulong pinned_vm;
    ulong shared_vm;
    ulong exec_vm;
    ulong stack_vm;
    ulong data_vm;


    ulong start_code, end_code;
    ulong start_data, end_data;
    ulong start_brk, brk;
    ulong start_stack;
    ulong arg_start, arg_end;
    ulong env_start, env_end;


    u32 mm_users;
    u32 mm_count;

    u32 core_state;
    spinlock_t ioctx_lock;
    
};


struct fs_struct{
    int users;
    spinlock_t lock;
    struct path root;
    struct path pwd;

    mode_t umask;
}


struct files_struct{
    u32 count;
    spinlock_t file_lock;


    struct file **fdt;
    u32 max_fds;
    u32 next_fd;

    struct file *fd_array[32];
};


struct signal_struct{
    u32 count ;
    u32 live;
    struct k_sigaction action[64];
    spinlock_t siglock;
    pid_t pgrp;
    pid_t session;
    u64 utime;
    u64, stime;
    u64 cutime;
    u64 cstime;

    struct rlimit rlim[16];
};

struct sched_entity
{
    struct load_weight load;
    struct rb_node run_node;
    struct list_head group_node;
    u64 exec_start;
    u64 sum_exec_runtime;
    u64 vruntime;
    u64 prev_sum_exec_runtime;


    u64 nr_migrations;
    u64 start_runtime;
    u64 avg_overlap;
    u64 avg_wakeup;
    u64 avg_running;


    s64 sum_sleep_runtime;
    s64 sum_block_runtime;
    s64 exec_max;
    s64 slice_max;
u64 nr_migrations_cold;
    u64 nr_failed_migrations_affine;
    u64 nr_failed_migrations_running;
    u64 nr_failed_migrations_hot;
    u64 nr_forced_migrations;

    u64 nr_wakeups;
    u64 nr_wakeups_sync;
    u64 nr_wakeups_migrate;
    u64 nr_wakeups_local;
    u64 nr_wakeups_remote;
    u64 nr_wakeups_affine;
    u64 nr_wakeups_affine_attempts;
    u64 nr_wakeups_passive;
    u64 nr_wakeups_idle;
    /* data */
};


struct load_weight {
    u64 weight;
    u64 inv_weight;
};

struct rlomit{
    u64 rlim_cur;
    u64 rlim_max;
};


struct k_sigaction
{

    void (*sa_handler)(int);
    u64 sa_flags;
    void (*sa_restorer)(void);
    u64 sa_mask;
    /* data */
};


struct path{
    struct vfsmount *mnt;
    struct dentry *dentry;
};


struct task_struct{
    volatile long state;
    int exit_state;
    int exit_code;
    int exit_signal;


    u32 flags;
    u32 ptrace;

    pid_t pid;
    pid_t tgid;
    pid_t ppid;
    pid_t pgrp;
    pid_t session;

    uid_t uid;
    uid_t gid;
    uid_t euid;
    uid_t egid;
    uid_t suid;
    uid_t sgid;
    uid_t fsuid;
    uid_t fsgid;


    int prio;
    int static_prio;
    int normal_prio;
    struct sched_entity se;
    struct sched_rt_entity rt;
    


    u32 policy;
    u64 se_vruntime;
    u64 se_sum_exec_runtime;


    u64 cpus_allowed;
    u32 nr_cpus_allowed;


    struct task_struct *real_parent;
    struct task_struct *parent;
    struct list_head children;
    struct list_head sibling;
    struct task_struct *group_leader;




    struct list_head tasks;
    struct rb_node run_node;

    struct mm_struct *mm;
    struct mm_struct *active_mm;


    struct fs_struct *fs;
    struct files_struct *files;


    struct signal_struct *signal;
    struct signal_struct *sighand;
    sigset_t blocked;
    sigset_t real_blocked;
    sigset_t saved_sigmask;
    struct sigpending pending;

    void *stack;
    u64 stack_size;


    struct pt_regs *regs;


    u64 utime;
    u64 stime;
    u64 gtime;
    u64 start_time;
    u64 real_start_time;



    ulong min_flt;
    ulong maj_flt;
    ulong nvcsw;
    ulong nivcsw;


    char comm[16];
    

    int exit_code;
    int exit_signal;       ;


    int (*thread_fn)(void *data);
    void *thread_data;
    

    wait_queue_head_t *wait_child_exit;

    struct nsproxy *nsproxy;

    u32 audit_context;

    struct perf_envent_context *perf_event_ctxp;


    struct task_group *sched_task_group;
    struct cgoup_subsys_staten  *cgroups;



    struct list_head ptraced;
    struct list_head ptrace_entry;


    spinlock allock_lock;

    u32 personality;
    u32 did_exec:1;
    u32 in_execve:1;
    u32 in_iowait:1;
    u32 no_new_privs:1;
    u32 sched_reset_on_fork:1;
    u32 sched_contributes_to_load:1;
    u32 sched_migrated:1;



    u32 preempt_count;


    struct restart_block restart_block;



    void *security;
    struct audit_context *audit_context;
    ulong trace;
    usigned long trace_recursion;

    struct rcu_head rcu;
    struct task_delay_info *delays;
    struct fault_attr *make_it_fail;


    int make_it_fail;




    struct list_head *scm_work_list;

    ulong *stack_canary;


    atomic_t usage;
    atomic_t live;


    wait_queue_head_t wait_chldexit;

    struct atomic_notifier_head *task_exit_notifier;

    struct timespec start_time;
    struct timespec real_start_time;


    struct sched_info sched_info;


    struct task_stats *stats;


    struct lock_class_key *lock_class_key;


    struct mempolicy *mempolicy;
    short il_next;
    short pref_node_fork;



    struct bio_list *bio_list;
    struct blk_plug *plug;


    struct mutex_waiter *blocked_on;
    

    struct task_delay_info *delays;
    
    struct tracer *tracer;


    struct page_freg task_freg;


    struct sk_buff_head sk_receive_queue;


    struct sk_buff_head sk_write_queue;

    struct sk_buff_head sk_error_queue;

    struct socket *socket;

    struct net *ns_net;

    int last_cpu;


    int wake_cpu;


    int migrate_disable;


    int RCU_read_lock_nesting;


    int RCU_read_unlock_special;

struct RCU_node *RCU_blocked_node;

    struct list_head rcu_tasks_hold_out_list;



    int rcu_tasks_idle_cpu;

    atomic_t usage;

    struct kref kref;



};



struct sched_rt_entity
{
    struct list_head run_list;
    ulong timeout;
    ulong watchdog_stamp;
    unsigned inrt time_slice;

    struct sched_rt_entity *back;
    struct sched_rt_entity *parent;
    struct rt_rq *rt_rq;
    struct rt_rq *my_q;

    /* data */
};



struct sighand_struct
{
  atomic_t count;
  struct k_sigaction action[64];
    spinlock_t siglock;
    wait_queue_head_t signalfd_wqh;
    /* data */
};


struct sigpending{
    struct list_hand list;
    sigset_t signal;
}


typedef struct {
    ulong sig[2];
} sigset_t;


struct pt_regs{
    ulong r15;
    ulong r14;
    ulong r13;
    ulong r12;
    ulong r11;
    ulong r10;
    ulong r9;
    ulong r8;
    ulong rax;
    ulong rbx;
    ulong rcx;
    ulong rdx;      
    ulong rsi;
    ulong rdi;
    ulong rbp;
    ulong eflags;
    ulong rip;
    ulong cs;
    ulong ss;
    ulong ds;
    ulong es;
    ulong fs;
    ulong gs;
    ulong err;
    ulong trapno;
    ulong cr2;
    ulong sp;   
    ulong rsp;
    ulong fs_base;
    ulong gs_base;
    ulong ds_base;
    ulong es_base;
    ulong ss_base;  



};

/*it's a totally shit  restart_block I need to rewrite*/
struct restart_block {
    int (*fn)(struct pt_regs *);
    struct pt_regs *regs;
    unsigned long args[6];
    unsigned long error;
    unsigned long saved_rip;
    unsigned long saved_rsp;
    unsigned long saved_rflags;
};




struct sched_info{
    ulong pcount;
    u64 run_delay;
    u64 last_arrival;
    u64 last_queued;
    ulong pcnt;
};


struct task_stats{
    u64 ac_etime;
    u64 ac_utime;
    u64 ac_stime;
    u64 ac_minflt;
    u64 ac_majflt;
    u64 coremem;
    u64 virtmem;
    u64 hiwater_rss;
    u64 hiwater_vm;
    u64 read_char;
    u64 write_cahr;
    u64 read_syscalls;
    u64 read_bytes;
    u64 write_bytes;
    u64 cancelled_write_bytes;
    u64 nvcsw;
    u64 nivcsw;
    u64 ac_utimescaled;
    u64 ac_stimescaled;
    u64 cpu_count;
    u64 cpu_delay_total;
    u64 blkio_count;
    u64 blkio_delay_total;
    u64 swapin_count;
    u64 swapin_delay_total;
    u64 cpu_run_real_total;
    u64 cpu_run_virtual_total;
    u64 ac_comm[32];
    u64 ac_exe_dev;
    u64 ac_exe_inode;
    u64 wpcopy_count;
    u64 wpcopy_delay_total;


};




typedef struct wait_queue_head {
    spinlock_t lock;
    struct list_head task_list;
} wait_queue_head_t;

typedef struct wait_queue_entry {
    struct list_head task_list;
    int flags;
    int private;
    void (*func)(wait_queue_entry_t *, unsigned int, int);
    void *data;
} wait_queue_entry_t;


extern struct task_struct *current;
#define current get_current()

#define task_is_running(p) ((p)->state == TASK_RUNNING)
#define task_is_stopped(p) ((p)->state == TASK_STOPPED)
#define task_is_zombie(p) ((p)->state == TASK_ZOMBIE)
#define task_is_interruptible(p) ((p)->state == TASK_INTERRUPTIBLE)
#define task_is_uninterruptible(p) ((p)->state == TASK_UNINTERRUPRIBLE)
#define task_is_traced(p) ((p)->state & TASK_TRACED)


extern struct task_struct *get_current(void);


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
