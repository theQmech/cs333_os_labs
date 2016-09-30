5800 //
5801 // File-system system calls.
5802 // Mostly argument checking, since we don't trust
5803 // user code, and calls into file.c and fs.c.
5804 //
5805 
5806 #include "types.h"
5807 #include "defs.h"
5808 #include "param.h"
5809 #include "stat.h"
5810 #include "mmu.h"
5811 #include "proc.h"
5812 #include "fs.h"
5813 #include "file.h"
5814 #include "fcntl.h"
5815 
5816 // Fetch the nth word-sized system call argument as a file descriptor
5817 // and return both the descriptor and the corresponding struct file.
5818 static int
5819 argfd(int n, int *pfd, struct file **pf)
5820 {
5821   int fd;
5822   struct file *f;
5823 
5824   if(argint(n, &fd) < 0)
5825     return -1;
5826   if(fd < 0 || fd >= NOFILE || (f=proc->ofile[fd]) == 0)
5827     return -1;
5828   if(pfd)
5829     *pfd = fd;
5830   if(pf)
5831     *pf = f;
5832   return 0;
5833 }
5834 
5835 // Allocate a file descriptor for the given file.
5836 // Takes over file reference from caller on success.
5837 static int
5838 fdalloc(struct file *f)
5839 {
5840   int fd;
5841 
5842   for(fd = 0; fd < NOFILE; fd++){
5843     if(proc->ofile[fd] == 0){
5844       proc->ofile[fd] = f;
5845       return fd;
5846     }
5847   }
5848   return -1;
5849 }
5850 int
5851 sys_dup(void)
5852 {
5853   struct file *f;
5854   int fd;
5855 
5856   if(argfd(0, 0, &f) < 0)
5857     return -1;
5858   if((fd=fdalloc(f)) < 0)
5859     return -1;
5860   filedup(f);
5861   return fd;
5862 }
5863 
5864 int
5865 sys_read(void)
5866 {
5867   struct file *f;
5868   int n;
5869   char *p;
5870 
5871   if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
5872     return -1;
5873   return fileread(f, p, n);
5874 }
5875 
5876 int
5877 sys_write(void)
5878 {
5879   struct file *f;
5880   int n;
5881   char *p;
5882 
5883   if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
5884     return -1;
5885   return filewrite(f, p, n);
5886 }
5887 
5888 int
5889 sys_close(void)
5890 {
5891   int fd;
5892   struct file *f;
5893 
5894   if(argfd(0, &fd, &f) < 0)
5895     return -1;
5896   proc->ofile[fd] = 0;
5897   fileclose(f);
5898   return 0;
5899 }
5900 int
5901 sys_fstat(void)
5902 {
5903   struct file *f;
5904   struct stat *st;
5905 
5906   if(argfd(0, 0, &f) < 0 || argptr(1, (void*)&st, sizeof(*st)) < 0)
5907     return -1;
5908   return filestat(f, st);
5909 }
5910 
5911 // Create the path new as a link to the same inode as old.
5912 int
5913 sys_link(void)
5914 {
5915   char name[DIRSIZ], *new, *old;
5916   struct inode *dp, *ip;
5917 
5918   if(argstr(0, &old) < 0 || argstr(1, &new) < 0)
5919     return -1;
5920 
5921   begin_op();
5922   if((ip = namei(old)) == 0){
5923     end_op();
5924     return -1;
5925   }
5926 
5927   ilock(ip);
5928   if(ip->type == T_DIR){
5929     iunlockput(ip);
5930     end_op();
5931     return -1;
5932   }
5933 
5934   ip->nlink++;
5935   iupdate(ip);
5936   iunlock(ip);
5937 
5938   if((dp = nameiparent(new, name)) == 0)
5939     goto bad;
5940   ilock(dp);
5941   if(dp->dev != ip->dev || dirlink(dp, name, ip->inum) < 0){
5942     iunlockput(dp);
5943     goto bad;
5944   }
5945   iunlockput(dp);
5946   iput(ip);
5947 
5948   end_op();
5949 
5950   return 0;
5951 
5952 bad:
5953   ilock(ip);
5954   ip->nlink--;
5955   iupdate(ip);
5956   iunlockput(ip);
5957   end_op();
5958   return -1;
5959 }
5960 
5961 // Is the directory dp empty except for "." and ".." ?
5962 static int
5963 isdirempty(struct inode *dp)
5964 {
5965   int off;
5966   struct dirent de;
5967 
5968   for(off=2*sizeof(de); off<dp->size; off+=sizeof(de)){
5969     if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
5970       panic("isdirempty: readi");
5971     if(de.inum != 0)
5972       return 0;
5973   }
5974   return 1;
5975 }
5976 
5977 
5978 
5979 
5980 
5981 
5982 
5983 
5984 
5985 
5986 
5987 
5988 
5989 
5990 
5991 
5992 
5993 
5994 
5995 
5996 
5997 
5998 
5999 
6000 int
6001 sys_unlink(void)
6002 {
6003   struct inode *ip, *dp;
6004   struct dirent de;
6005   char name[DIRSIZ], *path;
6006   uint off;
6007 
6008   if(argstr(0, &path) < 0)
6009     return -1;
6010 
6011   begin_op();
6012   if((dp = nameiparent(path, name)) == 0){
6013     end_op();
6014     return -1;
6015   }
6016 
6017   ilock(dp);
6018 
6019   // Cannot unlink "." or "..".
6020   if(namecmp(name, ".") == 0 || namecmp(name, "..") == 0)
6021     goto bad;
6022 
6023   if((ip = dirlookup(dp, name, &off)) == 0)
6024     goto bad;
6025   ilock(ip);
6026 
6027   if(ip->nlink < 1)
6028     panic("unlink: nlink < 1");
6029   if(ip->type == T_DIR && !isdirempty(ip)){
6030     iunlockput(ip);
6031     goto bad;
6032   }
6033 
6034   memset(&de, 0, sizeof(de));
6035   if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
6036     panic("unlink: writei");
6037   if(ip->type == T_DIR){
6038     dp->nlink--;
6039     iupdate(dp);
6040   }
6041   iunlockput(dp);
6042 
6043   ip->nlink--;
6044   iupdate(ip);
6045   iunlockput(ip);
6046 
6047   end_op();
6048 
6049   return 0;
6050 bad:
6051   iunlockput(dp);
6052   end_op();
6053   return -1;
6054 }
6055 
6056 static struct inode*
6057 create(char *path, short type, short major, short minor)
6058 {
6059   uint off;
6060   struct inode *ip, *dp;
6061   char name[DIRSIZ];
6062 
6063   if((dp = nameiparent(path, name)) == 0)
6064     return 0;
6065   ilock(dp);
6066 
6067   if((ip = dirlookup(dp, name, &off)) != 0){
6068     iunlockput(dp);
6069     ilock(ip);
6070     if(type == T_FILE && ip->type == T_FILE)
6071       return ip;
6072     iunlockput(ip);
6073     return 0;
6074   }
6075 
6076   if((ip = ialloc(dp->dev, type)) == 0)
6077     panic("create: ialloc");
6078 
6079   ilock(ip);
6080   ip->major = major;
6081   ip->minor = minor;
6082   ip->nlink = 1;
6083   iupdate(ip);
6084 
6085   if(type == T_DIR){  // Create . and .. entries.
6086     dp->nlink++;  // for ".."
6087     iupdate(dp);
6088     // No ip->nlink++ for ".": avoid cyclic ref count.
6089     if(dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
6090       panic("create dots");
6091   }
6092 
6093   if(dirlink(dp, name, ip->inum) < 0)
6094     panic("create: dirlink");
6095 
6096   iunlockput(dp);
6097 
6098   return ip;
6099 }
6100 int
6101 sys_open(void)
6102 {
6103   char *path;
6104   int fd, omode;
6105   struct file *f;
6106   struct inode *ip;
6107 
6108   if(argstr(0, &path) < 0 || argint(1, &omode) < 0)
6109     return -1;
6110 
6111   begin_op();
6112 
6113   if(omode & O_CREATE){
6114     ip = create(path, T_FILE, 0, 0);
6115     if(ip == 0){
6116       end_op();
6117       return -1;
6118     }
6119   } else {
6120     if((ip = namei(path)) == 0){
6121       end_op();
6122       return -1;
6123     }
6124     ilock(ip);
6125     if(ip->type == T_DIR && omode != O_RDONLY){
6126       iunlockput(ip);
6127       end_op();
6128       return -1;
6129     }
6130   }
6131 
6132   if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){
6133     if(f)
6134       fileclose(f);
6135     iunlockput(ip);
6136     end_op();
6137     return -1;
6138   }
6139   iunlock(ip);
6140   end_op();
6141 
6142   f->type = FD_INODE;
6143   f->ip = ip;
6144   f->off = 0;
6145   f->readable = !(omode & O_WRONLY);
6146   f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
6147   return fd;
6148 }
6149 
6150 int
6151 sys_mkdir(void)
6152 {
6153   char *path;
6154   struct inode *ip;
6155 
6156   begin_op();
6157   if(argstr(0, &path) < 0 || (ip = create(path, T_DIR, 0, 0)) == 0){
6158     end_op();
6159     return -1;
6160   }
6161   iunlockput(ip);
6162   end_op();
6163   return 0;
6164 }
6165 
6166 int
6167 sys_mknod(void)
6168 {
6169   struct inode *ip;
6170   char *path;
6171   int len;
6172   int major, minor;
6173 
6174   begin_op();
6175   if((len=argstr(0, &path)) < 0 ||
6176      argint(1, &major) < 0 ||
6177      argint(2, &minor) < 0 ||
6178      (ip = create(path, T_DEV, major, minor)) == 0){
6179     end_op();
6180     return -1;
6181   }
6182   iunlockput(ip);
6183   end_op();
6184   return 0;
6185 }
6186 
6187 
6188 
6189 
6190 
6191 
6192 
6193 
6194 
6195 
6196 
6197 
6198 
6199 
6200 int
6201 sys_chdir(void)
6202 {
6203   char *path;
6204   struct inode *ip;
6205 
6206   begin_op();
6207   if(argstr(0, &path) < 0 || (ip = namei(path)) == 0){
6208     end_op();
6209     return -1;
6210   }
6211   ilock(ip);
6212   if(ip->type != T_DIR){
6213     iunlockput(ip);
6214     end_op();
6215     return -1;
6216   }
6217   iunlock(ip);
6218   iput(proc->cwd);
6219   end_op();
6220   proc->cwd = ip;
6221   return 0;
6222 }
6223 
6224 int
6225 sys_exec(void)
6226 {
6227   char *path, *argv[MAXARG];
6228   int i;
6229   uint uargv, uarg;
6230 
6231   if(argstr(0, &path) < 0 || argint(1, (int*)&uargv) < 0){
6232     return -1;
6233   }
6234   memset(argv, 0, sizeof(argv));
6235   for(i=0;; i++){
6236     if(i >= NELEM(argv))
6237       return -1;
6238     if(fetchint(uargv+4*i, (int*)&uarg) < 0)
6239       return -1;
6240     if(uarg == 0){
6241       argv[i] = 0;
6242       break;
6243     }
6244     if(fetchstr(uarg, &argv[i]) < 0)
6245       return -1;
6246   }
6247   return exec(path, argv);
6248 }
6249 
6250 int
6251 sys_pipe(void)
6252 {
6253   int *fd;
6254   struct file *rf, *wf;
6255   int fd0, fd1;
6256 
6257   if(argptr(0, (void*)&fd, 2*sizeof(fd[0])) < 0)
6258     return -1;
6259   if(pipealloc(&rf, &wf) < 0)
6260     return -1;
6261   fd0 = -1;
6262   if((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0){
6263     if(fd0 >= 0)
6264       proc->ofile[fd0] = 0;
6265     fileclose(rf);
6266     fileclose(wf);
6267     return -1;
6268   }
6269   fd[0] = fd0;
6270   fd[1] = fd1;
6271   return 0;
6272 }
6273 
6274 
6275 
6276 
6277 
6278 
6279 
6280 
6281 
6282 
6283 
6284 
6285 
6286 
6287 
6288 
6289 
6290 
6291 
6292 
6293 
6294 
6295 
6296 
6297 
6298 
6299 
