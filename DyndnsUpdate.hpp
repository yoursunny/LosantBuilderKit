#ifndef DYNDNS_UPDATE_HPP
#define DYNDNS_UPDATE_HPP

#include <ESPAsyncTCP.h>

/**
 * \brief perform periodical dyndns update
 */
class DyndnsUpdate
{
public:
  DyndnsUpdate(const char* server, const char* host, const char* auth, int updateInterval = 1800000, int initialUpdate = 3000);

  void
  loop();

  void
  update();

private:
  static void
  tcpConnectH(void* self, AsyncClient*);

  void
  tcpConnect();

  static void
  tcpDisconnectH(void* self, AsyncClient*);

  void
  tcpDisconnect();

private:
  const char* const m_server;
  const char* const m_host;
  const char* const m_auth;
  int m_updateInterval;
  unsigned long m_nextUpdate;
  AsyncClient m_tcp;
};

#endif // DYNDNS_UPDATE_HPP
