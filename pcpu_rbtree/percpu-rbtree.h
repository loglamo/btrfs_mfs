/* Per-cpu red-black tree by Lan Anh Nguyen
 * 
 */

#define _GNU_SOURCE
#ifndef PERCPU_RBTREE_H
#define PERCPU_RBTREE_H

#include <linux/kernel.h>
#include <linux/stddef.h>
#include <linux/percpu.h>
#include <linux/rbtree_types.h>
#include <linux/rbtree.h>

struct entry {
    int seq;
    struct rb_node rb_node;
};

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

int pcpu_rbtree_add(struct entry *new_rbnode, struct rb_root *pcpu_tree) {
     struct rb_root *current_tree;
     
     preempt_disable();
     current_tree = this_cpu_ptr(pcpu_tree);
     struct rb_node **p = &current_tree->rb_node;
     struct rb_node *parent = NULL;
     struct entry *temp_entry;
     while (*p) {
         parent = *p;
         temp_entry = rb_entry(parent, struct entry, rb_node);

         if (new_rbnode->seq < temp_entry->seq)
             p = &(*p)->rb_left;
         else 
             p = &(*p)->rb_right;
     }
     rb_link_node(&new_rbnode->rb_node, parent, p);
     rb_insert_color(&new_rbnode->rb_node, current_tree);
     preempt_enable();

     return 0;
}

struct pcpu_trans {
    int id;
    int state;
    struct rb_root tree;
    struct list_head list;
};

int pcpu_trans_init (struct pcpu_trans **ppcpu_trans) {
    struct pcpu_trans *pcpu_trans = alloc_percpu(struct pcpu_trans);
    int cpu;

    if (!pcpu_trans)
        return -ENOMEM;
    for_each_possible_cpu(cpu) {
        struct pcpu_trans *trans = per_cpu_ptr(pcpu_trans, cpu);
        trans->id = 0;
        trans->state = 0;
        trans->tree = RB_ROOT;
        INIT_LIST_HEAD(&trans->list);
    }
    *ppcpu_trans = pcpu_trans;
   return 0; 
}

int pcpu_trans_add_state (int state, struct pcpu_trans *pcpu_trans) {
    struct pcpu_trans *current_pcpu_trans;
    preempt_disable();
    current_pcpu_trans = this_cpu_ptr(pcpu_trans);
    current_pcpu_trans->state = state;
    printk(KERN_INFO "Already changed state to %d for transaction %d\n", current_pcpu_trans->state, current_pcpu_trans->id);
    preempt_enable();

    return 0;
};
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
#endif /* __PERCPU_RBTREE_H */
