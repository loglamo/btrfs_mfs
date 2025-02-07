/* Lan Anh Nguyen:
 * Lockfree ring buffer
 * v1: Implement ring_buffer using pure poiters without locking by using Compare-And-Swap
 * v2: Implement ring_buffer using list_head struct provided by <linux/list.h> with lock for synchronization
 * v3: Implement ring_buffer using list_head without locking by using atomic operations
*/

#include <linux/spinlock.h>
#include <linux/list.h>


struct entry_data {
    u64 seq;
    u64 offset;
    u64 length;
};

struct rb_entry {
    struct rb_entry *next_entry;
    struct entry_data *payload;
    int should_free;
};

// New version of entry with list_head
struct rb_entry_v2 {
    struct list_head entry_list;
    struct entry_data payload;
};

struct lf_queue_entry_v2 {
    struct list_head entry_list;
    struct entry_data payload;
    atomic_t removed;
    struct list_head gc_list;
};

struct ring_buffer_v2 {
    int id;
    int CPU_id;
    u64 capacity;
    u64 count;
    struct list_head rb_entry_head;
    struct list_head rb_entry_tail;
    struct list_head LIST;
    spinlock_t buffer_lock;
};

struct lf_queue_v2 {
    int id;
    u64 capacity;
    atomic_t count;
    struct list_head rb_entry_head;
    struct list_head rb_entry_tail;
    struct list_head LIST;
    atomic_t removed;
    struct list_head GC;
};

struct ring_buffer_lf {
    int id;
    int CPU_id;
    u64 capacity;
    void *buffer;
    void *buffer_end;
    void *rb_entry_head;
    void *rb_entry_tail;
    atomic_t count;
    struct rb_entry *LIST;

};

// Initialization for locked buffer
void ring_buffer_init_v2(struct ring_buffer_v2 *rb, int id, u64 capacity) {
        rb->id = id;
        INIT_LIST_HEAD(&rb->rb_entry_tail);
        INIT_LIST_HEAD(&rb->LIST);
        rb->rb_entry_head = rb->LIST;
        rb->count = 0;
        rb->capacity = capacity;
        spin_lock_init(&rb->buffer_lock);
        printk(KERN_INFO "Initialize sucessfully the  ring_buffer %d\n", rb->id);
}
// Init list head for lf
void INIT_LF_LIST_HEAD(struct list_head *list) {
    list->next = NULL;
    list->prev = list;
}

void lf_queue_init_v2(struct lf_queue_v2 *rb, int id, u64 capacity) {
        rb->id = id;
        INIT_LF_LIST_HEAD(&rb->rb_entry_tail);
        INIT_LF_LIST_HEAD(&rb->rb_entry_head);
        INIT_LF_LIST_HEAD(&rb->LIST);
        INIT_LF_LIST_HEAD(&rb->GC);
        atomic_set(&rb->count, 0);
        rb->capacity = capacity;
        printk(KERN_INFO "Initialize sucessfully the queue %d\n", rb->id);
}

void lf_list_add_tail(struct list_head *entry, struct list_head *head, struct list_head *tail) {
    entry->prev = __sync_lock_test_and_set(&tail, entry);
    if (entry->prev == NULL)
        head = entry;
    else
        entry->prev->next = entry;
}

void lf_enqueue_entry_v2(struct lf_queue_v2 *rb, struct lf_queue_entry_v2 *new_entry) { 
    if (atomic_read(&rb->count) < rb->capacity) {
        lf_list_add_tail(&new_entry->entry_list, &rb->rb_entry_head, &rb->rb_entry_tail);
        atomic_inc(&rb->count);
    } else {
      //  printk(KERN_INFO "Can not enqueue new entry, exceed to the capacity!!!\n");
    }
}

void lf_list_add_tail_gc(struct list_head *entry, struct list_head *list) {
    entry->prev = __sync_lock_test_and_set(&list->prev, entry);
    if (entry->prev == NULL)
        list = entry;
    else
        entry->prev->next = entry;
}

static inline void __gc_list_del(struct list_head *prev, struct list_head *next, struct list_head *list) {
    if (next)
        next->prev = prev;
    else
        list->prev = prev;
    prev->next = next;
}

void gc_list_del(struct list_head *entry, struct list_head *list) {
    __gc_list_del(entry->prev, entry->next, list);
}


struct entry_data* lf_dequeue_entry_v2(struct lf_queue_v2 *rb) {
    struct entry_data *payload = NULL;
    if (&rb->rb_entry_head != NULL) {
        struct lf_queue_entry_v2 *entry = container_of(&rb->rb_entry_head, struct lf_queue_entry_v2, entry_list);
        atomic_set(&entry->removed, 1);
        lf_list_add_tail_gc(&entry->gc_list, &rb->GC);
        atomic_dec(&rb->count);
        payload = &entry->payload;
        gc_list_del(&entry->entry_list, &rb->LIST);
        gc_list_del(&entry->gc_list, &rb->GC);
     } else 
        printk(KERN_INFO "Can not dequeue, queue is empty!!!\n");
     return payload;
}


