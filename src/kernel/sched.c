#include "../../include/sched.h"
#include "../../include/types.h"
#include "../../include/list.h"
#include "../../include/spinlock.h"
#include "../../include/mm.h"

/* 全局变量 */
static struct task_struct *current_task = NULL;
static struct list_head task_list;
static spinlock_t task_list_lock;
static pid_t next_pid = 1;
static struct task_struct *init_task = NULL;

static struct rq runqueues[NR_CPUS];
static struct task_struct *idle_tasks[NR_CPUS];

#define DEFAULT_TIMESLICE_MS    100
#define HZ                      1000
#define NICE_TO_WEIGHT_SHIFT    10

static const int prio_to_weight[40] = {
    /* -20 */ 88761, 71755, 56483, 46273, 36291,
    /* -15 */ 29154, 23254, 18705, 14949, 11916,
    /* -10 */ 9548, 7620, 6100, 4904, 3906,
    /*  -5 */ 3121, 2501, 1991, 1586, 1277,
    /*   0 */ 1024, 820, 655, 526, 423,
    /*   5 */ 335, 272, 215, 172, 137,
    /*  10 */ 110, 87, 70, 56, 45,
    /*  15 */ 36, 29, 23, 18, 15,
};

void sched_init(void)
{
    int cpu;

    INIT_LIST_HEAD(&task_list);
    spin_lock_init(&task_list_lock);

    for (cpu = 0; cpu < NR_CPUS; cpu++) {
        struct rq *rq = &runqueues[cpu];

        spin_lock_init(&rq->lock);
        rq->nr_running = 0;
        rq->load.weight = 0;
        rq->load.inv_weight = 0;
        rq->curr = NULL;
        rq->idle = NULL;
        rq->stop = NULL;
        rq->cpu = cpu;
        rq->online = 1;

        rq->cfs.tasks_timeline = RB_ROOT;
        rq->cfs.min_vruntime = 0;
        rq->cfs.nr_running = 0;
        rq->cfs.load.weight = 0;
        rq->cfs.load.inv_weight = 0;
        INIT_LIST_HEAD(&rq->cfs_tasks);

        rq->rt.rt_nr_running = 0;
        rq->rt.highest_prio.curr = MAX_RT_PRIO;
        rq->rt.highest_prio.next = MAX_RT_PRIO;

        rq->dl.dl_nr_running = 0;
        rq->dl.dl_time = 0;
        rq->dl.dl_bw = 0;

        rq->clock = 0;
        rq->clock_task = 0;

        rq->next_balance = 0;
        rq->balance_callback = NULL;
        rq->idle_balance = 0;
        rq->active_balance = 0;
        rq->push_cpu = -1;

        rq->nr_switches = 0;
        rq->nr_load_updates = 0;
        rq->avg_idle = 0;
        rq->idle_stamp = 0;
        rq->age_stamp = 0;
        rq->rt_avg = 0;
        rq->prev_irq_time = 0;
        rq->prev_steal_time = 0;
        rq->prev_steal_time_rq = 0;

        rq->nohz_tick_stopped = 0;
        atomic_set(&rq->nohz_flags, 0);
        rq->nohz_stamp = 0;
        rq->last_load_update_tick = 0;
        rq->last_blocked_load_update_tick = 0;
        rq->has_blocked_load = 0;

        rq->max_idle_balance_cost = 0;
        rq->wake_stamp = 0;
        rq->wake_avg_idle = 0;
        rq->balance_cpu = -1;
        rq->ttwu_count = 0;
        rq->ttwu_local = 0;

        memset(&rq->rq_sched_info, 0, sizeof(rq->rq_sched_info));
    }

    printk("Scheduler initialized\n");
}

struct task_struct *get_current(void)
{
    return current_task;
}

void set_current(struct task_struct *task)
{
    current_task = task;
}

static pid_t alloc_pid(void)
{
    pid_t pid;

    pid = next_pid++;
    if (next_pid >= PID_MAX)
        next_pid = 1;

    return pid;
}

struct task_struct *alloc_task_struct(void)
{
    struct task_struct *task;

    task = kmalloc(sizeof(struct task_struct), GFP_KERNEL);
    if (!task)
        return NULL;

