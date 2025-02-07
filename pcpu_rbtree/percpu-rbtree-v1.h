/* Per-cpu rbtree adopted from 2016 Hewlett-Packard Enterprise Development LP, Authors: Waiman Long
 * Currently modified by Lan Anh Nguyen 
 * */

#ifndef PERCPU_RBTREE_H
#define PERCPU_RBTREE_H

#include <linux/kernel.h>
#include <linux/stddef.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/percpu.h>
#include <linux/ktime.h>
#include <linux/rbtree_types.h>
#include <linux/rbtree.h>



 
int init_pcpu_rbroot(struct rb_root**);


#endif /* __PERCPU_RBTREE_H */
