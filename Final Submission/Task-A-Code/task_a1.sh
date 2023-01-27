#!/bin/bash

# varying packets sent per seconds
./waf --run "scratch/Task_A1 --nodes=20 --flows=20 --packets_sent_per_sec=100 --filename='_2020_packets_100'"
./waf --run "scratch/Task_A1 --nodes=20 --flows=20 --packets_sent_per_sec=200 --filename='_2020_packets_200'"
./waf --run "scratch/Task_A1 --nodes=20 --flows=20 --packets_sent_per_sec=300 --filename='_2020_packets_300'"
./waf --run "scratch/Task_A1 --nodes=20 --flows=20 --packets_sent_per_sec=400 --filename='_2020_packets_400'"
./waf --run "scratch/Task_A1 --nodes=20 --flows=20 --packets_sent_per_sec=500 --filename='_2020_packets_500'"

# varying flow
./waf --run "scratch/Task_A1 --nodes=20 --flows=10 --filename='nodes_20_flows_10'"
./waf --run "scratch/Task_A1 --nodes=20 --flows=20 --filename='nodes_20_flows_20'"
./waf --run "scratch/Task_A1 --nodes=20 --flows=30 --filename='nodes_20_flows_30'"
./waf --run "scratch/Task_A1 --nodes=20 --flows=40 --filename='nodes_20_flows_40'"
./waf --run "scratch/Task_A1 --nodes=20 --flows=50 --filename='nodes_20_flows_50'"

# varying node numbers
./waf --run "scratch/Task_A1 --nodes=20 --flows=20 --filename='nodes_20_flows_20'"
./waf --run "scratch/Task_A1 --nodes=40 --flows=20 --filename='nodes_40_flows_20'"
./waf --run "scratch/Task_A1 --nodes=60 --flows=20 --filename='nodes_60_flows_20'"
./waf --run "scratch/Task_A1 --nodes=80 --flows=20 --filename='nodes_80_flows_20'"
./waf --run "scratch/Task_A1 --nodes=100 --flows=20 --filename='nodes_100_flows_20'"

cd Task_A1
./parse.py
