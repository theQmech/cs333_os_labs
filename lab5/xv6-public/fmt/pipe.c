6450 #include "types.h"
6451 #include "defs.h"
6452 #include "param.h"
6453 #include "mmu.h"
6454 #include "proc.h"
6455 #include "fs.h"
6456 #include "file.h"
6457 #include "spinlock.h"
6458 
6459 #define PIPESIZE 512
6460 
6461 struct pipe {
6462   struct spinlock lock;
6463   char data[PIPESIZE];
6464   uint nread;     // number of bytes read
6465   uint nwrite;    // number of bytes written
6466   int readopen;   // read fd is still open
6467   int writeopen;  // write fd is still open
6468 };
6469 
6470 int
6471 pipealloc(struct file **f0, struct file **f1)
6472 {
6473   struct pipe *p;
6474 
6475   p = 0;
6476   *f0 = *f1 = 0;
6477   if((*f0 = filealloc()) == 0 || (*f1 = filealloc()) == 0)
6478     goto bad;
6479   if((p = (struct pipe*)kalloc()) == 0)
6480     goto bad;
6481   p->readopen = 1;
6482   p->writeopen = 1;
6483   p->nwrite = 0;
6484   p->nread = 0;
6485   initlock(&p->lock, "pipe");
6486   (*f0)->type = FD_PIPE;
6487   (*f0)->readable = 1;
6488   (*f0)->writable = 0;
6489   (*f0)->pipe = p;
6490   (*f1)->type = FD_PIPE;
6491   (*f1)->readable = 0;
6492   (*f1)->writable = 1;
6493   (*f1)->pipe = p;
6494   return 0;
6495 
6496 
6497 
6498 
6499 
6500  bad:
6501   if(p)
6502     kfree((char*)p);
6503   if(*f0)
6504     fileclose(*f0);
6505   if(*f1)
6506     fileclose(*f1);
6507   return -1;
6508 }
6509 
6510 void
6511 pipeclose(struct pipe *p, int writable)
6512 {
6513   acquire(&p->lock);
6514   if(writable){
6515     p->writeopen = 0;
6516     wakeup(&p->nread);
6517   } else {
6518     p->readopen = 0;
6519     wakeup(&p->nwrite);
6520   }
6521   if(p->readopen == 0 && p->writeopen == 0){
6522     release(&p->lock);
6523     kfree((char*)p);
6524   } else
6525     release(&p->lock);
6526 }
6527 
6528 
6529 int
6530 pipewrite(struct pipe *p, char *addr, int n)
6531 {
6532   int i;
6533 
6534   acquire(&p->lock);
6535   for(i = 0; i < n; i++){
6536     while(p->nwrite == p->nread + PIPESIZE){  //DOC: pipewrite-full
6537       if(p->readopen == 0 || proc->killed){
6538         release(&p->lock);
6539         return -1;
6540       }
6541       wakeup(&p->nread);
6542       sleep(&p->nwrite, &p->lock);  //DOC: pipewrite-sleep
6543     }
6544     p->data[p->nwrite++ % PIPESIZE] = addr[i];
6545   }
6546   wakeup(&p->nread);  //DOC: pipewrite-wakeup1
6547   release(&p->lock);
6548   return n;
6549 }
6550 int
6551 piperead(struct pipe *p, char *addr, int n)
6552 {
6553   int i;
6554 
6555   acquire(&p->lock);
6556   while(p->nread == p->nwrite && p->writeopen){  //DOC: pipe-empty
6557     if(proc->killed){
6558       release(&p->lock);
6559       return -1;
6560     }
6561     sleep(&p->nread, &p->lock); //DOC: piperead-sleep
6562   }
6563   for(i = 0; i < n; i++){  //DOC: piperead-copy
6564     if(p->nread == p->nwrite)
6565       break;
6566     addr[i] = p->data[p->nread++ % PIPESIZE];
6567   }
6568   wakeup(&p->nwrite);  //DOC: piperead-wakeup
6569   release(&p->lock);
6570   return i;
6571 }
6572 
6573 
6574 
6575 
6576 
6577 
6578 
6579 
6580 
6581 
6582 
6583 
6584 
6585 
6586 
6587 
6588 
6589 
6590 
6591 
6592 
6593 
6594 
6595 
6596 
6597 
6598 
6599 
