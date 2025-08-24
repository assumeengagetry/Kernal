#include "../../include/sched.h"
#include "../../include/types.h"
#include "../../include/list.h"
#include "../../include/spinlock.h"
#include "../../include/mm.h"

#define SCHED_LATENCY_NS        (6 * 1000000ULL)
#define SCHED_MIN_GRANULARITY_NS (750000ULL)
#define SCHED_WAKEUP_GRANULARITY_NS (1000000ULL)

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

static const u32 prio_to_wmult[40] = {
    /* -20 */ 48388, 59856, 76040, 92818, 118348,
    /* -15 */ 147320, 184698, 229616, 287308, 360437,
    /* -10 */ 449829, 563644, 704093, 875809, 1099582,
    /*  -5 */ 1376151, 1717300, 2157191, 2708050, 3363326,
    /*   0 */ 4194304, 5237765, 6557202, 8165337, 10153587,
    /*   5 */ 12820798, 15790321, 19976592, 24970740, 31350126,
    /*  10 */ 39045157, 49367440, 61356676, 76695844, 95443717,
    /*  15 */ 119304647, 148102320, 186737708, 238609294, 286331153,
};

struct cfs_rq {
    struct load_weight load;
    unsigned int nr_running;
    unsigned int h_nr_running;

    u64 exec_clock;
    u64 min_vruntime;

    struct rb_root tasks_timeline;
    struct rb_node *rb_leftmost;

    struct sched_entity *curr;
    struct sched_entity *next;
    struct sched_entity *last;
    struct sched_entity *skip;

    unsigned int nr_spread_over;

    struct rq *rq;
    struct task_group *tg;

    int runtime_enabled;
    u64 runtime_expires;
    s64 runtime_remaining;

    u64 throttled_clock;
    u64 throttled_clock_task;
    int throttled;
    int throttle_count;
    struct list_head throttled_list;
};

static void update_curr(struct cfs_rq *cfs_rq)
{
    struct sched_entity *curr = cfs_rq->curr;
    u64 now = sched_clock_cpu(cpu_of(rq_of(cfs_rq)));
    u64 delta_exec;

    if (unlikely(!curr))
        return;

    delta_exec = now - curr->exec_start;
    if (unlikely((s64)delta_exec <= 0))
        return;

    curr->exec_start = now;

    schedstat_set(curr->statistics.exec_max,
                  max(delta_exec, curr->statistics.exec_max));

    curr->sum_exec_runtime += delta_exec;
    schedstat_add(cfs_rq->exec_clock, delta_exec);

    curr->vruntime += calc_delta_fair(delta_exec, curr);
    update_min_vruntime(cfs_rq);

    if (entity_is_task(curr)) {
        struct task_struct *curtask = task_of(curr);

        trace_sched_stat_runtime(curtask, delta_exec, curr->vruntime);
        cpuacct_charge(curtask, delta_exec);
        account_group_exec_runtime(curtask, delta_exec);
    }

    account_cfs_rq_runtime(cfs_rq, delta_exec);
}

static u64 calc_delta_fair(u64 delta, struct sched_entity *se)
{
    if (unlikely(se->load.weight != NICE_0_LOAD))
        delta = __calc_delta(delta, NICE_0_LOAD, &se->load);

    return delta;
}

static u64 __calc_delta(u64 delta_exec, ulong weight, struct load_weight *lw)
{
    u64 fact = scale_load_down(weight);
    int shift = WMULT_SHIFT;

    __update_inv_weight(lw);

    if (unlikely(fact >> 32)) {
        while (fact >> 32) {
            fact >>= 1;
            shift--;
        }
    }

    fact = (u64)(u32)fact * lw->inv_weight;

    while (fact >> 32) {
        fact >>= 1;
        shift--;
    }

    return mul_u64_u32_shr(delta_exec, fact, shift);
}

static void __update_inv_weight(struct load_weight *lw)
{
    ulong w;

    if (likely(lw->inv_weight))
        return;

    w = scale_load_down(lw->weight);

    if (BITS_PER_LONG > 32 && unlikely(w >= WMULT_CONST))
        lw->inv_weight = 1;
    else if (unlikely(!w))
        lw->inv_weight = WMULT_CONST;
    else
        lw->inv_weight = WMULT_CONST / w;
}