    memset(task, 0, sizeof(struct task_struct));

    task->stack = kmalloc(THREAD_SIZE, GFP_KERNEL);
    if (!task->stack) {
        kfree(task);
        return NULL;
    }

    task->stack_size = THREAD_SIZE;

    task->state = TASK_RUNNING;
    task->exit_state = 0;
    task->exit_code = 0;
    task->exit_signal = SIGCHLD;
    task->flags = 0;
    task->ptrace = 0;

    task->pid = alloc_pid();
    task->tgid = task->pid;
    task->ppid = 0;
    task->pgrp = 0;
    task->session = 0;

    task->uid = 0;
    task->gid = 0;
    task->euid = 0;
    task->egid = 0;
    task->suid = 0;
    task->sgid = 0;
    task->fsuid = 0;
    task->fsgid = 0;

    task->prio = DEFAULT_PRIO;
    task->static_prio = DEFAULT_PRIO;
    task->normal_prio = DEFAULT_PRIO;
    task->policy = SCHED_NORMAL;

    task->se.load.weight = prio_to_weight[task->static_prio - MAX_RT_PRIO];
    task->se.load.inv_weight = 0;
    task->se.exec_start = 0;
    task->se.sum_exec_runtime = 0;
    task->se.vruntime = 0;
    task->se.prev_sum_exec_runtime = 0;
    task->se.nr_migrations = 0;

    INIT_LIST_HEAD(&task->rt.run_list);
    task->rt.timeout = 0;
    task->rt.watchdog_stamp = 0;
    task->rt.time_slice = DEFAULT_TIMESLICE;
    task->rt.back = NULL;
    task->rt.parent = NULL;
    task->rt.rt_rq = NULL;
    task->rt.my_q = NULL;

    task->cpus_allowed = (1UL << NR_CPUS) - 1;  /* 允许所有CPU */
    task->nr_cpus_allowed = NR_CPUS;

    task->real_parent = NULL;
    task->parent = NULL;
    INIT_LIST_HEAD(&task->children);
    INIT_LIST_HEAD(&task->sibling);
    task->group_leader = task;

    INIT_LIST_HEAD(&task->tasks);
    RB_CLEAR_NODE(&task->run_node);

    task->mm = NULL;
    task->active_mm = NULL;

    task->fs = NULL;
    task->files = NULL;

    task->signal = NULL;
    task->sighand = NULL;
    sigemptyset(&task->blocked);
    sigemptyset(&task->real_blocked);
    sigemptyset(&task->saved_sigmask);

    task->regs = NULL;

    task->utime = 0;
    task->stime = 0;
    task->gtime = 0;
    task->start_time = 0;
    task->real_start_time = 0;

    task->min_flt = 0;
    task->maj_flt = 0;
    task->nvcsw = 0;
    task->nivcsw = 0;

    strcpy(task->comm, "suibian");

    task->thread_fn = NULL;
    task->thread_data = NULL;

    task->nsproxy = NULL;

    task->audit_context = 0;

    task->perf_event_ctxp = NULL;

    task->sched_task_group = NULL;

    task->cgroups = NULL;

    INIT_LIST_HEAD(&task->ptraced);
    INIT_LIST_HEAD(&task->ptrace_entry);


    task->personality = 0;
    task->did_exec = 0;
    task->in_execve = 0;
    task->in_iowait = 0;
    task->no_new_privs = 0;
    task->sched_reset_on_fork = 0;
    task->sched_contributes_to_load = 0;
    task->sched_migrated = 0;

    task->preempt_count = 0;

    task->security = NULL;

    task->trace = 0;
    task->trace_recursion = 0;

    task->delays = NULL;

    task->make_it_fail = 0;

    task->scm_work_list = NULL;


    atomic_set(&task->usage, 1);

    init_waitqueue_head(&task->wait_chldexit);

    task->task_exit_notifier = NULL;

    memset(&task->sched_info, 0, sizeof(task->sched_info));

    task->stats = NULL;

    task->mempolicy = NULL;
    task->il_next = 0;
    task->pref_node_fork = 0;

    task->bio_list = NULL;
    task->plug = NULL;

    task->blocked_on = NULL;

    task->tracer = NULL;

