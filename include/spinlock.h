/*
 * spinlock.h - 自旋锁和读写锁实现
 * 提供内核同步原语，支持多CPU环境下的锁保护。
 */

#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

#include "types.h"

/* 自旋锁结构体定义 */
typedef struct {
    volatile u32 lock;              /* 锁状态 */
    volatile u32 owner_cpu;         /* 拥有者CPU */
    volatile void *owner;           /* 拥有者指针 */
    const char *name;               /* 锁名称 */
    u32 magic;                      /* 魔数 */
} spinlock_t;

/* 读写锁结构体定义 */
typedef struct {
    volatile u32 lock;              /* 锁状态 */
    volatile u32 reader_count;      /* 读者计数 */
    volatile u32 writer_waiting;    /* 写者等待标志 */
    const char *name;               /* 锁名称 */
    u32 magic;                      /* 魔数 */
} rwlock_t;

/* 自旋锁和读写锁魔数 */
#define SPINLOCK_MAGIC      0xDEADBEEF
#define RWLOCK_MAGIC        0xFACEFEED

/* 自旋锁状态宏 */
#define SPINLOCK_UNLOCKED   0
#define SPINLOCK_LOCKED     1

/* 读写锁状态位宏 */
#define RWLOCK_BIAS         0x01000000
#define RWLOCK_WRITE_BIAS   0x01000000

/* 自旋锁初始化宏 */
#define SPINLOCK_INIT(name) { \
    .lock = SPINLOCK_UNLOCKED, \
    .owner_cpu = 0xFFFFFFFF, \
    .owner = NULL, \
    .name = #name, \
    .magic = SPINLOCK_MAGIC \
}

#define DEFINE_SPINLOCK(name) spinlock_t name = SPINLOCK_INIT(name)

/* 读写锁初始化宏 */
#define RWLOCK_INIT(name) { \
    .lock = 0, \
    .reader_count = 0, \
    .writer_waiting = 0, \
    .name = #name, \
    .magic = RWLOCK_MAGIC \
}

#define DEFINE_RWLOCK(name) rwlock_t name = RWLOCK_INIT(name)

/* 自旋锁操作函数 */
static inline void spin_lock_init(spinlock_t *lock)
{
    lock->lock = SPINLOCK_UNLOCKED;
    lock->owner_cpu = 0xFFFFFFFF;
    lock->owner = NULL;
    lock->name = "unknown";
    lock->magic = SPINLOCK_MAGIC;
}

/* 获取自旋锁 */
static inline void spin_lock(spinlock_t *lock)
{
    u32 cpu = smp_processor_id();
    
    /* 检查魔数 */
    if (lock->magic != SPINLOCK_MAGIC) {
        panic("Bad spinlock magic: %p\n", lock);
    }
    
    /* 检查递归锁定 */
    if (lock->owner_cpu == cpu) {
        panic("Recursive spinlock: %s on CPU %d\n", lock->name, cpu);
    }
    
    /* 自旋等待锁 */
    while (1) {
        /* 尝试获取锁 */
        if (__sync_bool_compare_and_swap(&lock->lock, SPINLOCK_UNLOCKED, SPINLOCK_LOCKED)) {
            /* 成功获取锁 */
            lock->owner_cpu = cpu;
            lock->owner = __builtin_return_address(0);
            break;
        }
        
        /* 自旋等待 */
        while (lock->lock == SPINLOCK_LOCKED) {
            cpu_relax();
        }
    }
}

/* 尝试获取自旋锁 */
static inline int spin_trylock(spinlock_t *lock)
{
    u32 cpu = smp_processor_id();
    
    /* 检查魔数 */
    if (lock->magic != SPINLOCK_MAGIC) {
        panic("Bad spinlock magic: %p\n", lock);
    }
    
    /* 检查递归锁定 */
    if (lock->owner_cpu == cpu) {
        panic("Recursive spinlock: %s on CPU %d\n", lock->name, cpu);
    }
    
    /* 尝试获取锁 */
    if (__sync_bool_compare_and_swap(&lock->lock, SPINLOCK_UNLOCKED, SPINLOCK_LOCKED)) {
        lock->owner_cpu = cpu;
        lock->owner = __builtin_return_address(0);
        return 1;
    }
    
    return 0;
}

/* 释放自旋锁 */
static inline void spin_unlock(spinlock_t *lock)
{
    u32 cpu = smp_processor_id();
    
    /* 检查魔数 */
    if (lock->magic != SPINLOCK_MAGIC) {
        panic("Bad spinlock magic: %p\n", lock);
    }
    
    /* 检查锁的所有者 */
    if (lock->owner_cpu != cpu) {
        panic("Spinlock not owned by current CPU: %s, owner=%d, current=%d\n",
              lock->name, lock->owner_cpu, cpu);
    }
    
    /* 清除所有者信息 */
    lock->owner_cpu = 0xFFFFFFFF;
    lock->owner = NULL;
    
    /* 释放锁 */
    wmb();  /* 写内存屏障 */
    lock->lock = SPINLOCK_UNLOCKED;
}

