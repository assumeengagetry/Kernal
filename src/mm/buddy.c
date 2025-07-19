#include "../../include/mm.h"
#include "../../include/types.h"
#include "../../include/list.h"
#include "../../include/spinlock.h"

/* 伙伴系统常量 */
#define MAX_ORDER               11
#define MIGRATE_UNMOVABLE       0
#define MIGRATE_MOVABLE         1
#define MIGRATE_RECLAIMABLE     2
#define MIGRATE_PCPTYPES        3
#define MIGRATE_RESERVE         3
#define MIGRATE_CMA             4
#define MIGRATE_ISOLATE         5
#define MIGRATE_TYPES           6

/* 页面块标志 */
#define PB_migrate_end          (PB_migrate + 3 - 1)
#define PB_migrate_mask         ((1UL << (PB_migrate_end + 1)) - (1UL << PB_migrate))
#define PB_migrate_skip         (PB_migrate + 3)

/* 全局变量 */
static struct zone *zones[MAX_NR_ZONES];
static int nr_zones = 0;
static struct pglist_data *node_data[MAX_NUMNODES];
static int nr_nodes = 0;

/* 内存统计 */
static ulong total_pages = 0;
static ulong free_pages = 0;
static ulong reserved_pages = 0;

/* 伙伴系统锁 */
static spinlock_t zone_lock;

/* 初始化伙伴系统 */
void buddy_init(void)
{
    int i, j;

    spin_lock_init(&zone_lock);

    /* 初始化所有区域 */
    for (i = 0; i < MAX_NR_ZONES; i++) {
        zones[i] = NULL;
    }

    /* 初始化所有节点 */
    for (i = 0; i < MAX_NUMNODES; i++) {
        node_data[i] = NULL;
    }

    printk("Buddy system initialized\n");
}

/* 初始化内存区域 */
int init_zone(struct zone *zone, ulong start_pfn, ulong size)
{
    int order, migratetype;
    ulong flags;

    if (!zone)
        return -EINVAL;

    /* 初始化区域基本信息 */
    zone->zone_start_pfn = start_pfn;
    zone->spanned_pages = size;
    zone->present_pages = size;
    zone->managed_pages = size;

    /* 初始化锁 */
    spin_lock_init(&zone->lock);
    spin_lock_init(&zone->lru_lock);

    /* 初始化空闲区域 */
    for (order = 0; order < MAX_ORDER; order++) {
        for (migratetype = 0; migratetype < MIGRATE_TYPES; migratetype++) {
            INIT_LIST_HEAD(&zone->free_area[order].free_list[migratetype]);
        }
        zone->free_area[order].nr_free = 0;
    }

    /* 初始化水位标记 */
    zone->watermark[WMARK_MIN] = size / 256;    /* 0.39% */
    zone->watermark[WMARK_LOW] = size / 128;    /* 0.78% */
    zone->watermark[WMARK_HIGH] = size / 64;    /* 1.56% */

    /* 初始化统计信息 */
    zone->free_pages = 0;
    zone->pages_scanned = 0;

    /* 初始化LRU */
    for (migratetype = 0; migratetype < NR_LRU_LISTS; migratetype++) {
        INIT_LIST_HEAD(&zone->lruvec.lists[migratetype]);
    }

    /* 初始化压缩相关 */
    zone->compact_cached_free_pfn = start_pfn;
    zone->compact_cached_migrate_pfn[0] = start_pfn;
    zone->compact_cached_migrate_pfn[1] = start_pfn;
    zone->compact_init_migrate_pfn = start_pfn;
    zone->compact_init_free_pfn = start_pfn + size;

    zone->compact_considered = 0;
    zone->compact_defer_shift = 0;
    zone->compact_order_failed = -1;

    /* 初始化等待队列 */
    zone->wait_table = NULL;
    zone->wait_table_hash_nr_entries = 0;
    zone->wait_table_bits = 0;

    /* 添加到全局区域数组 */
    spin_lock_irqsave(&zone_lock, flags);
    if (nr_zones < MAX_NR_ZONES) {
        zones[nr_zones++] = zone;
    }
    spin_unlock_irqrestore(&zone_lock, flags);

    return 0;
}

