#ifndef __FILEMAP_H
#define __FILEMAP_H

int btrfs_filemap_get_pages(struct kiocb *iocb, size_t count, struct folio_batch *fbatch, bool need_uptodate);
ssize_t btrfs_filemap_read(struct kiocb *iocb, struct iov_iter *iter, ssize_t already_read);

#endif /* __FILEMAP_H */
