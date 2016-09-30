0250 struct buf;
0251 struct context;
0252 struct file;
0253 struct inode;
0254 struct pipe;
0255 struct proc;
0256 struct rtcdate;
0257 struct spinlock;
0258 struct stat;
0259 struct superblock;
0260 
0261 // bio.c
0262 void            binit(void);
0263 struct buf*     bread(uint, uint);
0264 void            brelse(struct buf*);
0265 void            bwrite(struct buf*);
0266 
0267 // console.c
0268 void            consoleinit(void);
0269 void            cprintf(char*, ...);
0270 void            consoleintr(int(*)(void));
0271 void            panic(char*) __attribute__((noreturn));
0272 
0273 // exec.c
0274 int             exec(char*, char**);
0275 
0276 // file.c
0277 struct file*    filealloc(void);
0278 void            fileclose(struct file*);
0279 struct file*    filedup(struct file*);
0280 void            fileinit(void);
0281 int             fileread(struct file*, char*, int n);
0282 int             filestat(struct file*, struct stat*);
0283 int             filewrite(struct file*, char*, int n);
0284 
0285 // fs.c
0286 void            readsb(int dev, struct superblock *sb);
0287 int             dirlink(struct inode*, char*, uint);
0288 struct inode*   dirlookup(struct inode*, char*, uint*);
0289 struct inode*   ialloc(uint, short);
0290 struct inode*   idup(struct inode*);
0291 void            iinit(int dev);
0292 void            ilock(struct inode*);
0293 void            iput(struct inode*);
0294 void            iunlock(struct inode*);
0295 void            iunlockput(struct inode*);
0296 void            iupdate(struct inode*);
0297 int             namecmp(const char*, const char*);
0298 struct inode*   namei(char*);
0299 struct inode*   nameiparent(char*, char*);
0300 int             readi(struct inode*, char*, uint, uint);
0301 void            stati(struct inode*, struct stat*);
0302 int             writei(struct inode*, char*, uint, uint);
0303 
0304 // ide.c
0305 void            ideinit(void);
0306 void            ideintr(void);
0307 void            iderw(struct buf*);
0308 
0309 // ioapic.c
0310 void            ioapicenable(int irq, int cpu);
0311 extern uchar    ioapicid;
0312 void            ioapicinit(void);
0313 
0314 // kalloc.c
0315 char*           kalloc(void);
0316 void            kfree(char*);
0317 void            kinit1(void*, void*);
0318 void            kinit2(void*, void*);
0319 
0320 // kbd.c
0321 void            kbdintr(void);
0322 
0323 // lapic.c
0324 void            cmostime(struct rtcdate *r);
0325 int             cpunum(void);
0326 extern volatile uint*    lapic;
0327 void            lapiceoi(void);
0328 void            lapicinit(void);
0329 void            lapicstartap(uchar, uint);
0330 void            microdelay(int);
0331 
0332 // log.c
0333 void            initlog(int dev);
0334 void            log_write(struct buf*);
0335 void            begin_op();
0336 void            end_op();
0337 
0338 // mp.c
0339 extern int      ismp;
0340 int             mpbcpu(void);
0341 void            mpinit(void);
0342 void            mpstartthem(void);
0343 
0344 // picirq.c
0345 void            picenable(int);
0346 void            picinit(void);
0347 
0348 
0349 
0350 // pipe.c
0351 int             pipealloc(struct file**, struct file**);
0352 void            pipeclose(struct pipe*, int);
0353 int             piperead(struct pipe*, char*, int);
0354 int             pipewrite(struct pipe*, char*, int);
0355 
0356 
0357 // proc.c
0358 struct proc*    copyproc(struct proc*);
0359 void            exit(void);
0360 int             fork(void);
0361 int             growproc(int);
0362 int             kill(int);
0363 void            pinit(void);
0364 void            procdump(void);
0365 void            scheduler(void) __attribute__((noreturn));
0366 void            sched(void);
0367 void            sleep(void*, struct spinlock*);
0368 void            userinit(void);
0369 int             wait(void);
0370 void            wakeup(void*);
0371 void            yield(void);
0372 
0373 // swtch.S
0374 void            swtch(struct context**, struct context*);
0375 
0376 // spinlock.c
0377 void            acquire(struct spinlock*);
0378 void            getcallerpcs(void*, uint*);
0379 int             holding(struct spinlock*);
0380 void            initlock(struct spinlock*, char*);
0381 void            release(struct spinlock*);
0382 void            pushcli(void);
0383 void            popcli(void);
0384 
0385 // string.c
0386 int             memcmp(const void*, const void*, uint);
0387 void*           memmove(void*, const void*, uint);
0388 void*           memset(void*, int, uint);
0389 char*           safestrcpy(char*, const char*, int);
0390 int             strlen(const char*);
0391 int             strncmp(const char*, const char*, uint);
0392 char*           strncpy(char*, const char*, int);
0393 
0394 // syscall.c
0395 int             argint(int, int*);
0396 int             argptr(int, char**, int);
0397 int             argstr(int, char**);
0398 int             fetchint(uint, int*);
0399 int             fetchstr(uint, char**);
0400 void            syscall(void);
0401 
0402 // timer.c
0403 void            timerinit(void);
0404 
0405 // trap.c
0406 void            idtinit(void);
0407 extern uint     ticks;
0408 void            tvinit(void);
0409 extern struct spinlock tickslock;
0410 
0411 // uart.c
0412 void            uartinit(void);
0413 void            uartintr(void);
0414 void            uartputc(int);
0415 
0416 // vm.c
0417 void            seginit(void);
0418 void            kvmalloc(void);
0419 void            vmenable(void);
0420 pde_t*          setupkvm(void);
0421 char*           uva2ka(pde_t*, char*);
0422 int             allocuvm(pde_t*, uint, uint);
0423 int             deallocuvm(pde_t*, uint, uint);
0424 void            freevm(pde_t*);
0425 void            inituvm(pde_t*, char*, uint);
0426 int             loaduvm(pde_t*, char*, struct inode*, uint, uint);
0427 pde_t*          copyuvm(pde_t*, uint);
0428 void            switchuvm(struct proc*);
0429 void            switchkvm(void);
0430 int             copyout(pde_t*, uint, void*, uint);
0431 void            clearpteu(pde_t *pgdir, char *uva);
0432 
0433 // number of elements in fixed-size array
0434 #define NELEM(x) (sizeof(x)/sizeof((x)[0]))
0435 
0436 
0437 
0438 
0439 
0440 
0441 
0442 
0443 
0444 
0445 
0446 
0447 
0448 
0449 
