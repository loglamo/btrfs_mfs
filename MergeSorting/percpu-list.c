/* Per-cpu list adopted from 2016 Hewlett-Packard Enterprise Development LP, Authors: Waiman long
 * Currently modified by Lan Anh Nguyen
 */
#define _GNU_SOURCE
#include "percpu-list.h"
#include <linux/lockdep.h>

static struct lock_class_key percpu_list_key;

// Initialize per-cpu list head 

static inline int  init_pcpu_list_head(struct pcpu_list_head **ppcpu_head) {
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
     node->lockptr = &myhead->lock;
     list_add(&node->list, &myhead->list);
     spin_unlock(&myhead->lock);
     preempt_enable();
 }
// La: Add list_head struct to a per-cpu list 
static inline void pcpu_list_add_v2(struct pcpu_list_head *node, struct pcpu_list_head *head) {
    struct pcpu_list_head *myhead;
    preempt_disable();
    myhead = this_cpu_ptr(head);
    list_add(&node->list, &myhead->list);
    preempt_enable();
}

// Delete entry of a per-cpu list

static inline void pcpu_list_del(struct pcpu_list_node *node) {
    spinlock_t *lock = READ_ONCE(node->lockptr);
    if (unlike(!lock)) {
        WARN(1, "pcpu_list_del: node 0x%lx has no associated lock\n", (unsigned long)node);
        return;
    }

    spin_lock(lock);
    if (likely(lock == node->lockptr)) {
        list_del_init(&node->list);
        node->lockptr = NULL;
    } else {
        WARN_ON(1);
    }
    spin_unlock(lock);
}

// La: Get global list from per-cpu lists
/*
static inline void pcpu_list_(struct pcpu_list_head*) {
       int cpu;
       spin_lock(&pcpu_list_head->lock);
       // La: Iterate lists on per-cpu lists
       for_each_possible_cpu(cpu) {
           (!list_empty(&per_cpu_ptr(pcpu_list_head, cpu)->list)) {
               get_
           }
       }
}*/
