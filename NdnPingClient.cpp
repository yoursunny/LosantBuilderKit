#include "NdnPingClient.hpp"
#include "logger.hpp"

#define NDNPINGCLIENT_DBG(...) DBG(NdnPingClient, __VA_ARGS__)

NdnPingClient::NdnPingClient(NdnFace& face, ndn::InterestLite& interest, int pingInterval)
  : m_face(face)
  , m_interest(interest)
  , m_pingInterval(pingInterval)
  , m_lastPing(millis())
  , m_isPending(false)
  , m_handler(nullptr)
{
}

void
NdnPingClient::loop()
{  
  if (millis() - m_lastPing < m_pingInterval) {
    return;
  }

  this->ping();
}

bool
NdnPingClient::processData(const ndn::DataLite& data)
{
  if (!m_interest.getName().match(data.getName())) {
    return false;
  }
  m_isPending = false;
  NDNPINGCLIENT_DBG(F("received reply rtt=") << _DEC(millis() - m_lastPing) << "ms");

  if (m_handler != nullptr) {
    m_handler(NdnPingEvent::RESPONSE, 0);
  }

  return true;
}

bool
NdnPingClient::ping()
{
  if (m_isPending) {
    NDNPINGCLIENT_DBG(F("last ping timed out"));
    if (m_handler != nullptr) {
      m_handler(NdnPingEvent::TIMEOUT, 0);
    }
  }

  ndn::NameLite& name = m_interest.getName();
  uint64_t seq;
  if (name.get(-1).isSequenceNumber()) {
    name.get(-1).toSequenceNumber(seq);
    ++seq;
    --reinterpret_cast<ndn_Name&>(name).nComponents;
  }
  else {
    ndn_generateRandomBytes(reinterpret_cast<uint8_t*>(&seq), sizeof(seq));
  }
  seq = static_cast<uint32_t>(seq);
  name.appendSequenceNumber(seq, m_seqBuf, sizeof(m_seqBuf));

  NDNPINGCLIENT_DBG(F("sending ping seq=") << _HEX(static_cast<uint32_t>(seq)));
  m_face.sendInterest(m_interest);

  m_isPending = true;
  m_lastPing = millis();

  if (m_handler != nullptr) {
    m_handler(NdnPingEvent::PROBE, 0);
  }
}
