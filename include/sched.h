/*
 * sched.h - 单核极简调度器定义
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include "types.h"

#define TASK_RUNNING 0

struct task_struct {
    long state;
    int pid;
};

extern struct task_struct *get_current(void);
extern void set_current(struct task_struct *task);
extern struct task_struct *alloc_task_struct(void);
extern void free_task_struct(struct task_struct *tsk);
extern void sched_init(void);
extern void schedule(void);
extern void yield(void);
extern void wake_up_new_task(struct task_struct *p);

#define current get_current()

#endif /* __SCHED_H__ */

