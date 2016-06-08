#include "NdnPrefixRegistration.hpp"
#include "logger.hpp"
#include <PString.h>
#include <Streaming.h>

#define NDNPREFIXREG_DBG(...) DBG(NdnPrefixRegistration, __VA_ARGS__)

NdnPrefixRegistration::NdnPrefixRegistration(ndn::Face& face, const char* httpHost, uint16_t httpPort, const char* httpUri, int registerInterval, int retryInterval)
  : m_httpHost(httpHost)
  , m_httpPort(httpPort)
  , m_httpUri(httpUri)
  , m_face(face)
  , m_registerInterval(registerInterval)
  , m_retryInterval(retryInterval)
  , m_lastRegister(millis())
  , m_step(STEP_FAIL)
  , m_state(State::NONE)
  , m_payload(nullptr)
  , m_hasMore(false)
{
  m_tcp.setRxTimeout(8000);
  m_tcp.onConnect(&NdnPrefixRegistration::tcpConnectH, this);
  m_tcp.onData(&NdnPrefixRegistration::tcpDataH, this);
  m_tcp.onDisconnect(&NdnPrefixRegistration::tcpDisconnectH, this);
}

NdnPrefixRegistration::~NdnPrefixRegistration()
{
  if (m_payload != nullptr) {
    free(m_payload);
  }
}

void
NdnPrefixRegistration::loop()
{
  if ((m_step == STEP_OK && millis() - m_lastRegister < m_registerInterval) ||
      (m_step == STEP_FAIL && millis() - m_lastRegister < m_retryInterval)) {
    return;
  }

  if (m_state == State::NDN_SENT && millis() - m_lastRegister >= 4000) {
    NDNPREFIXREG_DBG(F("NDN timeout"));
    m_state = State::FAIL;
  }

  if (m_state == State::FAIL) {
    m_tcp.abort();
    m_step = STEP_FAIL;
    m_state = State::NONE;
    m_lastRegister = millis();
    return;
  }

  if (m_step < 0) {
    m_step = 0;
    m_state = State::DONE;
    m_hasMore = true;
  }

  if (m_state == State::DONE) {
    m_tcp.abort();
    m_lastRegister = millis();
    if (m_hasMore) {
      ++m_step;
      m_state = State::HTTP_CONNECTING;
      m_tcp.connect(m_httpHost, m_httpPort);
    }
    else {
      m_step = STEP_OK;
      m_state = State::NONE;
    }
  }
}

void
NdnPrefixRegistration::tcpConnectH(void* self, AsyncClient*)
{
  reinterpret_cast<NdnPrefixRegistration*>(self)->tcpConnect();
}

void
NdnPrefixRegistration::tcpConnect()
{
  this->httpSend();
}

void
NdnPrefixRegistration::httpSend()
{
  NDNPREFIXREG_DBG(F("Sending HTTP request ") << _DEC(m_step));

  PString httpReq(m_httpHeader, sizeof(m_httpHeader));
  httpReq << "GET " << m_httpUri << _DEC(m_step) << F(" HTTP/1.1\r\n\r\n");

  m_tcp.add(httpReq, httpReq.length());
  m_state = State::HTTP_SENT;
  m_payloadLen = -1;
  m_hasMore = false;
}

void
NdnPrefixRegistration::tcpDataH(void* self, AsyncClient*, void* data, size_t dataLen)
{
  reinterpret_cast<NdnPrefixRegistration*>(self)->tcpData(data, dataLen);
}

void
NdnPrefixRegistration::tcpData(void* data, size_t dataLen)
{
  for (size_t i = 0; i < dataLen; ++i) {
    switch (m_state) {
      case State::HTTP_SENT:
        m_state = State::HTTP_RECV_HEADER;
        m_pos = 0;
        // fallthrough
      case State::HTTP_RECV_HEADER: {
        const char ch = reinterpret_cast<char*>(data)[i];
        if (ch == '\n') {
          this->parseHttpHeader();
          m_pos = 0;
        }
        else if (ch != '\r' && m_pos < sizeof(m_httpHeader) - 1) {
          m_httpHeader[m_pos++] = ch;
        }
        break;
      }
      case State::HTTP_RECV_PAYLOAD:
        if (m_pos < m_payloadLen && m_payload != nullptr) {
          m_payload[m_pos++] = reinterpret_cast<uint8_t*>(data)[i];
        }
        break;
      default:
        break;
    }
  }

  if (m_state == State::HTTP_RECV_PAYLOAD && m_pos == m_payloadLen) {
    m_state = State::PAYLOAD_READY;
    m_tcp.close(true);
  }
}

void
NdnPrefixRegistration::parseHttpHeader()
{
  m_httpHeader[m_pos] = '\0';

  if (m_pos == 0) {
    if (m_payloadLen > 0) {
      NDNPREFIXREG_DBG(F("HTTP payload size ") << m_payloadLen);
      m_state = State::HTTP_RECV_PAYLOAD;
      m_payload = reinterpret_cast<uint8_t*>(malloc(m_payloadLen));
    }
    else {
      NDNPREFIXREG_DBG(F("HTTP Content-Length missing"));
      m_state = State::FAIL;
    }
    return;
  }

  if (strncmp(m_httpHeader, "HTTP/1.1 ", 9) == 0) {
    if (strncmp(m_httpHeader + 9, "200", 3) != 0) {
      NDNPREFIXREG_DBG(F("HTTP non-200"));
      m_state = State::FAIL;
    }
  }
  else if (strncmp(m_httpHeader, "Content-Length: ", 16) == 0) {
    m_payloadLen = atoi(m_httpHeader + 16);
  }
  else if (strcmp(m_httpHeader, "X-Has-More: yes") == 0) {
    NDNPREFIXREG_DBG(F("HTTP has-more"));
    m_hasMore = true;
  }
}

void
NdnPrefixRegistration::tcpDisconnectH(void* self, AsyncClient*)
{
  reinterpret_cast<NdnPrefixRegistration*>(self)->tcpDisconnect();
}

void
NdnPrefixRegistration::tcpDisconnect()
{
  NDNPREFIXREG_DBG(F("TCP disconnected"));
  if (m_state == State::PAYLOAD_READY) {
    this->ndnSend();
  }
  else {
    m_state = State::FAIL;
  }
}

void
NdnPrefixRegistration::ndnSend()
{
  NDNPREFIXREG_DBG(F("Sending NDN command ") << _DEC(m_step));
  m_face.sendPacket(m_payload, m_payloadLen);
  free(m_payload);
  m_payload = nullptr;
  m_state = State::NDN_SENT;
  m_lastRegister = millis();
}

bool
NdnPrefixRegistration::processData(const ndn::DataLite& data)
{
  static ndn_NameComponent nameComps[4];
  ndn::NameLite name(nameComps, 4);
  name.append("localhop");
  name.append("nfd");
  name.append("rib");

  if (!name.match(data.getName())) {
    return false;
  }

  NDNPREFIXREG_DBG("Received NDN response " << _DEC(m_step));
  m_state = State::DONE;

  return true;
}
