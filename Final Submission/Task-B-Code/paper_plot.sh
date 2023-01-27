

algo="newreno"

# output congestion window vs time
set terminal png size 1280, 960


# output throughput vs time
set output "throughput_newreno.png"
plot "throughput_vs_time_paper_TcpNewReno" using 1:2 title 'throughput newreno' with linespoints

set output "throughput_asran.png"
plot "throughput_vs_time_paper_TcpAsran" using 1:2 title 'throughput asran' with linespoints

set output "throughput_cubic.png"
plot "throughput_vs_time_paper_TcpCubic" using 1:2 title 'throughput cubic' with linespoints

# output congestion window vs time
set terminal png size 1500, 960
# output congestion window vs time
set output "cwnd_newreno.png"
plot "paper_ns3::TcpNewReno_cwnd.dat" using 1:2 title 'Cwnd in newReno' with linespoints

set output "cwnd_asran.png"
plot "paper_ns3::TcpAsran_cwnd.dat" using 1:2 title 'Cwnd in Asran' with linespoints

set output "cwnd_cubic.png"
plot "paper_ns3::TcpCubic_cwnd.dat" using 1:2 title 'Cwnd in Cubic' with linespoints

# output ss threshold vs time
set output "ssthresh_newreno.png"
plot "paper_ns3::TcpNewReno_ssthresh.dat" using 1:2 title 'SSthresh in newReno' with linespoints

set output "ssthresh_asran.png"
plot "paper_ns3::TcpAsran_ssthresh.dat" using 1:2 title 'SSthresh in Asran' with linespoints

set output "ssthresh_cubic.png"
plot "paper_ns3::TcpCubic_ssthresh.dat" using 1:2 title 'SSthresh in Cubic' with linespoints



