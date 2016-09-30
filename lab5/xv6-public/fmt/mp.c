6900 // Multiprocessor support
6901 // Search memory for MP description structures.
6902 // http://developer.intel.com/design/pentium/datashts/24201606.pdf
6903 
6904 #include "types.h"
6905 #include "defs.h"
6906 #include "param.h"
6907 #include "memlayout.h"
6908 #include "mp.h"
6909 #include "x86.h"
6910 #include "mmu.h"
6911 #include "proc.h"
6912 
6913 struct cpu cpus[NCPU];
6914 static struct cpu *bcpu;
6915 int ismp;
6916 int ncpu;
6917 uchar ioapicid;
6918 
6919 int
6920 mpbcpu(void)
6921 {
6922   return bcpu-cpus;
6923 }
6924 
6925 static uchar
6926 sum(uchar *addr, int len)
6927 {
6928   int i, sum;
6929 
6930   sum = 0;
6931   for(i=0; i<len; i++)
6932     sum += addr[i];
6933   return sum;
6934 }
6935 
6936 // Look for an MP structure in the len bytes at addr.
6937 static struct mp*
6938 mpsearch1(uint a, int len)
6939 {
6940   uchar *e, *p, *addr;
6941 
6942   addr = p2v(a);
6943   e = addr+len;
6944   for(p = addr; p < e; p += sizeof(struct mp))
6945     if(memcmp(p, "_MP_", 4) == 0 && sum(p, sizeof(struct mp)) == 0)
6946       return (struct mp*)p;
6947   return 0;
6948 }
6949 
6950 // Search for the MP Floating Pointer Structure, which according to the
6951 // spec is in one of the following three locations:
6952 // 1) in the first KB of the EBDA;
6953 // 2) in the last KB of system base memory;
6954 // 3) in the BIOS ROM between 0xE0000 and 0xFFFFF.
6955 static struct mp*
6956 mpsearch(void)
6957 {
6958   uchar *bda;
6959   uint p;
6960   struct mp *mp;
6961 
6962   bda = (uchar *) P2V(0x400);
6963   if((p = ((bda[0x0F]<<8)| bda[0x0E]) << 4)){
6964     if((mp = mpsearch1(p, 1024)))
6965       return mp;
6966   } else {
6967     p = ((bda[0x14]<<8)|bda[0x13])*1024;
6968     if((mp = mpsearch1(p-1024, 1024)))
6969       return mp;
6970   }
6971   return mpsearch1(0xF0000, 0x10000);
6972 }
6973 
6974 // Search for an MP configuration table.  For now,
6975 // don't accept the default configurations (physaddr == 0).
6976 // Check for correct signature, calculate the checksum and,
6977 // if correct, check the version.
6978 // To do: check extended table checksum.
6979 static struct mpconf*
6980 mpconfig(struct mp **pmp)
6981 {
6982   struct mpconf *conf;
6983   struct mp *mp;
6984 
6985   if((mp = mpsearch()) == 0 || mp->physaddr == 0)
6986     return 0;
6987   conf = (struct mpconf*) p2v((uint) mp->physaddr);
6988   if(memcmp(conf, "PCMP", 4) != 0)
6989     return 0;
6990   if(conf->version != 1 && conf->version != 4)
6991     return 0;
6992   if(sum((uchar*)conf, conf->length) != 0)
6993     return 0;
6994   *pmp = mp;
6995   return conf;
6996 }
6997 
6998 
6999 
7000 void
7001 mpinit(void)
7002 {
7003   uchar *p, *e;
7004   struct mp *mp;
7005   struct mpconf *conf;
7006   struct mpproc *proc;
7007   struct mpioapic *ioapic;
7008 
7009   bcpu = &cpus[0];
7010   if((conf = mpconfig(&mp)) == 0)
7011     return;
7012   ismp = 1;
7013   lapic = (uint*)conf->lapicaddr;
7014   for(p=(uchar*)(conf+1), e=(uchar*)conf+conf->length; p<e; ){
7015     switch(*p){
7016     case MPPROC:
7017       proc = (struct mpproc*)p;
7018       if(ncpu != proc->apicid){
7019         cprintf("mpinit: ncpu=%d apicid=%d\n", ncpu, proc->apicid);
7020         ismp = 0;
7021       }
7022       if(proc->flags & MPBOOT)
7023         bcpu = &cpus[ncpu];
7024       cpus[ncpu].id = ncpu;
7025       ncpu++;
7026       p += sizeof(struct mpproc);
7027       continue;
7028     case MPIOAPIC:
7029       ioapic = (struct mpioapic*)p;
7030       ioapicid = ioapic->apicno;
7031       p += sizeof(struct mpioapic);
7032       continue;
7033     case MPBUS:
7034     case MPIOINTR:
7035     case MPLINTR:
7036       p += 8;
7037       continue;
7038     default:
7039       cprintf("mpinit: unknown config type %x\n", *p);
7040       ismp = 0;
7041     }
7042   }
7043   if(!ismp){
7044     // Didn't like what we found; fall back to no MP.
7045     ncpu = 1;
7046     lapic = 0;
7047     ioapicid = 0;
7048     return;
7049   }
7050   if(mp->imcrp){
7051     // Bochs doesn't support IMCR, so this doesn't run on Bochs.
7052     // But it would on real hardware.
7053     outb(0x22, 0x70);   // Select IMCR
7054     outb(0x23, inb(0x23) | 1);  // Mask external interrupts.
7055   }
7056 }
7057 
7058 
7059 
7060 
7061 
7062 
7063 
7064 
7065 
7066 
7067 
7068 
7069 
7070 
7071 
7072 
7073 
7074 
7075 
7076 
7077 
7078 
7079 
7080 
7081 
7082 
7083 
7084 
7085 
7086 
7087 
7088 
7089 
7090 
7091 
7092 
7093 
7094 
7095 
7096 
7097 
7098 
7099 
