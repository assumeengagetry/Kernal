#include "../../include/types.h"
#include "../../include/sched.h"
#include "../../include/mm.h"
#include "../../include/list.h"
#include "../../include/spinlock.h"

#define KERNEL_VERSION "0.1.0"
#define KERNEL_NAME "MicroKernel"

static bool kernel_initialized = false;
static struct task_struct *init_task = NULL;
static struct mm_struct *init_mm = NULL;

typedef long (*syscall_fn_t)(unsigned long, unsigned long, unsigned long,
                             unsigned long, unsigned long, unsigned long);

#define __NR_read       0
#define __NR_write      1
#define __NR_open       2
#define __NR_close      3
#define __NR_stat       4
#define __NR_fstat      5
#define __NR_lstat      6
#define __NR_poll       7
#define __NR_lseek      8
#define __NR_mmap       9
#define __NR_mprotect   10
#define __NR_munmap     11
#define __NR_brk        12
#define __NR_rt_sigaction    13
#define __NR_rt_sigprocmask  14
#define __NR_rt_sigreturn    15
#define __NR_ioctl      16
#define __NR_pread64    17
#define __NR_pwrite64   18
#define __NR_readv      19
#define __NR_writev     20
#define __NR_access     21
#define __NR_pipe       22
#define __NR_select     23
#define __NR_sched_yield     24
#define __NR_mremap     25
#define __NR_msync      26
#define __NR_mincore    27
#define __NR_madvise    28
#define __NR_shmget     29
#define __NR_shmat      30
#define __NR_shmctl     31
#define __NR_dup        32
#define __NR_dup2       33
#define __NR_pause      34
#define __NR_nanosleep  35
#define __NR_getitimer  36
#define __NR_alarm      37
#define __NR_setitimer  38
#define __NR_getpid     39
#define __NR_sendfile   40
#define __NR_socket     41
#define __NR_connect    42
#define __NR_accept     43
#define __NR_sendto     44
#define __NR_recvfrom   45
#define __NR_sendmsg    46
#define __NR_recvmsg    47
#define __NR_shutdown   48
#define __NR_bind       49
#define __NR_listen     50
#define __NR_getsockname     51
#define __NR_getpeername     52
#define __NR_socketpair 53
#define __NR_setsockopt 54
#define __NR_getsockopt 55
#define __NR_clone      56
#define __NR_fork       57
#define __NR_vfork      58
#define __NR_execve     59
#define __NR_exit       60
#define __NR_wait4      61
#define __NR_kill       62
#define __NR_uname      63
#define __NR_semget     64
#define __NR_semop      65
#define __NR_semctl     66
#define __NR_shmdt      67
#define __NR_msgget     68
#define __NR_msgsnd     69
#define __NR_msgrcv     70
#define __NR_msgctl     71
#define __NR_fcntl      72
#define __NR_flock      73
#define __NR_fsync      74
#define __NR_fdatasync  75
#define __NR_truncate   76
#define __NR_ftruncate  77
#define __NR_getdents   78
#define __NR_getcwd     79
#define __NR_chdir      80
#define __NR_fchdir     81
#define __NR_rename     82
#define __NR_mkdir      83
#define __NR_rmdir      84
#define __NR_creat      85
#define __NR_link       86
#define __NR_unlink     87
#define __NR_symlink    88
#define __NR_readlink   89
#define __NR_chmod      90
#define __NR_fchmod     91
#define __NR_chown      92
#define __NR_fchown     93
#define __NR_lchown     94
#define __NR_umask      95
#define __NR_gettimeofday    96
#define __NR_getrlimit  97
#define __NR_getrusage  98
#define __NR_sysinfo    99
#define __NR_times      100

#define NR_syscalls     101

