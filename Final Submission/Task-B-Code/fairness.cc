/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015, IMDEA Networks Institute
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Hany Assasa <hany.assasa@gmail.com>
.*
*/

#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/mobility-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/packet-sink.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/tcp-westwood.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/dsdv-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor-helper.h"

/*
////////////////////////////// Simulation Topology ////////////////////////////////////

                                              
                                NODE 0                        NODE 1              


                                      RECEIVER1    RECEIVER2
                                          4             5


                                NODE 2                        NODE 3
////////////////////////////////////////////////////////////////////////////////////////
*/




NS_LOG_COMPONENT_DEFINE ("paper");

using namespace ns3;

std::string tcpVariant = "TcpNewReno";             /* TCP variant type. */
Ptr<OutputStreamWrapper> throughput_stream;

double sec_y_coord = 90.0;
double third_y_coord = 120.0;
double forth_y_coord = 150.0;

double first_x_coord = 20.0;
double sec_x_coord = 50.0;
double third_x_coord = 65.0;

uint32_t num_flows = 8;


// --------------------------Fairness Index---------------------------------------
std::vector<double> throughput_array;
std::vector<uint32_t> is_new_reno;

void
calculate_fairness(void)
{
    double sum = 0;
    double sum_square = 0;
    uint32_t n  = 0;

    for(uint32_t i = 0 ; i < num_flows ; i++){
      NS_LOG_INFO("ENTERED");
      if(is_new_reno[i] != 0) continue;
      NS_LOG_INFO("BLAH DUH");
      n++;
      sum += throughput_array[i];
      NS_LOG_INFO("BLAH");
      sum_square += throughput_array[i]*throughput_array[i];
      NS_LOG_INFO(i);
    }
    double sum_avg = (sum * sum) / n;
    double fairness = sum_avg / sum_square;
    NS_LOG_INFO("Jain's Fairness using TCP ASRAN: " << fairness);

    sum = 0;
    sum_square = 0;
    n  = 0;
    for(uint32_t i = 0 ; i < num_flows ; i++){
      if(is_new_reno[i] != 1) continue;
      n++;
      sum += throughput_array[i];
      sum_square += throughput_array[i]*throughput_array[i];
    }
    sum_avg = (sum * sum) / n;
    fairness = sum_avg / sum_square;
    NS_LOG_INFO("Jain's Fairness using TCP Cubic: " << fairness);
}


// ------------------------------------------Fairness Index ---------------------------------------------


Ptr<PacketSink> sink;                         /* Pointer to the packet sink application */
uint64_t lastTotalRx = 0;                     /* The value of the last total received bytes */
// uint64_t last_40_Rx = 0;


AsciiTraceHelper asciiTraceHelper;


