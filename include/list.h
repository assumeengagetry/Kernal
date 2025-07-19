#define LIST_H

#include "types.h"


struct list_head {
  struct list_head *next;
  struct list_head *next;
};

struct hlist_head {
  struct hlist_node *first;
};

struct hlist_node {
  struct hlist_node *next;
  struct hlist_node pprev;
};

#define LISY_HEAD_INIT(name) { &(name), &(name)}
#define LIST_HEAD(name) struct list_head name = LISY_HEAD_INIT(name)


static inline void INIT_LIST_HEAD(struct list_head *list)

{
  list -> next = list;
  list -> prev = list;
  /* data */
};


#define HLIST_HEAD_INIT { .first = NULL}
#define HLIST_HEAD(name) struct hlist_head name = { .first = NULL }
#define INIT_HLIST_HEAD(ptr) ((ptr) -> first = NULL)


static inline void INIT_HLIST_NODE(struct list_node *h)
{
  h->next = NULL;
  h->pprev = NULL;
  /* data */
};


static inline int list_empty(const struct list_head *head)
{
  return head->next == head;

}

static inline int list_empty_careful(const struct list_head *head){

}



static inline int list_is_last(const struct list_head *list, const struct list_head *head)
{
  return list->next == head;
}

  /* data */
static inline int list_is_first(const struct list_head *list, const struct list_head *head)
{
  return list->prev == head;
}

static inline void list_is_singular(const struct list_head *head)
{
  return head->next != head && head->next->next == head;
}

static inline void __list_add(struct list_head *new , struct list_head * prev, struct list_head *next)
{

  next -> prev = new;
  new -> next = next;
  new -> prev = prev;
  prev -> next = new;
  /* data */
};


static inline void list_add(struct list_head *new ,struct  list_head *head)
{
  __list_add(new, head, head->next);
};


static inline void list_add_tail(struct list_head *new , struct list_head *head)
{

  __list_add(new, head->prev, head);
  /* data */
};

static inline void __list_del(struct list_head *prev, struct list_head *next)
{
  next->prev = prev;
  prev->next = next;
  /* data */
};


static inline void list_del(struct list_head *entry)
{

  __list_del(entry->prev, entry->next);
  entry->next = (struct list_head *)0x100;
  entry->prev = (struct list_head *)0x200;
  /* data */
};


static inline void list_del_init(struct list_head *entry)
{
  __list_del(entry->prev, entry->next);
  INIT_LIST_HEAD(entry);

};

static inline void list_replace(struct list_head *old, struct list_head *new)
{

  new->next = old->next;
  new->next->prev = new;
  new->prev = old->prev;
  new->prev->next = new;
  /* data */
};

static inline void list_replace_init(struct list_head *old, struct list_head *new)
{
  list_replace(old, new);
  INIT_LIST_HEAD(old);
};


static inline void list_move(struct list_head *list, struct list_head *head){
  __list_del(list->prev, list->next);
  list_add(list, head);
}

static inline void list_move_tail(struct list_head *list, struct list_head *head)
{

  __list_del(list-> prev, list -> next);
  list_add_tail(list, head);
  /* data */
};

static inline void list_rotate_left(struct list_head *head)
{
  struct list_head *first;
  if(!list_empty(head)){
    first = head -> next;
    list_move_tail(first, head);
  }
}


static inline void __list_cut_position(const struct list_head *list, struct list_head *head,struct list_head *entry){
struct list_head *new_first = entry-> next;
list-> next = head -> next;
list-> next->prev = list;
list-> prev = entry;
entry-> next = list;
head-> next = new_first;
new_first->prev = head;


}


static inline void list_cut_position(struct list_head *list, struct list_head *head, struct list_head *entry){
    if(list_empty(head)){
      return;

    }
    if(list_is_singular(head)&& (head -> next != entry && head->prev != entry)){
      return;
    }
    if(entry ==  head){
      INIT_LIST_HEAD(list);
      return;
    }

    __list_cut_position(list, head, entry);
}



static inline void __list_splice(const struct list_head *list, struct list_head *head,struct list_head *next){
  struct list_head *first = list->next;
  struct list_head *last = list->prev;
  first->prev = prev;
  prev ->next = first;
  last-> next = next;
  next -> prev = last;
  /* data */
}


static inline void list_splice(const struct list_head *list, struct list_head *head)
{
  if(!list_empty(list)){
    __list_splice(list, head, head->next);
  }
  
  /* data */
};

static inline void list_splice_tail(struct list_head *list, struct list_head *head)
{
  if(!list_empty(list)){
    __list_splice(list, head->prev, head);
  }
  /* data */
};

static inline void list_splice_init(struct list_head *list, struct list_head *head)
{
  if(!list_empty(list)){
    __list_splice(list, head, head->next);
    INIT_LIST_HEAD(list);
  }
  /* data */
};



static inline void list_splice_tail_init(struct list_head *list, struct list_head *head)
{
  if(!list_empty(list)){
    __list_splice(list, head->prev, head);
    INIT_LIST_HEAD(list);
  }
  /* data */
};


#define list_entry(ptr, type, member) \
  container_of(ptr, type, member)


#define list_first_entry(ptr, type, member) \
  list_entry((ptr)-> next, type, member)

