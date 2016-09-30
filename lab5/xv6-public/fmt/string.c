6600 #include "types.h"
6601 #include "x86.h"
6602 
6603 void*
6604 memset(void *dst, int c, uint n)
6605 {
6606   if ((int)dst%4 == 0 && n%4 == 0){
6607     c &= 0xFF;
6608     stosl(dst, (c<<24)|(c<<16)|(c<<8)|c, n/4);
6609   } else
6610     stosb(dst, c, n);
6611   return dst;
6612 }
6613 
6614 int
6615 memcmp(const void *v1, const void *v2, uint n)
6616 {
6617   const uchar *s1, *s2;
6618 
6619   s1 = v1;
6620   s2 = v2;
6621   while(n-- > 0){
6622     if(*s1 != *s2)
6623       return *s1 - *s2;
6624     s1++, s2++;
6625   }
6626 
6627   return 0;
6628 }
6629 
6630 void*
6631 memmove(void *dst, const void *src, uint n)
6632 {
6633   const char *s;
6634   char *d;
6635 
6636   s = src;
6637   d = dst;
6638   if(s < d && s + n > d){
6639     s += n;
6640     d += n;
6641     while(n-- > 0)
6642       *--d = *--s;
6643   } else
6644     while(n-- > 0)
6645       *d++ = *s++;
6646 
6647   return dst;
6648 }
6649 
6650 // memcpy exists to placate GCC.  Use memmove.
6651 void*
6652 memcpy(void *dst, const void *src, uint n)
6653 {
6654   return memmove(dst, src, n);
6655 }
6656 
6657 int
6658 strncmp(const char *p, const char *q, uint n)
6659 {
6660   while(n > 0 && *p && *p == *q)
6661     n--, p++, q++;
6662   if(n == 0)
6663     return 0;
6664   return (uchar)*p - (uchar)*q;
6665 }
6666 
6667 char*
6668 strncpy(char *s, const char *t, int n)
6669 {
6670   char *os;
6671 
6672   os = s;
6673   while(n-- > 0 && (*s++ = *t++) != 0)
6674     ;
6675   while(n-- > 0)
6676     *s++ = 0;
6677   return os;
6678 }
6679 
6680 // Like strncpy but guaranteed to NUL-terminate.
6681 char*
6682 safestrcpy(char *s, const char *t, int n)
6683 {
6684   char *os;
6685 
6686   os = s;
6687   if(n <= 0)
6688     return os;
6689   while(--n > 0 && (*s++ = *t++) != 0)
6690     ;
6691   *s = 0;
6692   return os;
6693 }
6694 
6695 
6696 
6697 
6698 
6699 
6700 int
6701 strlen(const char *s)
6702 {
6703   int n;
6704 
6705   for(n = 0; s[n]; n++)
6706     ;
6707   return n;
6708 }
6709 
6710 
6711 
6712 
6713 
6714 
6715 
6716 
6717 
6718 
6719 
6720 
6721 
6722 
6723 
6724 
6725 
6726 
6727 
6728 
6729 
6730 
6731 
6732 
6733 
6734 
6735 
6736 
6737 
6738 
6739 
6740 
6741 
6742 
6743 
6744 
6745 
6746 
6747 
6748 
6749 
