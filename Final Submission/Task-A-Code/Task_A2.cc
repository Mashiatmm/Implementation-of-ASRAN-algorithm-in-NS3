#include <fstream>
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/mobility-module.h"
#include "ns3/spectrum-module.h"
#include "ns3/propagation-module.h"
#include "ns3/sixlowpan-module.h"
#include "ns3/lr-wpan-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv6-flow-classifier.h"
#include "ns3/flow-monitor-helper.h"
#include <ns3/lr-wpan-error-model.h>
#include "ns3/netanim-module.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WpanTaskA2");

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
  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, DataRate dataRate, uint32_t app_id);

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
  uint32_t        m_app_id;
};

MyApp::MyApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0),
    m_app_id (0)
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
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, DataRate dataRate, uint32_t app_id)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_dataRate = dataRate;
  m_app_id = app_id;
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
  m_packetsSent += 1;
  // NS_LOG_INFO("app id : " << m_app_id << " ; Sent Packets : " << m_packetsSent);
  ScheduleTx();
}

void
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}



bool tracing = true;
// uint16_t nSourceNodes=1;
uint32_t nWsnNodes;
uint16_t sinkPort=9;
uint32_t mtu_bytes = 180;
uint32_t tcp_adu_size;
uint64_t data_mbytes = 0;
double start_time = 0;
double duration = 100.0;
double stop_time;
bool sack = true;
std::string recovery = "ns3::TcpClassicRecovery";
std::string congestionAlgo = "TcpNewReno";
std::string filePrefix;
std::string delay = "0.01ms";
uint32_t nodes = 10;
uint32_t num_flows = 20;
uint32_t packets_sent_per_sec = 500;
uint32_t speed = 5;
std::string flow_mon_filename = "demo";
std::string dataRate = "1Mbps";

Ptr<LrWpanErrorModel>  lrWpanError;

AsciiTraceHelper asciiTraceHelper;
Ptr<OutputStreamWrapper> throughput_stream = asciiTraceHelper.CreateFileStream("Task_A2/throughput_vs_time");

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


void initialize(int argc, char** argv) {
  CommandLine cmd (__FILE__);
  cmd.AddValue ("nodes", "Number of nodes", nodes);
  cmd.AddValue ("flows", "Number of flows", num_flows);
  cmd.AddValue ("packets_sent_per_sec", "Packets sent per second for each sender ", packets_sent_per_sec);
  cmd.AddValue ("speed", "Speed of each node ", speed);
  cmd.AddValue ("tracing", "turn on log components", tracing);
  cmd.AddValue ("filename", "Flow Monitor Filename", flow_mon_filename);
  // cmd.AddValue ("nSourceNodes", "turn on log components", nSourceNodes);
  // cmd.AddValue ("tracedNode", "turn on log components", tracedNode);
  cmd.Parse (argc, argv);

  if( tracing ) {
    LogComponentEnable("PacketSink", LOG_LEVEL_INFO);
    LogComponentEnable("WpanTaskA2", LOG_LEVEL_INFO);
  }

  nWsnNodes = nodes + 1;
  // tracedNode = std::max(1, tracedNode);
  filePrefix = "wpan-sourceCount" + std::to_string(nodes);

  Config::SetDefault ("ns3::TcpL4Protocol::RecoveryType",
                      TypeIdValue (TypeId::LookupByName (recovery)));

  congestionAlgo = "ns3::" + congestionAlgo;
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", 
                      TypeIdValue (TypeId::LookupByName (congestionAlgo)));

  // 2 MB of TCP buffer
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (1 << 21));
  Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (1 << 21));
  Config::SetDefault ("ns3::TcpSocketBase::Sack", BooleanValue (sack));

  Header* temp_header = new Ipv6Header ();
  uint32_t ip_header = temp_header->GetSerializedSize ();
  delete temp_header;
  temp_header = new TcpHeader ();
  uint32_t tcp_header = temp_header->GetSerializedSize ();
  delete temp_header;
  tcp_adu_size = mtu_bytes - 20 - (ip_header + tcp_header);

  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (tcp_adu_size));

  NS_LOG_INFO("TCP ADU SIZE : " << tcp_adu_size);

  stop_time = start_time + duration;

  lrWpanError = CreateObject<LrWpanErrorModel> ();

  std::string bw_val = dataRate;
    if( packets_sent_per_sec == 100 ){
      bw_val = "80kbps";
    }
    else if( packets_sent_per_sec == 200){
      bw_val = "160kbps";
    }
    else if( packets_sent_per_sec == 300){
      bw_val = "240kbps";
    }
    else if( packets_sent_per_sec == 400){
      bw_val = "320kbps";
    }
    else if( packets_sent_per_sec == 500){
      bw_val = "400kbps";
    }

    dataRate = bw_val;


  std::cout << "------------------------------------------------------\n"; 
  std::cout << "Nodes: " << nodes << "\n"; 
  std::cout << "Datarate: " << dataRate << "\n"; 
  std::cout << "Flows: " << num_flows << "\n"; 
  std::cout << "------------------------------------------------------\n"; 
}

