9000 #include "asm.h"
9001 #include "memlayout.h"
9002 #include "mmu.h"
9003 
9004 # Start the first CPU: switch to 32-bit protected mode, jump into C.
9005 # The BIOS loads this code from the first sector of the hard disk into
9006 # memory at physical address 0x7c00 and starts executing in real mode
9007 # with %cs=0 %ip=7c00.
9008 
9009 .code16                       # Assemble for 16-bit mode
9010 .globl start
9011 start:
9012   cli                         # BIOS enabled interrupts; disable
9013 
9014   # Zero data segment registers DS, ES, and SS.
9015   xorw    %ax,%ax             # Set %ax to zero
9016   movw    %ax,%ds             # -> Data Segment
9017   movw    %ax,%es             # -> Extra Segment
9018   movw    %ax,%ss             # -> Stack Segment
9019 
9020   # Physical address line A20 is tied to zero so that the first PCs
9021   # with 2 MB would run software that assumed 1 MB.  Undo that.
9022 seta20.1:
9023   inb     $0x64,%al               # Wait for not busy
9024   testb   $0x2,%al
9025   jnz     seta20.1
9026 
9027   movb    $0xd1,%al               # 0xd1 -> port 0x64
9028   outb    %al,$0x64
9029 
9030 seta20.2:
9031   inb     $0x64,%al               # Wait for not busy
9032   testb   $0x2,%al
9033   jnz     seta20.2
9034 
9035   movb    $0xdf,%al               # 0xdf -> port 0x60
9036   outb    %al,$0x60
9037 
9038   # Switch from real to protected mode.  Use a bootstrap GDT that makes
9039   # virtual addresses map directly to physical addresses so that the
9040   # effective memory map doesn't change during the transition.
9041   lgdt    gdtdesc
9042   movl    %cr0, %eax
9043   orl     $CR0_PE, %eax
9044   movl    %eax, %cr0
9045 
9046 
9047 
9048 
9049 
9050   # Complete transition to 32-bit protected mode by using long jmp
9051   # to reload %cs and %eip.  The segment descriptors are set up with no
9052   # translation, so that the mapping is still the identity mapping.
9053   ljmp    $(SEG_KCODE<<3), $start32
9054 
9055 .code32  # Tell assembler to generate 32-bit code now.
9056 start32:
9057   # Set up the protected-mode data segment registers
9058   movw    $(SEG_KDATA<<3), %ax    # Our data segment selector
9059   movw    %ax, %ds                # -> DS: Data Segment
9060   movw    %ax, %es                # -> ES: Extra Segment
9061   movw    %ax, %ss                # -> SS: Stack Segment
9062   movw    $0, %ax                 # Zero segments not ready for use
9063   movw    %ax, %fs                # -> FS
9064   movw    %ax, %gs                # -> GS
9065 
9066   # Set up the stack pointer and call into C.
9067   movl    $start, %esp
9068   call    bootmain
9069 
9070   # If bootmain returns (it shouldn't), trigger a Bochs
9071   # breakpoint if running under Bochs, then loop.
9072   movw    $0x8a00, %ax            # 0x8a00 -> port 0x8a00
9073   movw    %ax, %dx
9074   outw    %ax, %dx
9075   movw    $0x8ae0, %ax            # 0x8ae0 -> port 0x8a00
9076   outw    %ax, %dx
9077 spin:
9078   jmp     spin
9079 
9080 # Bootstrap GDT
9081 .p2align 2                                # force 4 byte alignment
9082 gdt:
9083   SEG_NULLASM                             # null seg
9084   SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)   # code seg
9085   SEG_ASM(STA_W, 0x0, 0xffffffff)         # data seg
9086 
9087 gdtdesc:
9088   .word   (gdtdesc - gdt - 1)             # sizeof(gdt) - 1
9089   .long   gdt                             # address gdt
9090 
9091 
9092 
9093 
9094 
9095 
9096 
9097 
9098 
9099 