/* 获取页面阶数 */
static int get_order(ulong size)
{
    int order = 0;

    size = (size - 1) >> PAGE_SHIFT;
    while (size) {
        size >>= 1;
        order++;
    }

    return order;
}

/* 获取页面块的迁移类型 */
static int get_pageblock_migratetype(struct page *page)
{
    if (!page)
        return MIGRATE_UNMOVABLE;

    /* 简化实现，实际需要从页面块位图中读取 */
    return MIGRATE_UNMOVABLE;
}

/* 设置页面块的迁移类型 */
static void set_pageblock_migratetype(struct page *page, int migratetype)
{
    if (!page)
        return;

    /* 简化实现，实际需要设置页面块位图 */
}

/* 获取伙伴页面 */
static struct page *get_buddy_page(struct page *page, unsigned int order)
{
    ulong buddy_pfn;
    ulong page_pfn = page_to_pfn(page);

    buddy_pfn = __find_buddy_pfn(page_pfn, order);
    return pfn_to_page(buddy_pfn);
}

/* 查找伙伴页面帧号 */
static ulong __find_buddy_pfn(ulong page_pfn, unsigned int order)
{
    return page_pfn ^ (1 << order);
}

/* 检查页面是否为伙伴 */
static int page_is_buddy(struct page *page, struct page *buddy, unsigned int order)
{
    if (!page || !buddy)
        return 0;

    /* 检查页面是否在伙伴系统中 */
    if (!PageBuddy(buddy))
        return 0;

    /* 检查阶数是否匹配 */
    if (page_order(buddy) != order)
        return 0;

    /* 检查是否在同一个区域 */
    if (page_zone_id(page) != page_zone_id(buddy))
        return 0;

    return 1;
}

/* 从空闲链表中移除页面 */
static void __del_page_from_free_list(struct page *page, struct zone *zone,
                                     unsigned int order, int migratetype)
{
    /* 设置页面非伙伴状态 */
    __ClearPageBuddy(page);
    set_page_private(page, 0);

    /* 从空闲链表中移除 */
    list_del(&page->lru);

    /* 更新统计信息 */
    zone->free_area[order].nr_free--;
    zone->free_pages -= 1UL << order;
}

/* 将页面添加到空闲链表 */
static void __add_page_to_free_list(struct page *page, struct zone *zone,
                                   unsigned int order, int migratetype)
{
    struct free_area *area = &zone->free_area[order];

    /* 添加到空闲链表 */
    list_add(&page->lru, &area->free_list[migratetype]);

    /* 设置页面为伙伴状态 */
    __SetPageBuddy(page);
    set_page_private(page, order);

    /* 更新统计信息 */
    area->nr_free++;
    zone->free_pages += 1UL << order;
}

/* 扩展内存块 */
static void expand(struct zone *zone, struct page *page,
                  int low, int high, struct free_area *area,
                  int migratetype)
{
    ulong size = 1 << high;

    while (high > low) {
        area--;
        high--;
        size >>= 1;

        /* 将一半页面添加到低一级的空闲链表 */
        __add_page_to_free_list(&page[size], zone, high, migratetype);

        /* 设置页面标志 */
        set_page_guard(zone, &page[size], high, migratetype);
    }
}

/* 设置页面保护 */
static void set_page_guard(struct zone *zone, struct page *page,
                          unsigned int order, int migratetype)
{
    if (order >= DEBUG_GUARDPAGE_MINORDER) {
        __SetPageGuard(page);
        set_page_private(page, order);
    }
}

/* 清除页面保护 */
static void clear_page_guard(struct zone *zone, struct page *page,
                            unsigned int order, int migratetype)
{
    if (order >= DEBUG_GUARDPAGE_MINORDER) {
        __ClearPageGuard(page);
        set_page_private(page, 0);
    }
}

