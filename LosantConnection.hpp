#ifndef LOSANT_CONNECTION_HPP
#define LOSANT_CONNECTION_HPP

#define LOSANT_CONNECTION_DBG Serial.print

#include <Losant.h>
#include "WifiConnection.hpp"

/**
 * \brief establish and maintain connection to Losant platform
 */
class LosantConnection
{
public:
  LosantConnection(WifiConnection& wifi, const char* deviceId, const char* accessKey, const char* accessSecret);

  void
  loop();

  bool
  isConnected() const;

  LosantDevice&
  getDevice()
  {
    return m_device;
  }

private:
  static const int RETRY_INTERVAL = 60000;
  WifiConnection& m_wifi;
  WiFiClientSecure m_client;
  LosantDevice m_device;
  const char* const m_accessKey;
  const char* const m_accessSecret;
  bool m_isConnecting;
  unsigned long m_lastRetry;
};

#endif // LOSANT_CONNECTION_HPP
