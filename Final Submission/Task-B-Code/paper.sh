./waf --run "scratch/paper_3 --tcpVariant='TcpAsran'"
./waf --run "scratch/paper_3 --tcpVariant='TcpNewReno'"
./waf --run "scratch/paper_3 --tcpVariant='TcpCubic'"
cd paper
gnuplot ./paper_plot.sh
