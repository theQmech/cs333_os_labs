Experimental Setup:
===================
We used two machines in SL2. Both were quad core i5-3330 with 3.00GHz and 
8 GB memory. Maximum network card speed being 1000Mbps. 

Compiling Code:
===============
To compile simply run `make server`
`make clean` clean all the files

The code `server-mt.cpp` has a macro defined `PRINT` which is set to 0.
Change PRINT to 1 to enable printing on server side.
 
Sampe Run (with printing disabled):
====================================
$ ./multi-client 127.0.0.1 54320 5 10 0 fixed
	Starting all clients 
	Thread 0 started
	Thread 1 started
	Thread 2 started
	Thread 3 started
	Thread 4 started
	Thread 3 exit. Count is 948.
	Thread 1 exit. Count is 947.
	Thread 2 exit. Count is 949.
	Thread 4 exit. Count is 946.
	Thread 0 exit. Count is 946.
	Thread 0: 946, 9.934173 sec
	Thread 1: 947, 9.916804 sec
	Thread 2: 949, 9.928620 sec
	Thread 3: 948, 9.923166 sec
	Thread 4: 946, 9.924946 sec
	
	Done ! 
	Throughput is 473.600000 reqs/sec.
	Average response time = 0.010479

$ ./server-mt 54320 1 0
	Server started on port 54320

Experimental Data:
==================
1.	N_workers	AVG_THR
	---------------------
	1			44.600000
	2			44.616667
	3			44.341667
	4			55.833333	
	5			55.383333
	10			55.775000
	
2.	N_workers	AVG_THR
	---------------------
	1			42.800000
	2			62.358333
	3			73.700000
	4			73.291667	
	5			75.716667
	10			88.608333
	20			88.886667

Note:
=====
Our earlier implementation of multi-client used multiple invocations
of `gethostbyname()`. It didn't come to light until this lab,
that this function isn't reliable and can lead to serious errors
in cases when it is invoked often. Hence, we changed the previous code
(submitted in lab2) a little bit to handle this. The new code is also
submitted.
Ref: http://stackoverflow.com/questions/15670713/using-gethostbyname

