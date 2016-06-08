#ifndef NDN_PING_SERVER_HPP
#define NDN_PING_SERVER_HPP

#include <esp8266ndn.h>
#include <PString.h>

class NdnPingServer
{
public:
  typedef void (*MakePayload)(PString&);

  NdnPingServer(ndn::Face& face, const ndn::NameLite& prefix);

  bool
  processInterest(const ndn::InterestLite& interest);

  MakePayload makePayload;

private:
  ndn::Face& m_face;
  const ndn::NameLite& m_prefix;
};

#endif // NDN_PING_SERVER_HPP
