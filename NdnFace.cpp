#include "NdnFace.hpp"
#include <Arduino.h>

#define NDNFACE_DBG Serial.print

ndn_NameComponent NdnFace::s_nameComps[NDNFACE_NAMECOMPS_MAX];
ndn_ExcludeEntry NdnFace::s_excludeEntries[NDNFACE_EXCLUDE_MAX];
ndn_NameComponent NdnFace::s_keyNameComps[NDNFACE_KEYNAMECOMPS_MAX];

NdnFace::NdnFace(const char* routerHost, uint16_t routerPort, uint16_t localPort, void* pktBuf, size_t pktBufSize)
  : m_buf(reinterpret_cast<uint8_t*>(pktBuf))
  , m_bufSize(pktBufSize)
  , m_routerHost(routerHost)
  , m_routerPort(routerPort)
{
  m_udp.begin(localPort);
}

void
NdnFace::loop()
{
  while (m_udp.parsePacket() > 0) {
    int len = m_udp.read(m_buf, m_bufSize);
    if (len <= 0) {
      return;
    }
    this->processPacket(m_buf, len);
    yield();
  }
}

void
NdnFace::processPacket(const uint8_t* pkt, size_t len)
{
  switch (pkt[0]) {
    case ndn_Tlv_Interest: {
      if (m_interestHandler == nullptr) {
        NDNFACE_DBG("[NdnFace] received Interest, no handler\n");
        return;
      }
      ndn::InterestLite interest(s_nameComps, NDNFACE_NAMECOMPS_MAX, s_excludeEntries, NDNFACE_EXCLUDE_MAX, s_keyNameComps, NDNFACE_KEYNAMECOMPS_MAX);
      size_t signedBegin, signedEnd;
      ndn_Error error = ndn::Tlv0_1_1WireFormatLite::decodeInterest(interest, pkt, len, &signedBegin, &signedEnd);
      if (error) {
        NDNFACE_DBG("[NdnFace] received Interest decoding error: ");
        NDNFACE_DBG(error, DEC);
        NDNFACE_DBG("\n");
        return;
      }
      m_interestHandler(interest);
      break;
    }
    case ndn_Tlv_Data: {
      if (m_dataHandler == nullptr) {
        NDNFACE_DBG("[NdnFace] received Data, no handler\n");
        return;
      }
      ndn::DataLite data(s_nameComps, NDNFACE_NAMECOMPS_MAX, s_keyNameComps, NDNFACE_KEYNAMECOMPS_MAX);
      size_t signedBegin, signedEnd;
      ndn_Error error = ndn::Tlv0_1_1WireFormatLite::decodeData(data, pkt, len, &signedBegin, &signedEnd);
      if (error) {
        NDNFACE_DBG("[NdnFace] received Data decoding error: ");
        NDNFACE_DBG(error, DEC);
        NDNFACE_DBG("\n");
        return;
      }
      m_dataHandler(data);
      break;
    }
    default: {
      NDNFACE_DBG("[NdnFace] received unknown TLV-TYPE: 0x");
      NDNFACE_DBG(pkt[0], HEX);
      NDNFACE_DBG("\n");
      break;
    }
  }
}

void
NdnFace::sendInterest(const ndn::InterestLite& interest)
{
  ndn::DynamicUInt8ArrayLite output(m_buf, m_bufSize, nullptr);
  size_t signedBegin, signedEnd, len;
  ndn_Error error = ndn::Tlv0_1_1WireFormatLite::encodeInterest(interest, &signedBegin, &signedEnd, output, &len);
  if (error) {
    NDNFACE_DBG("[NdnFace] send Interest encoding error: ");
    NDNFACE_DBG(error, DEC);
    NDNFACE_DBG("\n");
    return;
  }

  m_udp.beginPacket(m_routerHost, m_routerPort);
  m_udp.write(m_buf, len);
  m_udp.endPacket();
}

void
NdnFace::sendData(const ndn::DataLite& data)
{
  NDNFACE_DBG("[NdnFace] sendInterest not-implemented\n");
}
