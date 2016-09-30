7100 // The local APIC manages internal (non-I/O) interrupts.
7101 // See Chapter 8 & Appendix C of Intel processor manual volume 3.
7102 
7103 #include "types.h"
7104 #include "defs.h"
7105 #include "date.h"
7106 #include "memlayout.h"
7107 #include "traps.h"
7108 #include "mmu.h"
7109 #include "x86.h"
7110 
7111 // Local APIC registers, divided by 4 for use as uint[] indices.
7112 #define ID      (0x0020/4)   // ID
7113 #define VER     (0x0030/4)   // Version
7114 #define TPR     (0x0080/4)   // Task Priority
7115 #define EOI     (0x00B0/4)   // EOI
7116 #define SVR     (0x00F0/4)   // Spurious Interrupt Vector
7117   #define ENABLE     0x00000100   // Unit Enable
7118 #define ESR     (0x0280/4)   // Error Status
7119 #define ICRLO   (0x0300/4)   // Interrupt Command
7120   #define INIT       0x00000500   // INIT/RESET
7121   #define STARTUP    0x00000600   // Startup IPI
7122   #define DELIVS     0x00001000   // Delivery status
7123   #define ASSERT     0x00004000   // Assert interrupt (vs deassert)
7124   #define DEASSERT   0x00000000
7125   #define LEVEL      0x00008000   // Level triggered
7126   #define BCAST      0x00080000   // Send to all APICs, including self.
7127   #define BUSY       0x00001000
7128   #define FIXED      0x00000000
7129 #define ICRHI   (0x0310/4)   // Interrupt Command [63:32]
7130 #define TIMER   (0x0320/4)   // Local Vector Table 0 (TIMER)
7131   #define X1         0x0000000B   // divide counts by 1
7132   #define PERIODIC   0x00020000   // Periodic
7133 #define PCINT   (0x0340/4)   // Performance Counter LVT
7134 #define LINT0   (0x0350/4)   // Local Vector Table 1 (LINT0)
7135 #define LINT1   (0x0360/4)   // Local Vector Table 2 (LINT1)
7136 #define ERROR   (0x0370/4)   // Local Vector Table 3 (ERROR)
7137   #define MASKED     0x00010000   // Interrupt masked
7138 #define TICR    (0x0380/4)   // Timer Initial Count
7139 #define TCCR    (0x0390/4)   // Timer Current Count
7140 #define TDCR    (0x03E0/4)   // Timer Divide Configuration
7141 
7142 volatile uint *lapic;  // Initialized in mp.c
7143 
7144 static void
7145 lapicw(int index, int value)
7146 {
7147   lapic[index] = value;
7148   lapic[ID];  // wait for write to finish, by reading
7149 }
7150 
7151 
7152 
7153 
7154 
7155 
7156 
7157 
7158 
7159 
7160 
7161 
7162 
7163 
7164 
7165 
7166 
7167 
7168 
7169 
7170 
7171 
7172 
7173 
7174 
7175 
7176 
7177 
7178 
7179 
7180 
7181 
7182 
7183 
7184 
7185 
7186 
7187 
7188 
7189 
7190 
7191 
7192 
7193 
7194 
7195 
7196 
7197 
7198 
7199 
7200 void
7201 lapicinit(void)
7202 {
7203   if(!lapic)
7204     return;
7205 
7206   // Enable local APIC; set spurious interrupt vector.
7207   lapicw(SVR, ENABLE | (T_IRQ0 + IRQ_SPURIOUS));
7208 
7209   // The timer repeatedly counts down at bus frequency
7210   // from lapic[TICR] and then issues an interrupt.
7211   // If xv6 cared more about precise timekeeping,
7212   // TICR would be calibrated using an external time source.
7213   lapicw(TDCR, X1);
7214   lapicw(TIMER, PERIODIC | (T_IRQ0 + IRQ_TIMER));
7215   lapicw(TICR, 10000000);
7216 
7217   // Disable logical interrupt lines.
7218   lapicw(LINT0, MASKED);
7219   lapicw(LINT1, MASKED);
7220 
7221   // Disable performance counter overflow interrupts
7222   // on machines that provide that interrupt entry.
7223   if(((lapic[VER]>>16) & 0xFF) >= 4)
7224     lapicw(PCINT, MASKED);
7225 
7226   // Map error interrupt to IRQ_ERROR.
7227   lapicw(ERROR, T_IRQ0 + IRQ_ERROR);
7228 
7229   // Clear error status register (requires back-to-back writes).
7230   lapicw(ESR, 0);
7231   lapicw(ESR, 0);
7232 
7233   // Ack any outstanding interrupts.
7234   lapicw(EOI, 0);
7235 
7236   // Send an Init Level De-Assert to synchronise arbitration ID's.
7237   lapicw(ICRHI, 0);
7238   lapicw(ICRLO, BCAST | INIT | LEVEL);
7239   while(lapic[ICRLO] & DELIVS)
7240     ;
7241 
7242   // Enable interrupts on the APIC (but not on the processor).
7243   lapicw(TPR, 0);
7244 }
7245 
7246 
7247 
7248 
7249 
7250 int
7251 cpunum(void)
7252 {
7253   // Cannot call cpu when interrupts are enabled:
7254   // result not guaranteed to last long enough to be used!
7255   // Would prefer to panic but even printing is chancy here:
7256   // almost everything, including cprintf and panic, calls cpu,
7257   // often indirectly through acquire and release.
7258   if(readeflags()&FL_IF){
7259     static int n;
7260     if(n++ == 0)
7261       cprintf("cpu called from %x with interrupts enabled\n",
7262         __builtin_return_address(0));
7263   }
7264 
7265   if(lapic)
7266     return lapic[ID]>>24;
7267   return 0;
7268 }
7269 
7270 // Acknowledge interrupt.
7271 void
7272 lapiceoi(void)
7273 {
7274   if(lapic)
7275     lapicw(EOI, 0);
7276 }
7277 
7278 // Spin for a given number of microseconds.
7279 // On real hardware would want to tune this dynamically.
7280 void
7281 microdelay(int us)
7282 {
7283 }
7284 
7285 #define CMOS_PORT    0x70
7286 #define CMOS_RETURN  0x71
7287 
7288 // Start additional processor running entry code at addr.
7289 // See Appendix B of MultiProcessor Specification.
7290 void
7291 lapicstartap(uchar apicid, uint addr)
7292 {
7293   int i;
7294   ushort *wrv;
7295 
7296   // "The BSP must initialize CMOS shutdown code to 0AH
7297   // and the warm reset vector (DWORD based at 40:67) to point at
7298   // the AP startup code prior to the [universal startup algorithm]."
7299   outb(CMOS_PORT, 0xF);  // offset 0xF is shutdown code
7300   outb(CMOS_PORT+1, 0x0A);
7301   wrv = (ushort*)P2V((0x40<<4 | 0x67));  // Warm reset vector
7302   wrv[0] = 0;
7303   wrv[1] = addr >> 4;
7304 
7305   // "Universal startup algorithm."
7306   // Send INIT (level-triggered) interrupt to reset other CPU.
7307   lapicw(ICRHI, apicid<<24);
7308   lapicw(ICRLO, INIT | LEVEL | ASSERT);
7309   microdelay(200);
7310   lapicw(ICRLO, INIT | LEVEL);
7311   microdelay(100);    // should be 10ms, but too slow in Bochs!
7312 
7313   // Send startup IPI (twice!) to enter code.
7314   // Regular hardware is supposed to only accept a STARTUP
7315   // when it is in the halted state due to an INIT.  So the second
7316   // should be ignored, but it is part of the official Intel algorithm.
7317   // Bochs complains about the second one.  Too bad for Bochs.
7318   for(i = 0; i < 2; i++){
7319     lapicw(ICRHI, apicid<<24);
7320     lapicw(ICRLO, STARTUP | (addr>>12));
7321     microdelay(200);
7322   }
7323 }
7324 
7325 #define CMOS_STATA   0x0a
7326 #define CMOS_STATB   0x0b
7327 #define CMOS_UIP    (1 << 7)        // RTC update in progress
7328 
7329 #define SECS    0x00
7330 #define MINS    0x02
7331 #define HOURS   0x04
7332 #define DAY     0x07
7333 #define MONTH   0x08
7334 #define YEAR    0x09
7335 
7336 static uint cmos_read(uint reg)
7337 {
7338   outb(CMOS_PORT,  reg);
7339   microdelay(200);
7340 
7341   return inb(CMOS_RETURN);
7342 }
7343 
7344 
7345 
7346 
7347 
7348 
7349 
7350 static void fill_rtcdate(struct rtcdate *r)
7351 {
7352   r->second = cmos_read(SECS);
7353   r->minute = cmos_read(MINS);
7354   r->hour   = cmos_read(HOURS);
7355   r->day    = cmos_read(DAY);
7356   r->month  = cmos_read(MONTH);
7357   r->year   = cmos_read(YEAR);
7358 }
7359 
7360 // qemu seems to use 24-hour GWT and the values are BCD encoded
7361 void cmostime(struct rtcdate *r)
7362 {
7363   struct rtcdate t1, t2;
7364   int sb, bcd;
7365 
7366   sb = cmos_read(CMOS_STATB);
7367 
7368   bcd = (sb & (1 << 2)) == 0;
7369 
7370   // make sure CMOS doesn't modify time while we read it
7371   for (;;) {
7372     fill_rtcdate(&t1);
7373     if (cmos_read(CMOS_STATA) & CMOS_UIP)
7374         continue;
7375     fill_rtcdate(&t2);
7376     if (memcmp(&t1, &t2, sizeof(t1)) == 0)
7377       break;
7378   }
7379 
7380   // convert
7381   if (bcd) {
7382 #define    CONV(x)     (t1.x = ((t1.x >> 4) * 10) + (t1.x & 0xf))
7383     CONV(second);
7384     CONV(minute);
7385     CONV(hour  );
7386     CONV(day   );
7387     CONV(month );
7388     CONV(year  );
7389 #undef     CONV
7390   }
7391 
7392   *r = t1;
7393   r->year += 2000;
7394 }
7395 
7396 
7397 
7398 
7399 