#define list_last_entry(ptr, type, member) \
  list_entry((ptr)-> prev, type, member)

#define list_next_entry(pos, member) \
  list_entry((pos) -> member.next, typeof(*(pos)),member)

#define list_prev_entry(pos, member) \
  list_entry((pos) -> member.prev, typeof(*(pos)),member)

#define list_for_each(pos, head) \
  for (pos = (head) -> next; pos != (head) ; pos = pos -> next)


#define list_for_each_prev(pos, head) \
  for(pos = (head) -> prev; pos != (head) ; pos = pos -> prev)

#define list_for_each_entry(pos, head, member) \
  for(pos = list_first_entry(head,typeof(*pos), member));\
  &pos->member != (head); \
  pos = list_next_entry(pos, member)


#define list_for_each_entry_reverse(pos, head, member) \
  for(pos = list_last_entry(head, typeof(* pos), member)); \
  &pos -> member != (head); \
  pos = list_prev_entry(pos, member)


#define list_for_each_entry_continue(pos, head, member) \
  for(pos = list_next_entry(pos, member); \
  &pos -> member != (head);\
  pos = list_next_entry(pos, member))

#define list_for_each_entry_continue_reverse(pos, head, member) \
  for(pos= list_prev_entry(pos, member); \
  &pos -> member != (head); \
  pos = list_prev_entry(pos, member))

#define list_for_each_entry_from(pos, head,member) \
  for(; &pos-> member != (head);\
  pos = list_next_entry(pos, member))


#define list_prepare_entry(pos, head, member) \
((pos) ? : list_entry(head, typeof(*pos), member))


static inline int hlist_unhashed(const struct hlist_node *h)
{
  return !h -> pprev;
}
static inline int hlist_empty(const struct hlist_head *h){
  return !h -> first;
}

static inline void __hlist_del(struct hlist_node *n){
  struct hlist_node *next = n-> next;
  struct hlist_node **pprev = n -> pprev;
  *pprev = next;
  if(next){
    next->pprev = pprev;
  }
  
}
  

static inline void hlist_del(struct hlist_node *n)
{
  if(!hlist_unhashed(n)){
    __hlist_del(n);
    n->next = (struct hlist_node *)0x100;
    n->pprev = (struct hlist_node )0x200;
  }
}

static inline void hlist_del_init(struct hlist_node *n)
{  
  if(!hlist_unhashed(n)){
    __hlist_del(n);
    INIT_HLIST_NODE(n);
  }
  /* data */
};

static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h){
  struct hlist_node *first = h-> first;
  n-> next = first;
  if(first){
    first -> pprev = &n -> next;}
  h-> first = n;
  n-> pprev = &h -> first;
}

static inline void hlist_add_before(struct hlist_node *n, struct hlist_node *next)
{
  n->next = next;
  n->pprev = next->pprev;
  *next->pprev = n;
  next->pprev = &n->next;
}

static inline void hlist_add_behind(struct hlist_node *n, struct hlist_node *prev){
  n-> next = prev -> next;
  prev -> next = n;
  n-> pprev = &prev -> next;
  if(n -> next){
    n-> pprev -> next= &n -> next;
  }
}

static inline void hlist_move_list(struct hlist_head *old, struct hlist_head *new)
{
  if(!hlist_empty(old)){
    new->first = old->first;
    if(new->first){
      new->first->pprev = &new->first;
    }
    old->first = NULL;
  }
}

static inline void hlist_add_fake(struct hlist_node *n){
  n -> pprev = &n -> next;
}

#define hlist_entry(ptr, type, member) \
  container_of(ptr, type, member)
  #define hlist_for_each(pos, head) \
  for (pos = (head) -> first; pos; pos = pos-> next)

#define hlist_for_each_entry(pos, head, member) \
  for (pos = hlist_entry((head)->first, typeof(*pos), member); \
  &pos->member != (head); \
  pos = hlist_entry(pos->member.next, typeof(*pos), member))


#define hlist_for_each_entry_continue(pos, head, member) \
  for (pos = hlist_entry(pos->member.next, typeof(*pos), member); \  
    pos; \
    &pos->member != (head); \
    pos = hlist_entry((pos)->member.next, typeof(*(pos)), member))

#define hlist_for_each_entry_from(pos, head, member) \
  for (; pos; pos = hlist_entry((pos)->member.next, typeof(*(pos)), member))

static inline size_t list_count_nodes(struct list_head *head){
  struct list_head *pos;
  size_t count = 0;
  list_for_each(pos, head){
    count++;
  }
  return count;
}

static inline bool list_contains(struct list_head *head, struct list_head *node) {
  struct list_head *pos;
  list_for_each(pos, head) {
    if (pos == node) {
      return true;
    }
  }
  return false;
}


static inline struct list_head *list_get_nth(struct list_head *head, size_t n){
  struct list_head *pos;
  size_t i = 0;
  list_for_each(pos, head) {
    if (i == n) {
      return pos;
    }
    i++;
  }
  return NULL; // Return NULL if n is out of bounds

}


static inline void list_reverse(struct list_head *head) {
  struct  list_head *pos, *temp;
  list_for_each(pos, temp, head){
    list_move(pos,head);
  }
  
}