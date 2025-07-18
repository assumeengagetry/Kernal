/*
 * sched.c - 单核简化调度器，无CFS
 */

#include "../../include/sched.h"
#include "../../include/types.h"
#include "../../include/mm.h"

static struct task_struct *current_task = NULL;

void sched_init(void) { current_task = NULL; }

struct task_struct *get_current(void) { return current_task; }
void set_current(struct task_struct *task) { current_task = task; }

struct task_struct *alloc_task_struct(void)
{
    struct task_struct *task = kmalloc(sizeof(struct task_struct), 0);
    if (!task) return NULL;
    task->pid = 1;
    task->state = TASK_RUNNING;
    return task;
}

void free_task_struct(struct task_struct *tsk)
{
    if (tsk) kfree(tsk);
}

void wake_up_new_task(struct task_struct *p) { p->state = TASK_RUNNING; }

void schedule(void) { /* 单核，无需切换 */ }

void yield(void) { schedule(); }