/* 从指定迁移类型的空闲链表中分配页面 */
static struct page *__rmqueue_smallest(struct zone *zone, unsigned int order,
                                      int migratetype)
{
    unsigned int current_order;
    struct free_area *area;
    struct page *page;

    /* 从请求的阶数开始查找 */
    for (current_order = order; current_order < MAX_ORDER; current_order++) {
        area = &zone->free_area[current_order];

        /* 检查是否有空闲页面 */
        if (list_empty(&area->free_list[migratetype]))
            continue;

        /* 获取第一个空闲页面 */
        page = list_first_entry(&area->free_list[migratetype],
                               struct page, lru);

        /* 从空闲链表中移除 */
        __del_page_from_free_list(page, zone, current_order, migratetype);

        /* 如果需要，分割页面 */
        expand(zone, page, order, current_order, area, migratetype);

        /* 设置页面迁移类型 */
        set_freepage_migratetype(page, migratetype);

        return page;
    }

    return NULL;
}

/* 分配页面 */
static struct page *rmqueue(struct zone *zone, unsigned int order,
                           gfp_t gfp_flags, int migratetype)
{
    struct page *page;
    ulong flags;

    spin_lock_irqsave(&zone->lock, flags);

    /* 尝试从指定迁移类型分配 */
    page = __rmqueue_smallest(zone, order, migratetype);

    /* 如果失败，尝试从其他迁移类型分配 */
    if (!page) {
        page = __rmqueue_fallback(zone, order, migratetype);
    }

    spin_unlock_irqrestore(&zone->lock, flags);

    if (page) {
        /* 准备页面 */
        prep_new_page(page, order, gfp_flags);

        /* 更新统计信息 */
        __mod_zone_page_state(zone, NR_FREE_PAGES, -(1 << order));
    }

    return page;
}

/* 备用分配策略 */
static struct page *__rmqueue_fallback(struct zone *zone, unsigned int order,
                                      int start_migratetype)
{
    struct free_area *area;
    unsigned int current_order;
    struct page *page;
    int fallback_mt;
    bool can_steal;

    /* 优先级顺序：不可移动 -> 可回收 -> 可移动 */
    int fallbacks[MIGRATE_TYPES][4] = {
        [MIGRATE_UNMOVABLE]   = { MIGRATE_RECLAIMABLE, MIGRATE_MOVABLE,   MIGRATE_TYPES },
        [MIGRATE_RECLAIMABLE] = { MIGRATE_UNMOVABLE,   MIGRATE_MOVABLE,   MIGRATE_TYPES },
        [MIGRATE_MOVABLE]     = { MIGRATE_RECLAIMABLE, MIGRATE_UNMOVABLE, MIGRATE_TYPES },
        [MIGRATE_CMA]         = { MIGRATE_TYPES }, /* Never used */
        [MIGRATE_RESERVE]     = { MIGRATE_RESERVE }, /* Never used */
        [MIGRATE_ISOLATE]     = { MIGRATE_TYPES }, /* Never used */
    };

    /* 从大到小查找 */
    for (current_order = MAX_ORDER - 1; current_order >= order; current_order--) {
        area = &zone->free_area[current_order];
        fallback_mt = find_suitable_fallback(area, current_order,
                                            start_migratetype, false, &can_steal);
        if (fallback_mt == -1)
            continue;

        /* 获取页面 */
        page = list_first_entry(&area->free_list[fallback_mt],
                               struct page, lru);

        /* 从空闲链表中移除 */
        __del_page_from_free_list(page, zone, current_order, fallback_mt);

        /* 如果可以窃取，改变页面块的迁移类型 */
        if (can_steal) {
            steal_suitable_fallback(zone, page, start_migratetype);
        }

        /* 分割页面 */
        expand(zone, page, order, current_order, area, start_migratetype);

        /* 设置页面迁移类型 */
        set_freepage_migratetype(page, start_migratetype);

        return page;
    }

    return NULL;
}

