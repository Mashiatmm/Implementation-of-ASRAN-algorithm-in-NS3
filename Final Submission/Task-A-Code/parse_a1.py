#! /usr/bin/python3

from re import L
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import csv
from matplotlib.pyplot import figure

# import seaborn as sns
import os

# sns.set()
font = {'color':  'black',
        'weight': 'bold',
        'size': 15,
        }
try :
    os.mkdir("node_plots")
except :
    print("directory exists")

try :
    os.mkdir("flow_plots")
except :
    print("directory exists")
    
try :
    os.mkdir("packet_plots")
except :
    print("directory exists")



Parameters = {
    "Total sent packets " : [],
    "Total Received Packets" : [],
"Total Lost Packets" : [],
"Packet delivery ratio" : [],
"Packet Loss ratio": [],
"End to End Delay": [],
"End to End Jitter delay": [],
"Average Throughput": [],
}

#-------------------------------------------------varying flows-----------------------------------------------------
filenames = ['nodes_20_flows_10', 'nodes_20_flows_20', 'nodes_20_flows_30', 'nodes_20_flows_40', 'nodes_20_flows_50']

for f in filenames:
	with open(f) as infile :
	    	flow = infile.read().split("--------Total Results of the simulation----------")
	    	lines = flow[1].rstrip().split("\n")
	    	for l in lines:
	    		elements = l.split(" =")
	    		if len(elements) > 1 :
	    			# print(elements[0] +"sds")
	    			try :
	    				Parameters[elements[0]].append(elements[1].replace('Kbps','').replace('%',''))
	    			except :
	    				print("error")
	    				
	    				

new_key = "Average Throughput (Kbps)"
old_key = "Average Throughput"
Parameters[new_key] = Parameters. pop(old_key)


for param, values in Parameters.items():
    # print(param, values)
    if param == "Total sent packets ":
    	continue
    elif param == "Total Received Packets":
    	continue
    elif param == "Total Lost Packets":
    	continue
    elif param == "End to End Jitter delay":
    	continue
    k=10
    with open("flow_plots/"+param+".csv", "w") as outfile:
        csv_writer = csv.writer(outfile)
        csv_writer.writerow(['Flows', param])
        for val in values :
            csv_writer.writerow([k,val])
            k+=10
    df = pd.read_csv("flow_plots/"+param+".csv")
    
    font['size']=16
    figure(figsize=(16, 12), dpi=80)
    plt.scatter(df['Flows'], df[param], color ='maroon')
    plt.plot(df['Flows'], df[param], color ='maroon')
    # plt.bar(df['Flows'], df[param], color ='maroon', width = 1.0)
    plt.xticks(np.arange(0,51,10)) 
    plt.xlabel('Flow',fontdict=font)  # Add an x-label to the axes.
    plt.ylabel(param,fontdict=font)
    font['size']=18
    plt.title("Nodes = 20, Flow vs "+param,fontdict=font)
    plt.grid()

    plt.savefig("flow_plots/"+param+".jpg")
    plt.clf()
    if os.path.exists(param+".csv"):
        os.remove(param+".csv")
        

 
 
Parameters = {
    "Total sent packets " : [],
    "Total Received Packets" : [],
"Total Lost Packets" : [],
"Packet delivery ratio" : [],
"Packet Loss ratio": [],
"End to End Delay": [],
"End to End Jitter delay": [],
"Average Throughput": [],
}

#-------------------------------------------------varying nodes-----------------------------------------------------
filenames = ['nodes_20_flows_20', 'nodes_40_flows_20', 'nodes_60_flows_20', 'nodes_80_flows_20', 'nodes_100_flows_20' ]

for f in filenames:
	with open(f) as infile :
	    	flow = infile.read().split("--------Total Results of the simulation----------")
	    	lines = flow[1].rstrip().split("\n")
	    	for l in lines:
	    		elements = l.split(" =")
	    		if len(elements) > 1 :
	    			# print(elements[0] +"sds")
	    			try :
	    				Parameters[elements[0]].append(elements[1].replace('Kbps','').replace('%',''))
	    			except :
	    				print("error")
	    				
	    				

