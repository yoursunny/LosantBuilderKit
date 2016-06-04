#ifndef NDN_PING_CLIENT_HPP
#define NDN_PING_CLIENT_HPP

#include "NdnFace.hpp"

enum class NdnPingEvent {
  NONE,
  PROBE,
  RESPONSE,
  TIMEOUT,
  NACK
};

/**
 * \brief ping event handler
 * \param seq (not implemented)
 */
typedef void (*NdnPingHandler)(NdnPingEvent evt, uint64_t seq);

class NdnPingClient
{
public:
  NdnPingClient(NdnFace& face, ndn::InterestLite& interest, int pingInterval);

  void
  loop();

  bool
  processData(const ndn::DataLite& data);

  bool
  ping();

  void
  onEvent(NdnPingHandler cb)
  {
    m_handler = cb;
  }

private:
  NdnFace& m_face;
  ndn::InterestLite& m_interest;
  uint8_t m_seqBuf[9];
  const int m_pingInterval;
  unsigned long m_seq;
  unsigned long m_lastPing;
  bool m_isPending;
  NdnPingHandler m_handler;
};

#endif // NDN_PING_CLIENT_HPP