    task->socket = NULL;

    task->net_ns = NULL;

    task->last_cpu = -1;
    task->wake_cpu = -1;

    task->migrate_disable = 0;

    task->rcu_read_lock_nesting = 0;
    task->rcu_read_unlock_special = 0;
    task->rcu_blocked_node = NULL;
    INIT_LIST_HEAD(&task->rcu_tasks_holdout_list);
    task->rcu_tasks_holdout = 0;
    task->rcu_tasks_idle_cpu = -1;

    return task;
}

void free_task_struct(struct task_struct *tsk)
{
    if (!tsk)
        return;

    if (tsk->stack)
        kfree(tsk->stack);
    kfree(tsk);
}

void get_task_struct(struct task_struct *tsk)
{
    atomic_inc(&tsk->usage);
}

void put_task_struct(struct task_struct *tsk)
{
    if (atomic_dec_and_test(&tsk->usage)) {
        free_task_struct(tsk);
    }
}

struct task_struct *dup_task_struct(struct task_struct *orig)
{
    struct task_struct *tsk;

    tsk = alloc_task_struct();
    if (!tsk)
        return NULL;

    *tsk = *orig;

    tsk->pid = alloc_pid();
    tsk->state = TASK_RUNNING;
    tsk->exit_state = 0;
    tsk->exit_code = 0;

    /* 重新初始化链表节点 */
    INIT_LIST_HEAD(&tsk->tasks);
    INIT_LIST_HEAD(&tsk->children);
    INIT_LIST_HEAD(&tsk->sibling);
    RB_CLEAR_NODE(&tsk->run_node);

    /* 重新初始化锁 */
    spin_lock_init(&tsk->alloc_lock);

    /* 重新初始化等待队列 */
    init_waitqueue_head(&tsk->wait_chldexit);

    /* 重新初始化统计信息 */
    tsk->utime = 0;
    tsk->stime = 0;
    tsk->gtime = 0;
    tsk->start_time = get_jiffies_64();
    tsk->real_start_time = tsk->start_time;

    tsk->min_flt = 0;
    tsk->maj_flt = 0;
    tsk->nvcsw = 0;
    tsk->nivcsw = 0;

    /* 重新初始化调度实体 */
    tsk->se.exec_start = 0;
    tsk->se.sum_exec_runtime = 0;
    tsk->se.prev_sum_exec_runtime = 0;
    tsk->se.nr_migrations = 0;

    /* 重新初始化引用计数 */
    atomic_set(&tsk->usage, 1);

    return tsk;
}

/* 将任务添加到调度器 */
void sched_fork(struct task_struct *p)
{
    ulong flags;
    int cpu = smp_processor_id();
    struct rq *rq = cpu_rq(cpu);

    /* 初始化调度相关字段 */
    p->state = TASK_RUNNING;
    p->se.exec_start = 0;
    p->se.sum_exec_runtime = 0;
    p->se.prev_sum_exec_runtime = 0;
    p->se.vruntime = 0;
    p->se.nr_migrations = 0;

    /* 设置调度策略 */
    p->policy = SCHED_NORMAL;
    p->prio = current->normal_prio;
    p->static_prio = current->static_prio;
    p->normal_prio = current->normal_prio;

    /* 设置负载权重 */
    p->se.load.weight = prio_to_weight[p->static_prio - MAX_RT_PRIO];
    p->se.load.inv_weight = 0;

    /* 设置CPU亲和性 */
    p->cpus_allowed = current->cpus_allowed;
    p->nr_cpus_allowed = current->nr_cpus_allowed;

    /* 如果是内核线程，继承父进程的活跃内存描述符 */
    if (unlikely(p->flags & PF_KTHREAD)) {
        p->mm = NULL;
        p->active_mm = current->active_mm;
        if (p->active_mm)
            atomic_inc(&p->active_mm->mm_count);
    }

    /* 初始化调度统计 */
    memset(&p->sched_info, 0, sizeof(p->sched_info));

    /* 设置最后使用的CPU */
    p->last_cpu = cpu;
    p->wake_cpu = cpu;

    /* 初始化抢占计数 */
    p->preempt_count = FORK_PREEMPT_COUNT;

    /* 添加到全局任务列表 */
    spin_lock_irqsave(&task_list_lock, flags);
    list_add_tail(&p->tasks, &task_list);
    spin_unlock_irqrestore(&task_list_lock, flags);
}

