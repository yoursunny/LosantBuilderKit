#ifndef NDN_FACE_HPP
#define NDN_FACE_HPP

#include <ndn-cpp-lite.h>
#include <WiFiUdp.h>

typedef void (*NdnInterestCallback)(const ndn::InterestLite& interest);
typedef void (*NdnDataCallback)(const ndn::DataLite& data);

#define NDNFACE_NAMECOMPS_MAX 8
#define NDNFACE_EXCLUDE_MAX 4
#define NDNFACE_KEYNAMECOMPS_MAX 0

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
  loop();

  void
  sendInterest(const ndn::InterestLite& interest);

  void
  sendData(const ndn::DataLite& data);

private:
  void
  processPacket(const uint8_t* pkt, size_t pktSize);

public:
  uint8_t* m_buf;
  const size_t m_bufSize;
  static ndn_NameComponent s_nameComps[NDNFACE_NAMECOMPS_MAX];
  static ndn_ExcludeEntry s_excludeEntries[NDNFACE_EXCLUDE_MAX];
  static ndn_NameComponent s_keyNameComps[NDNFACE_KEYNAMECOMPS_MAX];

private:
  WiFiUDP m_udp;
  const char* m_routerHost;
  const uint16_t m_routerPort;

  NdnInterestCallback m_interestHandler;
  NdnDataCallback m_dataHandler;
};

#endif // NDN_FACE_HPP
