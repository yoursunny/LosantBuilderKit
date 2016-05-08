#ifndef WIFI_CONNECTION_HPP
#define WIFI_CONNECTION_HPP

#include <ESP8266WiFi.h>

/**
 * \brief establish and maintain WiFi connection
 */
class WifiConnection
{
public:
  WifiConnection(const char* ssid, const char* password);

  void
  loop();

  bool
  isConnected() const;

private:
  const char* const m_ssid;
  const char* const m_password;
  bool m_isConnected;
};

#endif // WIFI_CONNECTION_HPP
