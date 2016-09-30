7800 // Console input and output.
7801 // Input is from the keyboard or serial port.
7802 // Output is written to the screen and serial port.
7803 
7804 #include "types.h"
7805 #include "defs.h"
7806 #include "param.h"
7807 #include "traps.h"
7808 #include "spinlock.h"
7809 #include "fs.h"
7810 #include "file.h"
7811 #include "memlayout.h"
7812 #include "mmu.h"
7813 #include "proc.h"
7814 #include "x86.h"
7815 
7816 static void consputc(int);
7817 
7818 static int panicked = 0;
7819 
7820 static struct {
7821   struct spinlock lock;
7822   int locking;
7823 } cons;
7824 
7825 static void
7826 printint(int xx, int base, int sign)
7827 {
7828   static char digits[] = "0123456789abcdef";
7829   char buf[16];
7830   int i;
7831   uint x;
7832 
7833   if(sign && (sign = xx < 0))
7834     x = -xx;
7835   else
7836     x = xx;
7837 
7838   i = 0;
7839   do{
7840     buf[i++] = digits[x % base];
7841   }while((x /= base) != 0);
7842 
7843   if(sign)
7844     buf[i++] = '-';
7845 
7846   while(--i >= 0)
7847     consputc(buf[i]);
7848 }
7849 
7850 // Print to the console. only understands %d, %x, %p, %s.
7851 void
7852 cprintf(char *fmt, ...)
7853 {
7854   int i, c, locking;
7855   uint *argp;
7856   char *s;
7857 
7858   locking = cons.locking;
7859   if(locking)
7860     acquire(&cons.lock);
7861 
7862   if (fmt == 0)
7863     panic("null fmt");
7864 
7865   argp = (uint*)(void*)(&fmt + 1);
7866   for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
7867     if(c != '%'){
7868       consputc(c);
7869       continue;
7870     }
7871     c = fmt[++i] & 0xff;
7872     if(c == 0)
7873       break;
7874     switch(c){
7875     case 'd':
7876       printint(*argp++, 10, 1);
7877       break;
7878     case 'x':
7879     case 'p':
7880       printint(*argp++, 16, 0);
7881       break;
7882     case 's':
7883       if((s = (char*)*argp++) == 0)
7884         s = "(null)";
7885       for(; *s; s++)
7886         consputc(*s);
7887       break;
7888     case '%':
7889       consputc('%');
7890       break;
7891     default:
7892       // Print unknown % sequence to draw attention.
7893       consputc('%');
7894       consputc(c);
7895       break;
7896     }
7897   }
7898 
7899 
7900   if(locking)
7901     release(&cons.lock);
7902 }
7903 
7904 void
7905 panic(char *s)
7906 {
7907   int i;
7908   uint pcs[10];
7909 
7910   cli();
7911   cons.locking = 0;
7912   cprintf("cpu%d: panic: ", cpu->id);
7913   cprintf(s);
7914   cprintf("\n");
7915   getcallerpcs(&s, pcs);
7916   for(i=0; i<10; i++)
7917     cprintf(" %p", pcs[i]);
7918   panicked = 1; // freeze other CPU
7919   for(;;)
7920     ;
7921 }
7922 
7923 
7924 
7925 
7926 
7927 
7928 
7929 
7930 
7931 
7932 
7933 
7934 
7935 
7936 
7937 
7938 
7939 
7940 
7941 
7942 
7943 
7944 
7945 
7946 
7947 
7948 
7949 
7950 #define BACKSPACE 0x100
7951 #define CRTPORT 0x3d4
7952 static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory
7953 
7954 static void
7955 cgaputc(int c)
7956 {
7957   int pos;
7958 
7959   // Cursor position: col + 80*row.
7960   outb(CRTPORT, 14);
7961   pos = inb(CRTPORT+1) << 8;
7962   outb(CRTPORT, 15);
7963   pos |= inb(CRTPORT+1);
7964 
7965   if(c == '\n')
7966     pos += 80 - pos%80;
7967   else if(c == BACKSPACE){
7968     if(pos > 0) --pos;
7969   } else
7970     crt[pos++] = (c&0xff) | 0x0700;  // black on white
7971 
7972   if(pos < 0 || pos > 25*80)
7973     panic("pos under/overflow");
7974 
7975   if((pos/80) >= 24){  // Scroll up.
7976     memmove(crt, crt+80, sizeof(crt[0])*23*80);
7977     pos -= 80;
7978     memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
7979   }
7980 
7981   outb(CRTPORT, 14);
7982   outb(CRTPORT+1, pos>>8);
7983   outb(CRTPORT, 15);
7984   outb(CRTPORT+1, pos);
7985   crt[pos] = ' ' | 0x0700;
7986 }
7987 
7988 
7989 
7990 
7991 
7992 
7993 
7994 
7995 
7996 
7997 
7998 
7999 
8000 void
8001 consputc(int c)
8002 {
8003   if(panicked){
8004     cli();
8005     for(;;)
8006       ;
8007   }
8008 
8009   if(c == BACKSPACE){
8010     uartputc('\b'); uartputc(' '); uartputc('\b');
8011   } else
8012     uartputc(c);
8013   cgaputc(c);
8014 }
8015 
8016 #define INPUT_BUF 128
8017 struct {
8018   char buf[INPUT_BUF];
8019   uint r;  // Read index
8020   uint w;  // Write index
8021   uint e;  // Edit index
8022 } input;
8023 
8024 #define C(x)  ((x)-'@')  // Control-x
8025 
8026 void
8027 consoleintr(int (*getc)(void))
8028 {
8029   int c, doprocdump = 0;
8030 
8031   acquire(&cons.lock);
8032   while((c = getc()) >= 0){
8033     switch(c){
8034     case C('P'):  // Process listing.
8035       doprocdump = 1;   // procdump() locks cons.lock indirectly; invoke later
8036       break;
8037     case C('U'):  // Kill line.
8038       while(input.e != input.w &&
8039             input.buf[(input.e-1) % INPUT_BUF] != '\n'){
8040         input.e--;
8041         consputc(BACKSPACE);
8042       }
8043       break;
8044     case C('H'): case '\x7f':  // Backspace
8045       if(input.e != input.w){
8046         input.e--;
8047         consputc(BACKSPACE);
8048       }
8049       break;
8050     default:
8051       if(c != 0 && input.e-input.r < INPUT_BUF){
8052         c = (c == '\r') ? '\n' : c;
8053         input.buf[input.e++ % INPUT_BUF] = c;
8054         consputc(c);
8055         if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
8056           input.w = input.e;
8057           wakeup(&input.r);
8058         }
8059       }
8060       break;
8061     }
8062   }
8063   release(&cons.lock);
8064   if(doprocdump) {
8065     procdump();  // now call procdump() wo. cons.lock held
8066   }
8067 }
8068 
8069 int
8070 consoleread(struct inode *ip, char *dst, int n)
8071 {
8072   uint target;
8073   int c;
8074 
8075   iunlock(ip);
8076   target = n;
8077   acquire(&cons.lock);
8078   while(n > 0){
8079     while(input.r == input.w){
8080       if(proc->killed){
8081         release(&cons.lock);
8082         ilock(ip);
8083         return -1;
8084       }
8085       sleep(&input.r, &cons.lock);
8086     }
8087     c = input.buf[input.r++ % INPUT_BUF];
8088     if(c == C('D')){  // EOF
8089       if(n < target){
8090         // Save ^D for next time, to make sure
8091         // caller gets a 0-byte result.
8092         input.r--;
8093       }
8094       break;
8095     }
8096     *dst++ = c;
8097     --n;
8098     if(c == '\n')
8099       break;
8100   }
8101   release(&cons.lock);
8102   ilock(ip);
8103 
8104   return target - n;
8105 }
8106 
8107 int
8108 consolewrite(struct inode *ip, char *buf, int n)
8109 {
8110   int i;
8111 
8112   iunlock(ip);
8113   acquire(&cons.lock);
8114   for(i = 0; i < n; i++)
8115     consputc(buf[i] & 0xff);
8116   release(&cons.lock);
8117   ilock(ip);
8118 
8119   return n;
8120 }
8121 
8122 void
8123 consoleinit(void)
8124 {
8125   initlock(&cons.lock, "console");
8126 
8127   devsw[CONSOLE].write = consolewrite;
8128   devsw[CONSOLE].read = consoleread;
8129   cons.locking = 1;
8130 
8131   picenable(IRQ_KBD);
8132   ioapicenable(IRQ_KBD, 0);
8133 }
8134 
8135 
8136 
8137 
8138 
8139 
8140 
8141 
8142 
8143 
8144 
8145 
8146 
8147 
8148 
8149 
