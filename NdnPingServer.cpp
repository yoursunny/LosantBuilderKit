#include "NdnPingServer.hpp"
#include <Arduino.h>

#define NDNPINGSERVER_DBG Serial.print

NdnPingServer::NdnPingServer(NdnFace& face, const ndn::NameLite& prefix)
  : m_face(face)
  , m_prefix(prefix)
{
}

bool
NdnPingServer::processInterest(const ndn::InterestLite& interest)
{
  if (!m_prefix.match(interest.getName())) {
    return false;
  }
  static ndn_NameComponent nameComps[8];
  ndn::DataLite data(nameComps, 8, nullptr, 0);
  data.getName().set(interest.getName());
  NDNPINGSERVER_DBG("[NdnPingServer] process Interest\n");

  String payload;
  if (this->makePayload == nullptr) {
    payload += "OK";
  }
  else {
    this->makePayload(payload);
  }
  data.setContent(ndn::BlobLite(reinterpret_cast<const uint8_t*>(payload.c_str()), payload.length()));
  m_face.sendData(data);
}