/* 查找合适的备用迁移类型 */
static int find_suitable_fallback(struct free_area *area, unsigned int order,
                                 int migratetype, bool only_stealable, bool *can_steal)
{
    int i;
    int fallback_mt;

    if (area->nr_free == 0)
        return -1;

    *can_steal = false;

    /* 按优先级查找 */
    for (i = 0;; i++) {
        fallback_mt = fallbacks[migratetype][i];
        if (fallback_mt == MIGRATE_TYPES)
            break;

        if (list_empty(&area->free_list[fallback_mt]))
            continue;

        /* 检查是否可以窃取 */
        if (can_steal_fallback(order, migratetype))
            *can_steal = true;

        if (!only_stealable)
            return fallback_mt;

        if (*can_steal)
            return fallback_mt;
    }

    return -1;
}

/* 检查是否可以窃取备用页面 */
static bool can_steal_fallback(unsigned int order, int start_mt)
{
    /* 如果请求的阶数足够大，可以窃取 */
    if (order >= pageblock_order)
        return true;

    /* 如果是可移动类型，可以窃取 */
    if (start_mt == MIGRATE_MOVABLE)
        return true;

    return false;
}

/* 窃取合适的备用页面 */
static void steal_suitable_fallback(struct zone *zone, struct page *page,
                                   int start_type)
{
    unsigned int current_order = page_order(page);
    struct free_area *area;

    /* 只有在阶数足够大时才窃取 */
    if (current_order < pageblock_order)
        return;

    /* 改变页面块的迁移类型 */
    set_pageblock_migratetype(page, start_type);

    /* 将剩余的页面移动到正确的迁移类型链表 */
    area = &zone->free_area[current_order];
    move_freepages_block(zone, page, start_type);
}

/* 移动空闲页面到指定迁移类型 */
static int move_freepages_block(struct zone *zone, struct page *page,
                               int migratetype)
{
    ulong start_pfn, end_pfn;
    struct page *start_page, *end_page;

    start_pfn = page_to_pfn(page);
    start_pfn = start_pfn & ~(pageblock_nr_pages - 1);
    start_page = pfn_to_page(start_pfn);
    end_page = start_page + pageblock_nr_pages - 1;
    end_pfn = start_pfn + pageblock_nr_pages - 1;

    /* 检查页面范围是否有效 */
    if (start_pfn < zone->zone_start_pfn)
        start_page = pfn_to_page(zone->zone_start_pfn);
    if (end_pfn >= zone_end_pfn(zone))
        return 0;

    return move_freepages(zone, start_page, end_page, migratetype);
}

/* 移动空闲页面 */
static int move_freepages(struct zone *zone, struct page *start_page,
                         struct page *end_page, int migratetype)
{
    struct page *page;
    unsigned int order;
    int pages_moved = 0;

    for (page = start_page; page <= end_page;) {
        if (!pfn_valid_within(page_to_pfn(page))) {
            page++;
            continue;
        }

        if (!PageBuddy(page)) {
            page++;
            continue;
        }

        order = page_order(page);
        __del_page_from_free_list(page, zone, order,
                                 get_freepage_migratetype(page));
        __add_page_to_free_list(page, zone, order, migratetype);

        pages_moved += 1 << order;
        page += 1 << order;
    }

    return pages_moved;
}

/* 准备新页面 */
static void prep_new_page(struct page *page, unsigned int order, gfp_t gfp_flags)
{
    int i;

    for (i = 0; i < (1 << order); i++) {
        struct page *p = page + i;

        /* 清除页面标志 */
        clear_page_flags(p);

        /* 设置页面引用计数 */
        set_page_count(p, 1);

        /* 清除页面内容 */
        if (gfp_flags & __GFP_ZERO)
            clear_page(p);

        /* 设置页面私有数据 */
        set_page_private(p, 0);

        /* 初始化页面锁 */
        init_page_waiters(p);
    }
}

/* 清除页面标志 */
static void clear_page_flags(struct page *page)
{
    page->flags &= ~(1 << PG_locked | 1 << PG_error | 1 << PG_referenced |
                     1 << PG_dirty | 1 << PG_active | 1 << PG_reserved |
                     1 << PG_private | 1 << PG_writeback);
}

/* 释放页面 */
void __free_pages(struct page *page, unsigned int order)
{
    if (put_page_testzero(page)) {
        if (order == 0)
            free_hot_cold_page(page, false);
        else
            __free_pages_ok(page, order);
    }
}