/* 唤醒新任务 */
void wake_up_new_task(struct task_struct *p)
{
    ulong flags;
    struct rq *rq;
    int cpu = smp_processor_id();

    /* 设置任务状态为运行中 */
    p->state = TASK_RUNNING;

    /* 获取运行队列 */
    rq = task_rq_lock(p, &flags);

    /* 激活任务 */
    activate_task(rq, p, 0);

    /* 检查是否需要抢占 */
    check_preempt_curr(rq, p, WF_FORK);

    /* 释放运行队列锁 */
    task_rq_unlock(rq, p, &flags);
}

/* 激活任务 */
void activate_task(struct rq *rq, struct task_struct *p, int flags)
{
    if (task_contributes_to_load(p))
        rq->nr_uninterruptible--;

    enqueue_task(rq, p, flags);
}

/* 去激活任务 */
void deactivate_task(struct rq *rq, struct task_struct *p, int flags)
{
    if (task_contributes_to_load(p))
        rq->nr_uninterruptible++;

    dequeue_task(rq, p, flags);
}

/* 将任务加入运行队列 */
static void enqueue_task(struct rq *rq, struct task_struct *p, int flags)
{
    update_rq_clock(rq);

    if (!(flags & ENQUEUE_RESTORE))
        sched_info_queued(rq, p);

    p->sched_class->enqueue_task(rq, p, flags);
}

/* 将任务从运行队列移除 */
static void dequeue_task(struct rq *rq, struct task_struct *p, int flags)
{
    update_rq_clock(rq);

    if (!(flags & DEQUEUE_SAVE))
        sched_info_dequeued(rq, p);

    p->sched_class->dequeue_task(rq, p, flags);
}

/* 更新运行队列时钟 */
static void update_rq_clock(struct rq *rq)
{
    s64 delta;

    delta = sched_clock_cpu(cpu_of(rq)) - rq->clock;
    if (delta < 0)
        return;

    rq->clock += delta;
    update_rq_clock_task(rq, delta);
}

/* 更新运行队列任务时钟 */
static void update_rq_clock_task(struct rq *rq, s64 delta)
{
    s64 steal = 0, irq_delta = 0;

    irq_delta = irq_time_read(cpu_of(rq)) - rq->prev_irq_time;

    if (irq_delta > delta)
        irq_delta = delta;

    rq->prev_irq_time += irq_delta;
    delta -= irq_delta;

    steal = steal_account_process_tick(cpu_of(rq));

    if (steal > delta)
        steal = delta;

    rq->prev_steal_time_rq += steal;
    delta -= steal;

    rq->clock_task += delta;
}

/* 获取运行队列 */
struct rq *cpu_rq(int cpu)
{
    return &runqueues[cpu];
}

/* 获取任务的运行队列 */
static struct rq *task_rq(struct task_struct *p)
{
    return cpu_rq(task_cpu(p));
}

/* 获取任务所在的CPU */
static int task_cpu(struct task_struct *p)
{
    return p->last_cpu;
}

/* 锁定任务的运行队列 */
static struct rq *task_rq_lock(struct task_struct *p, ulong *flags)
{
    struct rq *rq;

    for (;;) {
        rq = task_rq(p);
        spin_lock_irqsave(&rq->lock, *flags);
        if (likely(rq == task_rq(p)))
            return rq;
        spin_unlock_irqrestore(&rq->lock, *flags);
    }
}

/* 解锁任务的运行队列 */
static void task_rq_unlock(struct rq *rq, struct task_struct *p, ulong *flags)
{
    spin_unlock_irqrestore(&rq->lock, *flags);
}

