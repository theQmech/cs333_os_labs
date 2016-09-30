3650 #include "types.h"
3651 #include "x86.h"
3652 #include "defs.h"
3653 #include "date.h"
3654 #include "param.h"
3655 #include "memlayout.h"
3656 #include "mmu.h"
3657 #include "proc.h"
3658 
3659 int
3660 sys_fork(void)
3661 {
3662   return fork();
3663 }
3664 
3665 int
3666 sys_exit(void)
3667 {
3668   exit();
3669   return 0;  // not reached
3670 }
3671 
3672 int
3673 sys_wait(void)
3674 {
3675   return wait();
3676 }
3677 
3678 int
3679 sys_kill(void)
3680 {
3681   int pid;
3682 
3683   if(argint(0, &pid) < 0)
3684     return -1;
3685   return kill(pid);
3686 }
3687 
3688 int
3689 sys_getpid(void)
3690 {
3691   return proc->pid;
3692 }
3693 
3694 
3695 
3696 
3697 
3698 
3699 
3700 int
3701 sys_sbrk(void)
3702 {
3703   int addr;
3704   int n;
3705 
3706   if(argint(0, &n) < 0)
3707     return -1;
3708   addr = proc->sz;
3709   if(growproc(n) < 0)
3710     return -1;
3711   return addr;
3712 }
3713 
3714 int
3715 sys_sleep(void)
3716 {
3717   int n;
3718   uint ticks0;
3719 
3720   if(argint(0, &n) < 0)
3721     return -1;
3722   acquire(&tickslock);
3723   ticks0 = ticks;
3724   while(ticks - ticks0 < n){
3725     if(proc->killed){
3726       release(&tickslock);
3727       return -1;
3728     }
3729     sleep(&ticks, &tickslock);
3730   }
3731   release(&tickslock);
3732   return 0;
3733 }
3734 
3735 // return how many clock tick interrupts have occurred
3736 // since start.
3737 int
3738 sys_uptime(void)
3739 {
3740   uint xticks;
3741 
3742   acquire(&tickslock);
3743   xticks = ticks;
3744   release(&tickslock);
3745   return xticks;
3746 }
3747 
3748 
3749 
