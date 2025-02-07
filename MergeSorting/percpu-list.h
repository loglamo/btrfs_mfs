/* Per-cpu list adopted from 2016 Hewlett-Packard Enterprise Development LP, Authors: Waiman Long
 * Currently modified by Lan Anh Nguyen 
 * */

#ifndef PERCPU_LIST_H
#define PERCPU_LIST_H

#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/percpu.h>
#include <linux/ktime.h>
/* some comments here
 */
static struct lock_class_key percpu_list_key;

struct pcpu_list_head {
    struct list_head list;
    spinlock_t lock;
};

// per-cpu list- node data structure 
struct pcpu_list_node {
    struct list_head list;
    spinlock_t *lockptr;
   };

// Initialize per-cpu list head
static inline int init_pcpu_list_head(struct pcpu_list_head **ppcpu_head) {
    struct pcpu_list_head *pcpu_head = alloc_percpu(struct pcpu_list_head);
    int cpu;

    if (!pcpu_head) {
        return -ENOMEM;
    }
    for_each_possible_cpu(cpu) {
        struct pcpu_list_head *head = per_cpu_ptr(pcpu_head, cpu);
        INIT_LIST_HEAD(&head->list);
        head->lock = __SPIN_LOCK_UNLOCKED(&head->lock);
        lockdep_set_class(&head->lock, &percpu_list_key);
    }
    *ppcpu_head = pcpu_head;
    return 0;
}

// Add entry to the per-cpu list
static inline void pcpu_list_add(struct pcpu_list_node *node, struct pcpu_list_head *head) {
    struct pcpu_list_head *myhead;
    // Disable the preemption to make sure that CPU won't get changed
    preempt_disable();
    myhead = this_cpu_ptr(head);
    spin_lock(&myhead->lock);
    list_add(&node->list, &myhead->list);
    spin_unlock(&myhead->lock);
    preempt_enable();
}


// Per-cpu list iteration state 
struct pcpu_list_state {
    int cpu;
    spinlock_t *lock;
    struct list_head *head;
    struct pcpu_list_node *curr;
    struct pcpu_list_node *next;
};



static inline void init_pcpu_list_state(struct pcpu_list_state *state) {
    state->cpu = -1;
    state->lock = NULL;
    state->head = NULL;
    state->curr = NULL;
    state->next = NULL;
}

#ifdef CONFIG_DEBUG_SPINLOCK
#define PERCPU_LIST_WARN_ON(x) WARN_ON(x)
#else
#define PERCPU_LIST_WARN_ON(x)
#endif

// Next per-cpu list next entry
#define pcpu_list_next_entry(pos, member) list_next_entry(pos, member.list)


#define PCPU_LIST_NODE_INIT(name)                 \
        {                                         \
            .list.prev = &name.list,              \
            .list.next = &name.list,              \
            .list.lockptr = NULL,                  \
        }

static inline void init_pcpu_list_node(struct pcpu_list_node *node) {
    INIT_LIST_HEAD(&node->list);
    node->lockptr = NULL;
}

static inline void free_pcpu_list_head(struct pcpu_list_head **ppcpu_head) {
    free_percpu(*ppcpu_head);
    *ppcpu_head = NULL;
}

// Check whether the per-cpu list is empty 
static inline bool pcpu_list_empty(struct pcpu_list_head *pcpu_head) {
    int cpu;
    for_each_possible_cpu(cpu)
        if (!list_empty(&per_cpu_ptr(pcpu_head, cpu)->list))
            return false;
    return true;
}

static bool __pcpu_list_next_cpu(struct pcpu_list_head *head, struct pcpu_list_state *state) {
           if (state->lock) {
               spin_unlock(state->lock);
           }
next_cpu:
           state->cpu = cpumask_next(state->cpu, cpu_possible_mask);
           if (state->cpu >= nr_cpu_ids) {
               return false;
           }
           state->head = &per_cpu_ptr(head, state->cpu)->list;
           if (list_empty(state->head)) {
               goto next_cpu;
           }
           state->lock = &per_cpu_ptr(head, state->cpu)->lock;
           spin_lock(state->lock);
           if (list_empty(state->head)) {
               spin_unlock(state->lock);
               goto next_cpu;
           }
           state->curr = list_entry(state->head->next, struct pcpu_list_node, list);
           return true;
}

// Iterate to the next entry of the group of per-cpu lists 
// True if the next entry is found, false if all entries iterated 
static inline bool pcpu_list_iterate(struct pcpu_list_head *head,
                                     struct pcpu_list_state *state) {
       // find next entry 
       if (state->curr) {
           state->curr = list_next_entry(state->curr, list);
       }
       if (!state->curr || (&state->curr->list == state->head)) {
           // current per-cpu list has been exhausted, try the next one
           if (!__pcpu_list_next_cpu(head, state)) {
               return false;
           }
       }
       PERCPU_LIST_WARN_ON(state->curr->lockptr != state->lock);
       return true;
}

// Iterate the next entry and safe against  the removal of list_entry 
// updated later
// Adding and deleting functions
 
void pcpu_list_add(struct pcpu_list_node*, struct pcpu_list_head*);
void pcpu_list_del(struct pcpu_list_node*);
int init_pcpu_list_head(struct pcpu_list_head**);
static inline void  pcpu_list_add_v2(struct pcpu_list_head *node, struct pcpu_list_head *head) {
                    struct pcpu_list_head *myhead;
                    preempt_disable();
                    myhead = this_cpu_ptr(head);
                    list_add(&node->list, &myhead->list);
                    preempt_enable();

}

static inline void pcpu_list_add_tail(struct pcpu_list_head *node, struct pcpu_list_head *head) {
	struct pcpu_list_head *myhead;
	preempt_disable();
	myhead = this_cpu_ptr(head);
	list_add_tail(&node->list, &myhead->list);
	preempt_enable();
}

#endif /* __LINUX_PERCPU_LIST_H */