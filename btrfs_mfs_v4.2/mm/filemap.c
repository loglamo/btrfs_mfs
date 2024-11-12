#include <linux/export.h>
#include <linux/compiler.h>
#include <linux/dax.h>
#include <linux/fs.h>
#include <linux/sched/signal.h>
#include <linux/uaccess.h>
#include <linux/capability.h>
#include <linux/kernel_stat.h>
#include <linux/gfp.h>
#include <linux/mm.h>
#include <linux/swap.h>
#include <linux/swapops.h>
#include <linux/syscalls.h>
#include <linux/mman.h>
#include <linux/pagemap.h>
#include <linux/file.h>
#include <linux/uio.h>
#include <linux/error-injection.h>
#include <linux/hash.h>
#include <linux/writeback.h>
#include <linux/backing-dev.h>
#include <linux/pagevec.h>
#include <linux/security.h>
#include <linux/cpuset.h>
#include <linux/hugetlb.h>
#include <linux/memcontrol.h>
#include <linux/shmem_fs.h>
#include <linux/rmap.h>
#include <linux/delayacct.h>
#include <linux/psi.h>
#include <linux/ramfs.h>
#include <linux/page_idle.h>
#include <linux/migrate.h>
#include <linux/pipe_fs_i.h>
#include <linux/splice.h>
#include <asm/pgalloc.h>
#include <asm/tlbflush.h>
#include "../fs.h"
#include "../super.h"
#include "internal.h"
#define CREATE_TRACE_POINTS
#include <trace/events/filemap.h>

#include <linux/buffer_head.h> /* for try_to_free_buffers */

#include <asm/mman.h>

/*
void btrfs_page_cache_sync_ra(struct readahead_control *ractl,                                                                                                                                                        
                  unsigned long req_count)
      {
                bool do_forced_ra = ractl->file && (ractl->file->f_mode & FMODE_RANDOM);
                  
                      /*
                       *        * Even if readahead is disabled, issue this request as readahead
                       *               * as we'll need it to satisfy the requested range. The forced
                       *                      * readahead will do the right thing and limit the read to just the
                       *                             * requested range, which we'll set to 1 page for this case.
                       *                                    */
  /*                    if (!ractl->ra->ra_pages || blk_cgroup_congested()) {
                                    if (!ractl->file)
                                                      return;
                                              req_count = 1;
                                                        do_forced_ra = true;
                                                              }
                        
                            /* be dumb */
    /*                        if (do_forced_ra) {
                                          force_page_cache_ra(ractl, req_count);
                                                    return;
                                                          }
                            printk(KERN_INFO "page_cache_sync_ra: ");  
                                  ondemand_readahead(ractl, NULL, req_count);
 }
*/

static inline                                                                                                                                                                                                   
  void btrfs_page_cache_sync_readahead(struct address_space *mapping,
                    struct file_ra_state *ra, struct file *file, pgoff_t index,
                              unsigned long req_count)
      {
                DEFINE_READAHEAD(ractl, file, ra, mapping, index);
                page_cache_sync_ra(&ractl, req_count);
                struct inode *host = mapping->host;
                struct super_block *sb = host->i_sb;
                //printk(KERN_INFO "sync_read_ahead: Max ra_pages is %llu\n", sb->s_bdi->ra_pages);
                //printk(KERN_INFO "sync_read_ahead: Max io_pages is %llu\n", sb->s_bdi->io_pages);
       }


bool pos_same_folio(loff_t pos1, loff_t pos2, struct folio *folio)                                                                                                                                              
      {
                unsigned int shift = folio_shift(folio);
                  
                      return (pos1 >> shift == pos2 >> shift);
                        }




static int filemap_readahead(struct kiocb *iocb, struct file *file,
                struct address_space *mapping, struct folio *folio,
                        pgoff_t last_index)
{
        DEFINE_READAHEAD(ractl, file, &file->f_ra, mapping, folio->index);

            if (iocb->ki_flags & IOCB_NOIO)
                        return -EAGAIN;
                page_cache_async_ra(&ractl, folio, last_index - folio->index);
                    return 0;
}

