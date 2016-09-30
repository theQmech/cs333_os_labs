7400 // The I/O APIC manages hardware interrupts for an SMP system.
7401 // http://www.intel.com/design/chipsets/datashts/29056601.pdf
7402 // See also picirq.c.
7403 
7404 #include "types.h"
7405 #include "defs.h"
7406 #include "traps.h"
7407 
7408 #define IOAPIC  0xFEC00000   // Default physical address of IO APIC
7409 
7410 #define REG_ID     0x00  // Register index: ID
7411 #define REG_VER    0x01  // Register index: version
7412 #define REG_TABLE  0x10  // Redirection table base
7413 
7414 // The redirection table starts at REG_TABLE and uses
7415 // two registers to configure each interrupt.
7416 // The first (low) register in a pair contains configuration bits.
7417 // The second (high) register contains a bitmask telling which
7418 // CPUs can serve that interrupt.
7419 #define INT_DISABLED   0x00010000  // Interrupt disabled
7420 #define INT_LEVEL      0x00008000  // Level-triggered (vs edge-)
7421 #define INT_ACTIVELOW  0x00002000  // Active low (vs high)
7422 #define INT_LOGICAL    0x00000800  // Destination is CPU id (vs APIC ID)
7423 
7424 volatile struct ioapic *ioapic;
7425 
7426 // IO APIC MMIO structure: write reg, then read or write data.
7427 struct ioapic {
7428   uint reg;
7429   uint pad[3];
7430   uint data;
7431 };
7432 
7433 static uint
7434 ioapicread(int reg)
7435 {
7436   ioapic->reg = reg;
7437   return ioapic->data;
7438 }
7439 
7440 static void
7441 ioapicwrite(int reg, uint data)
7442 {
7443   ioapic->reg = reg;
7444   ioapic->data = data;
7445 }
7446 
7447 
7448 
7449 
7450 void
7451 ioapicinit(void)
7452 {
7453   int i, id, maxintr;
7454 
7455   if(!ismp)
7456     return;
7457 
7458   ioapic = (volatile struct ioapic*)IOAPIC;
7459   maxintr = (ioapicread(REG_VER) >> 16) & 0xFF;
7460   id = ioapicread(REG_ID) >> 24;
7461   if(id != ioapicid)
7462     cprintf("ioapicinit: id isn't equal to ioapicid; not a MP\n");
7463 
7464   // Mark all interrupts edge-triggered, active high, disabled,
7465   // and not routed to any CPUs.
7466   for(i = 0; i <= maxintr; i++){
7467     ioapicwrite(REG_TABLE+2*i, INT_DISABLED | (T_IRQ0 + i));
7468     ioapicwrite(REG_TABLE+2*i+1, 0);
7469   }
7470 }
7471 
7472 void
7473 ioapicenable(int irq, int cpunum)
7474 {
7475   if(!ismp)
7476     return;
7477 
7478   // Mark interrupt edge-triggered, active high,
7479   // enabled, and routed to the given cpunum,
7480   // which happens to be that cpu's APIC ID.
7481   ioapicwrite(REG_TABLE+2*irq, T_IRQ0 + irq);
7482   ioapicwrite(REG_TABLE+2*irq+1, cpunum << 24);
7483 }
7484 
7485 
7486 
7487 
7488 
7489 
7490 
7491 
7492 
7493 
7494 
7495 
7496 
7497 
7498 
7499 
