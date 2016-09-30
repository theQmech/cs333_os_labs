3250 #include "mmu.h"
3251 
3252   # vectors.S sends all traps here.
3253 .globl alltraps
3254 alltraps:
3255   # Build trap frame.
3256   pushl %ds
3257   pushl %es
3258   pushl %fs
3259   pushl %gs
3260   pushal
3261 
3262   # Set up data and per-cpu segments.
3263   movw $(SEG_KDATA<<3), %ax
3264   movw %ax, %ds
3265   movw %ax, %es
3266   movw $(SEG_KCPU<<3), %ax
3267   movw %ax, %fs
3268   movw %ax, %gs
3269 
3270   # Call trap(tf), where tf=%esp
3271   pushl %esp
3272   call trap
3273   addl $4, %esp
3274 
3275   # Return falls through to trapret...
3276 .globl trapret
3277 trapret:
3278   popal
3279   popl %gs
3280   popl %fs
3281   popl %es
3282   popl %ds
3283   addl $0x8, %esp  # trapno and errcode
3284   iret
3285 
3286 
3287 
3288 
3289 
3290 
3291 
3292 
3293 
3294 
3295 
3296 
3297 
3298 
3299 
