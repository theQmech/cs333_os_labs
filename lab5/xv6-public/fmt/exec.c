6300 #include "types.h"
6301 #include "param.h"
6302 #include "memlayout.h"
6303 #include "mmu.h"
6304 #include "proc.h"
6305 #include "defs.h"
6306 #include "x86.h"
6307 #include "elf.h"
6308 
6309 int
6310 exec(char *path, char **argv)
6311 {
6312   char *s, *last;
6313   int i, off;
6314   uint argc, sz, sp, ustack[3+MAXARG+1];
6315   struct elfhdr elf;
6316   struct inode *ip;
6317   struct proghdr ph;
6318   pde_t *pgdir, *oldpgdir;
6319 
6320   begin_op();
6321   if((ip = namei(path)) == 0){
6322     end_op();
6323     return -1;
6324   }
6325   ilock(ip);
6326   pgdir = 0;
6327 
6328   // Check ELF header
6329   if(readi(ip, (char*)&elf, 0, sizeof(elf)) < sizeof(elf))
6330     goto bad;
6331   if(elf.magic != ELF_MAGIC)
6332     goto bad;
6333 
6334   if((pgdir = setupkvm()) == 0)
6335     goto bad;
6336 
6337   // Load program into memory.
6338   sz = 0;
6339   for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
6340     if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
6341       goto bad;
6342     if(ph.type != ELF_PROG_LOAD)
6343       continue;
6344     if(ph.memsz < ph.filesz)
6345       goto bad;
6346     if((sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz)) == 0)
6347       goto bad;
6348     if(loaduvm(pgdir, (char*)ph.vaddr, ip, ph.off, ph.filesz) < 0)
6349       goto bad;
6350   }
6351   iunlockput(ip);
6352   end_op();
6353   ip = 0;
6354 
6355   // Allocate two pages at the next page boundary.
6356   // Make the first inaccessible.  Use the second as the user stack.
6357   sz = PGROUNDUP(sz);
6358   if((sz = allocuvm(pgdir, sz, sz + 2*PGSIZE)) == 0)
6359     goto bad;
6360   clearpteu(pgdir, (char*)(sz - 2*PGSIZE));
6361   sp = sz;
6362 
6363   // Push argument strings, prepare rest of stack in ustack.
6364   for(argc = 0; argv[argc]; argc++) {
6365     if(argc >= MAXARG)
6366       goto bad;
6367     sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
6368     if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
6369       goto bad;
6370     ustack[3+argc] = sp;
6371   }
6372   ustack[3+argc] = 0;
6373 
6374   ustack[0] = 0xffffffff;  // fake return PC
6375   ustack[1] = argc;
6376   ustack[2] = sp - (argc+1)*4;  // argv pointer
6377 
6378   sp -= (3+argc+1) * 4;
6379   if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
6380     goto bad;
6381 
6382   // Save program name for debugging.
6383   for(last=s=path; *s; s++)
6384     if(*s == '/')
6385       last = s+1;
6386   safestrcpy(proc->name, last, sizeof(proc->name));
6387 
6388   // Commit to the user image.
6389   oldpgdir = proc->pgdir;
6390   proc->pgdir = pgdir;
6391   proc->sz = sz;
6392   proc->tf->eip = elf.entry;  // main
6393   proc->tf->esp = sp;
6394   switchuvm(proc);
6395   freevm(oldpgdir);
6396   return 0;
6397 
6398 
6399 
6400  bad:
6401   if(pgdir)
6402     freevm(pgdir);
6403   if(ip){
6404     iunlockput(ip);
6405     end_op();
6406   }
6407   return -1;
6408 }
6409 
6410 
6411 
6412 
6413 
6414 
6415 
6416 
6417 
6418 
6419 
6420 
6421 
6422 
6423 
6424 
6425 
6426 
6427 
6428 
6429 
6430 
6431 
6432 
6433 
6434 
6435 
6436 
6437 
6438 
6439 
6440 
6441 
6442 
6443 
6444 
6445 
6446 
6447 
6448 
6449 
