#ifndef __TYPES_H__
#define __TYPES_H__

/* 基本类型定义 */
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;

typedef signed char         s8;
typedef signed short        s16;
typedef signed int          s32;
typedef signed long long    s64;

typedef unsigned long       ulong;
typedef unsigned int        uint;
typedef unsigned short      ushort;
typedef unsigned char       uchar;

/* 指针类型 */
typedef void*               ptr_t;
typedef u64                 phys_addr_t;
typedef u64                 virt_addr_t;

/* 进程ID类型 */
typedef s32                 pid_t;
typedef u32                 uid_t;
typedef u32                 gid_t;

/* 文件系统相关类型 */
typedef u64                 ino_t;
typedef s64                 off_t;
typedef u64                 dev_t;
typedef u32                 mode_t;
typedef u32                 nlink_t;
typedef u64                 size_t;
typedef s64                 ssize_t;
typedef s64                 time_t;

/* 布尔类型 */
typedef enum {
    false = 0,
    true = 1
} bool;

/* 空指针定义 */
#ifndef NULL
#define NULL ((void*)0)
#endif

/* 状态码 */
typedef enum {
    OK = 0,
    ERROR = -1,
    ENOMEM = -2,
    EINVAL = -3,
    ENOENT = -4,
    EACCES = -5,
    EEXIST = -6,
    EBUSY = -7,
    EAGAIN = -8,
    EINTR = -9,
    EIO = -10,
    EFAULT = -11,
    EPERM = -12,
    ESRCH = -13,
    ECHILD = -14,
    EDEADLK = -15,
    ENOMSG = -16,
    EIDRM = -17,
    ENOSPC = -18,
    ENODEV = -19,
    ENOTDIR = -20,
    EISDIR = -21,
    EMFILE = -22,
    ENFILE = -23,
    ENOTTY = -24,
    ETXTBSY = -25,
    EFBIG = -26,
    ENOSYS = -27,
    ENOTEMPTY = -28,
    ENAMETOOLONG = -29,
    ELOOP = -30,
    ENOTSOCK = -31,
    EADDRINUSE = -32,
    ECONNREFUSED = -33,
    ENETUNREACH = -34,
    ETIMEDOUT = -35
} error_t;

/* 内存页面大小 */
#define PAGE_SIZE           4096
#define PAGE_SHIFT          12
#define PAGE_MASK           (~(PAGE_SIZE - 1))

/* 字节对齐宏 */
#define ALIGN_UP(x, align)      (((x) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(x, align)    ((x) & ~((align) - 1))
#define IS_ALIGNED(x, align)    (((x) & ((align) - 1)) == 0)

/* 通用宏 */
#define ARRAY_SIZE(arr)         (sizeof(arr) / sizeof((arr)[0]))
#define MIN(a, b)              ((a) < (b) ? (a) : (b))
#define MAX(a, b)              ((a) > (b) ? (a) : (b))
#define CLAMP(x, min, max)     (MIN(MAX(x, min), max))

/* 位操作宏 */
#define BIT(n)                  (1UL << (n))
#define SET_BIT(x, n)          ((x) |= BIT(n))
#define CLEAR_BIT(x, n)        ((x) &= ~BIT(n))
#define TEST_BIT(x, n)         (((x) & BIT(n)) != 0)

/* 内存屏障 */
#define mb()    asm volatile("mfence":::"memory")
#define rmb()   asm volatile("lfence":::"memory")
#define wmb()   asm volatile("sfence":::"memory")

/* 编译器属性 */
#define __packed               __attribute__((packed))
#define __aligned(x)           __attribute__((aligned(x)))
#define __section(x)           __attribute__((section(x)))
#define __noreturn             __attribute__((noreturn))
#define __weak                 __attribute__((weak))
#define __init                 __attribute__((section(".init.text")))
#define __initdata             __attribute__((section(".init.data")))

/* 容器获取宏 */
#define container_of(ptr, type, member) ({                      \
    const typeof(((type *)0)->member) *__mptr = (ptr);        \
    (type *)((char *)__mptr - offsetof(type, member));        \
})

/* 偏移量计算 */
#define offsetof(type, member)  ((size_t)&((type *)0)->member)

/* 内核内存地址转换 */
#define __pa(x)                 ((phys_addr_t)(x) - KERNEL_VIRTUAL_BASE)
#define __va(x)                 ((void *)((phys_addr_t)(x) + KERNEL_VIRTUAL_BASE))

/* 虚拟地址空间布局 */
#define KERNEL_VIRTUAL_BASE     0xFFFF800000000000UL
#define USER_VIRTUAL_BASE       0x0000000000000000UL
#define USER_VIRTUAL_END        0x0000800000000000UL

/* 文件权限位 */
#define S_IFMT      0170000
#define S_IFSOCK    0140000
#define S_IFLNK     0120000
#define S_IFREG     0100000
#define S_IFBLK     0060000
#define S_IFDIR     0040000
#define S_IFCHR     0020000
#define S_IFIFO     0010000

#define S_ISUID     0004000
#define S_ISGID     0002000
#define S_ISVTX     0001000

#define S_IRWXU     0000700
#define S_IRUSR     0000400
#define S_IWUSR     0000200
#define S_IXUSR     0000100

#define S_IRWXG     0000070
#define S_IRGRP     0000040
#define S_IWGRP     0000020
#define S_IXGRP     0000010

#define S_IRWXO     0000007
#define S_IROTH     0000004
#define S_IWOTH     0000002
#define S_IXOTH     0000001

/* 文件类型检查宏 */
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)

#endif /* __TYPES_H__ */