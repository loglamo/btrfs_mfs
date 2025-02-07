

#include <linux/string.h>
#include <linux/slab.h> // For kmalloc()
#include <linux/fcntl.h>
#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/list_sort.h>
#include <linux/ktime.h>
#include <linux/percpu-defs.h>
#include <linux/kthread.h>

#include "./ring_buffer.c"

#define ENQUEUE_THREAD_NUM 10
#define DEQUEUE_THREAD_NUM 3
#define BUF_CAPACITY 100

struct task_struct *enqueue_workers[ENQUEUE_THREAD_NUM];
struct task_struct *dequeue_workers[DEQUEUE_THREAD_NUM];

struct parameter_fn {
    struct ring_buffer_v2 *rb;
    struct rb_entry_v2 *entry;
};

struct lf_parameter_fn {
    struct lf_queue_v2 *rb;
    struct lf_queue_entry_v2 *entry;
};

struct lf_parameter_dequeue_fn {
    struct lf_queue_v2 *rb;
};

int enqueue_fn(void *input) {
    struct lf_parameter_fn *parameter = (struct lf_parameter_fn*) input;
    struct lf_queue_v2 *rb = parameter->rb;
    struct lf_queue_entry_v2 *new_entry = parameter->entry;
    lf_enqueue_entry_v2(rb, new_entry);
    printk(KERN_INFO "Entry with seq %d was inserted ...\n", new_entry->payload.seq);
    return 0;
}

int dequeue_fn(void *input) {
    struct lf_parameter_fn *parameter = (struct lf_parameter_fn*) input;
    struct lf_queue_v2 *rb = parameter->rb;
    struct entry_data *data; 
    data = lf_dequeue_entry_v2(rb);
    printk(KERN_INFO "Entry was dequeued with seq %d...\n", data->seq);
    return 0;
}


int  __init ring_buffer_module_init(void)
{
    printk(KERN_INFO "Hello world from Queue...\n");
    printk(KERN_INFO "--------------------------------\n");
    int ret;
    int id = 5;
    struct lf_queue_v2 rb;
    lf_queue_init_v2(&rb, id, BUF_CAPACITY);

    // Sequential testing
    /*
    for (int i = 0; i < THREAD_NUM; i++) {
        struct rb_entry_v2 *entryI = kmalloc(sizeof(struct rb_entry_v2), GFP_KERNEL);
        entryI->payload.seq = i;
        entryI->payload.offset = i+1;
        enqueue_rb_entry_v2(&rb, entryI);
        printk(KERN_INFO "Added entry %d to the queue\n", i);
    }
    print_info_ring_buffer_v2(&rb);
    struct rb_entry_v2 *first_entry = dequeue_rb_entry_v2(&rb);
    printk("------------------------------\n");
    printk("The dequeued entry has seq: %d \n", first_entry->payload.seq);
    printk("------------------------------\n");
    print_info_ring_buffer_v2(&rb);
    */

    for (int i = 0; i < ENQUEUE_THREAD_NUM; i++) {
        struct lf_parameter_fn *parameterI = kmalloc(sizeof(struct lf_parameter_fn), GFP_KERNEL);
        parameterI->rb = &rb;
        struct lf_queue_entry_v2 *entryI = kmalloc(sizeof(struct lf_queue_entry_v2), GFP_KERNEL); 
        entryI->payload.seq = i;
        parameterI->entry = entryI;

        enqueue_workers[i] = kthread_run(enqueue_fn, (void*) parameterI, "Enqueue kthreads");
    }
/*
    for (int i = 0; i < DEQUEUE_THREAD_NUM; i++) {
        struct lf_parameter_dequeue_fn *parameterI = kmalloc(sizeof(struct lf_parameter_dequeue_fn), GFP_KERNEL);
        parameterI->rb = &rb;

        dequeue_workers[i] = kthread_run(dequeue_fn, (void*) parameterI, "Dequeue kthreads");
    }
*/
    return 0;
}

void __exit ring_buffer_module_exit(void)
{
    printk("Did rmmod the sorting module\n");
}

module_init(ring_buffer_module_init);
module_exit(ring_buffer_module_exit);

MODULE_LICENSE("GPL");

