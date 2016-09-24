Files of interest.


proc.c -> change the scheduler algorithm here.
struct proc has int priority.


testsched.c -> write the testcases here in C.

usual commands to run.

make clean
make
qemu-system-i386 -serial mon:stdio -hdb fs.img xv6.img -smp 1 -m 512

