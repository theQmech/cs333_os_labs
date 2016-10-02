// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

#define N_FRAMES (PHYSTOP)/PGSIZE

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
int count_freepg(void);

struct {
  uint cnt[N_FRAMES];
  struct spinlock lock;
} rtable;

void init_rtable(void){
  for (int i=0; i<N_FRAMES; ++i) rtable.cnt[i] = 0;
}

void incr_rtable(char *va){
  acquire(&rtable.lock);
  //something
  rtable.cnt[V2P(va)>>PGSHIFT]++;
  release(&rtable.lock);
}

void decr_rtable(char *va){
  acquire(&rtable.lock);
  //something
  rtable.cnt[V2P(va)>>PGSHIFT]++;
  release(&rtable.lock);
}

int iszero_rtable(char *va){
  return (rtable.cnt[V2P(va)>>PGSHIFT]==0);
}


struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
  int freepg_cnt;
} kmem;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  freerange(vstart, vend);
  init_rtable();
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}

//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");


  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  //decrement only when reference count is zero
  if (iszero_rtable(v)){
    r = (struct run*)v;
    r->next = kmem.freelist;
    kmem.freelist = r;
    kmem.freepg_cnt++;
  }
  if(kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    kmem.freepg_cnt--;
  }
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}

int
count_freepg(void){
  return kmem.freepg_cnt;
}