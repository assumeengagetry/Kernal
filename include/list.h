#define __LIST_H__

#include "types.h"

/* 双向链表节点结构 */
struct list_head {
    struct list_head *next;
    struct list_head *prev;
};

/* 哈希链表节点结构 */
struct hlist_head {
    struct hlist_node *first;
};

struct hlist_node {
    struct hlist_node *next;
    struct hlist_node **pprev;
};

/* 链表初始化宏 */
#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

/* 静态初始化链表头 */
static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

/* 哈希链表初始化 */
#define HLIST_HEAD_INIT { .first = NULL }
#define HLIST_HEAD(name) struct hlist_head name = { .first = NULL }
#define INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)

static inline void INIT_HLIST_NODE(struct hlist_node *h)
{
    h->next = NULL;
    h->pprev = NULL;
}

/* 检查链表是否为空 */
static inline int list_empty(const struct list_head *head)
{
    return head->next == head;
}

/* 检查链表是否为空(安全版本) */
static inline int list_empty_careful(const struct list_head *head)
{
    struct list_head *next = head->next;
    return (next == head) && (next == head->prev);
}

/* 检查节点是否是链表的最后一个 */
static inline int list_is_last(const struct list_head *list,
                               const struct list_head *head)
{
    return list->next == head;
}

/* 检查节点是否是链表的第一个 */
static inline int list_is_first(const struct list_head *list,
                                const struct list_head *head)
{
    return list->prev == head;
}

/* 检查链表是否只有一个节点 */
static inline int list_is_singular(const struct list_head *head)
{
    return !list_empty(head) && (head->next == head->prev);
}

/* 内部函数：在两个已知节点之间插入新节点 */
static inline void __list_add(struct list_head *new,
                              struct list_head *prev,
                              struct list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

/* 在指定节点后插入新节点 */
static inline void list_add(struct list_head *new, struct list_head *head)
{
    __list_add(new, head, head->next);
}

/* 在指定节点前插入新节点(添加到链表尾部) */
static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
    __list_add(new, head->prev, head);
}

/* 内部函数：删除节点 */
static inline void __list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

/* 删除节点 */
static inline void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = (struct list_head *)0x100;
    entry->prev = (struct list_head *)0x200;
}

/* 删除节点并重新初始化 */
static inline void list_del_init(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    INIT_LIST_HEAD(entry);
}

/* 替换节点 */
static inline void list_replace(struct list_head *old,
                               struct list_head *new)
{
    new->next = old->next;
    new->next->prev = new;
    new->prev = old->prev;
    new->prev->next = new;
}

/* 替换节点并重新初始化旧节点 */
static inline void list_replace_init(struct list_head *old,
                                    struct list_head *new)
{
    list_replace(old, new);
    INIT_LIST_HEAD(old);
}

/* 移动节点到新位置 */
static inline void list_move(struct list_head *list, struct list_head *head)
{
    __list_del(list->prev, list->next);
    list_add(list, head);
}

/* 移动节点到链表尾部 */
static inline void list_move_tail(struct list_head *list,
                                 struct list_head *head)
{
    __list_del(list->prev, list->next);
    list_add_tail(list, head);
}

/* 旋转链表，使list成为新的头部 */
static inline void list_rotate_left(struct list_head *head)
{
    struct list_head *first;

    if (!list_empty(head)) {
        first = head->next;
        list_move_tail(first, head);
    }
}

/* 分割链表 */
static inline void list_cut_position(struct list_head *list,
                                    struct list_head *head,
                                    struct list_head *entry)
{
    if (list_empty(head))
        return;
    if (list_is_singular(head) && (head->next != entry && head != entry))
        return;
    if (entry == head) {
        INIT_LIST_HEAD(list);
        return;
    }
    __list_cut_position(list, head, entry);
}

/* 内部函数：分割链表 */
static inline void __list_cut_position(struct list_head *list,
                                      struct list_head *head,
                                      struct list_head *entry)
{
    struct list_head *new_first = entry->next;
    list->next = head->next;
    list->next->prev = list;
    list->prev = entry;
    entry->next = list;
    head->next = new_first;
    new_first->prev = head;
}

/* 连接两个链表 */
static inline void list_splice(const struct list_head *list,
                              struct list_head *head)
{
    if (!list_empty(list))
        __list_splice(list, head, head->next);
}

