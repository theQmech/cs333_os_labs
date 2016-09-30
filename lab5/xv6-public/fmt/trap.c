3300 #include "types.h"
3301 #include "defs.h"
3302 #include "param.h"
3303 #include "memlayout.h"
3304 #include "mmu.h"
3305 #include "proc.h"
3306 #include "x86.h"
3307 #include "traps.h"
3308 #include "spinlock.h"
3309 
3310 // Interrupt descriptor table (shared by all CPUs).
3311 struct gatedesc idt[256];
3312 extern uint vectors[];  // in vectors.S: array of 256 entry pointers
3313 struct spinlock tickslock;
3314 uint ticks;
3315 
3316 void
3317 tvinit(void)
3318 {
3319   int i;
3320 
3321   for(i = 0; i < 256; i++)
3322     SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
3323   SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);
3324 
3325   initlock(&tickslock, "time");
3326 }
3327 
3328 void
3329 idtinit(void)
3330 {
3331   lidt(idt, sizeof(idt));
3332 }
3333 
3334 
3335 
3336 
3337 
3338 
3339 
3340 
3341 
3342 
3343 
3344 
3345 
3346 
3347 
3348 
3349 
3350 void
3351 trap(struct trapframe *tf)
3352 {
3353   if(tf->trapno == T_SYSCALL){
3354     if(proc->killed)
3355       exit();
3356     proc->tf = tf;
3357     syscall();
3358     if(proc->killed)
3359       exit();
3360     return;
3361   }
3362 
3363   switch(tf->trapno){
3364   case T_IRQ0 + IRQ_TIMER:
3365     if(cpu->id == 0){
3366       acquire(&tickslock);
3367       ticks++;
3368       wakeup(&ticks);
3369       release(&tickslock);
3370     }
3371     lapiceoi();
3372     break;
3373   case T_IRQ0 + IRQ_IDE:
3374     ideintr();
3375     lapiceoi();
3376     break;
3377   case T_IRQ0 + IRQ_IDE+1:
3378     // Bochs generates spurious IDE1 interrupts.
3379     break;
3380   case T_IRQ0 + IRQ_KBD:
3381     kbdintr();
3382     lapiceoi();
3383     break;
3384   case T_IRQ0 + IRQ_COM1:
3385     uartintr();
3386     lapiceoi();
3387     break;
3388   case T_IRQ0 + 7:
3389   case T_IRQ0 + IRQ_SPURIOUS:
3390     cprintf("cpu%d: spurious interrupt at %x:%x\n",
3391             cpu->id, tf->cs, tf->eip);
3392     lapiceoi();
3393     break;
3394 
3395 
3396 
3397 
3398 
3399 
3400   default:
3401     if(proc == 0 || (tf->cs&3) == 0){
3402       // In kernel, it must be our mistake.
3403       cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
3404               tf->trapno, cpu->id, tf->eip, rcr2());
3405       panic("trap");
3406     }
3407     // In user space, assume process misbehaved.
3408     cprintf("pid %d %s: trap %d err %d on cpu %d "
3409             "eip 0x%x addr 0x%x--kill proc\n",
3410             proc->pid, proc->name, tf->trapno, tf->err, cpu->id, tf->eip,
3411             rcr2());
3412     proc->killed = 1;
3413   }
3414 
3415   // Force process exit if it has been killed and is in user space.
3416   // (If it is still executing in the kernel, let it keep running
3417   // until it gets to the regular system call return.)
3418   if(proc && proc->killed && (tf->cs&3) == DPL_USER)
3419     exit();
3420 
3421   // Force process to give up CPU on clock tick.
3422   // If interrupts were on while locks held, would need to check nlock.
3423   if(proc && proc->state == RUNNING && tf->trapno == T_IRQ0+IRQ_TIMER)
3424     yield();
3425 
3426   // Check if the process has been killed since we yielded
3427   if(proc && proc->killed && (tf->cs&3) == DPL_USER)
3428     exit();
3429 }
3430 
3431 
3432 
3433 
3434 
3435 
3436 
3437 
3438 
3439 
3440 
3441 
3442 
3443 
3444 
3445 
3446 
3447 
3448 
3449 
