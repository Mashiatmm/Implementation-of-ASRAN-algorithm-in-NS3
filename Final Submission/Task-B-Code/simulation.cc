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
 * This is a simple example to test TCP over 802.11n (with MPDU aggregation enabled).
 *
 * Network topology:
 *
 *   Ap    STA
 *   *      *
 *   |      |
 *   n1     n2
 *
 * In this example, an HT station sends TCP packets to the access point.
 * We report the total throughput received during a window of 100ms.
 * The user can specify the application data rate and choose the variant
 * of TCP i.e. congestion control algorithm to use.
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

                                              GCS (10.0.0.1)
                                          ||        ||
                                      ||                ||
                                  ||                        ||
                            Routing UAV 4                 Routing UAV 2     || || ||     Routing UAV 3
                      ( 10.0.0.3 )||                        || (10.0.0.4)                 ( 10.0.0.2 )
                                       ||                ||
                                           ||        ||
                                          Routing UAV 1 (10.0.0.5)
                                          ||
                                      ||
                                  ||
                           Operating UAV (10.0.0.6)               

////////////////////////////////////////////////////////////////////////////////////////
*/




NS_LOG_COMPONENT_DEFINE ("paper");

using namespace ns3;

std::string tcpVariant = "TcpAsran";             /* TCP variant type. */
Ptr<OutputStreamWrapper> throughput_stream;

double first_y_coord = 50.0;
double sec_y_coord = 90.0;
double third_y_coord = 120.0;
double forth_y_coord = 160.0;

double first_x_coord = 10.0;
double sec_x_coord = 50.0;
double third_x_coord = 80.0;
double forth_x_coord = 130.0;


Ptr<PacketSink> sink;                         /* Pointer to the packet sink application */
uint64_t lastTotalRx = 0;                     /* The value of the last total received bytes */
// uint64_t last_40_Rx = 0;


AsciiTraceHelper asciiTraceHelper;


// void
// AssignLastTotalRx()
// {
//     // lastTotalRx = sink->GetTotalRx ();
//     // last_40_Rx = lastTotalRx;
// }

static Ptr<OutputStreamWrapper> cWndStream;
static bool CwindFirstlineFlag = true;

static void
CwndTracer (uint32_t oldval, uint32_t newval)
{
    if (CwindFirstlineFlag)
    {
    *cWndStream->GetStream () << "6.0 " << oldval << std::endl;
    CwindFirstlineFlag = false;
    }
    *cWndStream->GetStream () << Simulator::Now ().GetSeconds () << "\t" << newval << std::endl;
}

