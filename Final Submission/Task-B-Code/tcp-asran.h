#ifndef TCPASRAN_H
#define TCPASRAN_H

#include "ns3/tcp-congestion-ops.h"
#include "ns3/tcp-socket-base.h"


namespace ns3 {
/**
 * \brief The Asran implementation
 *
 * New Reno introduces partial ACKs inside the well-established Reno algorithm.
 * This and other modifications are described in RFC 6582.
 *
 * \see IncreaseWindow
 */
class TcpAsran : public TcpCongestionOps
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  TcpAsran ();

  /**
   * \brief Copy constructor.
   * \param sock object to copy.
   */
  TcpAsran (const TcpAsran& sock);

  ~TcpAsran ();

  std::string GetName () const;

  virtual void IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);
  virtual uint32_t GetSsThresh (Ptr<const TcpSocketState> tcb,
                                uint32_t bytesInFlight);
  virtual Ptr<TcpCongestionOps> Fork ();


protected:
  virtual uint32_t SlowStart (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);
  virtual void CongestionAvoidance (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);

private:
   uint32_t     m_lastMaxCwnd;
   uint32_t     last_successful_bytes_sent;
};


} // namespace ns3

#endif // TCPCONGESTIONOPS_H