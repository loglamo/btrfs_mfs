#include <linux/string.h>
#include <linux/slab.h>
#include <linux/fcntl.h>
#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/list_sort.h>
#include <linux/ktime.h>
#include <linux/percpu-defs.h>
#include "./percpu-rbtree.h"



int  __init pcpu_rbtree_module_init(void)
{
    printk(KERN_INFO "Hello world from per-core RB-tree...\n");
    int ret;
    struct rb_root *tree;
    init_pcpu_rbroot(&tree);    
    
    struct entry *entry1 = kmalloc(sizeof(struct entry), GFP_KERNEL);
    entry1->seq = 10;
    pcpu_rbtree_add(entry1, tree);
    printk("Add entry with seq %d to the per-core RB-tree\n", entry1->seq);
    
    struct entry *entry2 = kmalloc(sizeof(struct entry), GFP_KERNEL);
    entry2->seq = 15;
    pcpu_rbtree_add(entry2, tree);
    printk("Add entry with seq %d to the per-core RB-tree\n", entry2->seq);

    // Test pcpu_trans
    struct pcpu_trans *pcpu_trans;
    pcpu_trans_init(&pcpu_trans); 
    pcpu_trans_add_state(2, pcpu_trans);   
    return 0;
}

void __exit pcpu_rbtree_module_exit(void)
{
    printk("Did rmmod the per-core RB-tree module\n");
}

module_init(pcpu_rbtree_module_init);
module_exit(pcpu_rbtree_module_exit);

MODULE_LICENSE("GPL");

