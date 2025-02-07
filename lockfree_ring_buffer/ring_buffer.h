/* ring_buffer.h */
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <linux/list.h>
#include <linux/spinlock.h>

#define BUF_NUM 8
#define BUF_LEN 100


struct entry_data {
    uint64_t seq;
    uint64_t offset;
    uint64_t length;
};


//typedef struct s_entry entry;
struct rb_entry {
        struct rb_entry *next_entry;
        struct entry_data payload;
};

// New entry version with list_head
struct rb_entry_v2 {
        struct list_head *entry_list;
        struct entry_data payload;
};



//ring_buffer: define structure of ring_buffer
struct ring_buffer {
        int id;
        int id_CPU;
        void *buffer;
        void *buffer_end;
        struct list_head *rb_entry_head;
        struct list_head *rb_entry_tail;
        uint64_t capacity;
        uint64_t count;
        struct list_head *LIST;
        spinlock_t buffer_lock;
};

struct ring_buffer_lf {
        int id;
        int id_CPU;
        uint64_t capacity;
        void *buffer;
        void *buffer_end;
        void *rb_entry_head;
        void *rb_entry_tail;
        atomic_t count;
        struct rb_entry *LIST;
};


//staging_buffer_lf staging_buffers[STAGING_BUF_NUM];
void ring_buffer_init(struct ring_buffer_lf*, int, uint64_t);
//void staging_buffer_free(staging_buffer_lf*);
//void add_s_entry(staging_buffer*, entry_data);
void add_rb_entry_lf(struct ring_buffer_lf*, struct entry_data);
//void print_info_staging_buffer(staging_buffer_lf*);

#endif
