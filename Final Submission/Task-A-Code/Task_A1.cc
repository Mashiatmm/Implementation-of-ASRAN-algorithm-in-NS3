/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 ResiliNets, ITTC, University of Kansas
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
 * Authors: Justin P. Rohrer, Truc Anh N. Nguyen <annguyen@ittc.ku.edu>, Siddharth Gangadhar <siddharth@ittc.ku.edu>
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  http://wiki.ittc.ku.edu/resilinets
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 *
 * “TCP Westwood(+) Protocol Implementation in ns-3”
 * Siddharth Gangadhar, Trúc Anh Ngọc Nguyễn , Greeshma Umapathi, and James P.G. Sterbenz,
 * ICST SIMUTools Workshop on ns-3 (WNS3), Cannes, France, March 2013
 */

#include <iostream>
#include <fstream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/error-model.h"
#include "ns3/tcp-header.h"
#include "ns3/udp-header.h"
#include "ns3/enum.h"
#include "ns3/event-id.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traffic-control-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpVariantsComparison");



class MyApp : public Application
{
public:
  MyApp ();
  virtual ~MyApp ();

  /**
   * Register this type.
   * \return The TypeId.
   */
  static TypeId GetTypeId (void);
  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

MyApp::MyApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

MyApp::~MyApp ()
{
  m_socket = 0;
}

/* static */
TypeId MyApp::GetTypeId (void)
{
  static TypeId tid = TypeId ("MyApp")
    .SetParent<Application> ()
    .SetGroupName ("Tutorial")
    .AddConstructor<MyApp> ()
    ;
  return tid;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_dataRate = dataRate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  if (InetSocketAddress::IsMatchingType (m_peer))
    {
      m_socket->Bind ();
    }
  else
    {
      m_socket->Bind6 ();
    }
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);
  ScheduleTx();
}

void
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time now = Simulator::Now ();    
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}






AsciiTraceHelper asciiTraceHelper;



Ptr<OutputStreamWrapper> throughput_stream = asciiTraceHelper.CreateFileStream("Task_A1/throughput_vs_time");

// static uint32_t receiveCount = 0;
Ptr<PacketSink> sink;                         /* Pointer to the packet sink application */
uint64_t lastTotalRx = 0;                     /* The value of the last total received bytes */




void
CalculateThroughput ()
{
  Time now = Simulator::Now ();                                         /* Return the simulator's virtual time. */
  double cur = (sink->GetTotalRx () - lastTotalRx) * (double) 8 / 1e5;     /* Convert Application RX Packets to KBits. */
  *throughput_stream->GetStream () << now.GetSeconds () << "\t" << cur << std::endl;
  // std::cout << now.GetSeconds () << "s: \t" << cur << " Mbit/s" << std::endl;
  lastTotalRx = sink->GetTotalRx ();
  Simulator::Schedule (MilliSeconds (100), &CalculateThroughput);
}