static void update_min_vruntime(struct cfs_rq *cfs_rq)
{
    struct sched_entity *curr = cfs_rq->curr;
    struct rb_node *leftmost = cfs_rq->rb_leftmost;

    u64 vruntime = cfs_rq->min_vruntime;

    if (curr) {
        if (curr->on_rq)
            vruntime = curr->vruntime;
        else
            curr = NULL;
    }

    if (leftmost) {
        struct sched_entity *se;
        se = rb_entry(leftmost, struct sched_entity, run_node);

        if (!curr)
            vruntime = se->vruntime;
        else
            vruntime = min_vruntime(vruntime, se->vruntime);
    }

    cfs_rq->min_vruntime = max_vruntime(cfs_rq->min_vruntime, vruntime);
}

static inline s64 entity_key(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
    return se->vruntime - cfs_rq->min_vruntime;
}

static void __enqueue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
    struct rb_node **link = &cfs_rq->tasks_timeline.rb_node;
    struct rb_node *parent = NULL;
    struct sched_entity *entry;
    int leftmost = 1;

    while (*link) {
        parent = *link;
        entry = rb_entry(parent, struct sched_entity, run_node);
        if (entity_key(cfs_rq, se) < entity_key(cfs_rq, entry)) {
            link = &parent->rb_left;
        } else {
            link = &parent->rb_right;
            leftmost = 0;
        }
    }

    if (leftmost)
        cfs_rq->rb_leftmost = &se->run_node;

    rb_link_node(&se->run_node, parent, link);
    rb_insert_color(&se->run_node, &cfs_rq->tasks_timeline);
}

static void __dequeue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
    if (cfs_rq->rb_leftmost == &se->run_node) {
        struct rb_node *next_node;

        next_node = rb_next(&se->run_node);
        cfs_rq->rb_leftmost = next_node;
    }

    rb_erase(&se->run_node, &cfs_rq->tasks_timeline);
}

static struct sched_entity *__pick_first_entity(struct cfs_rq *cfs_rq)
{
    struct rb_node *left = cfs_rq->rb_leftmost;

    if (!left)
        return NULL;

    return rb_entry(left, struct sched_entity, run_node);
}

static struct sched_entity *__pick_next_entity(struct sched_entity *se)
{
    struct rb_node *next = rb_next(&se->run_node);

    if (!next)
        return NULL;

    return rb_entry(next, struct sched_entity, run_node);
}

static struct task_struct *pick_next_task_fair(struct rq *rq, struct task_struct *prev)
{
    struct cfs_rq *cfs_rq = &rq->cfs;
    struct sched_entity *se;
    struct task_struct *p;

    if (!cfs_rq->nr_running)
        return NULL;

    if (prev && prev->sched_class == &fair_sched_class) {
        struct sched_entity *pse = &prev->se;

        update_curr(cfs_rq);

        if (pse->on_rq && entity_is_task(pse)) {
            se = __pick_first_entity(cfs_rq);
            if (se && entity_key(cfs_rq, pse) <= entity_key(cfs_rq, se))
                return prev;
        }

        put_prev_entity(cfs_rq, pse);
    }

    se = pick_next_entity(cfs_rq, NULL);
    if (!se)
        return NULL;

    set_next_entity(cfs_rq, se);

    p = task_of(se);

    return p;
}

static struct sched_entity *pick_next_entity(struct cfs_rq *cfs_rq, struct sched_entity *curr)
{
    struct sched_entity *left = __pick_first_entity(cfs_rq);
    struct sched_entity *se;


    if (!left || (curr && entity_before(curr, left)))
        left = curr;

    se = left;

        if (cfs_rq->skip == se) {
        struct sched_entity *second;

        if (se == curr) {
            second = __pick_first_entity(cfs_rq);
        } else {
            second = __pick_next_entity(se);
            if (!second || (curr && entity_before(curr, second)))
                second = curr;
        }

        if (second && wakeup_preempt_entity(second, left) < 1)
            se = second;
    }

        if (cfs_rq->last && wakeup_preempt_entity(cfs_rq->last, left) < 1)
        se = cfs_rq->last;

        if (cfs_rq->next && wakeup_preempt_entity(cfs_rq->next, left) < 1)
        se = cfs_rq->next;

    clear_buddies(cfs_rq, se);

    return se;
}

