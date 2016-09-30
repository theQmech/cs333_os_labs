2300 // Segments in proc->gdt.
2301 #define NSEGS     7
2302 
2303 // Per-CPU state
2304 struct cpu {
2305   uchar id;                    // Local APIC ID; index into cpus[] below
2306   struct context *scheduler;   // swtch() here to enter scheduler
2307   struct taskstate ts;         // Used by x86 to find stack for interrupt
2308   struct segdesc gdt[NSEGS];   // x86 global descriptor table
2309   volatile uint started;       // Has the CPU started?
2310   int ncli;                    // Depth of pushcli nesting.
2311   int intena;                  // Were interrupts enabled before pushcli?
2312 
2313   // Cpu-local storage variables; see below
2314   struct cpu *cpu;
2315   struct proc *proc;           // The currently-running process.
2316 };
2317 
2318 extern struct cpu cpus[NCPU];
2319 extern int ncpu;
2320 
2321 // Per-CPU variables, holding pointers to the
2322 // current cpu and to the current process.
2323 // The asm suffix tells gcc to use "%gs:0" to refer to cpu
2324 // and "%gs:4" to refer to proc.  seginit sets up the
2325 // %gs segment register so that %gs refers to the memory
2326 // holding those two variables in the local cpu's struct cpu.
2327 // This is similar to how thread-local variables are implemented
2328 // in thread libraries such as Linux pthreads.
2329 extern struct cpu *cpu asm("%gs:0");       // &cpus[cpunum()]
2330 extern struct proc *proc asm("%gs:4");     // cpus[cpunum()].proc
2331 
2332 
2333 // Saved registers for kernel context switches.
2334 // Don't need to save all the segment registers (%cs, etc),
2335 // because they are constant across kernel contexts.
2336 // Don't need to save %eax, %ecx, %edx, because the
2337 // x86 convention is that the caller has saved them.
2338 // Contexts are stored at the bottom of the stack they
2339 // describe; the stack pointer is the address of the context.
2340 // The layout of the context matches the layout of the stack in swtch.S
2341 // at the "Switch stacks" comment. Switch doesn't save eip explicitly,
2342 // but it is on the stack and allocproc() manipulates it.
2343 struct context {
2344   uint edi;
2345   uint esi;
2346   uint ebx;
2347   uint ebp;
2348   uint eip;
2349 };
2350 enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };
2351 
2352 // Per-process state
2353 struct proc {
2354   uint sz;                     // Size of process memory (bytes)
2355   pde_t* pgdir;                // Page table
2356   char *kstack;                // Bottom of kernel stack for this process
2357   enum procstate state;        // Process state
2358   int pid;                     // Process ID
2359   struct proc *parent;         // Parent process
2360   struct trapframe *tf;        // Trap frame for current syscall
2361   struct context *context;     // swtch() here to run process
2362   void *chan;                  // If non-zero, sleeping on chan
2363   int killed;                  // If non-zero, have been killed
2364   struct file *ofile[NOFILE];  // Open files
2365   struct inode *cwd;           // Current directory
2366   char name[16];               // Process name (debugging)
2367 };
2368 
2369 // Process memory is laid out contiguously, low addresses first:
2370 //   text
2371 //   original data and bss
2372 //   fixed-size stack
2373 //   expandable heap
2374 
2375 
2376 
2377 
2378 
2379 
2380 
2381 
2382 
2383 
2384 
2385 
2386 
2387 
2388 
2389 
2390 
2391 
2392 
2393 
2394 
2395 
2396 
2397 
2398 
2399 