int
main (int argc, char *argv[])
{
  uint32_t payloadSize = 1472;                       /* Transport layer payload size in bytes. */
  std::string dataRate = "10Mbps";                  /* Application layer datarate. */ // changed datarate from 200 to 1 Mbps
  
  std::string phyRate = "HtMcs7";                    /* Physical layer bitrate. */
  double simulationTime = 30;                        /* Simulation time in seconds. */
  bool pcapTracing = true;                          /* PCAP Tracing is enabled or not. */
  double range = 60.0;
  

  Config::SetDefault ("ns3::RangePropagationLossModel::MaxRange", DoubleValue (range));

  LogComponentEnable("paper", LOG_LEVEL_INFO);

 

  /* Command line argument parser setup. */
  CommandLine cmd (__FILE__);
  cmd.AddValue ("payloadSize", "Payload size in bytes", payloadSize);
  cmd.AddValue ("dataRate", "Application data ate", dataRate);
  cmd.AddValue ("tcpVariant", "Transport protocol to use: TcpNewReno, "
                "TcpHybla, TcpHighSpeed, TcpHtcp, TcpVegas, TcpScalable, TcpVeno, "
                "TcpBic, TcpYeah, TcpIllinois, TcpWestwood, TcpWestwoodPlus, TcpLedbat ", tcpVariant);
  cmd.AddValue ("phyRate", "Physical layer bitrate", phyRate);
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("pcap", "Enable/disable PCAP Tracing", pcapTracing);
  cmd.Parse (argc, argv);

  NS_LOG_INFO("Congestion Algorithm used : " << tcpVariant);
  NS_LOG_INFO("Application Layer DataRate : " << dataRate);
  throughput_stream = asciiTraceHelper.CreateFileStream("fairness/throughput_vs_time_paper_" + tcpVariant);

  tcpVariant = std::string ("ns3::") + tcpVariant;
  // Select TCP variant
  if (tcpVariant.compare ("ns3::TcpWestwoodPlus") == 0)
    {
      // TcpWestwoodPlus is not an actual TypeId name; we need TcpWestwood here
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId ()));
      // the default protocol type in ns3::TcpWestwood is WESTWOOD
      Config::SetDefault ("ns3::TcpWestwood::ProtocolType", EnumValue (TcpWestwood::WESTWOODPLUS));
    }
  else
    {
      TypeId tcpTid;
      NS_ABORT_MSG_UNLESS (TypeId::LookupByNameFailSafe (tcpVariant, &tcpTid), "TypeId " << tcpVariant << " not found");
      Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName (tcpVariant)));
    }

  /* Configure TCP Options */
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));


  
  WifiMacHelper wifiMac;
  WifiHelper wifiHelper;
  wifiHelper.SetStandard (WIFI_STANDARD_80211n_5GHZ);

  /* Set up Legacy Channel */
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  // wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel");

  /* Setup Physical Layer */
  YansWifiPhyHelper wifiPhy;
  wifiPhy.SetChannel (wifiChannel.Create ());
  wifiPhy.SetErrorRateModel ("ns3::YansErrorRateModel");
  wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                      "DataMode", StringValue (phyRate),
                                      "ControlMode", StringValue ("HtMcs0"));

  

  // All routers
  NodeContainer staWifiNode;
  staWifiNode.Create (6);

  wifiMac.SetType ("ns3::AdhocWifiMac");

  // Setting different cc algo for below two nodes
  std::string specificNode = "/NodeList/0/$ns3::TcpL4Protocol/SocketType";
  Config::Set (specificNode, TypeIdValue (TypeId::LookupByName ("ns3::TcpAsran")));
  specificNode = "/NodeList/2/$ns3::TcpL4Protocol/SocketType";
  Config::Set (specificNode, TypeIdValue (TypeId::LookupByName ("ns3::TcpAsran")));
  specificNode = "/NodeList/5/$ns3::TcpL4Protocol/SocketType";
  Config::Set (specificNode, TypeIdValue (TypeId::LookupByName ("ns3::TcpAsran")));



  NetDeviceContainer staDevices; // Routing UAV 1, 4 & Operating UAV
  staDevices = wifiHelper.Install (wifiPhy, wifiMac, staWifiNode);

  /* Mobility model */
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  positionAlloc->Add (Vector (first_x_coord, sec_y_coord, 0.0)); // 10.0.0.1
  positionAlloc->Add (Vector (third_x_coord, sec_y_coord, 0.0)); // 10.0.0.2
  
  positionAlloc->Add (Vector (first_x_coord, forth_y_coord, 0.0)); //10.0.0.3 
  positionAlloc->Add (Vector (third_x_coord, forth_y_coord, 0.0)); // 10.0.0.4
  positionAlloc->Add (Vector (sec_x_coord - 10, third_y_coord, 0.0)); // 10.0.0.5 
  positionAlloc->Add (Vector (sec_x_coord + 10, third_y_coord, 0.0)); // 10.0.0.6 

 

  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (staWifiNode);

  /* Internet stack */
  InternetStackHelper stack;
  DsdvHelper dsdv; 
  stack.SetRoutingHelper (dsdv);
  stack.Install (staWifiNode);

 

  // Assigning addresses  
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.255.255.0");

  ////////////////////////////////One Network//////////////////////////////////////////  
  
  Ipv4InterfaceContainer staInterface, apInterface, UAVInterface;
  staInterface = address.Assign (staDevices);


  /* Populate routing table */
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // ------------------------------Make Operating UAV Sender-------------------------------------------
  // /* Install TCP Receiver on the access point */
  // PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 9));
  // ApplicationContainer sinkApp = sinkHelper.Install (apWifiNode.Get (0)); //GCS Station
  // sink = StaticCast<PacketSink> (sinkApp.Get (0)); 

  // /* Install TCP/UDP Transmitter on the station */
  // OnOffHelper server ("ns3::TcpSocketFactory", (InetSocketAddress (apInterface.GetAddress (0), 9)));
  // server.SetAttribute ("PacketSize", UintegerValue (payloadSize));
  // server.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  // server.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  // server.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
  // ApplicationContainer serverApp = server.Install (staWifiNode.Get (4)); // Operating UAV

    // //int id = wifiStaNodes2.Get(receiver)->GetId();
    //     std::string specificNode = "/NodeList/" + to_string(id) + "/$ns3::TcpL4Protocol/SocketType";
    //     Config::Set (specificNode, TypeIdValue (TcpLedbat::GetTypeId()));
  // ------------------------- Make last node Receiver-------------------------------------
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 9));
  ApplicationContainer sinkApp1 = sinkHelper.Install (staWifiNode.Get (4)); 
  ApplicationContainer sinkApp2 = sinkHelper.Install (staWifiNode.Get (5)); 

  /* Install TCP/UDP Sender on the stations */
  uint32_t receiver = 3;
  for(int i = 0 ; i < 4 ; i++){
    if(i%2 == 0){
      receiver = 5;
    }
    else{
      receiver = 4;
    }
    OnOffHelper server ("ns3::TcpSocketFactory", (InetSocketAddress (staInterface.GetAddress (receiver), 9)));
    server.SetAttribute ("PacketSize", UintegerValue (payloadSize));
    server.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    server.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    server.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
    ApplicationContainer serverApp = server.Install (staWifiNode.Get (i)); // GCS Station
    serverApp.Start (Seconds (2.0));

  }
  
  // ------------------------------------------------------------------------------------------

  
  

  /* Start Applications */
  sinkApp1.Start (Seconds (1.0));
  sinkApp2.Start (Seconds (1.0));


