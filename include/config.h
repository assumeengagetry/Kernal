#define CONFIG_H



#define KERNAL_NAME     "MicroKernal"



#define CONFIG_x86_64    1
#define CONFIG_64BIT    1
#define CONFIG_SMP    0
#define CINFIG_PREEMPT    1


#define CONFIG_MMU    1
#define CONFIG_HIGHMEM    1
#define CONFIG_SWAP    0
#define CONFIG_SLAB    1
#define CONFIG_SLUB    0
#define CONFIG_SLOB    0

#define CONFIG_SCHED_DEBUG  1
#define CONFIG_FAIR_GROUP_SCHED 0
#define CONFIG_RT_GROUP_SCHED  0
#define CONFIG_CGROUP_SCHED  0


#define CONFIG_VFS    1
#define CONFIG_EXT2_FS    0
#define CONFIG_EXT3_FS    0
#define CONFIG_EXT4_FS    0
#define CONFIG_PROC_FS    1
#define CONFIG_SYSFS    1
#define CONFIG_TMPFS    1
#define  CONFIG_DEVTMPFS    1



#define  CONFIG_NET    1
#define CONFIG_INET    1
#define CONFIG_IPV6    0
#define CONFIG_UNIX    1
#define CONFIG_PACKET    1
#define CONFIG_NETLINK    1


#define CONFIG_BLOCK    1
#define CONFIG_CHAR_DEVICE  1
#define CONFIG_SERTAL_8250  1
#define CONFIG_KEYBOARD   1
#define CONFIG_MOUSE    1
#define CONFIG_VGA_CONSOLE  1


#define CONFIG_DEBUG_KERNAL  1
#define CONFIG_DEBUG_SLAB  1
#define CONFIG_DEBUG_SPINLOCK  1
#define CONFIG_DEBUG_MUTEXES  1
#define CONFIG_DEBUG_PAGEALLOC  1
#define CONFIG_DEBUG_INFO  1
#define CONFIG_FRAME_POINTER  1
#define CONFIG_STACK_TRACER  1



#define CONFIG_SECURITY    0
#define CONFIG_SECURITY_SELINUX  0
#define CONFIG_SECURITY_APPARMOR 0
#define CONFIG_SECURITY_SMACK  0


#define CONFIG_PM    0
#define CONFIG_ACPI    0
#define CONFIG_APM    0


#define CONFIG_VIRTUALIZATION  0
#define CONFIG_KVM    0
#define CONFIG_XEN    0


#define CONFIG_NAMESPACES  0
#define CONFIG_CGROUPS    0
#define CONFIG_CHECKPOINT_RESTORE 0



#define CONFIG_PROFILING  1
#define CONFIG_PERF_EVENTS  1
#define CONFIG_TRACING    1
#define CONFIG_FTRACE    1


#define CONFIG_CRYPTO    0
#define CONFIG_CRYPTO_AES  0
#define CONFIG_CRYPTO_SHA1  0
#define CONFIG_CRYPTO_SHA256  0

#define CONFIG_ZLIB_INFLATE  0
#define CONFIG_ZLIB_DEFLATE  0
#define CONFIG_LZO_COMPRESS  0
#define CONFIG_LZO_DECOMPRESS  0

#define CONFIG_BASE_FALL  1
#define CONFIG_EMBEDDED    0
#define CONFIG_EXPERIMENTAL  1
#define CONFIG_BROKEN    0



#define CONFIG_X86_MCE    0
#define CONFIG_X86_THERMAL  0
#define CONFIG_MICORCODE  1
#define CONFIG_X86_MSR    1
#define CONFIG_X86_CPUID  1



#define CONFIG_TICK_ONESHOT  1
#define CONFIG_NO_HZ    0
#define CONFIG_HIGH_RES_TIMERS  1
#define CONFIG_GENERIC_CLOCKEVENTS  1




#define CONFIG_GENERIC_HARDIRQS  1
#define CONFIG_SPACE_IRQ  0
#define CONFIG_NUMA_IRQ_DESC  0