/* 连接两个链表到尾部 */
static inline void list_splice_tail(struct list_head *list,
                                   struct list_head *head)
{
    if (!list_empty(list))
        __list_splice(list, head->prev, head);
}

/* 连接两个链表并重新初始化 */
static inline void list_splice_init(struct list_head *list,
                                   struct list_head *head)
{
    if (!list_empty(list)) {
        __list_splice(list, head, head->next);
        INIT_LIST_HEAD(list);
    }
}

/* 连接两个链表到尾部并重新初始化 */
static inline void list_splice_tail_init(struct list_head *list,
                                        struct list_head *head)
{
    if (!list_empty(list)) {
        __list_splice(list, head->prev, head);
        INIT_LIST_HEAD(list);
    }
}

/* 内部函数：连接链表 */
static inline void __list_splice(const struct list_head *list,
                                struct list_head *prev,
                                struct list_head *next)
{
    struct list_head *first = list->next;
    struct list_head *last = list->prev;

    first->prev = prev;
    prev->next = first;

    last->next = next;
    next->prev = last;
}

/* 获取链表条目 */
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

/* 获取第一个条目 */
#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)

/* 获取最后一个条目 */
#define list_last_entry(ptr, type, member) \
    list_entry((ptr)->prev, type, member)

/* 获取第一个条目或NULL */
#define list_first_entry_or_null(ptr, type, member) \
    (!list_empty(ptr) ? list_first_entry(ptr, type, member) : NULL)

/* 获取下一个条目 */
#define list_next_entry(pos, member) \
    list_entry((pos)->member.next, typeof(*(pos)), member)

/* 获取上一个条目 */
#define list_prev_entry(pos, member) \
    list_entry((pos)->member.prev, typeof(*(pos)), member)

/* 遍历链表 */
#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/* 反向遍历链表 */
#define list_for_each_prev(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

/* 安全遍历链表(可以在遍历中删除节点) */
#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
         pos = n, n = pos->next)

/* 反向安全遍历链表 */
#define list_for_each_prev_safe(pos, n, head) \
    for (pos = (head)->prev, n = pos->prev; \
         pos != (head); \
         pos = n, n = pos->prev)

/* 遍历链表条目 */
#define list_for_each_entry(pos, head, member) \
    for (pos = list_first_entry(head, typeof(*pos), member); \
         &pos->member != (head); \
         pos = list_next_entry(pos, member))

/* 反向遍历链表条目 */
#define list_for_each_entry_reverse(pos, head, member) \
    for (pos = list_last_entry(head, typeof(*pos), member); \
         &pos->member != (head); \
         pos = list_prev_entry(pos, member))

/* 继续遍历链表条目 */
#define list_for_each_entry_continue(pos, head, member) \
    for (pos = list_next_entry(pos, member); \
         &pos->member != (head); \
         pos = list_next_entry(pos, member))

/* 继续反向遍历链表条目 */
#define list_for_each_entry_continue_reverse(pos, head, member) \
    for (pos = list_prev_entry(pos, member); \
         &pos->member != (head); \
         pos = list_prev_entry(pos, member))

/* 从指定位置开始遍历链表条目 */
#define list_for_each_entry_from(pos, head, member) \
    for (; &pos->member != (head); \
         pos = list_next_entry(pos, member))

/* 安全遍历链表条目 */
#define list_for_each_entry_safe(pos, n, head, member) \
    for (pos = list_first_entry(head, typeof(*pos), member), \
         n = list_next_entry(pos, member); \
         &pos->member != (head); \
         pos = n, n = list_next_entry(n, member))

/* 继续安全遍历链表条目 */
#define list_for_each_entry_safe_continue(pos, n, head, member) \
    for (pos = list_next_entry(pos, member), \
         n = list_next_entry(pos, member); \
         &pos->member != (head); \
         pos = n, n = list_next_entry(n, member))

/* 从指定位置开始安全遍历链表条目 */
#define list_for_each_entry_safe_from(pos, n, head, member) \
    for (n = list_next_entry(pos, member); \
         &pos->member != (head); \
         pos = n, n = list_next_entry(n, member))

