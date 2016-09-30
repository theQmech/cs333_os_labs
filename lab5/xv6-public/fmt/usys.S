8350 #include "syscall.h"
8351 #include "traps.h"
8352 
8353 #define SYSCALL(name) \
8354   .globl name; \
8355   name: \
8356     movl $SYS_ ## name, %eax; \
8357     int $T_SYSCALL; \
8358     ret
8359 
8360 SYSCALL(fork)
8361 SYSCALL(exit)
8362 SYSCALL(wait)
8363 SYSCALL(pipe)
8364 SYSCALL(read)
8365 SYSCALL(write)
8366 SYSCALL(close)
8367 SYSCALL(kill)
8368 SYSCALL(exec)
8369 SYSCALL(open)
8370 SYSCALL(mknod)
8371 SYSCALL(unlink)
8372 SYSCALL(fstat)
8373 SYSCALL(link)
8374 SYSCALL(mkdir)
8375 SYSCALL(chdir)
8376 SYSCALL(dup)
8377 SYSCALL(getpid)
8378 SYSCALL(sbrk)
8379 SYSCALL(sleep)
8380 SYSCALL(uptime)
8381 
8382 
8383 
8384 
8385 
8386 
8387 
8388 
8389 
8390 
8391 
8392 
8393 
8394 
8395 
8396 
8397 
8398 
8399 