/* 检查自旋锁是否被锁定 */
static inline int spin_is_locked(spinlock_t *lock)
{
    return lock->lock == SPINLOCK_LOCKED;
}

/* 断言自旋锁已被锁定 */
static inline void spin_assert_locked(spinlock_t *lock)
{
    if (!spin_is_locked(lock)) {
        panic("Spinlock assertion failed: %s should be locked\n", lock->name);
    }
}

/* 禁用中断并获取自旋锁 */
static inline void spin_lock_irqsave(spinlock_t *lock, unsigned long *flags)
{
    *flags = local_irq_save();
    spin_lock(lock);
}

/* 释放自旋锁并恢复中断 */
static inline void spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
    spin_unlock(lock);
    local_irq_restore(flags);
}

/* 禁用中断并获取自旋锁 */
static inline void spin_lock_irq(spinlock_t *lock)
{
    local_irq_disable();
    spin_lock(lock);
}

/* 释放自旋锁并启用中断 */
static inline void spin_unlock_irq(spinlock_t *lock)
{
    spin_unlock(lock);
    local_irq_enable();
}

/* 禁用底半部并获取自旋锁 */
static inline void spin_lock_bh(spinlock_t *lock)
{
    local_bh_disable();
    spin_lock(lock);
}

/* 释放自旋锁并启用底半部 */
static inline void spin_unlock_bh(spinlock_t *lock)
{
    spin_unlock(lock);
    local_bh_enable();
}

/* 读写锁操作函数 */
static inline void rwlock_init(rwlock_t *lock)
{
    lock->lock = 0;
    lock->reader_count = 0;
    lock->writer_waiting = 0;
    lock->name = "unknown";
    lock->magic = RWLOCK_MAGIC;
}

/* 获取读锁 */
static inline void read_lock(rwlock_t *lock)
{
    /* 检查魔数 */
    if (lock->magic != RWLOCK_MAGIC) {
        panic("Bad rwlock magic: %p\n", lock);
    }
    
    while (1) {
        /* 等待写者完成 */
        while (lock->writer_waiting || (lock->lock & RWLOCK_WRITE_BIAS)) {
            cpu_relax();
        }
        
        /* 尝试增加读者计数 */
        if (__sync_add_and_fetch(&lock->reader_count, 1)) {
            /* 再次检查是否有写者 */
            if (lock->writer_waiting || (lock->lock & RWLOCK_WRITE_BIAS)) {
                /* 有写者，撤销读者计数 */
                __sync_sub_and_fetch(&lock->reader_count, 1);
                continue;
            }
            /* 成功获取读锁 */
            break;
        }
    }
}

/* 释放读锁 */
static inline void read_unlock(rwlock_t *lock)
{
    /* 检查魔数 */
    if (lock->magic != RWLOCK_MAGIC) {
        panic("Bad rwlock magic: %p\n", lock);
    }
    
    /* 减少读者计数 */
    __sync_sub_and_fetch(&lock->reader_count, 1);
}

/* 获取写锁 */
static inline void write_lock(rwlock_t *lock)
{
    /* 检查魔数 */
    if (lock->magic != RWLOCK_MAGIC) {
        panic("Bad rwlock magic: %p\n", lock);
    }
    
    /* 标记有写者等待 */
    __sync_add_and_fetch(&lock->writer_waiting, 1);
    
    while (1) {
        /* 尝试获取写锁 */
        if (__sync_bool_compare_and_swap(&lock->lock, 0, RWLOCK_WRITE_BIAS)) {
            /* 等待所有读者完成 */
            while (lock->reader_count > 0) {
                cpu_relax();
            }
            /* 成功获取写锁 */
            break;
        }
        
        cpu_relax();
    }
    
    /* 清除写者等待标志 */
    __sync_sub_and_fetch(&lock->writer_waiting, 1);
}

/* 释放写锁 */
static inline void write_unlock(rwlock_t *lock)
{
    /* 检查魔数 */
    if (lock->magic != RWLOCK_MAGIC) {
        panic("Bad rwlock magic: %p\n", lock);
    }
    
    /* 释放写锁 */
    wmb();  /* 写内存屏障 */
    lock->lock = 0;
}

/* 尝试获取读锁 */
static inline int read_trylock(rwlock_t *lock)
{
    /* 检查魔数 */
    if (lock->magic != RWLOCK_MAGIC) {
        panic("Bad rwlock magic: %p\n", lock);
    }
    
    /* 如果有写者等待，直接返回失败 */
    if (lock->writer_waiting || (lock->lock & RWLOCK_WRITE_BIAS)) {
        return 0;
    }
    
    /* 尝试增加读者计数 */
    if (__sync_add_and_fetch(&lock->reader_count, 1)) {
        /* 再次检查是否有写者 */
        if (lock->writer_waiting || (lock->lock & RWLOCK_WRITE_BIAS)) {
            /* 有写者，撤销读者计数 */
            __sync_sub_and_fetch(&lock->reader_count, 1);
            return 0;
        }
        /* 成功获取读锁 */
        return 1;
    }
    
    return 0;
}

