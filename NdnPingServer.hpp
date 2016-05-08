#ifndef NDN_PING_SERVER_HPP
#define NDN_PING_SERVER_HPP

#include "NdnFace.hpp"

class NdnPingServer
{
public:
  typedef void (*MakePayload)(String&);

  NdnPingServer(NdnFace& face, const ndn::NameLite& prefix);

  bool
  processInterest(const ndn::InterestLite& interest);

  MakePayload makePayload;

private:
  NdnFace& m_face;
  const ndn::NameLite& m_prefix;
};

#endif // NDN_PING_SERVER_HPP