// Store for calculating fairness later
  throughput_array.reserve (num_flows);
  is_new_reno.reserve(num_flows);
  for(uint32_t i = 0 ; i < num_flows ; i++){
      throughput_array[i] = 0;
      is_new_reno[i] = 1;
  }
  

  AnimationInterface anim ("fairness/paper.xml"); 
  anim.SetMaxPktsPerTraceFile( 210000 * 12);

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();
  /* Enable Traces */
 if (pcapTracing)
    {
      wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
      wifiPhy.EnablePcap ("fairness/Station", staDevices);
    }

  /* Start Simulation */
  Simulator::Stop (Seconds (simulationTime + 1));
  Simulator::Run (); 

  ///////////////////////// Calculate throughput and FlowMonitor statistics ////////////////////////////////////
  int j=0;
  float AvgThroughput = 0;
  Time Jitter;
  Time Delay;
  // variables for output measurement
  uint32_t SentPackets = 0;
  uint32_t ReceivedPackets = 0;
  uint32_t LostPackets = 0;

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

  for (auto iter = stats.begin (); iter != stats.end (); ++iter) {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first); 
          // classifier returns FiveTuple in correspondance to a flowID

      NS_LOG_UNCOND("----Flow ID:" <<iter->first);
      NS_LOG_UNCOND("Src Addr" <<t.sourceAddress << " -- Dst Addr "<< t.destinationAddress);
      NS_LOG_UNCOND("Sent Packets=" <<iter->second.txPackets);
      NS_LOG_UNCOND("Received Packets =" <<iter->second.rxPackets);
      NS_LOG_UNCOND("Lost Packets =" <<iter->second.txPackets-iter->second.rxPackets);
      NS_LOG_UNCOND("Packet delivery ratio =" <<iter->second.rxPackets*100.0/iter->second.txPackets << "%");
      NS_LOG_UNCOND("Packet loss ratio =" << (iter->second.txPackets-iter->second.rxPackets)*100.0/iter->second.txPackets << "%");
      // NS_LOG_UNCOND("Packet lost diff way = "<< iter->second.lostPackets);
      NS_LOG_UNCOND("Delay =" <<iter->second.delaySum);
      NS_LOG_UNCOND("Jitter =" <<iter->second.jitterSum);
      NS_LOG_UNCOND("Throughput =" <<iter->second.rxBytes * 8.0/(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())/1024<<"Kbps");
    
      if(t.destinationAddress == "10.0.0.6" || t.sourceAddress == "10.0.0.6"){
        is_new_reno[j] = 0;
      }
      throughput_array[j] = iter->second.rxBytes * 8.0/(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())/1024;

      SentPackets = SentPackets +(iter->second.txPackets);
      ReceivedPackets = ReceivedPackets + (iter->second.rxPackets);
      LostPackets = LostPackets + (iter->second.txPackets-iter->second.rxPackets);
      AvgThroughput = AvgThroughput + (iter->second.rxBytes * 8.0/(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())/1024);
      Delay = Delay + (iter->second.delaySum);
      Jitter = Jitter + (iter->second.jitterSum);

      j = j + 1;

  }

  AvgThroughput = AvgThroughput/j;
  Delay = Delay / ReceivedPackets;
  NS_LOG_UNCOND("--------Total Results of the simulation----------"<<std::endl);
  NS_LOG_UNCOND("Total sent packets  =" << SentPackets);
  NS_LOG_UNCOND("Total Received Packets =" << ReceivedPackets);
  NS_LOG_UNCOND("Total Lost Packets =" << LostPackets);
  NS_LOG_UNCOND("Packet Loss ratio =" << ((LostPackets*100.00)/SentPackets)<< "%");
  NS_LOG_UNCOND("Packet delivery ratio =" << ((ReceivedPackets*100.00)/SentPackets)<< "%");
  NS_LOG_UNCOND("Average Throughput =" << AvgThroughput<< "Kbps");
  NS_LOG_UNCOND("End to End Delay =" << Delay);
  NS_LOG_UNCOND("End to End Jitter delay =" << Jitter);
  NS_LOG_UNCOND("Total Flow id " << j);

  
  // Save FlowMonitor flow statistics in csv format
  monitor->SerializeToXmlFile( "fairness/paper_flow.xml" , true, true);
  
  // calculating avg throughput from 40 seconds
//   double averageThroughput = (( ( sink->GetTotalRx () - last_40_Rx ) * 8) / (1e6 * ( simulationTime - 40.0 ) ));
  // double averageThroughput = (( ( sink->GetTotalRx ()) * 8) / (1e6 * ( simulationTime ) ));
  
  Simulator::Destroy ();

  calculate_fairness();  
  // std::cout << "\nAverage throughput : " << averageThroughput << " Mbit/s" << std::endl;
  return 0;
}