#define CONFIG_NUMA    0
#define CONFIG_NODES_SHIFT  0
#define CONFIG_HOTPLUG_CPU  0
#define CONFIG_MEMORY_HOTPLUG  0
#define CONFIG_MEMORY_HOTREMOVE  0



  
#define CONFIG_MODULES    1
#define CONFIG_MODULE_UBLOAD  1
#define CONFIG_MODULE_FORCE_UNLOAD  0
#define CONFIG_MODVERSIONS  0
#define CONFIG_MODULE_SRCVERSION_ALL  0


#define NR_CPUS      1
#define THREAD_SIZE    16384
#define PAGE_SIZE    4096
#define PAGE_SHIFT    12
#define MAX_ORDER    11
#define FORK_PREEMPT_COUNT  2
#define HZ      10000
#define USER_HZ      100
#define CLOCKS_PER_SEC    1000000



#define KERNEL_VIRTUAL_BASE  0xFFFF800000000000UL
#define USER_VIRTUAL_BASE  0x0000000000000000UL
#define USER_VIRTUAL_END  0x0000800000000000UL
#define VMALLOC_START    0xFFFF880000000000UL
#define VMALLOC_END    0xFFFFC80000000000UL
#define MODULES_VADDR    0xFFFFFFFF80000000UL
#define MODULES_END    0xFFFFFFFFC0000000UL



#define KERNAL_STACK_SIZE  16384
#define IRQ_STACK_SIZE    16384


#define MAX_NICE    19
#define MIN_NICE    -20
#define NICE_WIDTH    40
#define DEFAULT_PRIO    120
#define MAX_USER_RT_PRIO  100
#define MAX_RT_PRIO    100
#define MAX_PRIO    140
#define NICE_0_LOAD    1024
#define NICE_0_SHIFT    10


#define MAX_NUMNODES    1
#define MAX_NR_ZONES    4
#define NR_LRU_LISTS    5
#define MIGRATE_TYPES    6
#define MIGRATE_PCPTYPES  3
#define PID_MAX      32768
#define BITS_PER_LONG    64
#define BITS_PER_BYTE    8


#define NAME_MAX    255
#define PATH_MAX    4096
#define PIPE_BUF    4096
#define MAX_LINKS    32



#define NSIG      64  
#define SIGRTMIN    32
#define SIGRTMAX    63

#define IFNAMSIZ    16
#define MAX_ADDR_LEN    32


#define stringify(x)    #x
#define stringify    __stringify(x)


#define IS_ENABLED(option)  (option)
#define IS_BUILTIN(option)  (option)
#define IS_MODULE(option)  0



#define  likely(x)    __buildtin_expected(!!(x), 1)
#define  unlikely(x)    __buildtin_expected(!!(x), 0)



#define __init      __attribute((section(".init.text")))
#define initdata    __attribute((section(".init.data")))
#define exit      __attribute((section(".exit.text")))
#define exitdata    __attribute((section(".exit.data")))
#define devinit    __init
#define  __devinitdata    __initdata
#define __devexit    __exit
#define __devexitdata    __exitdata


#if CONFIG_DEBUG_KERNAL
#define DEBUG 1
#else
#define DEBUG 0
#endif

#if CONFIG_DEUG_KERNAL
#define KERN_DEBUG    "<7>"
#define pr_debug(fmt, ...)  printk(KERB_DEBUG fmt, ##__VA_ARGS)
#else
#define pr_debug(fmt, ...)   do { } while (0)
#endif


#define KERN_EMERG    "<0>"
#define KERN_ALERT    "<1>"
#define KERN_CRIT    "<2>"
#define KERN_ERR    "<3>"
#define KERN_WARNING    "<4>"
#define KERN_NOTICE    "<5>"
#define KERN_INFO    "<6>"
#define KERN_DEBUG    "<7>"



#define GFP_KERNAL    0x01
#define GFP_ATOMIC    0x02
#define  GFP_USER    0x04
#define GFP_NOWAIT    0x08
#define GFP_NOIO    0x10
#define GFP_NOFS    0x20
#define GFP_ZERO    0x40



#define MODULE_LICENSE(x)  static const char __module_license[] = x
#define MODULE_AUTHOR(x)  static const char __module_author[] = x
#define MODULE_DESCRIPTION(x)  static const char __module_description[] = x
#define MODULE_VERSION(x)  static const char __module_version[] = x
#define MODULE_ALIAS(x)    static const char __module_alias[] =x