static void
TraceCwnd (std::string cwnd_tr_file_name)
{
    AsciiTraceHelper ascii;
    cWndStream = ascii.CreateFileStream (cwnd_tr_file_name.c_str ());
    Config::ConnectWithoutContext ("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeCallback (&CwndTracer));
}

static bool SshThrFirstlineFlag = true;
static Ptr<OutputStreamWrapper> ssThreshStream;

static void
SsThreshTracer (uint32_t oldval, uint32_t newval)
{
    if (SshThrFirstlineFlag)
    {
    *ssThreshStream->GetStream () << "0.0 " << oldval << std::endl;
    SshThrFirstlineFlag = false;
    }
    *ssThreshStream->GetStream () << Simulator::Now ().GetSeconds () << ", " << newval << std::endl;

}

static void
TraceSsThresh (std::string ssthresh_tr_file_name)
{
    AsciiTraceHelper ascii;
    ssThreshStream = ascii.CreateFileStream (ssthresh_tr_file_name.c_str ());
    Config::ConnectWithoutContext ("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/SlowStartThreshold", MakeCallback (&SsThreshTracer));
}


void
CalculateThroughput ()
{
  Time now = Simulator::Now ();                                         /* Return the simulator's virtual time. */
  double cur = (sink->GetTotalRx () - lastTotalRx) * (double) 8 / 1e5;     /* Convert Application RX Packets to MBits. */
  //std::cout << now.GetSeconds () << "s: \t" << cur << " Mbit/s" << std::endl;
  *throughput_stream->GetStream () << now.GetSeconds () << "\t" << cur << std::endl;
  lastTotalRx = sink->GetTotalRx ();
  Simulator::Schedule (MilliSeconds (100), &CalculateThroughput);
}

void ChangePosition4(Ptr<MobilityModel > mob ){
    Vector m_position = mob->GetPosition ();
    m_position.x = forth_x_coord + 10; 
    mob->SetPosition (m_position);
}

void ChangePosition5(Ptr<MobilityModel > mob ){
    Vector m_position = mob->GetPosition ();
    m_position.x = third_x_coord;
    mob->SetPosition (m_position);
}

int
main (int argc, char *argv[])
{
  uint32_t payloadSize = 1472;                       /* Transport layer payload size in bytes. */
  std::string dataRate = "2Mbps";                  /* Application layer datarate. */ // changed datarate from 200 to 1 Mbps
  
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
  throughput_stream = asciiTraceHelper.CreateFileStream("paper/throughput_vs_time_paper_" + tcpVariant);

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
  // wifiPhy.SetErrorRateModel ("ns3::YansErrorRateModel");
  wifiHelper.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                      "DataMode", StringValue (phyRate),
                                      "ControlMode", StringValue ("HtMcs0"));

  // GCS
  NodeContainer apWifiNode;
  apWifiNode.Create (1);

  // Routing UAV 4, 1 & Operating UAV
  NodeContainer staWifiNode;
  staWifiNode.Create (5);

//   // Routing UAV 2, 3
//   NodeContainer UAVNode;
//   UAVNode.Create (2);
 
  wifiMac.SetType ("ns3::AdhocWifiMac");

  NetDeviceContainer apDevice; // GCS Station
  apDevice = wifiHelper.Install (wifiPhy, wifiMac, apWifiNode);

  NetDeviceContainer staDevices; // Routing UAV 1, 4 & Operating UAV
  staDevices = wifiHelper.Install (wifiPhy, wifiMac, staWifiNode);

  /* Mobility model */
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  positionAlloc->Add (Vector (sec_x_coord, first_y_coord, 0.0)); // apWifiNode position -> GCS        - 10.0.0.1
  positionAlloc->Add (Vector (forth_x_coord, sec_y_coord, 0.0)); //staWifiNode (0) -> Routing UAV 3  - 10.0.0.2
  positionAlloc->Add (Vector (first_x_coord, sec_y_coord, 0.0)); //staWifiNode (1) -> Routing UAV 4   - 10.0.0.3 - 44.72 from 10.0.0.1
  positionAlloc->Add (Vector (third_x_coord, sec_y_coord, 0.0)); //staWifiNode (2) -> Routing UAV 2   - 10.0.0.4 - 56.57 from 10.0.0.1
  positionAlloc->Add (Vector (sec_x_coord, third_y_coord, 0.0)); //staWifiNode (3) -> Routing UAV 1  - 10.0.0.5 - 36 from 10.0.0.3, 50 from 10.0.0.4
  positionAlloc->Add (Vector (sec_x_coord, forth_y_coord, 0.0)); //staWifiNode (4) -> Operating UAV  - 10.0.0.6
 
  

  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (apWifiNode);
  mobility.Install (staWifiNode);

  /* Internet stack */
  InternetStackHelper stack;
  DsdvHelper dsdv; 
  stack.SetRoutingHelper (dsdv);
  stack.Install (apWifiNode);
  stack.Install (staWifiNode);

  // Print Routing Information to check hop and routes
  Ptr<OutputStreamWrapper> routingStream1 = Create<OutputStreamWrapper>("paper/dsdv_18.routes" + tcpVariant, std::ios::out);
  Ptr<OutputStreamWrapper> routingStream0 = Create<OutputStreamWrapper>("paper/dsdv_8.routes" + tcpVariant, std::ios::out);
  Ptr<OutputStreamWrapper> routingStream2 = Create<OutputStreamWrapper>("paper/dsdv_28.routes" + tcpVariant, std::ios::out);
//   Ptr<OutputStreamWrapper> routingStream3 = Create<OutputStreamWrapper>("paper/dsdv_40.routes" + tcpVariant, std::ios::out);
//   Ptr<OutputStreamWrapper> routingStream4 = Create<OutputStreamWrapper>("paper/dsdv_69.routes" + tcpVariant, std::ios::out);
//   Ptr<OutputStreamWrapper> routingStream5 = Create<OutputStreamWrapper>("paper/dsdv_52.routes" + tcpVariant, std::ios::out);


  // Routing UAV 2 & 3 changes its position in 35 and 38 seconds
  // According to paper, the routing should be changed in those moments. So outputting routing table at 18, 35, 40, 66 seconds
  dsdv.PrintRoutingTableAllAt(Seconds (18), routingStream1);
  dsdv.PrintRoutingTableAllAt(Seconds (8), routingStream0);
  dsdv.PrintRoutingTableAllAt(Seconds (28), routingStream2);
//   dsdv.PrintRoutingTableAllAt(Seconds (35), routingStream2);
//   dsdv.PrintRoutingTableAllAt(Seconds (40), routingStream3);
//   dsdv.PrintRoutingTableAllAt(Seconds (69), routingStream4);
//   dsdv.PrintRoutingTableAllAt(Seconds (52), routingStream5);

  // Assigning addresses  
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.255.255.0");

  ////////////////////////////////One Network//////////////////////////////////////////  
  
  Ipv4InterfaceContainer staInterface, apInterface, UAVInterface;
  apInterface = address.Assign (apDevice);
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


  // ------------------------- Make Operating UAV Receiver-------------------------------------
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 9));
  ApplicationContainer sinkApp = sinkHelper.Install (staWifiNode.Get (4)); //Operating UAV
  sink = StaticCast<PacketSink> (sinkApp.Get (0)); 

  /* Install TCP/UDP Sender on the station */
  OnOffHelper server ("ns3::TcpSocketFactory", (InetSocketAddress (staInterface.GetAddress (4), 9)));
  server.SetAttribute ("PacketSize", UintegerValue (payloadSize));
  server.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  server.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  server.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
  ApplicationContainer serverApp = server.Install (apWifiNode.Get (0)); // GCS Station
  // ------------------------------------------------------------------------------------------

  
  

  /* Start Applications */
  sinkApp.Start (Seconds (1.0));
  serverApp.Start (Seconds (2.0));
