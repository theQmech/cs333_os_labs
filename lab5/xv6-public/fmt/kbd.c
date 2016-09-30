7750 #include "types.h"
7751 #include "x86.h"
7752 #include "defs.h"
7753 #include "kbd.h"
7754 
7755 int
7756 kbdgetc(void)
7757 {
7758   static uint shift;
7759   static uchar *charcode[4] = {
7760     normalmap, shiftmap, ctlmap, ctlmap
7761   };
7762   uint st, data, c;
7763 
7764   st = inb(KBSTATP);
7765   if((st & KBS_DIB) == 0)
7766     return -1;
7767   data = inb(KBDATAP);
7768 
7769   if(data == 0xE0){
7770     shift |= E0ESC;
7771     return 0;
7772   } else if(data & 0x80){
7773     // Key released
7774     data = (shift & E0ESC ? data : data & 0x7F);
7775     shift &= ~(shiftcode[data] | E0ESC);
7776     return 0;
7777   } else if(shift & E0ESC){
7778     // Last character was an E0 escape; or with 0x80
7779     data |= 0x80;
7780     shift &= ~E0ESC;
7781   }
7782 
7783   shift |= shiftcode[data];
7784   shift ^= togglecode[data];
7785   c = charcode[shift & (CTL | SHIFT)][data];
7786   if(shift & CAPSLOCK){
7787     if('a' <= c && c <= 'z')
7788       c += 'A' - 'a';
7789     else if('A' <= c && c <= 'Z')
7790       c += 'a' - 'A';
7791   }
7792   return c;
7793 }
7794 
7795 void
7796 kbdintr(void)
7797 {
7798   consoleintr(kbdgetc);
7799 }
