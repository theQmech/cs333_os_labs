4000 struct file {
4001   enum { FD_NONE, FD_PIPE, FD_INODE } type;
4002   int ref; // reference count
4003   char readable;
4004   char writable;
4005   struct pipe *pipe;
4006   struct inode *ip;
4007   uint off;
4008 };
4009 
4010 
4011 // in-memory copy of an inode
4012 struct inode {
4013   uint dev;           // Device number
4014   uint inum;          // Inode number
4015   int ref;            // Reference count
4016   int flags;          // I_BUSY, I_VALID
4017 
4018   short type;         // copy of disk inode
4019   short major;
4020   short minor;
4021   short nlink;
4022   uint size;
4023   uint addrs[NDIRECT+1];
4024 };
4025 #define I_BUSY 0x1
4026 #define I_VALID 0x2
4027 
4028 // table mapping major device number to
4029 // device functions
4030 struct devsw {
4031   int (*read)(struct inode*, char*, int);
4032   int (*write)(struct inode*, char*, int);
4033 };
4034 
4035 extern struct devsw devsw[];
4036 
4037 #define CONSOLE 1
4038 
4039 
4040 
4041 
4042 
4043 
4044 
4045 
4046 
4047 
4048 
4049 
4050 // Blank page.
4051 
4052 
4053 
4054 
4055 
4056 
4057 
4058 
4059 
4060 
4061 
4062 
4063 
4064 
4065 
4066 
4067 
4068 
4069 
4070 
4071 
4072 
4073 
4074 
4075 
4076 
4077 
4078 
4079 
4080 
4081 
4082 
4083 
4084 
4085 
4086 
4087 
4088 
4089 
4090 
4091 
4092 
4093 
4094 
4095 
4096 
4097 
4098 
4099 
