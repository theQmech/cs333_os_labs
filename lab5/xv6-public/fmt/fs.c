4750 // File system implementation.  Five layers:
4751 //   + Blocks: allocator for raw disk blocks.
4752 //   + Log: crash recovery for multi-step updates.
4753 //   + Files: inode allocator, reading, writing, metadata.
4754 //   + Directories: inode with special contents (list of other inodes!)
4755 //   + Names: paths like /usr/rtm/xv6/fs.c for convenient naming.
4756 //
4757 // This file contains the low-level file system manipulation
4758 // routines.  The (higher-level) system call implementations
4759 // are in sysfile.c.
4760 
4761 #include "types.h"
4762 #include "defs.h"
4763 #include "param.h"
4764 #include "stat.h"
4765 #include "mmu.h"
4766 #include "proc.h"
4767 #include "spinlock.h"
4768 #include "fs.h"
4769 #include "buf.h"
4770 #include "file.h"
4771 
4772 #define min(a, b) ((a) < (b) ? (a) : (b))
4773 static void itrunc(struct inode*);
4774 struct superblock sb;   // there should be one per dev, but we run with one dev
4775 
4776 // Read the super block.
4777 void
4778 readsb(int dev, struct superblock *sb)
4779 {
4780   struct buf *bp;
4781 
4782   bp = bread(dev, 1);
4783   memmove(sb, bp->data, sizeof(*sb));
4784   brelse(bp);
4785 }
4786 
4787 // Zero a block.
4788 static void
4789 bzero(int dev, int bno)
4790 {
4791   struct buf *bp;
4792 
4793   bp = bread(dev, bno);
4794   memset(bp->data, 0, BSIZE);
4795   log_write(bp);
4796   brelse(bp);
4797 }
4798 
4799 
4800 // Blocks.
4801 
4802 // Allocate a zeroed disk block.
4803 static uint
4804 balloc(uint dev)
4805 {
4806   int b, bi, m;
4807   struct buf *bp;
4808 
4809   bp = 0;
4810   for(b = 0; b < sb.size; b += BPB){
4811     bp = bread(dev, BBLOCK(b, sb));
4812     for(bi = 0; bi < BPB && b + bi < sb.size; bi++){
4813       m = 1 << (bi % 8);
4814       if((bp->data[bi/8] & m) == 0){  // Is block free?
4815         bp->data[bi/8] |= m;  // Mark block in use.
4816         log_write(bp);
4817         brelse(bp);
4818         bzero(dev, b + bi);
4819         return b + bi;
4820       }
4821     }
4822     brelse(bp);
4823   }
4824   panic("balloc: out of blocks");
4825 }
4826 
4827 // Free a disk block.
4828 static void
4829 bfree(int dev, uint b)
4830 {
4831   struct buf *bp;
4832   int bi, m;
4833 
4834   readsb(dev, &sb);
4835   bp = bread(dev, BBLOCK(b, sb));
4836   bi = b % BPB;
4837   m = 1 << (bi % 8);
4838   if((bp->data[bi/8] & m) == 0)
4839     panic("freeing free block");
4840   bp->data[bi/8] &= ~m;
4841   log_write(bp);
4842   brelse(bp);
4843 }
4844 
4845 
4846 
4847 
4848 
4849 
4850 // Inodes.
4851 //
4852 // An inode describes a single unnamed file.
4853 // The inode disk structure holds metadata: the file's type,
4854 // its size, the number of links referring to it, and the
4855 // list of blocks holding the file's content.
4856 //
4857 // The inodes are laid out sequentially on disk at
4858 // sb.startinode. Each inode has a number, indicating its
4859 // position on the disk.
4860 //
4861 // The kernel keeps a cache of in-use inodes in memory
4862 // to provide a place for synchronizing access
4863 // to inodes used by multiple processes. The cached
4864 // inodes include book-keeping information that is
4865 // not stored on disk: ip->ref and ip->flags.
4866 //
4867 // An inode and its in-memory represtative go through a
4868 // sequence of states before they can be used by the
4869 // rest of the file system code.
4870 //
4871 // * Allocation: an inode is allocated if its type (on disk)
4872 //   is non-zero. ialloc() allocates, iput() frees if
4873 //   the link count has fallen to zero.
4874 //
4875 // * Referencing in cache: an entry in the inode cache
4876 //   is free if ip->ref is zero. Otherwise ip->ref tracks
4877 //   the number of in-memory pointers to the entry (open
4878 //   files and current directories). iget() to find or
4879 //   create a cache entry and increment its ref, iput()
4880 //   to decrement ref.
4881 //
4882 // * Valid: the information (type, size, &c) in an inode
4883 //   cache entry is only correct when the I_VALID bit
4884 //   is set in ip->flags. ilock() reads the inode from
4885 //   the disk and sets I_VALID, while iput() clears
4886 //   I_VALID if ip->ref has fallen to zero.
4887 //
4888 // * Locked: file system code may only examine and modify
4889 //   the information in an inode and its content if it
4890 //   has first locked the inode. The I_BUSY flag indicates
4891 //   that the inode is locked. ilock() sets I_BUSY,
4892 //   while iunlock clears it.
4893 //
4894 // Thus a typical sequence is:
4895 //   ip = iget(dev, inum)
4896 //   ilock(ip)
4897 //   ... examine and modify ip->xxx ...
4898 //   iunlock(ip)
4899 //   iput(ip)
4900 //
4901 // ilock() is separate from iget() so that system calls can
4902 // get a long-term reference to an inode (as for an open file)
4903 // and only lock it for short periods (e.g., in read()).
4904 // The separation also helps avoid deadlock and races during
4905 // pathname lookup. iget() increments ip->ref so that the inode
4906 // stays cached and pointers to it remain valid.
4907 //
4908 // Many internal file system functions expect the caller to
4909 // have locked the inodes involved; this lets callers create
4910 // multi-step atomic operations.
4911 
4912 struct {
4913   struct spinlock lock;
4914   struct inode inode[NINODE];
4915 } icache;
4916 
4917 void
4918 iinit(int dev)
4919 {
4920   initlock(&icache.lock, "icache");
4921   readsb(dev, &sb);
4922   cprintf("sb: size %d nblocks %d ninodes %d nlog %d logstart %d inodestart %d bmap start %d\n", sb.size,
4923           sb.nblocks, sb.ninodes, sb.nlog, sb.logstart, sb.inodestart, sb.bmapstart);
4924 }
4925 
4926 static struct inode* iget(uint dev, uint inum);
4927 
4928 
4929 
4930 
4931 
4932 
4933 
4934 
4935 
4936 
4937 
4938 
4939 
4940 
4941 
4942 
4943 
4944 
4945 
4946 
4947 
4948 
4949 
4950 // Allocate a new inode with the given type on device dev.
4951 // A free inode has a type of zero.
4952 struct inode*
4953 ialloc(uint dev, short type)
4954 {
4955   int inum;
4956   struct buf *bp;
4957   struct dinode *dip;
4958 
4959   for(inum = 1; inum < sb.ninodes; inum++){
4960     bp = bread(dev, IBLOCK(inum, sb));
4961     dip = (struct dinode*)bp->data + inum%IPB;
4962     if(dip->type == 0){  // a free inode
4963       memset(dip, 0, sizeof(*dip));
4964       dip->type = type;
4965       log_write(bp);   // mark it allocated on the disk
4966       brelse(bp);
4967       return iget(dev, inum);
4968     }
4969     brelse(bp);
4970   }
4971   panic("ialloc: no inodes");
4972 }
4973 
4974 // Copy a modified in-memory inode to disk.
4975 void
4976 iupdate(struct inode *ip)
4977 {
4978   struct buf *bp;
4979   struct dinode *dip;
4980 
4981   bp = bread(ip->dev, IBLOCK(ip->inum, sb));
4982   dip = (struct dinode*)bp->data + ip->inum%IPB;
4983   dip->type = ip->type;
4984   dip->major = ip->major;
4985   dip->minor = ip->minor;
4986   dip->nlink = ip->nlink;
4987   dip->size = ip->size;
4988   memmove(dip->addrs, ip->addrs, sizeof(ip->addrs));
4989   log_write(bp);
4990   brelse(bp);
4991 }
4992 
4993 
4994 
4995 
4996 
4997 
4998 
4999 
5000 // Find the inode with number inum on device dev
5001 // and return the in-memory copy. Does not lock
5002 // the inode and does not read it from disk.
5003 static struct inode*
5004 iget(uint dev, uint inum)
5005 {
5006   struct inode *ip, *empty;
5007 
5008   acquire(&icache.lock);
5009 
5010   // Is the inode already cached?
5011   empty = 0;
5012   for(ip = &icache.inode[0]; ip < &icache.inode[NINODE]; ip++){
5013     if(ip->ref > 0 && ip->dev == dev && ip->inum == inum){
5014       ip->ref++;
5015       release(&icache.lock);
5016       return ip;
5017     }
5018     if(empty == 0 && ip->ref == 0)    // Remember empty slot.
5019       empty = ip;
5020   }
5021 
5022   // Recycle an inode cache entry.
5023   if(empty == 0)
5024     panic("iget: no inodes");
5025 
5026   ip = empty;
5027   ip->dev = dev;
5028   ip->inum = inum;
5029   ip->ref = 1;
5030   ip->flags = 0;
5031   release(&icache.lock);
5032 
5033   return ip;
5034 }
5035 
5036 // Increment reference count for ip.
5037 // Returns ip to enable ip = idup(ip1) idiom.
5038 struct inode*
5039 idup(struct inode *ip)
5040 {
5041   acquire(&icache.lock);
5042   ip->ref++;
5043   release(&icache.lock);
5044   return ip;
5045 }
5046 
5047 
5048 
5049 
5050 // Lock the given inode.
5051 // Reads the inode from disk if necessary.
5052 void
5053 ilock(struct inode *ip)
5054 {
5055   struct buf *bp;
5056   struct dinode *dip;
5057 
5058   if(ip == 0 || ip->ref < 1)
5059     panic("ilock");
5060 
5061   acquire(&icache.lock);
5062   while(ip->flags & I_BUSY)
5063     sleep(ip, &icache.lock);
5064   ip->flags |= I_BUSY;
5065   release(&icache.lock);
5066 
5067   if(!(ip->flags & I_VALID)){
5068     bp = bread(ip->dev, IBLOCK(ip->inum, sb));
5069     dip = (struct dinode*)bp->data + ip->inum%IPB;
5070     ip->type = dip->type;
5071     ip->major = dip->major;
5072     ip->minor = dip->minor;
5073     ip->nlink = dip->nlink;
5074     ip->size = dip->size;
5075     memmove(ip->addrs, dip->addrs, sizeof(ip->addrs));
5076     brelse(bp);
5077     ip->flags |= I_VALID;
5078     if(ip->type == 0)
5079       panic("ilock: no type");
5080   }
5081 }
5082 
5083 // Unlock the given inode.
5084 void
5085 iunlock(struct inode *ip)
5086 {
5087   if(ip == 0 || !(ip->flags & I_BUSY) || ip->ref < 1)
5088     panic("iunlock");
5089 
5090   acquire(&icache.lock);
5091   ip->flags &= ~I_BUSY;
5092   wakeup(ip);
5093   release(&icache.lock);
5094 }
5095 
5096 
5097 
5098 
5099 
5100 // Drop a reference to an in-memory inode.
5101 // If that was the last reference, the inode cache entry can
5102 // be recycled.
5103 // If that was the last reference and the inode has no links
5104 // to it, free the inode (and its content) on disk.
5105 // All calls to iput() must be inside a transaction in
5106 // case it has to free the inode.
5107 void
5108 iput(struct inode *ip)
5109 {
5110   acquire(&icache.lock);
5111   if(ip->ref == 1 && (ip->flags & I_VALID) && ip->nlink == 0){
5112     // inode has no links and no other references: truncate and free.
5113     if(ip->flags & I_BUSY)
5114       panic("iput busy");
5115     ip->flags |= I_BUSY;
5116     release(&icache.lock);
5117     itrunc(ip);
5118     ip->type = 0;
5119     iupdate(ip);
5120     acquire(&icache.lock);
5121     ip->flags = 0;
5122     wakeup(ip);
5123   }
5124   ip->ref--;
5125   release(&icache.lock);
5126 }
5127 
5128 // Common idiom: unlock, then put.
5129 void
5130 iunlockput(struct inode *ip)
5131 {
5132   iunlock(ip);
5133   iput(ip);
5134 }
5135 
5136 
5137 
5138 
5139 
5140 
5141 
5142 
5143 
5144 
5145 
5146 
5147 
5148 
5149 
5150 // Inode content
5151 //
5152 // The content (data) associated with each inode is stored
5153 // in blocks on the disk. The first NDIRECT block numbers
5154 // are listed in ip->addrs[].  The next NINDIRECT blocks are
5155 // listed in block ip->addrs[NDIRECT].
5156 
5157 // Return the disk block address of the nth block in inode ip.
5158 // If there is no such block, bmap allocates one.
5159 static uint
5160 bmap(struct inode *ip, uint bn)
5161 {
5162   uint addr, *a;
5163   struct buf *bp;
5164 
5165   if(bn < NDIRECT){
5166     if((addr = ip->addrs[bn]) == 0)
5167       ip->addrs[bn] = addr = balloc(ip->dev);
5168     return addr;
5169   }
5170   bn -= NDIRECT;
5171 
5172   if(bn < NINDIRECT){
5173     // Load indirect block, allocating if necessary.
5174     if((addr = ip->addrs[NDIRECT]) == 0)
5175       ip->addrs[NDIRECT] = addr = balloc(ip->dev);
5176     bp = bread(ip->dev, addr);
5177     a = (uint*)bp->data;
5178     if((addr = a[bn]) == 0){
5179       a[bn] = addr = balloc(ip->dev);
5180       log_write(bp);
5181     }
5182     brelse(bp);
5183     return addr;
5184   }
5185 
5186   panic("bmap: out of range");
5187 }
5188 
5189 
5190 
5191 
5192 
5193 
5194 
5195 
5196 
5197 
5198 
5199 
5200 // Truncate inode (discard contents).
5201 // Only called when the inode has no links
5202 // to it (no directory entries referring to it)
5203 // and has no in-memory reference to it (is
5204 // not an open file or current directory).
5205 static void
5206 itrunc(struct inode *ip)
5207 {
5208   int i, j;
5209   struct buf *bp;
5210   uint *a;
5211 
5212   for(i = 0; i < NDIRECT; i++){
5213     if(ip->addrs[i]){
5214       bfree(ip->dev, ip->addrs[i]);
5215       ip->addrs[i] = 0;
5216     }
5217   }
5218 
5219   if(ip->addrs[NDIRECT]){
5220     bp = bread(ip->dev, ip->addrs[NDIRECT]);
5221     a = (uint*)bp->data;
5222     for(j = 0; j < NINDIRECT; j++){
5223       if(a[j])
5224         bfree(ip->dev, a[j]);
5225     }
5226     brelse(bp);
5227     bfree(ip->dev, ip->addrs[NDIRECT]);
5228     ip->addrs[NDIRECT] = 0;
5229   }
5230 
5231   ip->size = 0;
5232   iupdate(ip);
5233 }
5234 
5235 // Copy stat information from inode.
5236 void
5237 stati(struct inode *ip, struct stat *st)
5238 {
5239   st->dev = ip->dev;
5240   st->ino = ip->inum;
5241   st->type = ip->type;
5242   st->nlink = ip->nlink;
5243   st->size = ip->size;
5244 }
5245 
5246 
5247 
5248 
5249 
5250 // Read data from inode.
5251 int
5252 readi(struct inode *ip, char *dst, uint off, uint n)
5253 {
5254   uint tot, m;
5255   struct buf *bp;
5256 
5257   if(ip->type == T_DEV){
5258     if(ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].read)
5259       return -1;
5260     return devsw[ip->major].read(ip, dst, n);
5261   }
5262 
5263   if(off > ip->size || off + n < off)
5264     return -1;
5265   if(off + n > ip->size)
5266     n = ip->size - off;
5267 
5268   for(tot=0; tot<n; tot+=m, off+=m, dst+=m){
5269     bp = bread(ip->dev, bmap(ip, off/BSIZE));
5270     m = min(n - tot, BSIZE - off%BSIZE);
5271     memmove(dst, bp->data + off%BSIZE, m);
5272     brelse(bp);
5273   }
5274   return n;
5275 }
5276 
5277 
5278 
5279 
5280 
5281 
5282 
5283 
5284 
5285 
5286 
5287 
5288 
5289 
5290 
5291 
5292 
5293 
5294 
5295 
5296 
5297 
5298 
5299 
5300 // Write data to inode.
5301 int
5302 writei(struct inode *ip, char *src, uint off, uint n)
5303 {
5304   uint tot, m;
5305   struct buf *bp;
5306 
5307   if(ip->type == T_DEV){
5308     if(ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].write)
5309       return -1;
5310     return devsw[ip->major].write(ip, src, n);
5311   }
5312 
5313   if(off > ip->size || off + n < off)
5314     return -1;
5315   if(off + n > MAXFILE*BSIZE)
5316     return -1;
5317 
5318   for(tot=0; tot<n; tot+=m, off+=m, src+=m){
5319     bp = bread(ip->dev, bmap(ip, off/BSIZE));
5320     m = min(n - tot, BSIZE - off%BSIZE);
5321     memmove(bp->data + off%BSIZE, src, m);
5322     log_write(bp);
5323     brelse(bp);
5324   }
5325 
5326   if(n > 0 && off > ip->size){
5327     ip->size = off;
5328     iupdate(ip);
5329   }
5330   return n;
5331 }
5332 
5333 
5334 
5335 
5336 
5337 
5338 
5339 
5340 
5341 
5342 
5343 
5344 
5345 
5346 
5347 
5348 
5349 
5350 // Directories
5351 
5352 int
5353 namecmp(const char *s, const char *t)
5354 {
5355   return strncmp(s, t, DIRSIZ);
5356 }
5357 
5358 // Look for a directory entry in a directory.
5359 // If found, set *poff to byte offset of entry.
5360 struct inode*
5361 dirlookup(struct inode *dp, char *name, uint *poff)
5362 {
5363   uint off, inum;
5364   struct dirent de;
5365 
5366   if(dp->type != T_DIR)
5367     panic("dirlookup not DIR");
5368 
5369   for(off = 0; off < dp->size; off += sizeof(de)){
5370     if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
5371       panic("dirlink read");
5372     if(de.inum == 0)
5373       continue;
5374     if(namecmp(name, de.name) == 0){
5375       // entry matches path element
5376       if(poff)
5377         *poff = off;
5378       inum = de.inum;
5379       return iget(dp->dev, inum);
5380     }
5381   }
5382 
5383   return 0;
5384 }
5385 
5386 
5387 
5388 
5389 
5390 
5391 
5392 
5393 
5394 
5395 
5396 
5397 
5398 
5399 
5400 // Write a new directory entry (name, inum) into the directory dp.
5401 int
5402 dirlink(struct inode *dp, char *name, uint inum)
5403 {
5404   int off;
5405   struct dirent de;
5406   struct inode *ip;
5407 
5408   // Check that name is not present.
5409   if((ip = dirlookup(dp, name, 0)) != 0){
5410     iput(ip);
5411     return -1;
5412   }
5413 
5414   // Look for an empty dirent.
5415   for(off = 0; off < dp->size; off += sizeof(de)){
5416     if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
5417       panic("dirlink read");
5418     if(de.inum == 0)
5419       break;
5420   }
5421 
5422   strncpy(de.name, name, DIRSIZ);
5423   de.inum = inum;
5424   if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
5425     panic("dirlink");
5426 
5427   return 0;
5428 }
5429 
5430 
5431 
5432 
5433 
5434 
5435 
5436 
5437 
5438 
5439 
5440 
5441 
5442 
5443 
5444 
5445 
5446 
5447 
5448 
5449 
5450 // Paths
5451 
5452 // Copy the next path element from path into name.
5453 // Return a pointer to the element following the copied one.
5454 // The returned path has no leading slashes,
5455 // so the caller can check *path=='\0' to see if the name is the last one.
5456 // If no name to remove, return 0.
5457 //
5458 // Examples:
5459 //   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
5460 //   skipelem("///a//bb", name) = "bb", setting name = "a"
5461 //   skipelem("a", name) = "", setting name = "a"
5462 //   skipelem("", name) = skipelem("////", name) = 0
5463 //
5464 static char*
5465 skipelem(char *path, char *name)
5466 {
5467   char *s;
5468   int len;
5469 
5470   while(*path == '/')
5471     path++;
5472   if(*path == 0)
5473     return 0;
5474   s = path;
5475   while(*path != '/' && *path != 0)
5476     path++;
5477   len = path - s;
5478   if(len >= DIRSIZ)
5479     memmove(name, s, DIRSIZ);
5480   else {
5481     memmove(name, s, len);
5482     name[len] = 0;
5483   }
5484   while(*path == '/')
5485     path++;
5486   return path;
5487 }
5488 
5489 
5490 
5491 
5492 
5493 
5494 
5495 
5496 
5497 
5498 
5499 
5500 // Look up and return the inode for a path name.
5501 // If parent != 0, return the inode for the parent and copy the final
5502 // path element into name, which must have room for DIRSIZ bytes.
5503 // Must be called inside a transaction since it calls iput().
5504 static struct inode*
5505 namex(char *path, int nameiparent, char *name)
5506 {
5507   struct inode *ip, *next;
5508 
5509   if(*path == '/')
5510     ip = iget(ROOTDEV, ROOTINO);
5511   else
5512     ip = idup(proc->cwd);
5513 
5514   while((path = skipelem(path, name)) != 0){
5515     ilock(ip);
5516     if(ip->type != T_DIR){
5517       iunlockput(ip);
5518       return 0;
5519     }
5520     if(nameiparent && *path == '\0'){
5521       // Stop one level early.
5522       iunlock(ip);
5523       return ip;
5524     }
5525     if((next = dirlookup(ip, name, 0)) == 0){
5526       iunlockput(ip);
5527       return 0;
5528     }
5529     iunlockput(ip);
5530     ip = next;
5531   }
5532   if(nameiparent){
5533     iput(ip);
5534     return 0;
5535   }
5536   return ip;
5537 }
5538 
5539 struct inode*
5540 namei(char *path)
5541 {
5542   char name[DIRSIZ];
5543   return namex(path, 0, name);
5544 }
5545 
5546 
5547 
5548 
5549 
5550 struct inode*
5551 nameiparent(char *path, char *name)
5552 {
5553   return namex(path, 1, name);
5554 }
5555 
5556 
5557 
5558 
5559 
5560 
5561 
5562 
5563 
5564 
5565 
5566 
5567 
5568 
5569 
5570 
5571 
5572 
5573 
5574 
5575 
5576 
5577 
5578 
5579 
5580 
5581 
5582 
5583 
5584 
5585 
5586 
5587 
5588 
5589 
5590 
5591 
5592 
5593 
5594 
5595 
5596 
5597 
5598 
5599 
