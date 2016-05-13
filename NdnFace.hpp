#ifndef NDN_FACE_HPP
#define NDN_FACE_HPP

#include <ndn-cpp-lite.h>
#include <WiFiUdp.h>

/**
 * \brief parse NDN name from URI
 * \param name destination
 * \param uri URI like string; this buffer must be mutable and will be overwritten;
 *            limitation: every name component must be non-empty, and cannot contain '/' or '\0'
 */
void
ndn_parseName(ndn::NameLite& name, char* uri);

typedef void (*NdnInterestCallback)(const ndn::InterestLite& interest);
typedef void (*NdnDataCallback)(const ndn::DataLite& data);

#define NDNFACE_NAMECOMPS_MAX 16
#define NDNFACE_EXCLUDE_MAX 4
#define NDNFACE_KEYNAMECOMPS_MAX 8
#define NDNFACE_OUTBUF_SIZE 1500

class NdnFace
{
public:
  NdnFace(const char* routerHost, uint16_t routerPort, uint16_t localPort, void* pktBuf, size_t pktBufSize);

  void
  onInterest(NdnInterestCallback cb)
  {
    m_interestHandler = cb;
  }

  void
  onData(NdnDataCallback cb)
  {
    m_dataHandler = cb;
  }

  void
  setHmacKey(const uint8_t* hmacKey, size_t hmacKeySize);

  void
  loop(int maxPackets = -1);

  void
  sendPacket(const uint8_t* pkt, size_t pktSize);

  void
  sendInterest(ndn::InterestLite& interest);

  void
  sendData(ndn::DataLite& data);

private:
  void
  processPacket(const uint8_t* pkt, size_t pktSize);

public:
  static ndn_NameComponent s_nameComps[NDNFACE_NAMECOMPS_MAX];
  static ndn_ExcludeEntry s_excludeEntries[NDNFACE_EXCLUDE_MAX];
  static ndn_NameComponent s_keyNameComps[NDNFACE_KEYNAMECOMPS_MAX];

private:
  WiFiUDP m_udp;
  const char* const m_routerHost;
  IPAddress m_routerIp;
  const uint16_t m_routerPort;
  uint8_t* m_inBuf;
  const size_t m_inBufSize;

  const uint8_t* m_hmacKey;
  size_t m_hmacKeySize;
  uint8_t m_hmacKeyDigest[ndn_SHA256_DIGEST_SIZE];

  NdnInterestCallback m_interestHandler;
  NdnDataCallback m_dataHandler;
};

#endif // NDN_FACE_HPP
