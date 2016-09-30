7500 // Intel 8259A programmable interrupt controllers.
7501 
7502 #include "types.h"
7503 #include "x86.h"
7504 #include "traps.h"
7505 
7506 // I/O Addresses of the two programmable interrupt controllers
7507 #define IO_PIC1         0x20    // Master (IRQs 0-7)
7508 #define IO_PIC2         0xA0    // Slave (IRQs 8-15)
7509 
7510 #define IRQ_SLAVE       2       // IRQ at which slave connects to master
7511 
7512 // Current IRQ mask.
7513 // Initial IRQ mask has interrupt 2 enabled (for slave 8259A).
7514 static ushort irqmask = 0xFFFF & ~(1<<IRQ_SLAVE);
7515 
7516 static void
7517 picsetmask(ushort mask)
7518 {
7519   irqmask = mask;
7520   outb(IO_PIC1+1, mask);
7521   outb(IO_PIC2+1, mask >> 8);
7522 }
7523 
7524 void
7525 picenable(int irq)
7526 {
7527   picsetmask(irqmask & ~(1<<irq));
7528 }
7529 
7530 // Initialize the 8259A interrupt controllers.
7531 void
7532 picinit(void)
7533 {
7534   // mask all interrupts
7535   outb(IO_PIC1+1, 0xFF);
7536   outb(IO_PIC2+1, 0xFF);
7537 
7538   // Set up master (8259A-1)
7539 
7540   // ICW1:  0001g0hi
7541   //    g:  0 = edge triggering, 1 = level triggering
7542   //    h:  0 = cascaded PICs, 1 = master only
7543   //    i:  0 = no ICW4, 1 = ICW4 required
7544   outb(IO_PIC1, 0x11);
7545 
7546   // ICW2:  Vector offset
7547   outb(IO_PIC1+1, T_IRQ0);
7548 
7549 
7550   // ICW3:  (master PIC) bit mask of IR lines connected to slaves
7551   //        (slave PIC) 3-bit # of slave's connection to master
7552   outb(IO_PIC1+1, 1<<IRQ_SLAVE);
7553 
7554   // ICW4:  000nbmap
7555   //    n:  1 = special fully nested mode
7556   //    b:  1 = buffered mode
7557   //    m:  0 = slave PIC, 1 = master PIC
7558   //      (ignored when b is 0, as the master/slave role
7559   //      can be hardwired).
7560   //    a:  1 = Automatic EOI mode
7561   //    p:  0 = MCS-80/85 mode, 1 = intel x86 mode
7562   outb(IO_PIC1+1, 0x3);
7563 
7564   // Set up slave (8259A-2)
7565   outb(IO_PIC2, 0x11);                  // ICW1
7566   outb(IO_PIC2+1, T_IRQ0 + 8);      // ICW2
7567   outb(IO_PIC2+1, IRQ_SLAVE);           // ICW3
7568   // NB Automatic EOI mode doesn't tend to work on the slave.
7569   // Linux source code says it's "to be investigated".
7570   outb(IO_PIC2+1, 0x3);                 // ICW4
7571 
7572   // OCW3:  0ef01prs
7573   //   ef:  0x = NOP, 10 = clear specific mask, 11 = set specific mask
7574   //    p:  0 = no polling, 1 = polling mode
7575   //   rs:  0x = NOP, 10 = read IRR, 11 = read ISR
7576   outb(IO_PIC1, 0x68);             // clear specific mask
7577   outb(IO_PIC1, 0x0a);             // read IRR by default
7578 
7579   outb(IO_PIC2, 0x68);             // OCW3
7580   outb(IO_PIC2, 0x0a);             // OCW3
7581 
7582   if(irqmask != 0xFFFF)
7583     picsetmask(irqmask);
7584 }
7585 
7586 
7587 
7588 
7589 
7590 
7591 
7592 
7593 
7594 
7595 
7596 
7597 
7598 
7599 
