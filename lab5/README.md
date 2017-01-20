A New Scheduler in xv6
======================


Files of interest:
==================

proc.c -> change the scheduler algorithm here.
proc.h -> struct proc has int priority.
sysproc.c -> define new system calls here.
testsched.c -> write the testcases here in C.

To run tests:
=============
make 
make simulate
./testschd

depending on system architecture, `make simulate` may change


Other Commands:
===============
make clean
make
qemu-system-i386 -serial mon:stdio -hdb fs.img xv6.img -smp cores=4 -m 512
