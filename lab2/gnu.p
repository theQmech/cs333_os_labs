set term png
set   autoscale                        # scale axes automatically
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set title "Throughput vs. N"
set xlabel "Number of Clients(N)"
set ylabel "Throughput(reqs/sec)"
set key bottom right
set term png
set output "thr-2.png"
plot  "ex2.dat" using 1:2 title "Sleep 0 sec." with linespoints
set output "thr-3.png"
plot  "ex3.dat" using 1:2 title "Sleep 1 sec." with linespoints
set output "thr-4.png"
plot  "ex4.dat" using 1:2 title "Fixed file" with linespoints 

set title "Average Response Time(sec) vs. N"
set ylabel "Average Response Time"
set xlabel "Number of Clients (N)"
#set xr [0.0:12]
#set yr [0.0: 0.3]
set output "art-2.png"
plot  "ex2.dat" using 1:3 title "Sleep 0 sec." with linespoints
set output "art-3.png"
plot  "ex3.dat" using 1:3 title "Sleep 1 sec." with linespoints
set output "art-4.png"
plot  "ex4.dat" using 1:3 title "Fixed file" with linespoints 

