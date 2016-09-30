5600 //
5601 // File descriptors
5602 //
5603 
5604 #include "types.h"
5605 #include "defs.h"
5606 #include "param.h"
5607 #include "fs.h"
5608 #include "file.h"
5609 #include "spinlock.h"
5610 
5611 struct devsw devsw[NDEV];
5612 struct {
5613   struct spinlock lock;
5614   struct file file[NFILE];
5615 } ftable;
5616 
5617 void
5618 fileinit(void)
5619 {
5620   initlock(&ftable.lock, "ftable");
5621 }
5622 
5623 // Allocate a file structure.
5624 struct file*
5625 filealloc(void)
5626 {
5627   struct file *f;
5628 
5629   acquire(&ftable.lock);
5630   for(f = ftable.file; f < ftable.file + NFILE; f++){
5631     if(f->ref == 0){
5632       f->ref = 1;
5633       release(&ftable.lock);
5634       return f;
5635     }
5636   }
5637   release(&ftable.lock);
5638   return 0;
5639 }
5640 
5641 
5642 
5643 
5644 
5645 
5646 
5647 
5648 
5649 
5650 // Increment ref count for file f.
5651 struct file*
5652 filedup(struct file *f)
5653 {
5654   acquire(&ftable.lock);
5655   if(f->ref < 1)
5656     panic("filedup");
5657   f->ref++;
5658   release(&ftable.lock);
5659   return f;
5660 }
5661 
5662 // Close file f.  (Decrement ref count, close when reaches 0.)
5663 void
5664 fileclose(struct file *f)
5665 {
5666   struct file ff;
5667 
5668   acquire(&ftable.lock);
5669   if(f->ref < 1)
5670     panic("fileclose");
5671   if(--f->ref > 0){
5672     release(&ftable.lock);
5673     return;
5674   }
5675   ff = *f;
5676   f->ref = 0;
5677   f->type = FD_NONE;
5678   release(&ftable.lock);
5679 
5680   if(ff.type == FD_PIPE)
5681     pipeclose(ff.pipe, ff.writable);
5682   else if(ff.type == FD_INODE){
5683     begin_op();
5684     iput(ff.ip);
5685     end_op();
5686   }
5687 }
5688 
5689 
5690 
5691 
5692 
5693 
5694 
5695 
5696 
5697 
5698 
5699 
5700 // Get metadata about file f.
5701 int
5702 filestat(struct file *f, struct stat *st)
5703 {
5704   if(f->type == FD_INODE){
5705     ilock(f->ip);
5706     stati(f->ip, st);
5707     iunlock(f->ip);
5708     return 0;
5709   }
5710   return -1;
5711 }
5712 
5713 // Read from file f.
5714 int
5715 fileread(struct file *f, char *addr, int n)
5716 {
5717   int r;
5718 
5719   if(f->readable == 0)
5720     return -1;
5721   if(f->type == FD_PIPE)
5722     return piperead(f->pipe, addr, n);
5723   if(f->type == FD_INODE){
5724     ilock(f->ip);
5725     if((r = readi(f->ip, addr, f->off, n)) > 0)
5726       f->off += r;
5727     iunlock(f->ip);
5728     return r;
5729   }
5730   panic("fileread");
5731 }
5732 
5733 
5734 
5735 
5736 
5737 
5738 
5739 
5740 
5741 
5742 
5743 
5744 
5745 
5746 
5747 
5748 
5749 
5750 // Write to file f.
5751 int
5752 filewrite(struct file *f, char *addr, int n)
5753 {
5754   int r;
5755 
5756   if(f->writable == 0)
5757     return -1;
5758   if(f->type == FD_PIPE)
5759     return pipewrite(f->pipe, addr, n);
5760   if(f->type == FD_INODE){
5761     // write a few blocks at a time to avoid exceeding
5762     // the maximum log transaction size, including
5763     // i-node, indirect block, allocation blocks,
5764     // and 2 blocks of slop for non-aligned writes.
5765     // this really belongs lower down, since writei()
5766     // might be writing a device like the console.
5767     int max = ((LOGSIZE-1-1-2) / 2) * 512;
5768     int i = 0;
5769     while(i < n){
5770       int n1 = n - i;
5771       if(n1 > max)
5772         n1 = max;
5773 
5774       begin_op();
5775       ilock(f->ip);
5776       if ((r = writei(f->ip, addr + i, f->off, n1)) > 0)
5777         f->off += r;
5778       iunlock(f->ip);
5779       end_op();
5780 
5781       if(r < 0)
5782         break;
5783       if(r != n1)
5784         panic("short filewrite");
5785       i += r;
5786     }
5787     return i == n ? n : -1;
5788   }
5789   panic("filewrite");
5790 }
5791 
5792 
5793 
5794 
5795 
5796 
5797 
5798 
5799 