extern long sys_read(unsigned int fd, char __user *buf, size_t count);
extern long sys_write(unsigned int fd, const char __user *buf, size_t count);
extern long sys_open(const char __user *filename, int flags, umode_t mode);
extern long sys_close(unsigned int fd);
extern long sys_getpid(void);
extern long sys_clone(unsigned long clone_flags, unsigned long newsp,
                     int __user *parent_tidptr, int __user *child_tidptr);
extern long sys_fork(void);
extern long sys_vfork(void);
extern long sys_execve(const char __user *filename,
                      const char __user *const __user *argv,
                      const char __user *const __user *envp);
extern long sys_exit(int error_code);
extern long sys_wait4(pid_t upid, int __user *stat_addr, int options,
                     struct rusage __user *ru);
extern long sys_kill(pid_t pid, int sig);
extern long sys_sched_yield(void);
extern long sys_brk(unsigned long brk);
extern long sys_mmap(unsigned long addr, unsigned long len,
                    unsigned long prot, unsigned long flags,
                    unsigned long fd, unsigned long off);
extern long sys_munmap(unsigned long addr, size_t len);
extern long sys_sysinfo(struct sysinfo __user *info);
extern long sys_uname(struct utsname __user *name);

/* 系统调用表 */
static const syscall_fn_t sys_call_table[NR_syscalls] = {
    [__NR_read]         = (syscall_fn_t)sys_read,
    [__NR_write]        = (syscall_fn_t)sys_write,
    [__NR_open]         = (syscall_fn_t)sys_open,
    [__NR_close]        = (syscall_fn_t)sys_close,
    [__NR_getpid]       = (syscall_fn_t)sys_getpid,
    [__NR_clone]        = (syscall_fn_t)sys_clone,
    [__NR_fork]         = (syscall_fn_t)sys_fork,
    [__NR_vfork]        = (syscall_fn_t)sys_vfork,
    [__NR_execve]       = (syscall_fn_t)sys_execve,
    [__NR_exit]         = (syscall_fn_t)sys_exit,
    [__NR_wait4]        = (syscall_fn_t)sys_wait4,
    [__NR_kill]         = (syscall_fn_t)sys_kill,
    [__NR_sched_yield]  = (syscall_fn_t)sys_sched_yield,
    [__NR_brk]          = (syscall_fn_t)sys_brk,
    [__NR_mmap]         = (syscall_fn_t)sys_mmap,
    [__NR_munmap]       = (syscall_fn_t)sys_munmap,
    [__NR_sysinfo]      = (syscall_fn_t)sys_sysinfo,
    [__NR_uname]        = (syscall_fn_t)sys_uname,
};

int printk(const char *fmt, ...)
{
    va_list args;
    char buffer[1024];
    int len;

    va_start(args, fmt);
    len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    console_write(buffer, len);

    return len;
}

/* 控制台写入函数 */
void console_write(const char *buffer, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        serial_putc(buffer[i]);
    }
}

void serial_putc(char c)
{
    #define SERIAL_PORT 0x3F8

    while (!(inb(SERIAL_PORT + 5) & 0x20)) {
    }

    outb(SERIAL_PORT, c);
}