static int btrfs_filemap_get_pages(struct kiocb *iocb, size_t count,                                                                                                                                                  
                  struct folio_batch *fbatch, bool need_uptodate)
      {
                struct file *filp = iocb->ki_filp;
                struct address_space *mapping = filp->f_mapping;
                //struct inode *inode;
                struct inode *f_inode;
                f_inode = file_inode(iocb->ki_filp);
                 

                //printk(KERN_INFO "btrfs_filemap_get_pages...\n");
                // Now is actually remote inode
                
                if (f_inode->has_remote == 0) {
                    mapping = f_inode->i_mapping;
                    //printk(KERN_INFO "btrfs_filemap_get_pages: Accessing from a remote inode...\n");
                   }
      
                
                //inode = f_inode->associated_inode;
                //mapping = f_inode->i_mapping;
                struct btrfs_fs_info *fs_info = btrfs_sb(f_inode->i_sb);
                //mapping = inode->i_mapping;
                //printk(KERN_INFO "btrfs_filemap_get_pages: From the inode %llu of fs_info %d\n", f_inode->i_ino, fs_info->id);
                
                
                struct file_ra_state *ra = &filp->f_ra;                                                                                                                                                                     
                pgoff_t index = iocb->ki_pos >> PAGE_SHIFT;
                pgoff_t last_index;
                struct folio *folio;
                int err = 0;
                /* "last_index" is the index of the page beyond the end of the read */
                last_index = DIV_ROUND_UP(iocb->ki_pos + count, PAGE_SIZE);
retry:
                       if (fatal_signal_pending(current))
                           return -EINTR;
                                       
                       filemap_get_read_batch(mapping, index, last_index - 1, fbatch);
                       //printk(KERN_INFO "btrfs_filemap_get_pages: done filemap_get_read_batch directly\n");
                       if (!folio_batch_count(fbatch)) {
                           if (iocb->ki_flags & IOCB_NOIO)
                                   return -EAGAIN;
                           btrfs_page_cache_sync_readahead(mapping, ra, filp, index, last_index - index);
                           //printk(KERN_INFO "btrfs_filemap_get_pages: done pagecache_sync_ahead\n");
                           filemap_get_read_batch(mapping, index, last_index - 1, fbatch);
                          }
                       if (!folio_batch_count(fbatch)) {
                            if (iocb->ki_flags & (IOCB_NOWAIT | IOCB_WAITQ))
                                    return -EAGAIN;
                            err = filemap_create_folio(filp, mapping,
                                          iocb->ki_pos >> PAGE_SHIFT, fbatch);
                            if (err == AOP_TRUNCATED_PAGE)
                                    goto retry;
                            return err;
                            }
                                                         
                        folio = fbatch->folios[folio_batch_count(fbatch) - 1];
                        //printk(KERN_INFO "btrfs_filemap_get_pages: done creating folio\n");
                        if (folio_test_readahead(folio)) {
                             //printk(KERN_INFO "btrfs_filemap_get_pages: enter filemap_readahead\n");
                             err = filemap_readahead(iocb, filp, mapping, folio, last_index);
                             //printk(KERN_INFO "btrfs_filemap_get_pages: done filemap_readahead\n");
                             if (err)
                                goto err;
                            }
                        if (!folio_test_uptodate(folio)) {
                             if ((iocb->ki_flags & IOCB_WAITQ) && folio_batch_count(fbatch) > 1)
                             iocb->ki_flags |= IOCB_NOWAIT;
                             //printk(KERN_INFO "btrfs_filemap_get_pages: enter filemap_update_page\n");
                             err = filemap_update_page(iocb, mapping, count, folio,
                                                                                need_uptodate);
                             if (err)
                                    goto err;
                                                         }
                        return 0;                                                                                                                                                                                                   
err:
                        if (err < 0)
                              folio_put(folio);
                        if (likely(--fbatch->nr))
                              return 0;
                        if (err == AOP_TRUNCATED_PAGE)
                              goto retry;
                        return err;
}


