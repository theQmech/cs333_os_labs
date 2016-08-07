%  CS333: Operating Systems Lab 
%  Threads and Processes 
%  140050005, 140050009 

\maketitle

1. We used two machines in SL2. Both were quad core i5-3330 with 3.00GHz and 
	8 GB memory. Maximum network card speed being 1000Mbps. 
	
	The setup was a simple one. we used two SL machines connected via lab ethernet. 
	
	(i)		Max disk read bandwidth: 190 MBps
	(ii)	Max network bandwidth: 936 Mbps(117 MBps)
	117 MBps seems to be the bottleneck for throughput.
	However the actual throughput will always be lower.


\pagebreak

2. 	Experimental data:
	
 	![](./thr-2.png){width=35%, height=35%}\ ![](./art-2.png){width=35%, height=35%}

	In our case, saturation is at 55.7 reqs/sec corresponding to N=4.
	In terms of file size this translates to roughly 110MBps.

	Both throughput and response times show an increasing trend
	throughout. However, the throughput saturates at around N=4.

	The bottleneck resource was definitely network bandwidth.
	Also iostat revealed that disk utilization was quite less than
	maximum at N=4.

	The saturation value is quite close to max network bandwidth.
	This is just as we predicted.

\pagebreak

3. 	Experimental data:

 	![](./thr-3.png){width=35%, height=35%}\ ![](./art-3.png){width=35%, height=35%}

	In our case, saturation is at about 55.5 reqs/sec corresponding to N=65.
	In terms of file size this translates to roughly 111MBps.

	Both throughput and response times show an increasing trend
	throughout. However, the throughput saturates at around N=65.
	the saturation is reached quite late. This is due the fact that 
	the threads sleep for 1 sec. which is quite a lot of time.	

	Similar to the previous case, the bottleneck was network bandwidth.

	The saturation value is again quite close to max network bandwidth.


\pagebreak

4. 	Experimental data:

 	![](./thr-4.png){width=35%, height=35%}\ ![](./art-4.png){width=35%, height=35%}

	Saturation : 55.7 reqs/sec
	
	Throughput and response time show similar trends.
	Bottleneck again is the network bandwidth.
	
	In this case, the same file is being requested. 
	hence throughput is relatively higher compared to Exercise 2.
	Also the average response time is on relatively low, perhaps
	because all clients demand the same file.

