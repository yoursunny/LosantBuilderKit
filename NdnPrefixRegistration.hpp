#ifndef NDN_PREFIX_REGISTRATION_HPP
#define NDN_PREFIX_REGISTRATION_HPP

#include <ESPAsyncTCP.h>
#include "NdnFace.hpp"

/**
 * \brief perform periodical NDN prefix registrations with the assistance of an HTTP server
 */
class NdnPrefixRegistration
{
public:
  NdnPrefixRegistration(NdnFace& face, const char* httpHost, uint16_t httpPort, const char* httpUri, int registerInterval = 30000);

  ~NdnPrefixRegistration();

  void
  loop();

  bool
  processData(const ndn::DataLite& data);

private:
  static void
  tcpConnectH(void* self, AsyncClient*);

  void
  tcpConnect();

  void
  httpSend();

  static void
  tcpDataH(void* self, AsyncClient*, void* data, size_t dataLen);

  void
  tcpData(void* data, size_t dataLen);

  void
  parseHttpHeader();

  void
  ndnSend();

  static void
  tcpDisconnectH(void* self, AsyncClient*);

  void
  tcpDisconnect();

private:
  enum class State {
    NONE,
    HTTP_CONNECTING,
    HTTP_SENT,
    HTTP_RECV_HEADER,
    HTTP_RECV_PAYLOAD,
    PAYLOAD_READY,
    NDN_SENT,
    DONE,
    FAIL
  };

  AsyncClient m_tcp;
  const char* const m_httpHost;
  const uint16_t m_httpPort;
  const char* const m_httpUri;

  NdnFace& m_face;
  const int m_registerInterval;
  unsigned long m_lastRegister;

  int m_step;
  State m_state;
  char m_httpHeader[32];
  size_t m_pos;
  ssize_t m_payloadLen;
  uint8_t* m_payload;
  bool m_hasMore;
};

#endif // NDN_PREFIX_REGISTRATION_HPP