ssize_t btrfs_filemap_read(struct kiocb *iocb, struct iov_iter *iter,
          ssize_t already_read)
  {
      struct file *filp = iocb->ki_filp;
      struct file_ra_state *ra = &filp->f_ra;
      struct address_space *mapping = filp->f_mapping;
      struct inode *inode = mapping->host;
      //struct inode *inode = filp->f_inode;

      struct folio_batch fbatch;
      int i, error = 0;
      bool writably_mapped;
      loff_t isize, end_offset;
      loff_t last_pos = ra->prev_pos;

      if (unlikely(iocb->ki_pos >= inode->i_sb->s_maxbytes))
          return 0;
      if (unlikely(!iov_iter_count(iter)))
          return 0;
      
      //printk(KERN_INFO "btrfs_filemap_read ...\n");

      iov_iter_truncate(iter, inode->i_sb->s_maxbytes);
      folio_batch_init(&fbatch);

      do {
          cond_resched();

          /*
           * If we've already successfully copied some data, then we
           * can no longer safely return -EIOCBQUEUED. Hence mark
           * an async read NOWAIT at that point.
           */
          if ((iocb->ki_flags & IOCB_WAITQ) && already_read)
              iocb->ki_flags |= IOCB_NOWAIT;

          if (unlikely(iocb->ki_pos >= i_size_read(inode)))
              break;

          error = btrfs_filemap_get_pages(iocb, iter->count, &fbatch, false);
          if (error < 0)
              break;

          /*
           * i_size must be checked after we know the pages are Uptodate.
  *                                                                                                                                                                                                      
           * Checking i_size after the check allows us to calculate
           * the correct value for "nr", which means the zero-filled
           * part of the page is not copied back to userspace (unless
           * another truncate extends the file - this is desired though).
           */
          isize = i_size_read(inode);
          if (unlikely(iocb->ki_pos >= isize))
              goto put_folios;
          end_offset = min_t(loff_t, isize, iocb->ki_pos + iter->count);
  
          /*
           * Once we start copying data, we don't want to be touching any
           * cachelines that might be contended:
           */
          writably_mapped = mapping_writably_mapped(mapping);
  
          /*
           * When a read accesses the same folio several times, only
           * mark it as accessed the first time.
           */
          if (!pos_same_folio(iocb->ki_pos, last_pos - 1,
                      fbatch.folios[0]))
              folio_mark_accessed(fbatch.folios[0]);
  
          for (i = 0; i < folio_batch_count(&fbatch); i++) {
              struct folio *folio = fbatch.folios[i];
              size_t fsize = folio_size(folio);
              size_t offset = iocb->ki_pos & (fsize - 1);
              size_t bytes = min_t(loff_t, end_offset - iocb->ki_pos,
                           fsize - offset);
              size_t copied;
  
              if (end_offset < folio_pos(folio))
                  break;
              if (i > 0)
                  folio_mark_accessed(folio);
              /*
               * If users can be writing to this folio using arbitrary
               * virtual addresses, take care of potential aliasing
               * before reading the folio on the kernel side.
               */
              if (writably_mapped)
                  flush_dcache_folio(folio);
  
              copied = copy_folio_to_iter(folio, offset, bytes, iter);
              already_read += copied;                                                                                                                                                                             
              iocb->ki_pos += copied;
              last_pos = iocb->ki_pos;
  
              if (copied < bytes) {
                  error = -EFAULT;
                  break;
              }
          }
put_folios:
          for (i = 0; i < folio_batch_count(&fbatch); i++)
              folio_put(fbatch.folios[i]);
          folio_batch_init(&fbatch);
      } while (iov_iter_count(iter) && iocb->ki_pos < isize && !error);
  
      file_accessed(filp);
      ra->prev_pos = last_pos;
      return already_read ? already_read : error;
  }

