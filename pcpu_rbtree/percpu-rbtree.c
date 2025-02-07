/* Per-cpu red-black tree by Lan Anh Nguyen
 * 
 */

#define _GNU_SOURCE
#include "./percpu-rbtree.h"
#include <linux/kernel.h>
#include <linux/stddef.h>
#include <linux/percpu.h>
#include <linux/rbtree_types.h>
#include <linux/rbtree.h>


int init_pcpu_rbroot(struct rb_root **ppcpu_tree) {
          struct rb_root *pcpu_tree = alloc_percpu(struct rb_root);
          int cpu;
                    
          if (!pcpu_tree)
              return -ENOMEM;
                          
          for_each_possible_cpu(cpu) {
               struct rb_root *root = per_cpu_ptr(pcpu_tree, cpu);
               *root = RB_ROOT;
           }   
          *ppcpu_tree = pcpu_tree;
          printk("Did initialization of per-core RB-root\n");
          return 0;
}
// Add entry to the per-cpu list
/*
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
*/
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

