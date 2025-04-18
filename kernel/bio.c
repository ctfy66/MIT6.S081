// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"
// kernel/defs.h

struct {
  struct spinlock lock;
  struct spinlock bucket_lock[NBUCKET];
  struct buf* buckets[NBUCKET];
  struct buf buf[NBUF];
  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
} bcache;

void
binit(void)
{
  

  initlock(&bcache.lock, "bcache");

  // initialize hashtable of buffers
  for (int i = 0; i < NBUCKET; i++) {
    bcache.buckets[i] = 0;
    initlock(&bcache.bucket_lock[i], "bcache_bucket"); 
  }

}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  int hash = blockno % NBUCKET;
  acquire(&bcache.bucket_lock[hash]);

  // Is the block already cached?
  for(b = bcache.buckets[blockno % NBUCKET]; b != 0; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      b->time_stamp = ticks;
      
      release(&bcache.bucket_lock[hash]);
      acquiresleep(&b->lock);
      return b;
    }
  }
  

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  acquire(&bcache.lock);
  struct buf* lru = 0;
  uint oldest_time = 0xffffffff;
  for(b = bcache.buf; b <= bcache.buf + NBUF; b++){
    
    if(b->refcnt == 0 && b->time_stamp < oldest_time) {
      lru = b;
      oldest_time = b->time_stamp;
    }
  }
  if (lru) {
      lru->dev = dev;
      lru->blockno = blockno;
      lru->valid = 0;
      lru->refcnt = 1;
      lru->time_stamp = ticks;

      release(&bcache.lock);
      
      lru->next = bcache.buckets[hash];
      bcache.buckets[hash] = lru;
      
      release(&bcache.bucket_lock[hash]);
      acquiresleep(&lru->lock);
      return lru;
  }
  release(&bcache.lock);
  release(&bcache.bucket_lock[hash]);

  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
// bread function - fixed version
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  
  if(!b->valid) {
    
    virtio_disk_rw(b, 0);
    b->valid = 1;
    
  }
    return b;
  
  
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  b->refcnt--;
  
  if (b->refcnt == 0) {
    // no one is waiting for it.
    //remove buf form hashtable
    b->time_stamp = ticks;
    
    acquire(&bcache.bucket_lock[b->blockno % NBUCKET]);
    if (bcache.buckets[b->blockno % NBUCKET]->blockno == b->blockno && bcache.buckets[b->blockno % NBUCKET]->dev == b->dev) {
        bcache.buckets[b->blockno % NBUCKET] = b->next;
        b->prev = 0;
        b->next = 0;
    } else {
        if (b->prev)  b->prev->next = b->next;
        if (b->next) b->next->prev = b->prev;
        b->prev = 0;
        b->next = 0;
    }
    release(&bcache.bucket_lock[b->blockno % NBUCKET]);
  }
  
  
}

void
bpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt++;
  release(&bcache.lock);
}

void
bunpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt--;
  release(&bcache.lock);
}