//   Simulator::Schedule (Seconds (40.0), &AssignLastTotalRx);
  Simulator::Schedule (Seconds (1.1), &CalculateThroughput);

  
  // // Position Change event
  // Move Routing UAV 2 to 3's coordinates
  Ptr<MobilityModel> Mob4 = staWifiNode.Get (2)->GetObject<MobilityModel>();
  Simulator::Schedule (Seconds (9.1), &ChangePosition4, Mob4);

//   // Move Routing UAV 3 to 2's coordinates
//   Ptr<MobilityModel> Mob5 = staWifiNode.Get(0)->GetObject<MobilityModel>();
//   Simulator::Schedule (Seconds (38.1), &ChangePosition5, Mob5);

  // Note Congestion Window change
  Simulator::Schedule (Seconds (6.00001), &TraceCwnd, "paper/paper_" + tcpVariant + "_cwnd.dat");
  Simulator::Schedule (Seconds (6.00001), &TraceSsThresh, "paper/paper_" + tcpVariant + "_ssthresh.dat");


  

  AnimationInterface anim ("paper/paper.xml"); 
  anim.SetMaxPktsPerTraceFile( 210000 * 12);

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();
  /* Enable Traces */
 if (pcapTracing)
    {
      wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11_RADIO);
      wifiPhy.EnablePcap ("paper/AccessPoint", apDevice);
      wifiPhy.EnablePcap ("paper/Station", staDevices);
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
  monitor->SerializeToXmlFile( "paper/paper_flow.xml" , true, true);
  
  // calculating avg throughput from 40 seconds
//   double averageThroughput = (( ( sink->GetTotalRx () - last_40_Rx ) * 8) / (1e6 * ( simulationTime - 40.0 ) ));
  double averageThroughput = (( ( sink->GetTotalRx ()) * 8) / (1e6 * ( simulationTime ) ));
  
  Simulator::Destroy ();

  std::cout << "\nAverage throughput : " << averageThroughput << " Mbit/s" << std::endl;
  return 0;
}
