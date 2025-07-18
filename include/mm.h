#define __MM_H__

#include "types.h"
#include "list.h"
#include "spinlock.h"

/* 页面大小和位移 */
#define PAGE_SIZE           4096
#define PAGE_SHIFT          12
#define PAGE_MASK           (~(PAGE_SIZE - 1))

/* 页面状态标志 */
#define PG_locked           0   /* 页面已锁定 */
#define PG_error            1   /* 发生I/O错误 */
#define PG_referenced       2   /* 页面被引用 */
#define PG_uptodate         3   /* 页面数据是最新的 */
#define PG_dirty            4   /* 页面是脏的 */
#define PG_lru              5   /* 页面在LRU链表中 */
#define PG_active           6   /* 页面在活跃链表中 */
#define PG_slab             7   /* 页面被slab分配器使用 */
#define PG_owner_priv_1     8   /* 所有者私有标志1 */
#define PG_arch_1           9   /* 架构相关标志1 */
#define PG_reserved         10  /* 保留页面 */
#define PG_private          11  /* 有私有数据 */
#define PG_private_2        12  /* 有私有数据2 */
#define PG_writeback        13  /* 页面正在回写 */
#define PG_head             14  /* 复合页面头 */
#define PG_tail             15  /* 复合页面尾 */
#define PG_compound         16  /* 复合页面 */
#define PG_swapcache        17  /* 在交换缓存中 */
#define PG_mappedtodisk     18  /* 映射到磁盘 */
#define PG_reclaim          19  /* 待回收 */
#define PG_swapbacked       20  /* 由交换支持 */
#define PG_unevictable      21  /* 不可回收 */
#define PG_mlocked          22  /* 内存锁定 */
#define PG_uncached         23  /* 不可缓存 */
#define PG_hwpoison         24  /* 硬件中毒 */
#define PG_young            25  /* 年轻页面 */
#define PG_idle             26  /* 空闲页面 */

/* 页面状态操作宏 */
#define PageLocked(page)        test_bit(PG_locked, &(page)->flags)
#define PageError(page)         test_bit(PG_error, &(page)->flags)
#define PageReferenced(page)    test_bit(PG_referenced, &(page)->flags)
#define PageUptodate(page)      test_bit(PG_uptodate, &(page)->flags)
#define PageDirty(page)         test_bit(PG_dirty, &(page)->flags)
#define PageLRU(page)           test_bit(PG_lru, &(page)->flags)
#define PageActive(page)        test_bit(PG_active, &(page)->flags)
#define PageSlab(page)          test_bit(PG_slab, &(page)->flags)
#define PageReserved(page)      test_bit(PG_reserved, &(page)->flags)
#define PagePrivate(page)       test_bit(PG_private, &(page)->flags)
#define PageWriteback(page)     test_bit(PG_writeback, &(page)->flags)
#define PageHead(page)          test_bit(PG_head, &(page)->flags)
#define PageTail(page)          test_bit(PG_tail, &(page)->flags)
#define PageCompound(page)      test_bit(PG_compound, &(page)->flags)
#define PageSwapCache(page)     test_bit(PG_swapcache, &(page)->flags)
#define PageMappedToDisk(page)  test_bit(PG_mappedtodisk, &(page)->flags)
#define PageReclaim(page)       test_bit(PG_reclaim, &(page)->flags)
#define PageSwapBacked(page)    test_bit(PG_swapbacked, &(page)->flags)
#define PageUnevictable(page)   test_bit(PG_unevictable, &(page)->flags)
#define PageMlocked(page)       test_bit(PG_mlocked, &(page)->flags)
#define PageUncached(page)      test_bit(PG_uncached, &(page)->flags)
#define PageHWPoison(page)      test_bit(PG_hwpoison, &(page)->flags)
#define PageYoung(page)         test_bit(PG_young, &(page)->flags)
#define PageIdle(page)          test_bit(PG_idle, &(page)->flags)