int main (int argc, char *argv[])
{
    std::string transport_prot = "TcpCubic";
    uint16_t num_flows = 20;
    uint32_t nodes = 20;
    uint32_t packets_sent_per_sec = 1000;
    std::string dataRate = "1Mbps";

    //changing error_p for increasing error rate in error model - drop more packets - Mashiat
    double error_p = 0.0001;
    //double error_p = 0.0;
    
   
    std::string bandwidth = "2Mbps";
    std::string delay = "0.01ms";
    std::string access_bandwidth = "10Mbps";
    std::string access_delay = "45ms";
    bool tracing = true;
    std::string prefix_file_name = "Task_A1/TcpVariantsComparison";
    uint64_t data_mbytes = 0;
    uint32_t mtu_bytes = 572;
    
    double duration = 50.0;
    uint32_t run = 0;
    bool flow_monitor = true;
    bool pcap = true;
    bool sack = true;
    std::string queue_disc_type = "ns3::PfifoFastQueueDisc";
    std::string recovery = "ns3::TcpClassicRecovery";
    std::string flow_mon_filename = "demo";

    CommandLine cmd (__FILE__);
    cmd.AddValue ("transport_prot", "Transport protocol to use: TcpNewReno, TcpLinuxReno, "
                  "TcpHybla, TcpHighSpeed, TcpHtcp, TcpVegas, TcpScalable, TcpVeno, "
                  "TcpBic, TcpYeah, TcpIllinois, TcpWestwood, TcpWestwoodPlus, TcpLedbat, "
      "TcpLp, TcpDctcp, TcpCubic, TcpBbr", transport_prot);
    cmd.AddValue ("nodes", "Number of nodes", nodes);
    cmd.AddValue ("flows", "Number of flows", num_flows);
    cmd.AddValue ("packets_sent_per_sec", "Packets sent per second for each sender ", packets_sent_per_sec);
    cmd.AddValue ("error_p", "Packet error rate", error_p);
    cmd.AddValue ("bandwidth", "Bottleneck bandwidth", bandwidth);
    cmd.AddValue ("delay", "Bottleneck delay", delay);
    cmd.AddValue ("access_bandwidth", "Access link bandwidth", access_bandwidth);
    cmd.AddValue ("access_delay", "Access link delay", access_delay);
    cmd.AddValue ("tracing", "Flag to enable/disable tracing", tracing);
    cmd.AddValue ("prefix_name", "Prefix of output trace file", prefix_file_name);
    cmd.AddValue ("data", "Number of Megabytes of data to transmit", data_mbytes);
    cmd.AddValue ("mtu", "Size of IP packets to send in bytes", mtu_bytes);
    cmd.AddValue ("num_flows", "Number of flows", num_flows);
    cmd.AddValue ("duration", "Time to allow flows to run in seconds", duration);
    cmd.AddValue ("run", "Run index (for setting repeatable seeds)", run);
    cmd.AddValue ("flow_monitor", "Enable flow monitor", flow_monitor);
    cmd.AddValue ("pcap_tracing", "Enable or disable PCAP tracing", pcap);
    cmd.AddValue ("queue_disc_type", "Queue disc type for gateway (e.g. ns3::CoDelQueueDisc)", queue_disc_type);
    cmd.AddValue ("sack", "Enable or disable SACK option", sack);
    cmd.AddValue ("recovery", "Recovery algorithm type to use (e.g., ns3::TcpPrrRecovery", recovery);
    cmd.AddValue ("filename", "Flow Monitor Filename", flow_mon_filename);
    cmd.Parse (argc, argv);

    static Ptr<OutputStreamWrapper> flowStream = asciiTraceHelper.CreateFileStream("Task_A1/" + flow_mon_filename);

    transport_prot = std::string ("ns3::") + transport_prot;

    SeedManager::SetSeed (1);
    SeedManager::SetRun (run);

    // User may find it convenient to enable logging
    LogComponentEnable("TcpVariantsComparison", LOG_LEVEL_ALL);
    //LogComponentEnable("BulkSendApplication", LOG_LEVEL_INFO);
    //LogComponentEnable("PfifoFastQueueDisc", LOG_LEVEL_ALL);

    // Calculate the ADU size
    Header* temp_header = new Ipv4Header ();
    uint32_t ip_header = temp_header->GetSerializedSize ();
    NS_LOG_LOGIC ("IP Header size is: " << ip_header);
    delete temp_header;
    temp_header = new TcpHeader ();
    uint32_t tcp_header = temp_header->GetSerializedSize ();
    NS_LOG_LOGIC ("TCP Header size is: " << tcp_header);
    delete temp_header;
    uint32_t tcp_adu_size = mtu_bytes - 20 - (ip_header + tcp_header);
    NS_LOG_LOGIC ("TCP ADU size is: " << tcp_adu_size);

    std::string bw_val = dataRate;
    if( packets_sent_per_sec == 100 ){
      bw_val = "410kbps";
    }
    else if( packets_sent_per_sec == 200){
      bw_val = "820kbps";
    }
    else if( packets_sent_per_sec == 300){
      bw_val = "1230kbps";
    }
    else if( packets_sent_per_sec == 400){
      bw_val = "1640kbps";
    }
    else if( packets_sent_per_sec == 500){
      bw_val = "2050kbps";
    }

    // bandwidth = bw_val;
    // access_bandwidth = bw_val;
    dataRate = bw_val;
    NS_LOG_INFO("DataRate : " << dataRate);
    NS_LOG_INFO("Nodes : " << nodes);
    NS_LOG_INFO("Flows : " << num_flows);

    // Set the simulation start and stop time
    double start_time = 0.1;
    double stop_time = start_time + duration;

    // 2 MB of TCP buffer
    Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (1 << 21));
    Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (1 << 21));
    Config::SetDefault ("ns3::TcpSocketBase::Sack", BooleanValue (sack));

    Config::SetDefault ("ns3::TcpL4Protocol::RecoveryType",
                        TypeIdValue (TypeId::LookupByName (recovery)));
    // Select TCP variant
    if (transport_prot.compare ("ns3::TcpWestwoodPlus") == 0)
      { 
        // TcpWestwoodPlus is not an actual TypeId name; we need TcpWestwood here
        Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId ()));
        // the default protocol type in ns3::TcpWestwood is WESTWOOD
        Config::SetDefault ("ns3::TcpWestwood::ProtocolType", EnumValue (TcpWestwood::WESTWOODPLUS));
      }
    else
      {
        TypeId tcpTid;
        NS_ABORT_MSG_UNLESS (TypeId::LookupByNameFailSafe (transport_prot, &tcpTid), "TypeId " << transport_prot << " not found");
        Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TypeId::LookupByName (transport_prot)));
      }

    // Create gateways, sources, and sinks
    NodeContainer gateways;
    gateways.Create (1);
    NodeContainer sources;
    sources.Create (nodes / 2);
    NodeContainer sinks;
    sinks.Create (nodes / 2);

    // Configure the error model
    // Here we use RateErrorModel with packet error rate
    Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
    uv->SetStream (50);
    RateErrorModel error_model;
    error_model.SetRandomVariable (uv);
    error_model.SetUnit (RateErrorModel::ERROR_UNIT_PACKET);
    error_model.SetRate (error_p);

    PointToPointHelper UnReLink;
    UnReLink.SetDeviceAttribute ("DataRate", StringValue (bandwidth));
    UnReLink.SetChannelAttribute ("Delay", StringValue (delay));
    UnReLink.SetDeviceAttribute ("ReceiveErrorModel", PointerValue (&error_model));


    InternetStackHelper stack;
    stack.InstallAll ();

    // TrafficControlHelper tchPfifo;
    // tchPfifo.SetRootQueueDisc ("ns3::PfifoFastQueueDisc");

    // TrafficControlHelper tchCoDel;
    // tchCoDel.SetRootQueueDisc ("ns3::CoDelQueueDisc");

    Ipv4AddressHelper address;
    address.SetBase ("10.0.0.0", "255.255.255.0");

    // Configure the sources and sinks net devices
    // and the channels between the sources/sinks and the gateways
    PointToPointHelper LocalLink;
    LocalLink.SetDeviceAttribute ("DataRate", StringValue (access_bandwidth));
    LocalLink.SetChannelAttribute ("Delay", StringValue (access_delay));

    Ipv4InterfaceContainer sink_interfaces;

    DataRate access_b (access_bandwidth);
    DataRate bottle_b (bandwidth);
    Time access_d (access_delay);
    Time bottle_d (delay);

    // uint32_t size = static_cast<uint32_t>((std::min (access_b, bottle_b).GetBitRate () / 8) *
    //   ((access_d + bottle_d) * 2).GetSeconds ());

    // Config::SetDefault ("ns3::PfifoFastQueueDisc::MaxSize",
    //                     QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, size / mtu_bytes)));
    // Config::SetDefault ("ns3::CoDelQueueDisc::MaxSize",
    //                     QueueSizeValue (QueueSize (QueueSizeUnit::BYTES, size)));

    NetDeviceContainer sink_devices;

    for (uint32_t i = 0; i < sources.GetN (); i++)
      {
        NetDeviceContainer devices;
        devices = LocalLink.Install (sources.Get (i), gateways.Get (0));
        // devices - 0 : source   1 : gateway
        // tchPfifo.Install (devices);
        address.NewNetwork ();
        Ipv4InterfaceContainer interfaces = address.Assign (devices);

        devices = UnReLink.Install (gateways.Get (0), sinks.Get (i));
      
        address.NewNetwork ();
        interfaces = address.Assign (devices);
        sink_interfaces.Add (interfaces.Get (1)); // 0 th device - gateway , 1st device - sink

        // Extra added
        
        sink_devices.Add(devices.Get (1));
      }

    NS_LOG_INFO("sink devices no :" << sink_devices.GetN ());

    NS_LOG_INFO ("Initialize Global Routing.");
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    uint16_t port = 50000;
    
    ApplicationContainer sink_container;

    for (uint16_t j = 0; j < num_flows; j++)
      {
        uint16_t i = j % sources.GetN ();
        if( j % sources.GetN () == 0){
          port += 1;
        }
        // AddressValue remoteAddress (InetSocketAddress (sink_interfaces.GetAddress (i, 0), port));
        // Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (tcp_adu_size));
        // BulkSendHelper ftp ("ns3::TcpSocketFactory", Address ());
        // ftp.SetAttribute ("Remote", remoteAddress);
        // ftp.SetAttribute ("SendSize", UintegerValue (tcp_adu_size));

        // ApplicationContainer sourceApp = ftp.Install (sources.Get (i));

        // //CHANGING START TIME SO SENDER DOESN'T SEND BEFORE RECEIVER APP HAS STARTED - MASHIAT
        // // sourceApp.Start (Seconds (start_time * i + 1));
        // sourceApp.Start (Seconds (start_time));

        // sourceApp.Stop (Seconds (stop_time));

        // ----------------------------------- my app ---------------------------------------------------
        Address sinkAddress = InetSocketAddress (InetSocketAddress (sink_interfaces.GetAddress (i, 0), port) );
        Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (sources.Get (i), TcpSocketFactory::GetTypeId ());

        Ptr<MyApp> app = CreateObject<MyApp> ();
        app->Setup (ns3TcpSocket, sinkAddress, tcp_adu_size, DataRate (dataRate));
        sources.Get (i)->AddApplication (app);
        app->SetStartTime (Seconds (start_time));
        app->SetStopTime (Seconds (stop_time));
        //  ---------------------------------my app ------------------------------------


        Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
        PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);

        sinkHelper.SetAttribute ("Protocol", TypeIdValue (TcpSocketFactory::GetTypeId ()));
        ApplicationContainer sinkApp = sinkHelper.Install (sinks.Get (i));

        //Added by Mashiat
        sink_container.Add(sinkApp);

        sinkApp.Start (Seconds (start_time));
        sinkApp.Stop (Seconds (stop_time));
      }

      // Added by Mashiat
      sink = StaticCast<PacketSink> (sink_container.Get (0));//change in case of multiple flows
      Simulator::Schedule (Seconds (1.1), &CalculateThroughput);

    // Set up tracing if enabled
    if (tracing)
      {
        std::ofstream ascii;
        Ptr<OutputStreamWrapper> ascii_wrap;
        ascii.open ((prefix_file_name + "-ascii").c_str ());
        ascii_wrap = new OutputStreamWrapper ((prefix_file_name + "-ascii").c_str (),
                                              std::ios::out);
        stack.EnableAsciiIpv4All (ascii_wrap);

        
      }
    
    // if (pcap)
    //   {
    //     UnReLink.EnablePcapAll (prefix_file_name, true);
    //     LocalLink.EnablePcapAll (prefix_file_name, true);
    //   }

  //   PcapHelper pcaphelper;
  //   Ptr<PcapFileWrapper> throughput_file = pcaphelper.CreateFile("Task_A1/throughput.pcap", std::ios::out, PcapHelper::DLT_PPP);
  //   sink_devices.Get (0)-> TraceConnectWithoutContext("PhyRxEnd", MakeBoundCallback(&RxEnd, throughput_file) );


    // Flow monitor
    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

    Simulator::Stop (Seconds (stop_time));
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
    uint32_t TotalRxBytes = 0;
    uint32_t ActualLostPackets = 0;

    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

    for (auto iter = stats.begin (); iter != stats.end (); ++iter) {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first); 
            // classifier returns FiveTuple in correspondance to a flowID

        *flowStream->GetStream () << "----Flow ID:" <<iter->first << std::endl;
        *flowStream->GetStream () << "Src Addr" << t.sourceAddress << " -- Dst Addr "<< t.destinationAddress << std::endl;
        *flowStream->GetStream () << "Sent Packets=" <<iter->second.txPackets << std::endl;
        *flowStream->GetStream () << "Received Packets =" <<iter->second.rxPackets << std::endl;
        *flowStream->GetStream () << "Lost Packets =" <<iter->second.lostPackets << std::endl;
        *flowStream->GetStream () << "Packet delivery ratio =" <<iter->second.rxPackets*100.0/iter->second.txPackets << "%" << std::endl;
        *flowStream->GetStream () << "Packet loss ratio =" << (iter->second.txPackets-iter->second.rxPackets)*100.0/iter->second.txPackets << "%" << std::endl;
        *flowStream->GetStream () << "Delay =" <<iter->second.delaySum << std::endl;
        *flowStream->GetStream () << "Jitter =" <<iter->second.jitterSum << std::endl;
        *flowStream->GetStream () << "Throughput =" <<iter->second.rxBytes * 8.0/(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())/1024<<"Kbps" << std::endl;
      
        TotalRxBytes += iter->second.rxBytes;
        SentPackets = SentPackets +(iter->second.txPackets);
        ReceivedPackets = ReceivedPackets + (iter->second.rxPackets);
        LostPackets = LostPackets + (iter->second.txPackets-iter->second.rxPackets);
        AvgThroughput = AvgThroughput + (iter->second.rxBytes * 8.0/(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())/1024);
        Delay = Delay + (iter->second.delaySum);
        Jitter = Jitter + (iter->second.jitterSum);
        ActualLostPackets += iter->second.lostPackets;

        j = j + 1;

    }

    Delay = Delay / ReceivedPackets;
    AvgThroughput = TotalRxBytes * 8.0 / (stop_time - start_time) / 1024;
    *flowStream->GetStream () << "--------Total Results of the simulation----------"<<std::endl;
    *flowStream->GetStream () << "Total sent packets  =" << SentPackets << std::endl;
    *flowStream->GetStream () << "Total Received Packets =" << ReceivedPackets << std::endl;
    *flowStream->GetStream () << "Total Lost Packets =" << LostPackets << std::endl;
    *flowStream->GetStream () << "Packet Loss ratio =" << ((ActualLostPackets*100.00)/SentPackets)<< "%" << std::endl;
    *flowStream->GetStream () << "Packet delivery ratio =" << ((ReceivedPackets*100.00)/SentPackets)<< "%" << std::endl;
    *flowStream->GetStream () << "Average Throughput =" << AvgThroughput<< "Kbps" << std::endl;
    *flowStream->GetStream () << "End to End Delay =" << Delay << std::endl;
    *flowStream->GetStream () << "End to End Jitter delay =" << Jitter << std::endl;
    *flowStream->GetStream () << "Total Flow id " << j << std::endl;
    //-------------------------------- end flow monitor output----------------------------------------

//   if (flow_monitor)
//     {
//       flowHelper.SerializeToXmlFile (prefix_file_name + ".flowmonitor", true, true);
//     }

  // double throughPut = receiveCount * tcp_adu_size * 8 /(1e6 * (stop_time - start_time));
  
  double averageThroughput = ((sink->GetTotalRx () * 8) / (1e6 * (stop_time - start_time)));
  NS_LOG_INFO("sink get Total Rx : " << sink->GetTotalRx ());

  Simulator::Destroy ();

  // NS_LOG_INFO("receiveCount : " << receiveCount);
  // NS_LOG_INFO("THROUGHPUT OF SINK 0 : " << throughPut << " Megabits/sec");

  
  std::cout << "\nAverage throughput: " << averageThroughput << " Mbit/s" << std::endl;

  return 0;
}