/* 反向安全遍历链表条目 */
#define list_for_each_entry_safe_reverse(pos, n, head, member) \
    for (pos = list_last_entry(head, typeof(*pos), member), \
         n = list_prev_entry(pos, member); \
         &pos->member != (head); \
         pos = n, n = list_prev_entry(n, member))

/* 重置链表遍历 */
#define list_prepare_entry(pos, head, member) \
    ((pos) ? : list_entry(head, typeof(*pos), member))

/* 哈希链表操作 */
static inline int hlist_unhashed(const struct hlist_node *h)
{
    return !h->pprev;
}

static inline int hlist_empty(const struct hlist_head *h)
{
    return !h->first;
}

static inline void __hlist_del(struct hlist_node *n)
{
    struct hlist_node *next = n->next;
    struct hlist_node **pprev = n->pprev;
    *pprev = next;
    if (next)
        next->pprev = pprev;
}

static inline void hlist_del(struct hlist_node *n)
{
    __hlist_del(n);
    n->next = (struct hlist_node *)0x100;
    n->pprev = (struct hlist_node **)0x200;
}

static inline void hlist_del_init(struct hlist_node *n)
{
    if (!hlist_unhashed(n)) {
        __hlist_del(n);
        INIT_HLIST_NODE(n);
    }
}

static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
    struct hlist_node *first = h->first;
    n->next = first;
    if (first)
        first->pprev = &n->next;
    h->first = n;
    n->pprev = &h->first;
}

static inline void hlist_add_before(struct hlist_node *n,
                                   struct hlist_node *next)
{
    n->pprev = next->pprev;
    n->next = next;
    next->pprev = &n->next;
    *(n->pprev) = n;
}

static inline void hlist_add_behind(struct hlist_node *n,
                                   struct hlist_node *prev)
{
    n->next = prev->next;
    prev->next = n;
    n->pprev = &prev->next;

    if (n->next)
        n->next->pprev = &n->next;
}

static inline void hlist_add_fake(struct hlist_node *n)
{
    n->pprev = &n->next;
}

static inline void hlist_move_list(struct hlist_head *old,
                                  struct hlist_head *new)
{
    new->first = old->first;
    if (new->first)
        new->first->pprev = &new->first;
    old->first = NULL;
}

#define hlist_entry(ptr, type, member) container_of(ptr, type, member)

#define hlist_for_each(pos, head) \
    for (pos = (head)->first; pos; pos = pos->next)

#define hlist_for_each_safe(pos, n, head) \
    for (pos = (head)->first; pos && ({ n = pos->next; 1; }); \
         pos = n)

#define hlist_entry_safe(ptr, type, member) \
    ({ typeof(ptr) ____ptr = (ptr); \
       ____ptr ? hlist_entry(____ptr, type, member) : NULL; \
    })

#define hlist_for_each_entry(pos, head, member) \
    for (pos = hlist_entry_safe((head)->first, typeof(*(pos)), member);\
         pos; \
         pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

#define hlist_for_each_entry_continue(pos, member) \
    for (pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member);\
         pos; \
         pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

#define hlist_for_each_entry_from(pos, member) \
    for (; pos; \
         pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

#define hlist_for_each_entry_safe(pos, n, head, member) \
    for (pos = hlist_entry_safe((head)->first, typeof(*pos), member);\
         pos && ({ n = pos->member.next; 1; }); \
         pos = hlist_entry_safe(n, typeof(*pos), member))

/* 计算链表长度 */
static inline size_t list_count_nodes(struct list_head *head)
{
    struct list_head *pos;
    size_t count = 0;

    list_for_each(pos, head)
        count++;

    return count;
}

/* 检查链表是否包含指定节点 */
static inline bool list_contains(struct list_head *head, struct list_head *node)
{
    struct list_head *pos;

    list_for_each(pos, head) {
        if (pos == node)
            return true;
    }

    return false;
}

/* 获取链表中第n个节点 */
static inline struct list_head *list_get_nth(struct list_head *head, size_t n)
{
    struct list_head *pos;
    size_t i = 0;

    list_for_each(pos, head) {
        if (i == n)
            return pos;
        i++;
    }

    return NULL;
}

/* 反转链表 */
static inline void list_reverse(struct list_head *head)
{
    struct list_head *pos, *tmp;

    list_for_each_safe(pos, tmp, head) {
        list_move(pos, head);
    }
}