/* 设置页面状态 */
#define SetPageLocked(page)     set_bit(PG_locked, &(page)->flags)
#define SetPageError(page)      set_bit(PG_error, &(page)->flags)
#define SetPageReferenced(page) set_bit(PG_referenced, &(page)->flags)
#define SetPageUptodate(page)   set_bit(PG_uptodate, &(page)->flags)
#define SetPageDirty(page)      set_bit(PG_dirty, &(page)->flags)
#define SetPageLRU(page)        set_bit(PG_lru, &(page)->flags)
#define SetPageActive(page)     set_bit(PG_active, &(page)->flags)
#define SetPageSlab(page)       set_bit(PG_slab, &(page)->flags)
#define SetPageReserved(page)   set_bit(PG_reserved, &(page)->flags)
#define SetPagePrivate(page)    set_bit(PG_private, &(page)->flags)
#define SetPageWriteback(page)  set_bit(PG_writeback, &(page)->flags)
#define SetPageHead(page)       set_bit(PG_head, &(page)->flags)
#define SetPageTail(page)       set_bit(PG_tail, &(page)->flags)
#define SetPageCompound(page)   set_bit(PG_compound, &(page)->flags)
#define SetPageSwapCache(page)  set_bit(PG_swapcache, &(page)->flags)
#define SetPageMappedToDisk(page) set_bit(PG_mappedtodisk, &(page)->flags)
#define SetPageReclaim(page)    set_bit(PG_reclaim, &(page)->flags)
#define SetPageSwapBacked(page) set_bit(PG_swapbacked, &(page)->flags)
#define SetPageUnevictable(page) set_bit(PG_unevictable, &(page)->flags)
#define SetPageMlocked(page)    set_bit(PG_mlocked, &(page)->flags)
#define SetPageUncached(page)   set_bit(PG_uncached, &(page)->flags)
#define SetPageHWPoison(page)   set_bit(PG_hwpoison, &(page)->flags)
#define SetPageYoung(page)      set_bit(PG_young, &(page)->flags)
#define SetPageIdle(page)       set_bit(PG_idle, &(page)->flags)

/* 清除页面状态 */
#define ClearPageLocked(page)   clear_bit(PG_locked, &(page)->flags)
#define ClearPageError(page)    clear_bit(PG_error, &(page)->flags)
#define ClearPageReferenced(page) clear_bit(PG_referenced, &(page)->flags)
#define ClearPageUptodate(page) clear_bit(PG_uptodate, &(page)->flags)
#define ClearPageDirty(page)    clear_bit(PG_dirty, &(page)->flags)
#define ClearPageLRU(page)      clear_bit(PG_lru, &(page)->flags)
#define ClearPageActive(page)   clear_bit(PG_active, &(page)->flags)
#define ClearPageSlab(page)     clear_bit(PG_slab, &(page)->flags)
#define ClearPageReserved(page) clear_bit(PG_reserved, &(page)->flags)
#define ClearPagePrivate(page)  clear_bit(PG_private, &(page)->flags)
#define ClearPageWriteback(page) clear_bit(PG_writeback, &(page)->flags)
#define ClearPageHead(page)     clear_bit(PG_head, &(page)->flags)
#define ClearPageTail(page)     clear_bit(PG_tail, &(page)->flags)
#define ClearPageCompound(page) clear_bit(PG_compound, &(page)->flags)
#define ClearPageSwapCache(page) clear_bit(PG_swapcache, &(page)->flags)
#define ClearPageMappedToDisk(page) clear_bit(PG_mappedtodisk, &(page)->flags)
#define ClearPageReclaim(page)  clear_bit(PG_reclaim, &(page)->flags)
#define ClearPageSwapBacked(page) clear_bit(PG_swapbacked, &(page)->flags)
#define ClearPageUnevictable(page) clear_bit(PG_unevictable, &(page)->flags)
#define ClearPageMlocked(page)  clear_bit(PG_mlocked, &(page)->flags)
#define ClearPageUncached(page) clear_bit(PG_uncached, &(page)->flags)
#define ClearPageHWPoison(page) clear_bit(PG_hwpoison, &(page)->flags)
#define ClearPageYoung(page)    clear_bit(PG_young, &(page)->flags)
#define ClearPageIdle(page)     clear_bit(PG_idle, &(page)->flags)

