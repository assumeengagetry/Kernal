/*
 * mm.h - 单核简化内存管理，无buddy
 */

#ifndef __MM_H__
#define __MM_H__

#include "types.h"

static inline void *kmalloc(size_t size, int flags) { return NULL; }
static inline void kfree(void *ptr) {}

void buddy_init(void);

#endif /* __MM_H__ */