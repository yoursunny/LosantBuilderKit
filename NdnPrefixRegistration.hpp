#ifndef NDN_PREFIX_REGISTRATION_HPP
#define NDN_PREFIX_REGISTRATION_HPP

#include <ESPAsyncTCP.h>
#include "NdnFace.hpp"

/**
 * \brief perform periodical NDN prefix registrations
 *
 * This class requires an HTTP server that prepares and signs prefix registration commands.
 * Request: GET <uri><i> HTTP/1.1
 *   <uri>: httpUri
 *   <i>: a number starting with 1
 * Response status: 200
 * Response payload: the i-th prefix registration command Interest
 * Response header: "X-Has-More: yes", if there are more than i commands
 *   If present, request for <i+1> will be sent next
 */
class NdnPrefixRegistration
{
public:
  /**
   * \param registerInterval interval between successful registrations, in millis
   * \param retryInterval interval between unsucessful registrations, in millis
   */
  NdnPrefixRegistration(NdnFace& face, const char* httpHost, uint16_t httpPort, const char* httpUri, int registerInterval = 60000, int retryInterval = 10000);

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
  enum Step {
    STEP_OK = -1,
    STEP_FAIL = -2
  };
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
  const int m_retryInterval;
  unsigned long m_lastRegister;

  /**
   * <0: one of the special values indicating whether the last round of registrations were successful
   * >0: currently processing i-th registration command
   */
  int m_step;
  /**
   * progress within current step
   */
  State m_state;
  char m_httpHeader[32];
  size_t m_pos;
  ssize_t m_payloadLen;
  uint8_t* m_payload;
  bool m_hasMore;
};

#endif // NDN_PREFIX_REGISTRATION_HPP