/* 内存区域类型 */
enum zone_type {
    ZONE_DMA,           /* 直接内存访问区域 */
    ZONE_DMA32,         /* 32位DMA区域 */
    ZONE_NORMAL,        /* 普通区域 */
    ZONE_HIGHMEM,       /* 高端内存区域 */
    ZONE_MOVABLE,       /* 可移动区域 */
    ZONE_DEVICE,        /* 设备区域 */
    __MAX_NR_ZONES
};

/* 内存区域水位 */
enum zone_watermarks {
    WMARK_MIN,          /* 最小水位 */
    WMARK_LOW,          /* 低水位 */
    WMARK_HIGH,         /* 高水位 */
    NR_WMARK
};

/* 页面迁移类型 */
enum migratetype {
    MIGRATE_UNMOVABLE,  /* 不可移动 */
    MIGRATE_MOVABLE,    /* 可移动 */
    MIGRATE_RECLAIMABLE, /* 可回收 */
    MIGRATE_PCPTYPES,   /* PCP类型数量 */
    MIGRATE_RESERVE = MIGRATE_PCPTYPES,
    MIGRATE_CMA,        /* 连续内存分配器 */
    MIGRATE_ISOLATE,    /* 隔离页面 */
    MIGRATE_TYPES
};

/* 页面分配标志 */
#define __GFP_DMA           0x01u   /* 分配DMA内存 */
#define __GFP_HIGHMEM       0x02u   /* 分配高端内存 */
#define __GFP_DMA32         0x04u   /* 分配32位DMA内存 */
#define __GFP_MOVABLE       0x08u   /* 分配可移动内存 */
#define __GFP_RECLAIMABLE   0x10u   /* 分配可回收内存 */
#define __GFP_HIGH          0x20u   /* 高优先级分配 */
#define __GFP_IO            0x40u   /* 可以启动I/O */
#define __GFP_FS            0x80u   /* 可以调用文件系统 */
#define __GFP_COLD          0x100u  /* 分配冷页面 */
#define __GFP_NOWARN        0x200u  /* 不要警告 */
#define __GFP_REPEAT        0x400u  /* 重复尝试 */
#define __GFP_NOFAIL        0x800u  /* 不能失败 */
#define __GFP_NORETRY       0x1000u /* 不重试 */
#define __GFP_MEMALLOC      0x2000u /* 使用紧急保留 */
#define __GFP_COMP          0x4000u /* 添加复合页面元数据 */
#define __GFP_ZERO          0x8000u /* 返回零页面 */
#define __GFP_NOMEMALLOC    0x10000u /* 不使用紧急保留 */
#define __GFP_HARDWALL      0x20000u /* 强制cpuset边界 */
#define __GFP_THISNODE      0x40000u /* 只在此节点分配 */
#define __GFP_ATOMIC        0x80000u /* 原子分配 */
#define __GFP_ACCOUNT       0x100000u /* 计入memcg */
#define __GFP_DIRECT_RECLAIM 0x200000u /* 直接回收 */
#define __GFP_KSWAPD_RECLAIM 0x400000u /* kswapd回收 */
#define __GFP_WRITE         0x800000u /* 分配器可以写 */
#define __GFP_RECLAIM       (__GFP_DIRECT_RECLAIM | __GFP_KSWAPD_RECLAIM)

/* 常用的GFP标志组合 */
#define GFP_KERNEL          (__GFP_RECLAIM | __GFP_IO | __GFP_FS)
#define GFP_ATOMIC          (__GFP_HIGH | __GFP_ATOMIC | __GFP_KSWAPD_RECLAIM)
#define GFP_USER            (__GFP_RECLAIM | __GFP_IO | __GFP_FS | __GFP_HARDWALL)
#define GFP_HIGHUSER        (GFP_USER | __GFP_HIGHMEM)
#define GFP_HIGHUSER_MOVABLE (GFP_HIGHUSER | __GFP_MOVABLE)
#define GFP_DMA             (__GFP_DMA)
#define GFP_DMA32           (__GFP_DMA32)
#define GFP_NOFS            (__GFP_RECLAIM | __GFP_IO)
#define GFP_NOIO            (__GFP_RECLAIM)
#define GFP_NOWAIT          (__GFP_KSWAPD_RECLAIM)
#define GFP_TRANSHUGE       ((GFP_HIGHUSER_MOVABLE | __GFP_COMP | __GFP_NOWARN) & ~__GFP_RECLAIM)
#define GFP_TRANSHUGE_LIGHT (GFP_HIGHUSER_MOVABLE | __GFP_COMP | __GFP_NOWARN)