new_key = "Average Throughput (Kbps)"
old_key = "Average Throughput"
Parameters[new_key] = Parameters. pop(old_key)


for param, values in Parameters.items():
    # print(param, values)
    if param == "Total sent packets ":
    	continue
    elif param == "Total Received Packets":
    	continue
    elif param == "Total Lost Packets":
    	continue
    elif param == "End to End Jitter delay":
    	continue
    k=20
    with open("node_plots/"+param+".csv", "w") as outfile:
        csv_writer = csv.writer(outfile)
        csv_writer.writerow(['Nodes', param])
        for val in values :
            csv_writer.writerow([k,val])
            k+=20
    df = pd.read_csv("node_plots/"+param+".csv")
    
    font['size']=16
    figure(figsize=(16, 12), dpi=80)
    plt.scatter(df['Nodes'], df[param], color ='maroon')
    plt.plot(df['Nodes'], df[param], color ='maroon')
    # plt.bar(df['Nodes'], df[param], color ='maroon', width = 1.0)
    plt.xticks(np.arange(0,101,20)) 
    plt.xlabel('Nodes',fontdict=font)  # Add an x-label to the axes.
    plt.ylabel(param,fontdict=font)
    font['size']=18
    plt.title("Flow = 20, Nodes vs "+param,fontdict=font)
    plt.grid()

    plt.savefig("node_plots/"+param+".jpg")
    plt.clf()
    if os.path.exists(param+".csv"):
        os.remove(param+".csv")

        
        
Parameters = {
    "Total sent packets " : [],
    "Total Received Packets" : [],
"Total Lost Packets" : [],
"Packet delivery ratio" : [],
"Packet Loss ratio": [],
"End to End Delay": [],
"End to End Jitter delay": [],
"Average Throughput": [],
}

#-------------------------------------------------varying packetsPerSecond-----------------------------------------------------
filenames = ['_2020_packets_100', '_2020_packets_200', '_2020_packets_300', '_2020_packets_400', '_2020_packets_500' ]

for f in filenames:
	with open(f) as infile :
	    	flow = infile.read().split("--------Total Results of the simulation----------")
	    	lines = flow[1].rstrip().split("\n")
	    	for l in lines:
	    		elements = l.split(" =")
	    		if len(elements) > 1 :
	    			# print(elements[0] +"sds")
	    			try :
	    				Parameters[elements[0]].append(elements[1].replace('Kbps','').replace('%',''))
	    			except :
	    				print("error")
	    				
	    				

new_key = "Average Throughput (Kbps)"
old_key = "Average Throughput"
Parameters[new_key] = Parameters. pop(old_key)


for param, values in Parameters.items():
    # print(param, values)
    if param == "Total sent packets ":
    	continue
    elif param == "Total Received Packets":
    	continue
    elif param == "Total Lost Packets":
    	continue
    elif param == "End to End Jitter delay":
    	continue
    k=100
    with open("packet_plots/"+param+".csv", "w") as outfile:
        csv_writer = csv.writer(outfile)
        csv_writer.writerow(['Packets', param])
        for val in values :
            csv_writer.writerow([k,val])
            k+=100
    df = pd.read_csv("packet_plots/"+param+".csv")
    
   
    
    font['size']=16
    figure(figsize=(16, 12), dpi=80)
    plt.scatter(df['Packets'], df[param], color ='maroon')
    plt.plot(df['Packets'], df[param], color ='maroon')
    # plt.bar(df['Packets'], df[param], color ='maroon', width = 5.0)
    plt.xticks(np.arange(0,501,100)) 
    plt.xlabel('Packets sent per second',fontdict=font)  # Add an x-label to the axes.
    plt.ylabel(param,fontdict=font)
    font['size']=18
    plt.title("Packets sent per second vs "+param,fontdict=font)
    plt.grid()

    plt.savefig("packet_plots/"+param+".jpg")
    plt.clf()
    if os.path.exists(param+".csv"):
        os.remove(param+".csv")


	        
	    	
