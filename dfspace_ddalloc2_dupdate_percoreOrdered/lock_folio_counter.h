#include <linux/list.h>
#include <linux/rbtree.h>
#include <linux/pagemap.h>

struct lock_folio_counter {
	struct folio *foliop;
	struct list_head list;
};

struct counter_node {
	u64 key;
	unsigned long long count;
	struct rb_node rb;
};

void __sbtrfs_folio_lock(struct folio *folio);

/*
 * lock_page may only be called if we have the page's inode pinned.
 */
static inline void sbtrfs_folio_lock(struct folio *folio)
{
	might_sleep();
	if (!folio_trylock(folio))
		__sbtrfs_folio_lock(folio);
}

void create_and_insert_folio_counter(struct folio *folio);

void __init lock_folio_counter_init(void);
void __exit lock_folio_counter_exit(void);