int main (int argc, char** argv) {
  initialize(argc, argv);

  static Ptr<OutputStreamWrapper> flowStream = asciiTraceHelper.CreateFileStream("Task_A2/" + flow_mon_filename);

  Packet::EnablePrinting ();

  NodeContainer wsnNodes;
  wsnNodes.Create (nWsnNodes);


  MobilityHelper mobility;

  // ----------------------------------- 3D mobility model ---------------------------------------
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  std::stringstream ssSpeed;
  ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << speed << "]";
  mobility.SetMobilityModel("ns3::GaussMarkovMobilityModel",
                                 "MeanVelocity", StringValue(ssSpeed.str()),
                                 "Bounds", BoxValue(Box(0, 25, 0, 25, 0, 25)),
                                 "TimeStep", TimeValue(Seconds(5)));
  mobility.Install (wsnNodes);

  // ----------------------------------- static mobility model ---------------------------------------
  // MobilityHelper mobility;
  // mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  // mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
  //                                "MinX", DoubleValue (0.0),
  //                                "MinY", DoubleValue (0.0),
  //                                "DeltaX", DoubleValue (80),
  //                                "DeltaY", DoubleValue (80),
  //                                "GridWidth", UintegerValue (10),
  //                                "LayoutType", StringValue ("RowFirst"));
  // mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  // mobility.Install (wsnNodes);

  // ----------------------------------- 2D mobility model ---------------------------------------
  // mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
  //                                "Mode", StringValue ("Time"),
  //                                "Time", StringValue ("2s"),
  //                                "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
  //                                "Bounds", RectangleValue (Rectangle (0, 25, 0, 25)));
  // mobility.Install (wsnNodes);

  LrWpanHelper lrWpanHelper;
  NetDeviceContainer lrwpanDevices = lrWpanHelper.Install (wsnNodes);

  lrWpanHelper.AssociateToPan (lrwpanDevices, 0);

  InternetStackHelper internetv6;
  internetv6.Install (wsnNodes);
  // internetv6.Install (wiredNodes.Get (0));

  SixLowPanHelper sixLowPanHelper;
  NetDeviceContainer sixLowPanDevices = sixLowPanHelper.Install (lrwpanDevices);

  // CsmaHelper csmaHelper;
  // NetDeviceContainer csmaDevices = csmaHelper.Install (wiredNodes);

  Ipv6AddressHelper ipv6;

  ipv6.SetBase (Ipv6Address ("2001:f00d::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer wsnDeviceInterfaces;
  wsnDeviceInterfaces = ipv6.Assign (sixLowPanDevices);
  wsnDeviceInterfaces.SetForwarding (0, true);
  wsnDeviceInterfaces.SetDefaultRouteInAllNodes (0);

  for (uint32_t i = 0; i < sixLowPanDevices.GetN (); i++) {
    Ptr<NetDevice> dev = sixLowPanDevices.Get (i);
    dev->SetAttribute ("UseMeshUnder", BooleanValue (true));
    dev->SetAttribute ("MeshUnderRadius", UintegerValue (10));
  }
  int  i = 1;
  int app_no = 0;
  for( uint32_t f = 1; f <= num_flows; f++ ) {
    // BulkSendHelper sourceApp ("ns3::TcpSocketFactory",
    //                           Inet6SocketAddress (wsnDeviceInterfaces.GetAddress (0, 1), 
    //                           sinkPort));
    // sourceApp.SetAttribute ("SendSize", UintegerValue (tcp_adu_size));
    // // sourceApp.SetAttribute ("MaxBytes", UintegerValue (data_mbytes * 1000000));
    // ApplicationContainer sourceApps = sourceApp.Install (wsnNodes.Get (i));
    // sourceApps.Start (Seconds (start_time));
    // sourceApps.Stop (Seconds (stop_time));

    Address sinkAddress = Inet6SocketAddress (Inet6SocketAddress (wsnDeviceInterfaces.GetAddress (0, 1), sinkPort) );
    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (wsnNodes.Get (i), TcpSocketFactory::GetTypeId ());

    Ptr<MyApp> app = CreateObject<MyApp> ();
    app->Setup (ns3TcpSocket, sinkAddress, tcp_adu_size, DataRate (dataRate), app_no);
    wsnNodes.Get (i)->AddApplication (app);
    app->SetStartTime (Seconds (start_time));
    app->SetStopTime (Seconds (stop_time));

    PacketSinkHelper sinkApp ("ns3::TcpSocketFactory",
    Inet6SocketAddress (Ipv6Address::GetAny (), sinkPort));
    sinkApp.SetAttribute ("Protocol", TypeIdValue (TcpSocketFactory::GetTypeId ()));
    ApplicationContainer sinkApps = sinkApp.Install (wsnNodes.Get(0));
    
    sink = StaticCast<PacketSink> (sinkApps.Get (0));
    
    sinkApps.Start (Seconds (0.0));
    sinkApps.Stop (Seconds (stop_time));

    sinkPort++;
    i++;
    app_no++;
    if( i % ( nodes + 1 ) == 0){
      i = 1;
    }
  }

  // if (tracing) {
  //   AsciiTraceHelper ascii;
  //   lrWpanHelper.EnableAsciiAll (ascii.CreateFileStream (filePrefix + ".tr"));
  //   lrWpanHelper.EnablePcapAll (filePrefix, false);

  //   csmaHelper.EnableAsciiAll (ascii.CreateFileStream (filePrefix + ".tr"));
  //   csmaHelper.EnablePcapAll (filePrefix, false);

  //   Simulator::Schedule (Seconds (0.00001), &TraceCwnd, filePrefix + "-cwnd.data");
  //   Simulator::Schedule (Seconds (0.00001), &TraceSsThresh, filePrefix + "-ssth.data");
  // }

  Simulator::Schedule (Seconds (1.1), &CalculateThroughput);

  // AnimationInterface anim ("Task_A2/mywpan.xml"); 

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

    Ptr<Ipv6FlowClassifier> classifier = DynamicCast<Ipv6FlowClassifier> (flowmon.GetClassifier6 ());
    FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
    for (auto iter = stats.begin (); iter != stats.end (); ++iter) {
        Ipv6FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first); 
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
        ActualLostPackets += iter->second.lostPackets;
        if(iter->second.rxPackets != 0){
          AvgThroughput = AvgThroughput + (iter->second.rxBytes * 8.0/(iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds())/1024);
        }
        
        Delay = Delay + (iter->second.delaySum);
        Jitter = Jitter + (iter->second.jitterSum);

        j = j + 1;

    }
    
    AvgThroughput = AvgThroughput/j;
    AvgThroughput = TotalRxBytes * 8.0 / (stop_time - start_time) / 1024;
    Delay = Delay / ReceivedPackets;
    NS_LOG_INFO("Average Throughput : " << AvgThroughput);
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

  Simulator::Destroy ();

  return 0;
}

