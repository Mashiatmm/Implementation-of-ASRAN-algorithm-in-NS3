// ASRAN
#define NS_LOG_APPEND_CONTEXT \
  { std::clog << Simulator::Now ().GetSeconds () << " "; }

#include "tcp-asran.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("TcpAsran");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TcpAsran);

TypeId
TcpAsran::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpAsran")
    .SetParent<TcpCongestionOps> ()
    .SetGroupName ("Internet")
    .AddConstructor<TcpAsran> ()
  ;
  return tid;
}

TcpAsran::TcpAsran (void) : 
    TcpCongestionOps (), 
    m_lastMaxCwnd (0),
    last_successful_bytes_sent (0)
{

  NS_LOG_FUNCTION (this);
}

TcpAsran::TcpAsran (const TcpAsran& sock)
  : TcpCongestionOps (sock),
  m_lastMaxCwnd (sock.m_lastMaxCwnd),
  last_successful_bytes_sent(sock.last_successful_bytes_sent)
{
  NS_LOG_FUNCTION (this);
}

TcpAsran::~TcpAsran (void)
{
}


uint32_t
TcpAsran::SlowStart (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  if (segmentsAcked >= 1)
    {
      last_successful_bytes_sent = tcb->m_cWnd; // Added by Me
      tcb->m_cWnd += tcb->m_segmentSize;
      NS_LOG_INFO ("In SlowStart, updated to cwnd " << tcb->m_cWnd << " ssthresh " << tcb->m_ssThresh);
      return segmentsAcked - 1;
    }

  return 0;
}


void
TcpAsran::CongestionAvoidance (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  if (segmentsAcked > 0)
    {
      double adder = static_cast<double> (tcb->m_segmentSize * tcb->m_segmentSize) / tcb->m_cWnd.Get ();
      adder = std::max (1.0, adder);
      tcb->m_cWnd += static_cast<uint32_t> (adder);
      NS_LOG_INFO ("In CongAvoid, updated to cwnd " << tcb->m_cWnd <<
                   " ssthresh " << tcb->m_ssThresh);
    }
}

/**
 * \brief Try to increase the cWnd following the NewReno specification
 *
 * \see SlowStart
 * \see CongestionAvoidance
 *
 * \param tcb internal congestion state
 * \param segmentsAcked count of segments acked
 */
void
TcpAsran::IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
{
  NS_LOG_FUNCTION (this << tcb << segmentsAcked);

  if (tcb->m_cWnd < tcb->m_ssThresh)
    {
      segmentsAcked = SlowStart (tcb, segmentsAcked);
    }

  if (tcb->m_cWnd >= tcb->m_ssThresh)
    {
      CongestionAvoidance (tcb, segmentsAcked);
    }

  /* At this point, we could have segmentsAcked != 0. This because RFC says
   * that in slow start, we should increase cWnd by min (N, SMSS); if in
   * slow start we receive a cumulative ACK, it counts only for 1 SMSS of
   * increase, wasting the others.
   *
   * // Incorrect assert, I am sorry
   * NS_ASSERT (segmentsAcked == 0);
   */
}

std::string
TcpAsran::GetName () const
{
  return "TcpAsran";
}

uint32_t
TcpAsran::GetSsThresh (Ptr<const TcpSocketState> state,
                         uint32_t bytesInFlight)
{
  NS_LOG_FUNCTION (this << state << bytesInFlight);
  // std::cout << Simulator::Now ().GetSeconds () << "\t" << bytesInFlight << "\t" << state->m_congState << std::endl;
  // std::cout << "last max cwnd : " << m_lastMaxCwnd << "\t" << std::endl;
  // uint32_t segCwnd = state->GetCwndInSegments ();
  // std::cout << "Loss at cWnd=" << segCwnd << " segments in flight=" << bytesInFlight / state->m_segmentSize << std::endl;

  uint32_t original_ssthresh = std::max (2 * state->m_segmentSize, bytesInFlight / 2);
  // try to get the last successful bytesInFlight that has been acknowledged
  // uint32_t original_ssthresh = bytesInFlight / 2;
  // std::cout << "original threshold : " << original_ssthresh << " \t last_max_cwnd" << m_lastMaxCwnd << std::endl;
  uint32_t threshold = std::max (original_ssthresh, m_lastMaxCwnd);
  // reason not retransmission timeout
  if( state->m_congState == TcpSocketState::CA_RECOVERY ){
    // m_lastMaxCwnd = last_successful_bytes_sent;
    if(bytesInFlight != 0){
      m_lastMaxCwnd = bytesInFlight;
    }
 
  }
  
  // std::cout << "threshold: " << threshold << "\t" << std::endl;

  return threshold;
}

Ptr<TcpCongestionOps>
TcpAsran::Fork ()
{
  return CopyObject<TcpAsran> (this);
}

} // namespace ns3