/* 释放单个页面 */
static void free_hot_cold_page(struct page *page, bool cold)
{
    struct zone *zone = page_zone(page);
    struct per_cpu_pages *pcp;
    ulong flags;
    int migratetype;

    if (!free_page_prepare(page, 0))
        return;

    migratetype = get_freepage_migratetype(page);

    local_irq_save(flags);
    __count_vm_event(PGFREE);

    /* 使用每CPU缓存 */
    pcp = &zone->per_cpu_pageset[smp_processor_id()];

    if (cold)
        list_add_tail(&page->lru, &pcp->lists[migratetype]);
    else
        list_add(&page->lru, &pcp->lists[migratetype]);

    pcp->count++;

    /* 如果缓存满了，释放一批到伙伴系统 */
    if (pcp->count >= pcp->high) {
        ulong batch = READ_ONCE(pcp->batch);
        free_pcppages_bulk(zone, batch, pcp);
        pcp->count -= batch;
    }

    local_irq_restore(flags);
}

/* 释放页面到伙伴系统 */
static void __free_pages_ok(struct page *page, unsigned int order)
{
    ulong flags;
    int migratetype;

    if (!free_page_prepare(page, order))
        return;

    migratetype = get_freepage_migratetype(page);

    local_irq_save(flags);
    __count_vm_events(PGFREE, 1 << order);
    free_one_page(page_zone(page), page, page_to_pfn(page), order, migratetype);
    local_irq_restore(flags);
}

/* 释放单个页面到伙伴系统 */
static void free_one_page(struct zone *zone, struct page *page,
                         ulong pfn, unsigned int order,
                         int migratetype)
{
    spin_lock(&zone->lock);
    __free_one_page(page, pfn, zone, order, migratetype);
    spin_unlock(&zone->lock);
}

/* 核心释放函数 */
static void __free_one_page(struct page *page, ulong pfn,
                           struct zone *zone, unsigned int order,
                           int migratetype)
{
    struct page *buddy;
    ulong buddy_pfn;
    ulong combined_pfn;
    struct page *combined_page;

    VM_BUG_ON(!zone_is_initialized(zone));
    VM_BUG_ON_PAGE(page->flags & PAGE_FLAGS_CHECK_AT_PREP, page);

    VM_BUG_ON(migratetype == -1);

    while (order < MAX_ORDER - 1) {
        buddy_pfn = __find_buddy_pfn(pfn, order);
        buddy = page + (buddy_pfn - pfn);

        if (!page_is_buddy(page, buddy, order))
            goto done_merging;

        /* 合并伙伴页面 */
        __del_page_from_free_list(buddy, zone, order,
                                 get_freepage_migratetype(buddy));

        combined_pfn = buddy_pfn & pfn;
        combined_page = page + (combined_pfn - pfn);
        pfn = combined_pfn;
        order++;
        page = combined_page;
    }

done_merging:
    set_page_order(page, order);

    /* 添加到空闲链表 */
    __add_page_to_free_list(page, zone, order, migratetype);
}

/* 准备页面释放 */
static bool free_page_prepare(struct page *page, unsigned int order)
{
    bool compound = PageCompound(page);
    int i, bad = 0;

    VM_BUG_ON_PAGE(PageTail(page), page);
    VM_BUG_ON_PAGE(compound && compound_order(page) != order, page);

    trace_mm_page_free(page, order);

    for (i = 0; i < (1 << order); i++) {
        struct page *pg = page + i;

        if (PageAnon(pg))
            pg->mapping = NULL;

        bad += free_page_check(pg);
    }

    if (bad)
        return false;

    page_cpupid_reset_last(page);
    page->flags &= ~PAGE_FLAGS_CHECK_AT_PREP;
    reset_page_owner(page, order);

    if (!PageHighMem(page)) {
        debug_check_no_locks_freed(page_address(page), PAGE_SIZE << order);
        debug_check_no_obj_freed(page_address(page), PAGE_SIZE << order);
    }

    arch_free_page(page, order);
    kernel_map_pages(page, 1 << order, 0);

    return true;
}

