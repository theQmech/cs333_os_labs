8150 // Intel 8253/8254/82C54 Programmable Interval Timer (PIT).
8151 // Only used on uniprocessors;
8152 // SMP machines use the local APIC timer.
8153 
8154 #include "types.h"
8155 #include "defs.h"
8156 #include "traps.h"
8157 #include "x86.h"
8158 
8159 #define IO_TIMER1       0x040           // 8253 Timer #1
8160 
8161 // Frequency of all three count-down timers;
8162 // (TIMER_FREQ/freq) is the appropriate count
8163 // to generate a frequency of freq Hz.
8164 
8165 #define TIMER_FREQ      1193182
8166 #define TIMER_DIV(x)    ((TIMER_FREQ+(x)/2)/(x))
8167 
8168 #define TIMER_MODE      (IO_TIMER1 + 3) // timer mode port
8169 #define TIMER_SEL0      0x00    // select counter 0
8170 #define TIMER_RATEGEN   0x04    // mode 2, rate generator
8171 #define TIMER_16BIT     0x30    // r/w counter 16 bits, LSB first
8172 
8173 void
8174 timerinit(void)
8175 {
8176   // Interrupt 100 times/sec.
8177   outb(TIMER_MODE, TIMER_SEL0 | TIMER_RATEGEN | TIMER_16BIT);
8178   outb(IO_TIMER1, TIMER_DIV(100) % 256);
8179   outb(IO_TIMER1, TIMER_DIV(100) / 256);
8180   picenable(IRQ_TIMER);
8181 }
8182 
8183 
8184 
8185 
8186 
8187 
8188 
8189 
8190 
8191 
8192 
8193 
8194 
8195 
8196 
8197 
8198 
8199 