/* 页面结构 */
struct page {
    unsigned long flags;        /* 页面标志 */

    union {
        struct {
            struct list_head lru;   /* LRU链表 */
            struct address_space *mapping; /* 地址空间 */
            pgoff_t index;          /* 页面索引 */
            void *private;          /* 私有数据 */
        };

        struct {
            struct page *next;      /* 下一页 */
            int pages;              /* 页面数量 */
            int pobjects;           /* 部分对象数量 */
        };

        struct {
            struct page *first_page; /* 复合页面首页 */
        };

        struct {
            struct list_head list;  /* 通用链表 */
            void *s_mem;            /* slab内存 */
            struct slab *slab_page; /* slab页面 */
        };
    };

    union {
        atomic_t _mapcount;         /* 映射计数 */
        struct {
            unsigned inuse:16;      /* 使用中的对象数量 */
            unsigned objects:15;    /* 对象总数 */
            unsigned frozen:1;      /* 冻结标志 */
        };
    };

    union {
        atomic_t _refcount;         /* 引用计数 */
        unsigned int active;        /* 活跃对象数量 */
        struct {
            unsigned long counters;
        };
    };

    struct mem_cgroup *mem_cgroup;  /* 内存控制组 */

    /* 调试信息 */
    void *virtual;                  /* 虚拟地址 */

    /* 锁和等待队列 */
    wait_queue_head_t *waiters;     /* 等待队列 */

    /* 页面所属的内存区域 */
    struct zone *zone;              /* 内存区域 */

    /* 页面所属的节点 */
    struct pglist_data *pgdat;      /* 节点数据 */
};

/* 页面偏移类型 */
typedef unsigned long pgoff_t;

/* 页面帧号 */
typedef unsigned long pfn_t;

/* 地址空间结构 */
struct address_space {
    struct inode *host;             /* 宿主inode */
    struct radix_tree_root page_tree; /* 页面基数树 */
    spinlock_t tree_lock;           /* 树锁 */
    atomic_t i_mmap_writable;       /* 可写映射计数 */
    struct rb_root i_mmap;          /* 私有映射 */
    struct rw_semaphore i_mmap_rwsem; /* 映射读写信号量 */
    unsigned long nrpages;          /* 页面数量 */
    unsigned long nrexceptional;    /* 异常条目数量 */
    pgoff_t writeback_index;        /* 回写索引 */
    const struct address_space_operations *a_ops; /* 操作函数 */
    unsigned long flags;            /* 标志 */
    struct backing_dev_info *backing_dev_info; /* 后备设备信息 */
    spinlock_t private_lock;        /* 私有锁 */
    struct list_head private_list;  /* 私有链表 */
    void *private_data;             /* 私有数据 */
};

/* 虚拟内存区域 */
struct vm_area_struct {
    struct mm_struct *vm_mm;        /* 所属的内存描述符 */
    unsigned long vm_start;         /* 起始虚拟地址 */
    unsigned long vm_end;           /* 结束虚拟地址 */

    struct vm_area_struct *vm_next; /* 下一个VMA */
    struct vm_area_struct *vm_prev; /* 上一个VMA */

    struct rb_node vm_rb;           /* 红黑树节点 */

    unsigned long rb_subtree_last;  /* 子树最大结束地址 */

    pgprot_t vm_page_prot;          /* 页面保护 */
    unsigned long vm_flags;         /* 标志 */

    struct {
        struct rb_node rb;          /* 红黑树节点 */
        unsigned long rb_subtree_last;
    } shared;

    struct list_head anon_vma_chain; /* 匿名VMA链 */
    struct anon_vma *anon_vma;      /* 匿名VMA */

    const struct vm_operations_struct *vm_ops; /* 操作函数 */

    unsigned long vm_pgoff;         /* 页面偏移 */
    struct file *vm_file;           /* 映射文件 */
    void *vm_private_data;          /* 私有数据 */

