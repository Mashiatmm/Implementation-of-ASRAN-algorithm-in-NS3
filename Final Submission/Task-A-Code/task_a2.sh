#!/bin/bash

# varying packets sent per seconds
./waf --run "scratch/Task_A2 --nodes=20 --flows=20 --packets_sent_per_sec=100 --filename='_2020_packets_100'"
./waf --run "scratch/Task_A2 --nodes=20 --flows=20 --packets_sent_per_sec=200 --filename='_2020_packets_200'"
./waf --run "scratch/Task_A2 --nodes=20 --flows=20 --packets_sent_per_sec=300 --filename='_2020_packets_300'"
./waf --run "scratch/Task_A2 --nodes=20 --flows=20 --packets_sent_per_sec=400 --filename='_2020_packets_400'"
./waf --run "scratch/Task_A2 --nodes=20 --flows=20 --packets_sent_per_sec=500 --filename='_2020_packets_500'"

# varying flow
./waf --run "scratch/Task_A2 --nodes=10 --flows=10 --filename='nodes_10_flows_10'"
./waf --run "scratch/Task_A2 --nodes=10 --flows=20 --filename='nodes_10_flows_20'"
./waf --run "scratch/Task_A2 --nodes=10 --flows=30 --filename='nodes_10_flows_30'"
./waf --run "scratch/Task_A2 --nodes=10 --flows=40 --filename='nodes_10_flows_40'"
./waf --run "scratch/Task_A2 --nodes=10 --flows=50 --filename='nodes_10_flows_50'"

# varying node numbers
./waf --run "scratch/Task_A2 --nodes=20 --flows=20 --filename='nodes_20_flows_20'"
./waf --run "scratch/Task_A2 --nodes=40 --flows=20 --filename='nodes_40_flows_20'"
./waf --run "scratch/Task_A2 --nodes=60 --flows=20 --filename='nodes_60_flows_20'"
./waf --run "scratch/Task_A2 --nodes=80 --flows=20 --filename='nodes_80_flows_20'"
./waf --run "scratch/Task_A2 --nodes=100 --flows=20 --filename='nodes_100_flows_20'"

# varying speed
./waf --run "scratch/Task_A2 --nodes=20 --flows=20 --speed=5 --filename='speed_5'"
./waf --run "scratch/Task_A2 --nodes=20 --flows=20 --speed=10 --filename='speed_10'"
./waf --run "scratch/Task_A2 --nodes=20 --flows=20 --speed=15 --filename='speed_15'"
./waf --run "scratch/Task_A2 --nodes=20 --flows=20 --speed=20 --filename='speed_20'"
./waf --run "scratch/Task_A2 --nodes=20 --flows=20 --speed=25 --filename='speed_25'"

cd Task_A2
./parse.py