static void set_next_entity(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
    if (se->on_rq) {
            update_stats_wait_end(cfs_rq, se);
        __dequeue_entity(cfs_rq, se);
    }

    update_stats_curr_start(cfs_rq, se);
    cfs_rq->curr = se;


    if (schedstat_enabled() && rq_of(cfs_rq)->load.weight >= 2*se->load.weight) {
        schedstat_set(se->statistics.slice_max,
                      max(se->statistics.slice_max,
                          se->sum_exec_runtime - se->prev_sum_exec_runtime));
    }

    se->prev_sum_exec_runtime = se->sum_exec_runtime;
    se->exec_start = sched_clock_cpu(cpu_of(rq_of(cfs_rq)));
}

static void put_prev_entity(struct cfs_rq *cfs_rq, struct sched_entity *prev)
{
       if (prev->on_rq)
        update_curr(cfs_rq);

    check_cfs_rq_runtime(cfs_rq);

    check_spread(cfs_rq, prev);

    if (prev->on_rq) {
        update_stats_wait_start(cfs_rq, prev);
        __enqueue_entity(cfs_rq, prev);
        update_load_avg(prev, 0);
    }
    cfs_rq->curr = NULL;
}

static void enqueue_task_fair(struct rq *rq, struct task_struct *p, int flags)
{
    struct cfs_rq *cfs_rq;
    struct sched_entity *se = &p->se;

    for_each_sched_entity(se) {
        if (se->on_rq)
            break;

        cfs_rq = cfs_rq_of(se);
        enqueue_entity(cfs_rq, se, flags);

           if (cfs_rq_throttled(cfs_rq))
            break;

        cfs_rq->h_nr_running++;

        flags = ENQUEUE_WAKEUP;
    }

    for_each_sched_entity(se) {
        cfs_rq = cfs_rq_of(se);
        cfs_rq->h_nr_running++;

        if (cfs_rq_throttled(cfs_rq))
            break;

        update_load_avg(se, 1);
        update_cfs_shares(cfs_rq);
    }

    if (!se)
        add_nr_running(rq, 1);

    hrtick_update(rq);
}

static void dequeue_task_fair(struct rq *rq, struct task_struct *p, int flags)
{
    struct cfs_rq *cfs_rq;
    struct sched_entity *se = &p->se;
    int task_sleep = flags & DEQUEUE_SLEEP;

    for_each_sched_entity(se) {
        cfs_rq = cfs_rq_of(se);
        dequeue_entity(cfs_rq, se, flags);


        if (cfs_rq_throttled(cfs_rq))
            break;

        cfs_rq->h_nr_running--;

        if (cfs_rq->load.weight) {
            se = parent_entity(se);
                   if (task_sleep && se && !throttled_hierarchy(cfs_rq))
                set_next_buddy(se);
            break;
        }
        flags |= DEQUEUE_SLEEP;
    }

    for_each_sched_entity(se) {
        cfs_rq = cfs_rq_of(se);
        cfs_rq->h_nr_running--;

        if (cfs_rq_throttled(cfs_rq))
            break;

        update_load_avg(se, 1);
        update_cfs_shares(cfs_rq);
    }

    if (!se)
        sub_nr_running(rq, 1);

    hrtick_update(rq);
}

static void enqueue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se, int flags)
{

    if (!(flags & ENQUEUE_WAKEUP) || (flags & ENQUEUE_WAKING))
        se->vruntime += cfs_rq->min_vruntime;


    update_curr(cfs_rq);
    enqueue_entity_load_avg(cfs_rq, se);
    account_entity_enqueue(cfs_rq, se);
    update_cfs_shares(cfs_rq);

    if (flags & ENQUEUE_WAKEUP) {
        place_entity(cfs_rq, se, 0);
        if (schedstat_enabled())
            enqueue_sleeper(cfs_rq, se);
    }

    update_stats_enqueue(cfs_rq, se);
    check_spread(cfs_rq, se);
    if (se != cfs_rq->curr)
        __enqueue_entity(cfs_rq, se);
    se->on_rq = 1;

    if (cfs_rq->nr_running == 1) {
        list_add_leaf_cfs_rq(cfs_rq);
        check_enqueue_throttle(cfs_rq);
    }
}