/* 尝试获取写锁 */
static inline int write_trylock(rwlock_t *lock)
{
    /* 检查魔数 */
    if (lock->magic != RWLOCK_MAGIC) {
        panic("Bad rwlock magic: %p\n", lock);
    }
    
    /* 尝试获取写锁 */
    if (__sync_bool_compare_and_swap(&lock->lock, 0, RWLOCK_WRITE_BIAS)) {
        /* 检查是否有读者 */
        if (lock->reader_count > 0) {
            /* 有读者，释放写锁 */
            lock->lock = 0;
            return 0;
        }
        /* 成功获取写锁 */
        return 1;
    }
    
    return 0;
}

/* 禁用中断并获取读锁 */
static inline void read_lock_irqsave(rwlock_t *lock, unsigned long *flags)
{
    *flags = local_irq_save();
    read_lock(lock);
}

/* 释放读锁并恢复中断 */
static inline void read_unlock_irqrestore(rwlock_t *lock, unsigned long flags)
{
    read_unlock(lock);
    local_irq_restore(flags);
}

/* 禁用中断并获取写锁 */
static inline void write_lock_irqsave(rwlock_t *lock, unsigned long *flags)
{
    *flags = local_irq_save();
    write_lock(lock);
}

/* 释放写锁并恢复中断 */
static inline void write_unlock_irqrestore(rwlock_t *lock, unsigned long flags)
{
    write_unlock(lock);
    local_irq_restore(flags);
}

/* 禁用中断并获取读锁 */
static inline void read_lock_irq(rwlock_t *lock)
{
    local_irq_disable();
    read_lock(lock);
}

/* 释放读锁并启用中断 */
static inline void read_unlock_irq(rwlock_t *lock)
{
    read_unlock(lock);
    local_irq_enable();
}

/* 禁用中断并获取写锁 */
static inline void write_lock_irq(rwlock_t *lock)
{
    local_irq_disable();
    write_lock(lock);
}

/* 释放写锁并启用中断 */
static inline void write_unlock_irq(rwlock_t *lock)
{
    write_unlock(lock);
    local_irq_enable();
}

/* 禁用底半部并获取读锁 */
static inline void read_lock_bh(rwlock_t *lock)
{
    local_bh_disable();
    read_lock(lock);
}

/* 释放读锁并启用底半部 */
static inline void read_unlock_bh(rwlock_t *lock)
{
    read_unlock(lock);
    local_bh_enable();
}

/* 禁用底半部并获取写锁 */
static inline void write_lock_bh(rwlock_t *lock)
{
    local_bh_disable();
    write_lock(lock);
}

/* 释放写锁并启用底半部 */
static inline void write_unlock_bh(rwlock_t *lock)
{
    write_unlock(lock);
    local_bh_enable();
}

/* CPU相关函数声明 */
extern u32 smp_processor_id(void);
extern void cpu_relax(void);

/* 中断控制函数声明 */
extern unsigned long local_irq_save(void);
extern void local_irq_restore(unsigned long flags);
extern void local_irq_disable(void);
extern void local_irq_enable(void);

/* 底半部控制函数声明 */
extern void local_bh_disable(void);
extern void local_bh_enable(void);

/* 调试函数声明 */
extern void panic(const char *fmt, ...) __noreturn;

/* 锁排序规则枚举 */
enum {
    LOCK_CLASS_MM = 0,
    LOCK_CLASS_SIGNAL,
    LOCK_CLASS_FS,
    LOCK_CLASS_SCHED,
    LOCK_CLASS_NET,
    LOCK_CLASS_IRQ,
    LOCK_CLASS_MAX
};

/* 锁依赖检查和统计宏 */
#ifdef CONFIG_LOCKDEP
extern void lock_acquire(struct lockdep_map *lock, unsigned int subclass,
                        int trylock, int read, int check,
                        struct lockdep_map *nest_lock, unsigned long ip);
extern void lock_release(struct lockdep_map *lock, int nested,
                        unsigned long ip);
#else
#define lock_acquire(lock, subclass, trylock, read, check, nest_lock, ip) do { } while (0)
#define lock_release(lock, nested, ip) do { } while (0)
#endif

#ifdef CONFIG_LOCK_STAT
extern void lock_contended(struct lockdep_map *lock, unsigned long ip);
extern void lock_acquired(struct lockdep_map *lock, unsigned long ip);
#else
#define lock_contended(lock, ip) do { } while (0)
#define lock_acquired(lock, ip) do { } while (0)
#endif

#endif /* __SPINLOCK_H__ */