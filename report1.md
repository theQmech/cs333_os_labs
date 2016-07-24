1.	The CPU has 4 cores. Total memory: 8146892 kB.
 
	Free memory: 1193748 kB. 14.65% memory free. 
	6328631 context switches. 
	14034 processes forked since startup

2. 	**cpu** Memory. The CPU usage was always above 98%. 
		This is expected since the code is intensive on arithmetic operation.
	cpu-print Disk operation. Output of command pidstat reveals 
		that the process makes lot of write operations on 
		disk(we wrote to file /tmp/myfile). This is exactly what is 
		expected because the bottleneck seems to be the printf() 
		or the write() system calls which are expensive.
	disk - 	Disk. Output of `pidstat -dl -p <pid>` reveals that 
		the process is using lots of read operations. The process reads
		files in a random order and hence the buffer isn't really useful here  
	disk1-	CPU. The CPU usage is always high ie. almost 100%. The process reads
		the same file over and over again and is hence stored in the 
		buffer. Thus disk operations aren't a bottleneck here.

3.	cat /proc/pid/

4.	"./cpu" - voluntary 1 nonvn 3000
   	"./cpu-print" - vln 65K nonvn 28Mil

   	Reason - cpu program has no system calls to be made hence no voluntary.
   	cpu-print has no system calls hence more vln calls 

5.	Used the command `pstree -ps $$`.
	init(1)---lightdm(1388)---lightdm(10435)---init(10460)---tmux(11653)---bash(12329)

6.	We use the tools `whereis`
	$ whereis {cd,ls,history,ps}
	cd:
	ls: /bin/ls /usr/share/man/man1/ls.1.gz
	history: /usr/share/man/man3/history.3readline.gz
	ps: /bin/ps /usr/share/man/man1/ps.1.gz
	Thus the last three are exec'ed and the first is implemented by bash.

7.	File desciptors are pointing to
	0 -> /dev/pts/0
	1 -> /users/ug14/iamutkarsh/cs347/temp.txt 
	2 -> /dev/pts/0 
	
	Every process has three io files descriptors 
	0 (default stdin) 1 (default stdout) and 2 (default stderr)
	
	On I/O redirection bash changes the corresponding file desciptors 
	(0/1/2) to the specified file desciptor. 
	./cpu-print > temp.txt changes file desciptor 1 from stdout to temp.txt

8.	There are two processes spawned by this command
	16723 iamutkars  20   0  4192   356   276 R 100.  0.0  9:28.58 ./cpu-print 
	16724 iamutkars  20   0 11740   912   784 R 26.4  0.0  2:26.08 grep hello  
	
	The 0(input) file decriptor of grep is the 1(output) file desciptor
	of "./cpu-print" hence in bash pipes point the output file desciptor 
	of left_process to the input file desciptor of the right_process. 
	Hence data written to the write end of the pipe is buffered by the kernel 
	until it is read from the read end of the pipe. 
	Pipes create a temporary file buffer in between the read and the write end 
	which can be seen in the file descriptors of the two processes. 

