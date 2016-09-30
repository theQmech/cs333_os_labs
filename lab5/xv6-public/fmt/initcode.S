8300 # Initial process execs /init.
8301 
8302 #include "syscall.h"
8303 #include "traps.h"
8304 
8305 
8306 # exec(init, argv)
8307 .globl start
8308 start:
8309   pushl $argv
8310   pushl $init
8311   pushl $0  // where caller pc would be
8312   movl $SYS_exec, %eax
8313   int $T_SYSCALL
8314 
8315 # for(;;) exit();
8316 exit:
8317   movl $SYS_exit, %eax
8318   int $T_SYSCALL
8319   jmp exit
8320 
8321 # char init[] = "/init\0";
8322 init:
8323   .string "/init\0"
8324 
8325 # char *argv[] = { init, 0 };
8326 .p2align 2
8327 argv:
8328   .long init
8329   .long 0
8330 
8331 
8332 
8333 
8334 
8335 
8336 
8337 
8338 
8339 
8340 
8341 
8342 
8343 
8344 
8345 
8346 
8347 
8348 
8349 
