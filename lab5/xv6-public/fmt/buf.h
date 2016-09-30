3750 struct buf {
3751   int flags;
3752   uint dev;
3753   uint blockno;
3754   struct buf *prev; // LRU cache list
3755   struct buf *next;
3756   struct buf *qnext; // disk queue
3757   uchar data[BSIZE];
3758 };
3759 #define B_BUSY  0x1  // buffer is locked by some process
3760 #define B_VALID 0x2  // buffer has been read from disk
3761 #define B_DIRTY 0x4  // buffer needs to be written to disk
3762 
3763 
3764 
3765 
3766 
3767 
3768 
3769 
3770 
3771 
3772 
3773 
3774 
3775 
3776 
3777 
3778 
3779 
3780 
3781 
3782 
3783 
3784 
3785 
3786 
3787 
3788 
3789 
3790 
3791 
3792 
3793 
3794 
3795 
3796 
3797 
3798 
3799 