    struct mempolicy *vm_policy;    /* NUMA策略 */
    struct vm_userfaultfd_ctx vm_userfaultfd_ctx; /* 用户故障上下文 */
};

/* VMA标志 */
#define VM_READ         0x00000001  /* 可读 */
#define VM_WRITE        0x00000002  /* 可写 */
#define VM_EXEC         0x00000004  /* 可执行 */
#define VM_SHARED       0x00000008  /* 共享 */
#define VM_MAYREAD      0x00000010  /* 可能可读 */
#define VM_MAYWRITE     0x00000020  /* 可能可写 */
#define VM_MAYEXEC      0x00000040  /* 可能可执行 */
#define VM_MAYSHARE     0x00000080  /* 可能共享 */
#define VM_GROWSDOWN    0x00000100  /* 向下增长 */
#define VM_GROWSUP      0x00000200  /* 向上增长 */
#define VM_PFNMAP       0x00000400  /* 页面帧号映射 */
#define VM_DENYWRITE    0x00000800  /* 拒绝写入 */
#define VM_LOCKED       0x00002000  /* 锁定在内存中 */
#define VM_IO           0x00004000  /* 内存映射I/O */
#define VM_SEQ_READ     0x00008000  /* 顺序读取 */
#define VM_RAND_READ    0x00010000  /* 随机读取 */
#define VM_DONTCOPY     0x00020000  /* 不复制给子进程 */
#define VM_DONTEXPAND   0x00040000  /* 不能扩展 */
#define VM_LOCKONFAULT  0x00080000  /* 故障时锁定 */
#define VM_ACCOUNT      0x00100000  /* 计入vm统计 */
#define VM_NORESERVE    0x00200000  /* 不保留交换空间 */
#define VM_HUGETLB      0x00400000  /* 大页面 */
#define VM_ARCH_1       0x01000000  /* 架构相关标志 */
#define VM_ARCH_2       0x02000000  /* 架构相关标志 */
#define VM_DONTDUMP     0x04000000  /* 不转储 */
#define VM_SOFTDIRTY    0x08000000  /* 软脏页 */
#define VM_MIXEDMAP     0x10000000  /* 混合映射 */
#define VM_HUGEPAGE     0x20000000  /* 大页面提示 */
#define VM_NOHUGEPAGE   0x40000000  /* 不使用大页面 */
#define VM_MERGEABLE    0x80000000  /* KSM可合并 */

/* 页面保护类型 */
typedef struct {
    unsigned long pgprot;
} pgprot_t;

/* 内存区域结构 */
struct zone {
    unsigned long watermark[NR_WMARK]; /* 水位标记 */
    unsigned long nr_reserved_highatomic; /* 高原子保留页面数 */

    long lowmem_reserve[MAX_NR_ZONES]; /* 低内存保留 */

    struct per_cpu_pages per_cpu_pageset[NR_CPUS]; /* 每CPU页面集 */

    spinlock_t lock;                /* 区域锁 */

    /* 空闲页面统计 */
    unsigned long free_pages;       /* 空闲页面数 */
    unsigned long min_unmapped_pages; /* 最小未映射页面数 */
    unsigned long min_slab_pages;   /* 最小slab页面数 */

    struct free_area free_area[MAX_ORDER]; /* 空闲区域 */

    /* 区域回收 */
    unsigned long pages_scanned;    /* 扫描的页面数 */
    spinlock_t lru_lock;           /* LRU锁 */
    struct lruvec lruvec;          /* LRU向量 */

    unsigned long compact_cached_free_pfn; /* 压缩缓存的空闲pfn */
    unsigned long compact_cached_migrate_pfn[2]; /* 压缩缓存的迁移pfn */
    unsigned long compact_init_migrate_pfn; /* 压缩初始迁移pfn */
    unsigned long compact_init_free_pfn; /* 压缩初始空闲pfn */

    unsigned int compact_considered; /* 压缩考虑数 */
    unsigned int compact_defer_shift; /* 压缩延迟偏移 */
    int compact_order_failed;       /* 压缩顺序失败 */

    bool contiguous;               /* 连续内存 */

