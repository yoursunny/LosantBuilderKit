#include "DyndnsUpdate.hpp"
#include "logger.hpp"
#include <PString.h>
#include <Streaming.h>

#define DYNDNS_DBG(...) DBG(DyndnsUpdate, __VA_ARGS__)

DyndnsUpdate::DyndnsUpdate(const char* server, const char* host, const char* auth, int updateInterval, int initialUpdate)
  : m_server(server)
  , m_host(host)
  , m_auth(auth)
  , m_updateInterval(updateInterval)
  , m_nextUpdate(millis() + initialUpdate)
{
  m_tcp.setRxTimeout(8000);
  m_tcp.onConnect(&DyndnsUpdate::tcpConnectH, this);
  m_tcp.onDisconnect(&DyndnsUpdate::tcpDisconnectH, this);
}

void
DyndnsUpdate::loop()
{
  unsigned long now = millis();
  if (now < m_nextUpdate) {
    return;
  }

  m_nextUpdate = now + m_updateInterval;
  this->update();
}

void
DyndnsUpdate::update()
{
  m_tcp.abort();
  m_tcp.connect(m_server, 80);
  DYNDNS_DBG(F("start update"));
}

void
DyndnsUpdate::tcpConnectH(void* self, AsyncClient*)
{
  reinterpret_cast<DyndnsUpdate*>(self)->tcpConnect();
}

void
DyndnsUpdate::tcpConnect()
{
  char buf[256];
  PString req(buf, sizeof(buf));
  req << F("GET /nic/update?hostname=") << m_host << F(" HTTP/1.1\r\n"
           "Host: ") << m_server << F("\r\n"
           "Authorization: Basic ") << m_auth << F("\r\n"
           "Connection: close\r\n\r\n");
  m_tcp.add(req, req.length());
}

void
DyndnsUpdate::tcpDisconnectH(void* self, AsyncClient*)
{
  reinterpret_cast<DyndnsUpdate*>(self)->tcpDisconnect();
}

void
DyndnsUpdate::tcpDisconnect()
{
  DYNDNS_DBG(F("end update"));
}
