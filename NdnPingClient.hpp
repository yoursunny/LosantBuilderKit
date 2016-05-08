#ifndef NDN_PING_CLIENT_HPP
#define NDN_PING_CLIENT_HPP

#include "NdnFace.hpp"

class NdnPingClient
{
public:
  NdnPingClient(NdnFace& face, ndn::InterestLite& interest, int pingInterval, int led = -1);

  void
  loop();

  bool
  processData(const ndn::DataLite& data);

  bool
  ping();

private:
  NdnFace& m_face;
  ndn::InterestLite& m_interest;
  uint8_t m_seqBuf[9];
  const int m_pingInterval;
  const int m_led;
  unsigned long m_seq;
  unsigned long m_lastPing;
  bool m_isPending;
};

#endif // NDN_PING_CLIENT_HPP
