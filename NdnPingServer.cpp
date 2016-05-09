#include "NdnPingServer.hpp"
#include "logger.hpp"

#define NDNPINGSERVER_DBG(...) DBG(NdnPingServer, __VA_ARGS__)

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
  NDNPINGSERVER_DBG("processing request");

  char payloadBuf[256];
  PString payload(payloadBuf, sizeof(payloadBuf));
  if (this->makePayload == nullptr) {
    payload << "OK";
  }
  else {
    this->makePayload(payload);
  }
  data.setContent(ndn::BlobLite(reinterpret_cast<const uint8_t*>(payloadBuf), payload.length()));
  m_face.sendData(data);
}
