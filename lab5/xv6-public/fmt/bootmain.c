9100 // Boot loader.
9101 //
9102 // Part of the boot block, along with bootasm.S, which calls bootmain().
9103 // bootasm.S has put the processor into protected 32-bit mode.
9104 // bootmain() loads an ELF kernel image from the disk starting at
9105 // sector 1 and then jumps to the kernel entry routine.
9106 
9107 #include "types.h"
9108 #include "elf.h"
9109 #include "x86.h"
9110 #include "memlayout.h"
9111 
9112 #define SECTSIZE  512
9113 
9114 void readseg(uchar*, uint, uint);
9115 
9116 void
9117 bootmain(void)
9118 {
9119   struct elfhdr *elf;
9120   struct proghdr *ph, *eph;
9121   void (*entry)(void);
9122   uchar* pa;
9123 
9124   elf = (struct elfhdr*)0x10000;  // scratch space
9125 
9126   // Read 1st page off disk
9127   readseg((uchar*)elf, 4096, 0);
9128 
9129   // Is this an ELF executable?
9130   if(elf->magic != ELF_MAGIC)
9131     return;  // let bootasm.S handle error
9132 
9133   // Load each program segment (ignores ph flags).
9134   ph = (struct proghdr*)((uchar*)elf + elf->phoff);
9135   eph = ph + elf->phnum;
9136   for(; ph < eph; ph++){
9137     pa = (uchar*)ph->paddr;
9138     readseg(pa, ph->filesz, ph->off);
9139     if(ph->memsz > ph->filesz)
9140       stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
9141   }
9142 
9143   // Call the entry point from the ELF header.
9144   // Does not return!
9145   entry = (void(*)(void))(elf->entry);
9146   entry();
9147 }
9148 
9149 
9150 void
9151 waitdisk(void)
9152 {
9153   // Wait for disk ready.
9154   while((inb(0x1F7) & 0xC0) != 0x40)
9155     ;
9156 }
9157 
9158 // Read a single sector at offset into dst.
9159 void
9160 readsect(void *dst, uint offset)
9161 {
9162   // Issue command.
9163   waitdisk();
9164   outb(0x1F2, 1);   // count = 1
9165   outb(0x1F3, offset);
9166   outb(0x1F4, offset >> 8);
9167   outb(0x1F5, offset >> 16);
9168   outb(0x1F6, (offset >> 24) | 0xE0);
9169   outb(0x1F7, 0x20);  // cmd 0x20 - read sectors
9170 
9171   // Read data.
9172   waitdisk();
9173   insl(0x1F0, dst, SECTSIZE/4);
9174 }
9175 
9176 // Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
9177 // Might copy more than asked.
9178 void
9179 readseg(uchar* pa, uint count, uint offset)
9180 {
9181   uchar* epa;
9182 
9183   epa = pa + count;
9184 
9185   // Round down to sector boundary.
9186   pa -= offset % SECTSIZE;
9187 
9188   // Translate from bytes to sectors; kernel starts at sector 1.
9189   offset = (offset / SECTSIZE) + 1;
9190 
9191   // If this is too slow, we could read lots of sectors at a time.
9192   // We'd write more to memory than asked, but it doesn't matter --
9193   // we load in increasing order.
9194   for(; pa < epa; pa += SECTSIZE, offset++)
9195     readsect(pa, offset);
9196 }
9197 
9198 
9199 
