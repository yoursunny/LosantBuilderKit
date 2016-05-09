#include "NdnPingClient.hpp"
#include <Arduino.h>
#include "logger.hpp"

#define NDNPINGCLIENT_DBG(...) DBG(NdnPingClient, __VA_ARGS__)

NdnPingClient::NdnPingClient(NdnFace& face, ndn::InterestLite& interest, int pingInterval, int led)
  : m_face(face)
  , m_interest(interest)
  , m_pingInterval(pingInterval)
  , m_led(led)
  , m_lastPing(millis())
  , m_isPending(false)
{
  if (m_led >= 0) {
    pinMode(m_led, OUTPUT);
    digitalWrite(m_led, HIGH);
  }
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
  NDNPINGCLIENT_DBG("received reply rtt=" << _DEC(millis() - m_lastPing) << "ms");
  if (m_led >= 0) {
    digitalWrite(m_led, HIGH);
  }
  return true;
}

bool
NdnPingClient::ping()
{
  if (m_isPending) {
    NDNPINGCLIENT_DBG("last ping timed out");
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

  NDNPINGCLIENT_DBG("sending ping seq=" << _HEX(static_cast<uint32_t>(seq)));
  m_face.sendInterest(m_interest);

  m_isPending = true;
  m_lastPing = millis();
  if (m_led >= 0) {
    digitalWrite(m_led, LOW);
  }
}
