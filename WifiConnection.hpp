#ifndef WIFI_CONNECTION_HPP
#define WIFI_CONNECTION_HPP

#include <ESP8266WiFi.h>

/**
 * \brief WiFi credential
 *
 * open: ssid only, password set to nullptr
 * WPA/WPA2: ssid+password
 */
typedef std::pair<const char*, const char*> WifiCredential;

/**
 * \brief establish and maintain WiFi connection
 */
class WifiConnection
{
public:
  /**
   * \param connectTimeout if a network cannot be connected within this timeout (in millis), switch to the next network given in credentials; -1 to disable
   */
  WifiConnection(const WifiCredential* credentials, size_t nCredentials, int connectTimeout = -1);

  void
  loop();

  /**
   * \brief switch to the next network given in credentials list
   */
  void
  changeNetwork();

  bool
  isConnected() const;

private:
  const WifiCredential* const m_credentials;
  const size_t m_nCredentials;
  size_t m_credentialIndex;
  bool m_wasConnected;
  int m_connectTimeout;
  int m_lastChangeNetwork;
};

#endif // WIFI_CONNECTION_HPP
