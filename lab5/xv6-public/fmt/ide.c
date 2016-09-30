4100 // Simple PIO-based (non-DMA) IDE driver code.
4101 
4102 #include "types.h"
4103 #include "defs.h"
4104 #include "param.h"
4105 #include "memlayout.h"
4106 #include "mmu.h"
4107 #include "proc.h"
4108 #include "x86.h"
4109 #include "traps.h"
4110 #include "spinlock.h"
4111 #include "fs.h"
4112 #include "buf.h"
4113 
4114 #define SECTOR_SIZE   512
4115 #define IDE_BSY       0x80
4116 #define IDE_DRDY      0x40
4117 #define IDE_DF        0x20
4118 #define IDE_ERR       0x01
4119 
4120 #define IDE_CMD_READ  0x20
4121 #define IDE_CMD_WRITE 0x30
4122 
4123 // idequeue points to the buf now being read/written to the disk.
4124 // idequeue->qnext points to the next buf to be processed.
4125 // You must hold idelock while manipulating queue.
4126 
4127 static struct spinlock idelock;
4128 static struct buf *idequeue;
4129 
4130 static int havedisk1;
4131 static void idestart(struct buf*);
4132 
4133 // Wait for IDE disk to become ready.
4134 static int
4135 idewait(int checkerr)
4136 {
4137   int r;
4138 
4139   while(((r = inb(0x1f7)) & (IDE_BSY|IDE_DRDY)) != IDE_DRDY)
4140     ;
4141   if(checkerr && (r & (IDE_DF|IDE_ERR)) != 0)
4142     return -1;
4143   return 0;
4144 }
4145 
4146 
4147 
4148 
4149 
4150 void
4151 ideinit(void)
4152 {
4153   int i;
4154 
4155   initlock(&idelock, "ide");
4156   picenable(IRQ_IDE);
4157   ioapicenable(IRQ_IDE, ncpu - 1);
4158   idewait(0);
4159 
4160   // Check if disk 1 is present
4161   outb(0x1f6, 0xe0 | (1<<4));
4162   for(i=0; i<1000; i++){
4163     if(inb(0x1f7) != 0){
4164       havedisk1 = 1;
4165       break;
4166     }
4167   }
4168 
4169   // Switch back to disk 0.
4170   outb(0x1f6, 0xe0 | (0<<4));
4171 }
4172 
4173 // Start the request for b.  Caller must hold idelock.
4174 static void
4175 idestart(struct buf *b)
4176 {
4177   if(b == 0)
4178     panic("idestart");
4179   if(b->blockno >= FSSIZE)
4180     panic("incorrect blockno");
4181   int sector_per_block =  BSIZE/SECTOR_SIZE;
4182   int sector = b->blockno * sector_per_block;
4183 
4184   if (sector_per_block > 7) panic("idestart");
4185 
4186   idewait(0);
4187   outb(0x3f6, 0);  // generate interrupt
4188   outb(0x1f2, sector_per_block);  // number of sectors
4189   outb(0x1f3, sector & 0xff);
4190   outb(0x1f4, (sector >> 8) & 0xff);
4191   outb(0x1f5, (sector >> 16) & 0xff);
4192   outb(0x1f6, 0xe0 | ((b->dev&1)<<4) | ((sector>>24)&0x0f));
4193   if(b->flags & B_DIRTY){
4194     outb(0x1f7, IDE_CMD_WRITE);
4195     outsl(0x1f0, b->data, BSIZE/4);
4196   } else {
4197     outb(0x1f7, IDE_CMD_READ);
4198   }
4199 }
4200 // Interrupt handler.
4201 void
4202 ideintr(void)
4203 {
4204   struct buf *b;
4205 
4206   // First queued buffer is the active request.
4207   acquire(&idelock);
4208   if((b = idequeue) == 0){
4209     release(&idelock);
4210     // cprintf("spurious IDE interrupt\n");
4211     return;
4212   }
4213   idequeue = b->qnext;
4214 
4215   // Read data if needed.
4216   if(!(b->flags & B_DIRTY) && idewait(1) >= 0)
4217     insl(0x1f0, b->data, BSIZE/4);
4218 
4219   // Wake process waiting for this buf.
4220   b->flags |= B_VALID;
4221   b->flags &= ~B_DIRTY;
4222   wakeup(b);
4223 
4224   // Start disk on next buf in queue.
4225   if(idequeue != 0)
4226     idestart(idequeue);
4227 
4228   release(&idelock);
4229 }
4230 
4231 
4232 
4233 
4234 
4235 
4236 
4237 
4238 
4239 
4240 
4241 
4242 
4243 
4244 
4245 
4246 
4247 
4248 
4249 
4250 // Sync buf with disk.
4251 // If B_DIRTY is set, write buf to disk, clear B_DIRTY, set B_VALID.
4252 // Else if B_VALID is not set, read buf from disk, set B_VALID.
4253 void
4254 iderw(struct buf *b)
4255 {
4256   struct buf **pp;
4257 
4258   if(!(b->flags & B_BUSY))
4259     panic("iderw: buf not busy");
4260   if((b->flags & (B_VALID|B_DIRTY)) == B_VALID)
4261     panic("iderw: nothing to do");
4262   if(b->dev != 0 && !havedisk1)
4263     panic("iderw: ide disk 1 not present");
4264 
4265   acquire(&idelock);  //DOC:acquire-lock
4266 
4267   // Append b to idequeue.
4268   b->qnext = 0;
4269   for(pp=&idequeue; *pp; pp=&(*pp)->qnext)  //DOC:insert-queue
4270     ;
4271   *pp = b;
4272 
4273   // Start disk if necessary.
4274   if(idequeue == b)
4275     idestart(b);
4276 
4277   // Wait for request to finish.
4278   while((b->flags & (B_VALID|B_DIRTY)) != B_VALID){
4279     sleep(b, &idelock);
4280   }
4281 
4282   release(&idelock);
4283 }
4284 
4285 
4286 
4287 
4288 
4289 
4290 
4291 
4292 
4293 
4294 
4295 
4296 
4297 
4298 
4299 