void ring_buffer_init(struct ring_buffer_lf *rb, int id, u64 capacity) {
        rb->id = id;
        rb->buffer = kmalloc(capacity*(sizeof(struct rb_entry)), GFP_KERNEL);
        rb->rb_entry_head = rb->LIST;
        rb->rb_entry_tail = NULL;
        atomic_set(&rb->count, 0);
        rb->capacity = capacity;
        //rb->LIST = kmalloc(sizeof(struct rb_entry), GFP_KERNEL);       
        rb->LIST = rb->buffer;       
        printk(KERN_INFO "Initialize sucessfully the  ring_buffer %d\n", rb->id);
}



//free one ring buffer with lock
void rb_free_v2(struct ring_buffer_v2 *rb) {
        spin_lock(&rb->buffer_lock);
        list_del_init(&rb->LIST);
        rb->count = 0;
        spin_unlock(&rb->buffer_lock);
}
// Enqueue with lock
void enqueue_rb_entry_v2(struct ring_buffer_v2 *rb, struct rb_entry_v2 *new_entry) { 
    spin_lock(&rb->buffer_lock);
    if (rb->count < rb->capacity) {
        list_add_tail(&new_entry->entry_list, &rb->LIST);
        rb->rb_entry_tail = new_entry->entry_list;
        rb->count++;
        spin_unlock(&rb->buffer_lock);
    } else {
        spin_unlock(&rb->buffer_lock);
        printk(KERN_INFO "Exceed to the capacity!!!\n");
    }
}

// Print info with lock
void print_info_ring_buffer_v2(struct ring_buffer_v2 *rb) {
        u64 ring_buffer_id = rb->id;
        struct rb_entry_v2 *entry;
        printk(KERN_INFO "Entries in the ring buffer %d are:\n", ring_buffer_id);
        list_for_each_entry(entry, &rb->LIST, entry_list) {
            printk(KERN_INFO "Entry seq is: %d \n", entry->payload.seq);
            printk(KERN_INFO "Entry offset is: %d \n", entry->payload.offset);
        }
        printk("Count of the ring buffer is %u \n", rb->count);
}

/*
void print_info_lf_ring_buffer_v2(struct lf_ring_buffer_v2 *rb) {
        u64 ring_buffer_id = rb->id;
        struct lf_rb_entry_v2 *entry;
        printk(KERN_INFO "Entries in the ring buffer %d are:\n", ring_buffer_id);
        list_for_each_entry(entry, &rb->LIST, entry_list) {
            printk(KERN_INFO "Entry seq is: %d \n", entry->payload.seq);
            printk(KERN_INFO "Entry offset is: %d \n", entry->payload.offset);
        }
        printk("Count of the ring buffer is %llu \n", atomic_read(&rb->count));
}
*/

// Dequeue with lock
struct rb_entry_v2*  dequeue_rb_entry_v2(struct ring_buffer_v2 *rb) {
    struct rb_entry_v2 *entry;
    spin_lock(&rb->buffer_lock);
    entry = list_first_entry_or_null(&rb->LIST, struct rb_entry_v2, entry_list);
    if (entry == NULL)
        printk(KERN_INFO "Can not dequeue, queue is empty!!!\n");
    list_del(&entry->entry_list);
    rb->count--;
    spin_unlock(&rb->buffer_lock);

    return entry;
}



void enqueue_rb_entry_lf(struct ring_buffer_lf *rb, struct entry_data *new_data) {
        struct rb_entry *new_entry = kmalloc(sizeof(struct rb_entry), GFP_KERNEL);
        new_entry->payload = new_data;
        struct rb_entry *old_next = kmalloc(sizeof(struct rb_entry), GFP_KERNEL);
        
        while(1) {
          
          new_entry->next_entry = rb->LIST->next_entry;
          old_next = rb->LIST->next_entry;
          
          if (__sync_bool_compare_and_swap(&rb->LIST->next_entry, old_next, new_entry)) {
             printk(KERN_INFO "add entry sucessfully to ring_buffer %d\n", rb->id);
             atomic_inc(&rb->count); 
             break;
           } 
        }
}

void print_info_ring_buffer(struct ring_buffer_lf *rb) {
        struct rb_entry *temp = rb->LIST->next_entry;
        u64 ring_buffer_id = rb->id;
        printk(KERN_INFO "Entries in the ring buffer %d are:\n", ring_buffer_id);
        while(temp != NULL) {
                printk(KERN_INFO "Seq: %lu\n", temp->payload->seq);
                printk(KERN_INFO "Offset: %lu\n", temp->payload->offset);
                printk(KERN_INFO "Length: %lu\n", temp->payload->length);
                temp = temp->next_entry;
           }
        printk("Count of the ring buffer is %u \n", atomic_read(&rb->count));
}


// Mark the entry which should be dequeued firstly according to FIFO
/*
void dequeue_rb_entry_lf(struct ring_buffer_lf *rb) {
    struct rb_entry *temp = rb->LIST->next_entry;
    struct rb_entry *temp1;
    printk(KERN_INFO "The head entry is with \n");
    while(temp != NULL) {
        temp1 = temp;
        temp = temp->next_entry;
        if (temp == NULL) {
          temp1->should_free = 1;
          printk(KERN_INFO "Seq: %lu\n", temp1->payload->seq);
        } 
    }
}
*/