/* 检查页面释放 */
static int free_page_check(struct page *page)
{
    const char *bad_reason = NULL;
    ulong bad_flags = 0;

    if (unlikely(page_mapcount(page)))
        bad_reason = "nonzero mapcount";
    if (unlikely(page->mapping != NULL))
        bad_reason = "non-NULL mapping";
    if (unlikely(page_ref_count(page) != 0))
        bad_reason = "nonzero _refcount";
    if (unlikely(page->flags & PAGE_FLAGS_CHECK_AT_FREE)) {
        bad_reason = "PAGE_FLAGS_CHECK_AT_FREE flag(s) set";
        bad_flags = PAGE_FLAGS_CHECK_AT_FREE;
    }

    if (unlikely(bad_reason)) {
        bad_page(page, bad_reason, bad_flags);
        return 1;
    }

    page_cpupid_reset_last(page);
    if (page->flags & PAGE_FLAGS_CHECK_AT_PREP)
        page->flags &= ~PAGE_FLAGS_CHECK_AT_PREP;

    return 0;
}

/* 分配页面 */
struct page *alloc_pages(gfp_t gfp_mask, unsigned int order)
{
    struct zone *zone;
    struct page *page;
    int migratetype = allocflags_to_migratetype(gfp_mask);

    /* 选择合适的内存区域 */
    zone = first_zones_zonelist(node_zonelist(0, gfp_mask),
                               high_zoneidx(gfp_mask), NULL);
    if (!zone)
        return NULL;

    /* 从区域分配页面 */
    page = rmqueue(zone, order, gfp_mask, migratetype);

    if (page) {
        /* 更新全局统计 */
        free_pages -= (1 << order);

        /* 设置页面区域 */
        set_page_zone(page, zone_idx(zone));
        set_page_node(page, zone_to_nid(zone));
    }

    return page;
}

/* 分配单个页面 */
struct page *alloc_page(gfp_t gfp_mask)
{
    return alloc_pages(gfp_mask, 0);
}

/* 分配页面并返回虚拟地址 */
ulong __get_free_pages(gfp_t gfp_mask, unsigned int order)
{
    struct page *page;

    page = alloc_pages(gfp_mask, order);
    if (!page)
        return 0;

    return (ulong)page_address(page);
}

/* 分配单个页面并返回虚拟地址 */
ulong __get_free_page(gfp_t gfp_mask)
{
    return __get_free_pages(gfp_mask, 0);
}

/* 分配清零的页面 */
ulong get_zeroed_page(gfp_t gfp_mask)
{
    return __get_free_pages(gfp_mask | __GFP_ZERO, 0);
}

/* 释放页面 */
void free_pages(ulong addr, unsigned int order)
{
    if (addr != 0) {
        VM_BUG_ON(!virt_addr_valid((void *)addr));
        __free_pages(virt_to_page((void *)addr), order);
    }
}

/* 释放单个页面 */
void free_page(ulong addr)
{
    free_pages(addr, 0);
}

/* 获取系统内存信息 */
void si_meminfo(struct sysinfo *val)
{
    val->totalram = total_pages;
    val->freeram = free_pages;
    val->sharedram = 0;
    val->bufferram = 0;
    val->totalswap = 0;
    val->freeswap = 0;
    val->totalhigh = 0;
    val->freehigh = 0;
    val->mem_unit = PAGE_SIZE;
}

/* 打印内存信息 */
void show_mem(unsigned int filter)
{
    struct zone *zone;
    ulong total = 0, free = 0, reserved = 0;
    int i;

    printk("Mem-info:\n");

    for (i = 0; i < nr_zones; i++) {
        zone = zones[i];
        if (!zone)
            continue;

        total += zone->present_pages;
        free += zone->free_pages;
        reserved += zone->present_pages - zone->managed_pages;

        printk("Zone %d: %lu pages, %lu free, %lu reserved\n",
               i, zone->present_pages, zone->free_pages,
               zone->present_pages - zone->managed_pages);
    }
