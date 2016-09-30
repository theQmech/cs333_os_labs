8200 // Intel 8250 serial port (UART).
8201 
8202 #include "types.h"
8203 #include "defs.h"
8204 #include "param.h"
8205 #include "traps.h"
8206 #include "spinlock.h"
8207 #include "fs.h"
8208 #include "file.h"
8209 #include "mmu.h"
8210 #include "proc.h"
8211 #include "x86.h"
8212 
8213 #define COM1    0x3f8
8214 
8215 static int uart;    // is there a uart?
8216 
8217 void
8218 uartinit(void)
8219 {
8220   char *p;
8221 
8222   // Turn off the FIFO
8223   outb(COM1+2, 0);
8224 
8225   // 9600 baud, 8 data bits, 1 stop bit, parity off.
8226   outb(COM1+3, 0x80);    // Unlock divisor
8227   outb(COM1+0, 115200/9600);
8228   outb(COM1+1, 0);
8229   outb(COM1+3, 0x03);    // Lock divisor, 8 data bits.
8230   outb(COM1+4, 0);
8231   outb(COM1+1, 0x01);    // Enable receive interrupts.
8232 
8233   // If status is 0xFF, no serial port.
8234   if(inb(COM1+5) == 0xFF)
8235     return;
8236   uart = 1;
8237 
8238   // Acknowledge pre-existing interrupt conditions;
8239   // enable interrupts.
8240   inb(COM1+2);
8241   inb(COM1+0);
8242   picenable(IRQ_COM1);
8243   ioapicenable(IRQ_COM1, 0);
8244 
8245   // Announce that we're here.
8246   for(p="xv6...\n"; *p; p++)
8247     uartputc(*p);
8248 }
8249 
8250 void
8251 uartputc(int c)
8252 {
8253   int i;
8254 
8255   if(!uart)
8256     return;
8257   for(i = 0; i < 128 && !(inb(COM1+5) & 0x20); i++)
8258     microdelay(10);
8259   outb(COM1+0, c);
8260 }
8261 
8262 static int
8263 uartgetc(void)
8264 {
8265   if(!uart)
8266     return -1;
8267   if(!(inb(COM1+5) & 0x01))
8268     return -1;
8269   return inb(COM1+0);
8270 }
8271 
8272 void
8273 uartintr(void)
8274 {
8275   consoleintr(uartgetc);
8276 }
8277 
8278 
8279 
8280 
8281 
8282 
8283 
8284 
8285 
8286 
8287 
8288 
8289 
8290 
8291 
8292 
8293 
8294 
8295 
8296 
8297 
8298 
8299 
