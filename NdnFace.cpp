#include "NdnFace.hpp"
#include "logger.hpp"

#define NDNFACE_DBG(...) DBG(NdnFace, __VA_ARGS__)

void
ndn_parseName(ndn::NameLite& name, char* uri)
{
  name.clear();
  char* token = strtok(uri, "/");
  while (token != nullptr) {
    name.append(token);
    token = strtok(nullptr, "/");
  }
}

ndn_NameComponent NdnFace::s_nameComps[NDNFACE_NAMECOMPS_MAX];
ndn_ExcludeEntry NdnFace::s_excludeEntries[NDNFACE_EXCLUDE_MAX];
ndn_NameComponent NdnFace::s_keyNameComps[NDNFACE_KEYNAMECOMPS_MAX];

NdnFace::NdnFace(const char* routerHost, uint16_t routerPort, uint16_t localPort, void* inBuf, size_t inBufSize)
  : m_routerHost(routerHost)
  , m_routerPort(routerPort)
  , m_inBuf(reinterpret_cast<uint8_t*>(inBuf))
  , m_inBufSize(inBufSize)
  , m_hmacKey(nullptr)
  , m_hmacKeySize(0)
{
  m_udp.begin(localPort);
}

void
NdnFace::setHmacKey(const uint8_t* hmacKey, size_t hmacKeySize)
{
  m_hmacKey = hmacKey;
  m_hmacKeySize = hmacKeySize;
  ndn_digestSha256(m_hmacKey, m_hmacKeySize, m_hmacKeyDigest);
}

void
NdnFace::loop(int maxPackets)
{
  int packetLimit = maxPackets;
  while (m_udp.parsePacket() > 0) {
    if (--packetLimit <= 0) {
      yield();
      continue;
    }
    int len = m_udp.read(m_inBuf, m_inBufSize);
    if (len <= 0) {
      return;
    }
    unsigned long t1 = micros();
    this->processPacket(m_inBuf, len);
    unsigned long t2 = micros();
    NDNFACE_DBG("packet processed in " << _DEC(t2 - t1) << "us");
    yield();
  }
  if (packetLimit < 0) {
    NDNFACE_DBG(_DEC(-packetLimit) << " packets discarded");
  }
}

void
NdnFace::processPacket(const uint8_t* pkt, size_t len)
{
  switch (pkt[0]) {
    case ndn_Tlv_Interest: {
      if (m_interestHandler == nullptr) {
        NDNFACE_DBG("received Interest, no handler");
        return;
      }
      ndn::InterestLite interest(s_nameComps, NDNFACE_NAMECOMPS_MAX, s_excludeEntries, NDNFACE_EXCLUDE_MAX, s_keyNameComps, NDNFACE_KEYNAMECOMPS_MAX);
      size_t signedBegin, signedEnd;
      ndn_Error error = ndn::Tlv0_1_1WireFormatLite::decodeInterest(interest, pkt, len, &signedBegin, &signedEnd);
      if (error) {
        NDNFACE_DBG("received Interest decoding error: " << _DEC(error));
        return;
      }
      m_interestHandler(interest);
      break;
    }
    case ndn_Tlv_Data: {
      if (m_dataHandler == nullptr) {
        NDNFACE_DBG("received Data, no handler");
        return;
      }
      ndn::DataLite data(s_nameComps, NDNFACE_NAMECOMPS_MAX, s_keyNameComps, NDNFACE_KEYNAMECOMPS_MAX);
      size_t signedBegin, signedEnd;
      ndn_Error error = ndn::Tlv0_1_1WireFormatLite::decodeData(data, pkt, len, &signedBegin, &signedEnd);
      if (error) {
        NDNFACE_DBG("received Data decoding error: " << _DEC(error));
        return;
      }
      m_dataHandler(data);
      break;
    }
    default: {
      NDNFACE_DBG("received unknown TLV-TYPE: 0x" << _HEX(pkt[0]));
      break;
    }
  }
}

void
NdnFace::sendPacket(const uint8_t* pkt, size_t pktSize)
{
  m_udp.beginPacket(m_routerHost, m_routerPort);
  m_udp.write(pkt, pktSize);
  m_udp.endPacket();
}

void
NdnFace::sendInterest(ndn::InterestLite& interest)
{
  uint8_t outBuf[NDNFACE_OUTBUF_SIZE];
  ndn::DynamicUInt8ArrayLite output(outBuf, NDNFACE_OUTBUF_SIZE, nullptr);
  size_t signedBegin, signedEnd, len;
  ndn_Error error = ndn::Tlv0_1_1WireFormatLite::encodeInterest(interest, &signedBegin, &signedEnd, output, &len);
  if (error) {
    NDNFACE_DBG("send Interest encoding error: " << _DEC(error));
    return;
  }

  this->sendPacket(outBuf, len);
}

void
NdnFace::sendData(ndn::DataLite& data)
{
  if (m_hmacKey == nullptr) {
    NDNFACE_DBG("cannot send Data: HMAC key is unset");
    return;
  }
  ndn::SignatureLite& signature = data.getSignature();
  signature.setType(ndn_SignatureType_DigestSha256Signature);
  signature.getKeyLocator().setType(ndn_KeyLocatorType_KEY_LOCATOR_DIGEST);
  signature.getKeyLocator().setKeyData(ndn::BlobLite(m_hmacKeyDigest, sizeof(m_hmacKeyDigest)));

  uint8_t outBuf[NDNFACE_OUTBUF_SIZE];
  ndn::DynamicUInt8ArrayLite output(outBuf, NDNFACE_OUTBUF_SIZE, nullptr);
  size_t signedBegin, signedEnd, len;
  ndn_Error error = ndn::Tlv0_1_1WireFormatLite::encodeData(data, &signedBegin, &signedEnd, output, &len);
  if (error) {
    NDNFACE_DBG("send Data encoding error: " << _DEC(error));
    return;
  }

  uint8_t signatureValue[ndn_SHA256_DIGEST_SIZE];
  ndn_computeHmacWithSha256(m_hmacKey, m_hmacKeySize, outBuf + signedBegin, signedEnd - signedBegin, signatureValue);
  data.getSignature().setSignature(ndn::BlobLite(signatureValue, ndn_SHA256_DIGEST_SIZE));
  error = ndn::Tlv0_1_1WireFormatLite::encodeData(data, &signedBegin, &signedEnd, output, &len);
  if (error) {
    NDNFACE_DBG("send Data encoding error: " << _DEC(error));
    return;
  }

  this->sendPacket(outBuf, len);
}