/* 主调度函数 */
void schedule(void)
{
    struct task_struct *prev, *next;
    struct rq *rq;
    ulong flags;
    int cpu;

    cpu = smp_processor_id();
    rq = cpu_rq(cpu);
    prev = rq->curr;

    local_irq_save(flags);

    /* 锁定运行队列 */
    spin_lock(&rq->lock);

    /* 更新时钟 */
    update_rq_clock(rq);

    /* 选择下一个任务 */
    next = pick_next_task(rq, prev);

    /* 清除需要重新调度的标志 */
    clear_tsk_need_resched(prev);

    if (likely(prev != next)) {
        rq->nr_switches++;
        rq->curr = next;

        /* 进行上下文切换 */
        context_switch(rq, prev, next);
    } else {
        /* 没有切换，只需要释放锁 */
        spin_unlock(&rq->lock);
    }

    local_irq_restore(flags);
}

/* 选择下一个任务 */
struct task_struct *pick_next_task(struct rq *rq, struct task_struct *prev)
{
    const struct sched_class *class;
    struct task_struct *p;

    /* 如果没有任务在运行，返回空闲任务 */
    if (likely(rq->nr_running == rq->cfs.h_nr_running)) {
        p = pick_next_task_fair(rq, prev);
        if (likely(p))
            return p;
    }

    /* 按优先级顺序检查各个调度类 */
    for_each_class(class) {
        p = class->pick_next_task(rq, prev);
        if (p)
            return p;
    }

    /* 如果没有找到任务，返回空闲任务 */
    return rq->idle;
}

/* 上下文切换 */
static void context_switch(struct rq *rq, struct task_struct *prev,
                          struct task_struct *next)
{
    struct mm_struct *mm, *oldmm;

    prepare_task_switch(rq, prev, next);

    mm = next->mm;
    oldmm = prev->active_mm;

    arch_start_context_switch(prev);

    if (!mm) {
        /* 内核线程 */
        next->active_mm = oldmm;
        atomic_inc(&oldmm->mm_count);
        enter_lazy_tlb(oldmm, next);
    } else {
        /* 用户进程 */
        switch_mm(oldmm, mm, next);
    }

    if (!prev->mm) {
        /* 内核线程 */
        prev->active_mm = NULL;
        rq->prev_mm = oldmm;
    }

    /* 切换寄存器状态和栈 */
    switch_to(prev, next, prev);

    barrier();

    finish_task_switch(prev);
}

/* 准备任务切换 */
static void prepare_task_switch(struct rq *rq, struct task_struct *prev,
                               struct task_struct *next)
{
    sched_info_switch(rq, prev, next);
    perf_event_task_sched_out(prev, next);
    fire_sched_out_preempt_notifiers(prev, next);
    prepare_lock_switch(rq, next);
    prepare_arch_switch(next);
}

/* 完成任务切换 */
static void finish_task_switch(struct task_struct *prev)
{
    struct rq *rq = this_rq();
    struct mm_struct *mm = rq->prev_mm;
    long prev_state;

    rq->prev_mm = NULL;

    prev_state = prev->state;

    perf_event_task_sched_in(prev, current);
    finish_arch_switch(prev);
    finish_lock_switch(rq, prev);
    fire_sched_in_preempt_notifiers(current);

    if (mm)
        mmdrop(mm);

    if (unlikely(prev_state == TASK_DEAD)) {
        /*
         * 移除退出的任务
         */
        put_task_struct(prev);
    }
}

/* 让出CPU */
void yield(void)
{
    set_current_state(TASK_RUNNING);
    sys_sched_yield();
}

/* 系统调用：让出CPU */
long sys_sched_yield(void)
{
    struct rq *rq = this_rq_lock();

    schedstat_inc(rq, yld_count);
    current->sched_class->yield_task(rq);

    preempt_disable();
    spin_unlock(&rq->lock);
    schedule();
    preempt_enable();

    return 0;
}

/* 唤醒进程 */
void wake_up_process(struct task_struct *p)
{
    ulong flags;

    raw_spin_lock_irqsave(&p->pi_lock, flags);
    if (!(p->state & TASK_NORMAL))
        goto out;

    if (!ttwu_remote(p, 0))
        ttwu_queue(p, smp_processor_id());

out:
    raw_spin_unlock_irqrestore(&p->pi_lock, flags);
}

/* 检查是否需要抢占当前任务 */
void check_preempt_curr(struct rq *rq, struct task_struct *p, int flags)
{
    const struct sched_class *class;

    if (p->sched_class == rq->curr->sched_class) {
        r
