3900 // On-disk file system format.
3901 // Both the kernel and user programs use this header file.
3902 
3903 
3904 #define ROOTINO 1  // root i-number
3905 #define BSIZE 512  // block size
3906 
3907 // Disk layout:
3908 // [ boot block | super block | log | inode blocks | free bit map | data blocks ]
3909 //
3910 // mkfs computes the super block and builds an initial file system. The super describes
3911 // the disk layout:
3912 struct superblock {
3913   uint size;         // Size of file system image (blocks)
3914   uint nblocks;      // Number of data blocks
3915   uint ninodes;      // Number of inodes.
3916   uint nlog;         // Number of log blocks
3917   uint logstart;     // Block number of first log block
3918   uint inodestart;   // Block number of first inode block
3919   uint bmapstart;    // Block number of first free map block
3920 };
3921 
3922 #define NDIRECT 12
3923 #define NINDIRECT (BSIZE / sizeof(uint))
3924 #define MAXFILE (NDIRECT + NINDIRECT)
3925 
3926 // On-disk inode structure
3927 struct dinode {
3928   short type;           // File type
3929   short major;          // Major device number (T_DEV only)
3930   short minor;          // Minor device number (T_DEV only)
3931   short nlink;          // Number of links to inode in file system
3932   uint size;            // Size of file (bytes)
3933   uint addrs[NDIRECT+1];   // Data block addresses
3934 };
3935 
3936 
3937 
3938 
3939 
3940 
3941 
3942 
3943 
3944 
3945 
3946 
3947 
3948 
3949 
3950 // Inodes per block.
3951 #define IPB           (BSIZE / sizeof(struct dinode))
3952 
3953 // Block containing inode i
3954 #define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)
3955 
3956 // Bitmap bits per block
3957 #define BPB           (BSIZE*8)
3958 
3959 // Block of free map containing bit for block b
3960 #define BBLOCK(b, sb) (b/BPB + sb.bmapstart)
3961 
3962 // Directory is a file containing a sequence of dirent structures.
3963 #define DIRSIZ 14
3964 
3965 struct dirent {
3966   ushort inum;
3967   char name[DIRSIZ];
3968 };
3969 
3970 
3971 
3972 
3973 
3974 
3975 
3976 
3977 
3978 
3979 
3980 
3981 
3982 
3983 
3984 
3985 
3986 
3987 
3988 
3989 
3990 
3991 
3992 
3993 
3994 
3995 
3996 
3997 
3998 
3999 
