/*
 * main.c - 单核简化版内核主入口
 */

#include "../../include/types.h"
#include "../../include/sched.h"
#include "../../include/mm.h"

#define KERNEL_VERSION "0.1.0"
#define KERNEL_NAME "MicroKernel"

static struct task_struct *init_task = NULL;

/* 系统调用表类型 */
typedef long (*syscall_fn_t)(unsigned long, unsigned long, unsigned long,
                             unsigned long, unsigned long, unsigned long);

/* 系统调用号定义（只保留最基本的） */
#define __NR_getpid     0
#define __NR_sched_yield 1
#define __NR_exit       2
#define __NR_fork       3
#define NR_syscalls     4

/* 系统调用函数声明 */
extern long sys_getpid(void);
extern long sys_sched_yield(void);
extern long sys_exit(int error_code);
extern long sys_fork(void);

/* 系统调用表 */
static const syscall_fn_t sys_call_table[NR_syscalls] = {
    [__NR_getpid]      = (syscall_fn_t)sys_getpid,
    [__NR_sched_yield] = (syscall_fn_t)sys_sched_yield,
    [__NR_exit]        = (syscall_fn_t)sys_exit,
    [__NR_fork]        = (syscall_fn_t)sys_fork,
};

/* printk - 简化日志输出 */
int printk(const char *fmt, ...) { return 0; }

/* panic - 内核恐慌处理 */
void panic(const char *fmt, ...) {
    for (;;) { asm volatile("hlt"); }
}

/* kernel_init - 只初始化调度器和创建init进程 */
static void kernel_init(void)
{
    sched_init();
    init_task = alloc_task_struct();
    if (!init_task) panic("Cannot allocate init task");
    set_current(init_task);
    wake_up_new_task(init_task);
}

/* kernel_main - 内核主函数 */
void kernel_main(void)
{
    kernel_init();
    schedule();
    panic("Scheduler returned!");
}

/* 系统调用分发 */
long do_syscall(unsigned long syscall_nr, unsigned long arg0,
                unsigned long arg1, unsigned long arg2,
                unsigned long arg3, unsigned long arg4,
                unsigned long arg5)
{
    if (syscall_nr >= NR_syscalls) return -1;
    syscall_fn_t fn = sys_call_table[syscall_nr];
    if (!fn) return -1;
    return fn(arg0, arg1, arg2, arg3, arg4, arg5);
}

/* 基本系统调用实现 */
long sys_getpid(void) { return current->pid; }
long sys_sched_yield(void) { yield(); return 0; }
long sys_exit(int error_code) { for (;;) schedule(); }
long sys_fork(void) { return -1; } // 不实现fork

/* start_kernel - 启动入口 */
void start_kernel(void) { kernel_main(); }