    /* 统计信息 */
    atomic_long_t vm_stat[NR_VM_ZONE_STAT_ITEMS]; /* 虚拟内存统计 */
    atomic_long_t vm_numa_stat[NR_VM_NUMA_STAT_ITEMS]; /* NUMA统计 */

    /* 区域信息 */
    unsigned long zone_start_pfn;   /* 区域起始pfn */
    unsigned long managed_pages;    /* 管理的页面数 */
    unsigned long spanned_pages;    /* 跨越的页面数 */
    unsigned long present_pages;    /* 存在的页面数 */

    const char *name;              /* 区域名称 */

    /* 内存热插拔 */
    seqlock_t span_seqlock;        /* 跨越序列锁 */

    /* 等待队列 */
    wait_queue_head_t *wait_table;  /* 等待表 */
    unsigned long wait_table_hash_nr_entries; /* 等待表哈希条目数 */
    unsigned long wait_table_bits;  /* 等待表位数 */

    /* 页面回收 */
    struct pglist_data *zone_pgdat; /* 区域页面数据 */

    /* 填充到缓存行边界 */
    char padding[ZONE_PADDING_SIZE];
};

/* 最大分配阶数 */
#define MAX_ORDER 11

/* 空闲区域结构 */
struct free_area {
    struct list_head free_list[MIGRATE_TYPES]; /* 空闲链表 */
    unsigned long nr_free;          /* 空闲页面数 */
};

/* 每CPU页面集 */
struct per_cpu_pages {
    int count;                      /* 页面数量 */
    int high;                       /* 高水位 */
    int batch;                      /* 批量大小 */
    struct list_head lists[MIGRATE_PCPTYPES]; /* 页面链表 */
};

/* LRU向量 */
struct lruvec {
    struct list_head lists[NR_LRU_LISTS]; /* LRU链表 */
    struct zone_reclaim_stat reclaim_stat; /* 回收统计 */
    struct mem_cgroup_per_zone *mem_cgroup_zone; /* 内存控制组区域 */
};

/* LRU链表类型 */
enum lru_list {
    LRU_INACTIVE_ANON = LRU_BASE,   /* 不活跃匿名页面 */
    LRU_ACTIVE_ANON = LRU_BASE + LRU_ACTIVE, /* 活跃匿名页面 */
    LRU_INACTIVE_FILE = LRU_BASE + LRU_FILE, /* 不活跃文件页面 */
    LRU_ACTIVE_FILE = LRU_BASE + LRU_FILE + LRU_ACTIVE, /* 活跃文件页面 */
    LRU_UNEVICTABLE,               /* 不可回收页面 */
    NR_LRU_LISTS
};

#define LRU_BASE 0
#define LRU_ACTIVE 1
#define LRU_FILE 2

/* 内存节点数据 */
struct pglist_data {
    struct zone node_zones[MAX_NR_ZONES]; /* 节点区域 */
    struct zonelist node_zonelists[MAX_ZONELISTS]; /* 区域列表 */
    int nr_zones;                   /* 区域数量 */

    struct page *node_mem_map;      /* 节点内存映射 */

    unsigned long node_start_pfn;   /* 节点起始pfn */
    unsigned long node_present_pages; /* 节点存在页面数 */
    unsigned long node_spanned_pages; /* 节点跨越页面数 */

    int node_id;                    /* 节点ID */
    wait_queue_head_t kswapd_wait;  /* kswapd等待队列 */
    wait_queue_head_t pfmemalloc_wait; /* pfmemalloc等待队列 */
    struct task_struct *kswapd;     /* kswapd任务 */
    int kswapd_order;              /* kswapd阶数 */
    enum zone_type kswapd_classzone_idx; /* kswapd类区域索引 */

    int kswapd_failures;           /* kswapd失败次数 */

    int kcompactd_max_order;       /* kcompactd最大阶数 */
    enum zone_type kcompactd_classzone_idx; /* kcompactd类区域索引 */
    wait_queue_head_t kcompactd_wait; /* kcompactd等待队列 */
    struct task_struct *kcompactd;  /* kcompactd任务 */

    spinlock_t numabalancing_migrate_lock; /* NUMA平衡迁移锁 */

    unsigned long totalreserve_pages