static inline unsigned char inb(unsigned short port)
{
    unsigned char result;
    asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outb(unsigned short port, unsigned char value)
{
    asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

void panic(const char *fmt, ...)
{
    va_list args;
    char buffer[1024];

    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    printk("KERNEL PANIC: %s\n", buffer);

    local_irq_disable();

    for (;;) {
        halt();
    }
}

static inline void halt(void)
{
    asm volatile("hlt");
}

static struct task_struct *create_init_process(void)
{
    struct task_struct *task;
    struct mm_struct *mm;

    task = alloc_task_struct();
    if (!task) {
        panic("Cannot allocate init task");
    }

    mm = mm_alloc();
    if (!mm) {
        free_task_struct(task);
        panic("Cannot allocate init mm");
    }

    task->pid = 1;
    task->tgid = 1;
    task->ppid = 0;
    task->pgrp = 1;
    task->session = 1;

    task->uid = 0;
    task->gid = 0;
    task->euid = 0;
    task->egid = 0;
    task->suid = 0;
    task->sgid = 0;
    task->fsuid = 0;
    task->fsgid = 0;

    strcpy(task->comm, "init");

    task->mm = mm;
    task->active_mm = mm;

    task->state = TASK_RUNNING;
    task->prio = DEFAULT_PRIO;
    task->static_prio = DEFAULT_PRIO;
    task->normal_prio = DEFAULT_PRIO;
    task->policy = SCHED_NORMAL;

    task->cpus_allowed = (1UL << NR_CPUS) - 1;
    task->nr_cpus_allowed = NR_CPUS;

    task->real_parent = task;
    task->parent = task;
    task->group_leader = task;

    task->start_time = get_jiffies_64();
    task->real_start_time = task->start_time;

    sched_fork(task);

    return task;
}

static void kernel_init(void)
{
    printk("Initializing %s %s\n", KERNEL_NAME, KERNEL_VERSION);

    mm_init();
    buddy_init();

    sched_init();

    ipc_init();

    vfs_init();

    net_init();

    driver_init();

    init_task = create_init_process();

    set_current(init_task);

    wake_up_new_task(init_task);

    kernel_initialized = true;
}

void kernel_main(void)
{
    kernel_init();

    schedule();

    panic("OOOOOOOO!!!!!! shit");
}

long do_syscall(unsigned long syscall_nr, unsigned long arg0,
                unsigned long arg1, unsigned long arg2,
                unsigned long arg3, unsigned long arg4,
                unsigned long arg5)
{
    syscall_fn_t syscall_fn;
    long result;

    if (syscall_nr >= NR_syscalls) {
        printk("Invalid syscall number: %lu\n", syscall_nr);
        return -ENOSYS;
    }

    syscall_fn = sys_call_table[syscall_nr];
    if (!syscall_fn) {
        printk("Unimplemented syscall: %lu\n", syscall_nr);
        return -ENOSYS;
    }

    result = syscall_fn(arg0, arg1, arg2, arg3, arg4, arg5);

    return result;
}

long sys_getpid(void)
{
    return current->pid;
}

long sys_sched_yield(void)
{
    yield();
    return 0;
}

long sys_exit(int error_code)
{
    do_exit(error_code);
    return 0;
}

long sys_fork(void)
{
    return do_fork(SIGCHLD, 0, 0, NULL, NULL);
}

long sys_vfork(void)
{
    return do_fork(CLONE_VFORK | CLONE_VM | SIGCHLD, 0, 0, NULL, NULL);
}

long sys_clone(unsigned long clone_flags, unsigned long newsp,
               int __user *parent_tidptr, int __user *child_tidptr)
{
    return do_fork(clone_flags, newsp, 0, parent_tidptr, child_tidptr);
}

long sys_wait4(pid_t upid, int __user *stat_addr, int options,
               struct rusage __user *ru)
{
    return do_wait(upid, stat_addr, options, ru);
}

long sys_kill(pid_t pid, int sig)
{
    return do_kill(pid, sig);
}

long sys_brk(unsigned long brk)
{
    return do_brk(brk);
}

long sys_mmap(unsigned long addr, unsigned long len,
              unsigned long prot, unsigned long flags,
              unsigned long fd, unsigned long off)
{
    return do_mmap(addr, len, prot, flags, fd, off);
}

long sys_munmap(unsigned long addr, size_t len)
{
    return do_munmap(addr, len);
}

long sys_sysinfo(struct sysinfo __user *info)
{
    struct sysinfo val;

    si_meminfo(&val);
    si_swapinfo(&val);

    if (copy_to_user(info, &val, sizeof(struct sysinfo)))
        return -EFAULT;

    return 0;
}

long sys_uname(struct utsname __user *name)
{
    struct utsname kernel_info;

    strcpy(kernel_info.sysname, "MicroKernel");
    strcpy(kernel_info.nodename, "localhost");
    strcpy(kernel_info.release, KERNEL_VERSION);
    strcpy(kernel_info.version, "1");
    strcpy(kernel_info.machine, "x86_64");
    strcpy(kernel_info.domainname, "localdomain");

    if (copy_to_user(name, &kernel_info, sizeof(struct utsname)))
        return -EFAULT;

    return 0;
}

long sys_read(unsigned int fd, char __user *buf, size_t count)
{
    return -ENOSYS;
}

long sys_write(unsigned int fd, const char __user *buf, size_t count)
{
    return -ENOSYS;
}

long sys_open(const char __user *filename, int flags, umode_t mode)
{
    return -ENOSYS;
}

long sys_close(unsigned int fd)
{
    return -ENOSYS;
}

long sys_execve(const char __user *filename,
                const char __user *const __user *argv,
                const char __user *const __user *envp)
{
    return -ENOSYS;
}

void handle_interrupt(int irq)
{
    printk(" 一大波中断来袭 %d received\n", irq);

    switch (irq) {
        case 0:
            timer_interrupt();
            break;
        case 1:
            keyboard_interrupt();
            break;
        default:
            printk("Unknown interrupt: %d\n", irq);
            break;
    }
}

void handle_exception(int exception, unsigned long error_code)
{
   /*抛报错逻辑要先写，不然真的调试不出来 */
    printk("Exception %d (error code: 0x%lx)\n", exception, error_code);

    switch (exception) {
        case 0:
            printk("Division by zero\n");
            break;
        case 6:
            printk("Invalid instruction\n");
            break;
        case 13:
            printk("General protection fault\n");
            break;
        case 14:
            handle_page_fault(error_code);
            break;
        default:
            printk("Unknown exception: %d\n", exception);
            break;
    }

    if (exception != 14) {
        panic("Unhandled exception in kernel");
    }
}

void timer_interrupt(void)
{
    update_jiffies();

    scheduler_tick();

    run_timer_softirq();
}

void keyboard_interrupt(void)
{
    unsigned char scancode = inb(0x60);

    handle_keyboard_input(scancode);
}

void handle_page_fault(unsigned long error_code)
{
    unsigned long address;

    asm volatile("movq %%cr2, %0" : "=r" (address));

    printk("Page fault at address 0x%lx, error code: 0x%lx\n",
           address, error_code);

    do_page_fault(address, error_code);
}

u64 get_jiffies_64(void)
{
    static u64 jiffies = 0;
    return jiffies++;
}

void update_jiffies(void)
{
    static u64 jiffies = 0;
    jiffies++;
}

u32 smp_processor_id(void)
{
    return 0;
}

void cpu_relax(void)
{
    asm volatile("pause" ::: "memory");
}

unsigned long local_irq_save(void)
{
    unsigned long flags;
    asm volatile("pushfq; popq %0; cli" : "=r" (flags) :: "memory");
    return flags;
}

void local_irq_restore(unsigned long flags)
{
    asm volatile("pushq %0; popfq" :: "r" (flags) : "memory");
}

void local_irq_disable(void)
{
    asm volatile("cli" ::: "memory");
}

void local_irq_enable(void)
{
    asm volatile("sti" ::: "memory");
}


void local_bh_enable(void)
{
}

void *kmalloc(size_t size, gfp_t flags)
{
    return NULL;
}

void kfree(void *ptr)
{
}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

char *strcpy(char *dest, const char *src)
{
    char *ret = dest;
    while ((*dest++ = *src++))
        ;
    return ret;
}

size_t strlen(const char *s)
{
    size_t len = 0;
    while (*s++)
        len++;
    return len;
}

void *memset(void *s, int c, size_t n)
{
    unsigned char *p = s;
    while (n--)
        *p++ = c;
    return s;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--)
        *d++ = *s++;
    return dest;
}

void start_kernel(void)
{
    /* 这是内核的真正入口点 */
    kernel_main();
}
