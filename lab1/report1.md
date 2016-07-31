#CS 333 Operating Systems Lab#
## 140050005 Rupanshu Ganvir
## 140050009 Utkarsh Gautam


1.	The CPU has 4 cores. Total memory: 8146892 kB.
 
	Free memory: 1193748 kB. 14.65% memory free. 
	
	6328631 context switches. 14034 processes forked since startup

2. 	**cpu** CPU. The CPU usage was always above 98%. This is expected since the code is intensive on arithmetic operation and diesnt nothing else.

	**cpu-print** Memory. In this case CPU usage is quite low. That can't be the bottleneck. Disk operations are nowhere involved. The expensive task seems to be the write operations which are handled by memory(when ouput is on default stdout). On using `./cpu-print >/tmp/myfile` we found that this was no longer the case since the output was then written on the disk. Disk Operations were the bottlenech in this case.
	
	**disk** Disk. Output of ``pidstat -dl -p <pid>`` reveals that the process is using lots of read operations. The process reads files in a random order and hence the buffer isn't really useful here  
	**disk1** CPU. The CPU usage is always high ie. almost 100%. The process reads the same file over and over again and is hence stored in the buffer. Thus disk operations aren't a bottleneck anymore.

3.	Following are the outputs:
	```
	cat /proc/pid/stat
	-----------------------------------
	Executable    system time user time
	----------    ----------- ---------
	`./cpu`             20465        28
	`./cpu-print`         238      1380
	-----------------------------------
	```
	`cpu` spends more time in user mode and `cpu-print` spends more time in kernel mode.

	`cpu` program has to make no system calls. All it does is computation which doesn't require program to go into kernel mode, whereas in cpu-print the program makes two system calls `gettimeofday()` and `printf()` very often for which it needs PC to go in kernel mode hence it spends more time in kernel mode.Computation for `cpu-print` is very less hence it spends less time user mode as compared to system mode.

4.	`./cpu` - volun 1 nonvln 14021  - Mostly non voluntary context switches.

   	`./cpu-print` - vln 65K nonvln 28Mil - Mostly non voluntary context switches

   	Reason -
   	`cpu` program has no system calls to be made only computation hence only one initial context switch to execute it.
   	Since memory is the bottleneck for this process, nonvoluntary context switches are due to excess memory utilization 
   	for a longer period of time("the forever while loop").

   	`cpu-print` has blocking system calls `gettimeofday()` which causes most of the voluntary context switches. Since disk(i/o) is the
   	bottleneck of this excess i/o system calls which keep CPU to wait are the reason for non voluntary context switches.
   	Rather than  forever waiting for i/o system calls(the forever while loop ), the program schduler switches context to other processes.

5.	Output of `pstree`
	```
	$ pstree -ps $$
	init(1)---lightdm(1388)---lightdm(10435)---init(10460)
	---tmux(11653)---bash(12329)---pstree(14990)
	```

6.	We use the tool `whereis`
	```	
	$ whereis {cd,ls,history,ps}
	cd:
	ls: /bin/ls /usr/share/man/man1/ls.1.gz
	history: /usr/share/man/man3/history.3readline.gz
	ps: /bin/ps /usr/share/man/man1/ps.1.gz
	```
	It is clear that `cd` and `history` have no executable. `history` only has a man page.	
	Thus,

	`cd` and `history` - implemented by bash

	`ls` and `ps` - exec'ed by bash

7.	File desciptors are pointing to
	```	
	0 -> /dev/pts/0
	1 -> /users/ug14/iamutkarsh/cs347/temp.txt 
	2 -> /dev/pts/0 
	```
	Every process has three io files descriptors 
	0 (default stdin) 1 (default stdout) and 2 (default stderr)
	
	On I/O redirection bash changes the corresponding file desciptors 
	(0/1/2) to the specified file desciptor. 
	`./cpu-print > temp.txt` changes file desciptor 1 from stdout to temp.txt

8.	There are two processes spawned by this command
	```
	16723 iamutkars  20   0  4192   356   276 R 100.  0.0  9:28.58 ./cpu-print 
	16724 iamutkars  20   0 11740   912   784 R 26.4  0.0  2:26.08 grep hello  
	```
		
 	![File descriptor of `cpu-print`](./cpu-print.png)

	![File descriptor of `grep`](./grep.png) 

	The 0(input) file decriptor of grep is the 1(output) file desciptor of `./cpu-print`
	hence in bash pipes point the output file desciptor  of left_process to the input file desciptor of 
	the right_process. Pipes creates a temporary file buffer in between the left and the right end of pipe 
	which can be seen in the file descriptors of the two processes. Data written to the right end of the pipe 
	is buffered by the kernel until it is read from the read end of the pipe. 
 