static void dequeue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se, int flags)
{

    update_curr(cfs_rq);
    dequeue_entity_load_avg(cfs_rq, se);

    update_stats_dequeue(cfs_rq, se);
    if (flags & DEQUEUE_SLEEP) {
        if (entity_is_task(se)) {
            struct task_struct *tsk = task_of(se);

            if (tsk->state & TASK_INTERRUPTIBLE)
                se->statistics.sleep_start = sched_clock_cpu(cpu_of(rq_of(cfs_rq)));
            if (tsk->state & TASK_UNINTERRUPTIBLE)
                se->statistics.block_start = sched_clock_cpu(cpu_of(rq_of(cfs_rq)));
        }
    }

    clear_buddies(cfs_rq, se);

    if (se != cfs_rq->curr)
        __dequeue_entity(cfs_rq, se);
    se->on_rq = 0;
    account_entity_dequeue(cfs_rq, se);


    if (!(flags & DEQUEUE_SLEEP))
        se->vruntime -= cfs_rq->min_vruntime;

    return_cfs_rq_runtime(cfs_rq);

    update_min_vruntime(cfs_rq);
    update_cfs_shares(cfs_rq);
}

static void place_entity(struct cfs_rq *cfs_rq, struct sched_entity *se, int initial)
{
    u64 vruntime = cfs_rq->min_vruntime;


    if (initial && sched_feat(START_DEBIT))
        vruntime += sched_vslice(cfs_rq, se);

    if (!initial) {
        ulong thresh = sysctl_sched_latency;

        if (sched_feat(GENTLE_FAIR_SLEEPERS))
            thresh >>= 1;

        vruntime -= thresh;
    }

    se->vruntime = max_vruntime(se->vruntime, vruntime);
}

static void check_preempt_wakeup(struct rq *rq, struct task_struct *p, int wake_flags)
{
    struct task_struct *curr = rq->curr;
    struct sched_entity *se = &curr->se, *pse = &p->se;
    struct cfs_rq *cfs_rq = task_cfs_rq(curr);
    int scale = cfs_rq->nr_running >= sched_nr_latency;
    int next_buddy_marked = 0;

    if (unlikely(se == pse))
        return;

    if (unlikely(throttled_hierarchy(cfs_rq_of(pse))))
        return;

    if (sched_feat(NEXT_BUDDY) && scale && !(wake_flags & WF_FORK)) {
        set_next_buddy(pse);
        next_buddy_marked = 1;
    }


    if (test_tsk_need_resched(curr))
        return;

    if (unlikely(curr->policy == SCHED_IDLE) &&
        likely(p->policy != SCHED_IDLE))
        goto preempt;


    if (unlikely(p->policy != SCHED_NORMAL) || !sched_feat(WAKEUP_PREEMPTION))
        return;

    find_matching_se(&se, &pse);
    update_curr(cfs_rq_of(se));
    BUG_ON(!pse);
    if (wakeup_preempt_entity(se, pse) == 1) {
        if (!next_buddy_marked)
            set_next_buddy(pse);
        goto preempt;
    }

    return;

preempt:
    resched_curr(rq);

    if (unlikely(!se->on_rq || curr == rq->idle))
        return;

    if (sched_feat(LAST_BUDDY) && scale && entity_is_task(se))
        set_last_buddy(se);
}

static void yield_task_fair(struct rq *rq)
{
    struct task_struct *curr = rq->curr;
    struct cfs_rq *cfs_rq = task_cfs_rq(curr);
    struct sched_entity *se = &curr->se;

    if (unlikely(rq->nr_running == 1))
        return;

    clear_buddies(cfs_rq, se);

    if (curr->policy != SCHED_BATCH) {
        update_rq_clock(rq);

        update_curr(cfs_rq);

        rq_clock_skip_update(rq, true);
    }

    set_skip_buddy(se);
}

const struct sched_class fair_sched_class = {
    .next                   = &idle_sched_class,
    .enqueue_task           = enqueue_task_fair,
    .dequeue_task           = dequeue_task_fair,
    .yield_task             = yield_task_fair,
    .yield_to_task          = yield_to_task_fair,

    .check_preempt_curr     = check_preempt_wakeup,

    .pick_next_task         = pick_next_task_fair,
    .put_prev_task          = put_prev_task_fair,

    .set_curr_task          = set_curr_task_fair,
