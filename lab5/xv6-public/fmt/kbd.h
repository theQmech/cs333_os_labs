7600 // PC keyboard interface constants
7601 
7602 #define KBSTATP         0x64    // kbd controller status port(I)
7603 #define KBS_DIB         0x01    // kbd data in buffer
7604 #define KBDATAP         0x60    // kbd data port(I)
7605 
7606 #define NO              0
7607 
7608 #define SHIFT           (1<<0)
7609 #define CTL             (1<<1)
7610 #define ALT             (1<<2)
7611 
7612 #define CAPSLOCK        (1<<3)
7613 #define NUMLOCK         (1<<4)
7614 #define SCROLLLOCK      (1<<5)
7615 
7616 #define E0ESC           (1<<6)
7617 
7618 // Special keycodes
7619 #define KEY_HOME        0xE0
7620 #define KEY_END         0xE1
7621 #define KEY_UP          0xE2
7622 #define KEY_DN          0xE3
7623 #define KEY_LF          0xE4
7624 #define KEY_RT          0xE5
7625 #define KEY_PGUP        0xE6
7626 #define KEY_PGDN        0xE7
7627 #define KEY_INS         0xE8
7628 #define KEY_DEL         0xE9
7629 
7630 // C('A') == Control-A
7631 #define C(x) (x - '@')
7632 
7633 static uchar shiftcode[256] =
7634 {
7635   [0x1D] CTL,
7636   [0x2A] SHIFT,
7637   [0x36] SHIFT,
7638   [0x38] ALT,
7639   [0x9D] CTL,
7640   [0xB8] ALT
7641 };
7642 
7643 static uchar togglecode[256] =
7644 {
7645   [0x3A] CAPSLOCK,
7646   [0x45] NUMLOCK,
7647   [0x46] SCROLLLOCK
7648 };
7649 
7650 static uchar normalmap[256] =
7651 {
7652   NO,   0x1B, '1',  '2',  '3',  '4',  '5',  '6',  // 0x00
7653   '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',
7654   'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  // 0x10
7655   'o',  'p',  '[',  ']',  '\n', NO,   'a',  's',
7656   'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',  // 0x20
7657   '\'', '`',  NO,   '\\', 'z',  'x',  'c',  'v',
7658   'b',  'n',  'm',  ',',  '.',  '/',  NO,   '*',  // 0x30
7659   NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
7660   NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
7661   '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
7662   '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
7663   [0x9C] '\n',      // KP_Enter
7664   [0xB5] '/',       // KP_Div
7665   [0xC8] KEY_UP,    [0xD0] KEY_DN,
7666   [0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
7667   [0xCB] KEY_LF,    [0xCD] KEY_RT,
7668   [0x97] KEY_HOME,  [0xCF] KEY_END,
7669   [0xD2] KEY_INS,   [0xD3] KEY_DEL
7670 };
7671 
7672 static uchar shiftmap[256] =
7673 {
7674   NO,   033,  '!',  '@',  '#',  '$',  '%',  '^',  // 0x00
7675   '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
7676   'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  // 0x10
7677   'O',  'P',  '{',  '}',  '\n', NO,   'A',  'S',
7678   'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  // 0x20
7679   '"',  '~',  NO,   '|',  'Z',  'X',  'C',  'V',
7680   'B',  'N',  'M',  '<',  '>',  '?',  NO,   '*',  // 0x30
7681   NO,   ' ',  NO,   NO,   NO,   NO,   NO,   NO,
7682   NO,   NO,   NO,   NO,   NO,   NO,   NO,   '7',  // 0x40
7683   '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
7684   '2',  '3',  '0',  '.',  NO,   NO,   NO,   NO,   // 0x50
7685   [0x9C] '\n',      // KP_Enter
7686   [0xB5] '/',       // KP_Div
7687   [0xC8] KEY_UP,    [0xD0] KEY_DN,
7688   [0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
7689   [0xCB] KEY_LF,    [0xCD] KEY_RT,
7690   [0x97] KEY_HOME,  [0xCF] KEY_END,
7691   [0xD2] KEY_INS,   [0xD3] KEY_DEL
7692 };
7693 
7694 
7695 
7696 
7697 
7698 
7699 
7700 static uchar ctlmap[256] =
7701 {
7702   NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
7703   NO,      NO,      NO,      NO,      NO,      NO,      NO,      NO,
7704   C('Q'),  C('W'),  C('E'),  C('R'),  C('T'),  C('Y'),  C('U'),  C('I'),
7705   C('O'),  C('P'),  NO,      NO,      '\r',    NO,      C('A'),  C('S'),
7706   C('D'),  C('F'),  C('G'),  C('H'),  C('J'),  C('K'),  C('L'),  NO,
7707   NO,      NO,      NO,      C('\\'), C('Z'),  C('X'),  C('C'),  C('V'),
7708   C('B'),  C('N'),  C('M'),  NO,      NO,      C('/'),  NO,      NO,
7709   [0x9C] '\r',      // KP_Enter
7710   [0xB5] C('/'),    // KP_Div
7711   [0xC8] KEY_UP,    [0xD0] KEY_DN,
7712   [0xC9] KEY_PGUP,  [0xD1] KEY_PGDN,
7713   [0xCB] KEY_LF,    [0xCD] KEY_RT,
7714   [0x97] KEY_HOME,  [0xCF] KEY_END,
7715   [0xD2] KEY_INS,   [0xD3] KEY_DEL
7716 };
7717 
7718 
7719 
7720 
7721 
7722 
7723 
7724 
7725 
7726 
7727 
7728 
7729 
7730 
7731 
7732 
7733 
7734 
7735 
7736 
7737 
7738 
7739 
7740 
7741 
7742 
7743 
7744 
7745 
7746 
7747 
7748 
7749 
