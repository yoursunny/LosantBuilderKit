#include "LosantConnection.hpp"
#include "logger.hpp"

#define LOSANT_CONNECTION_DBG(...) DBG(LosantConnection, __VA_ARGS__)

LosantConnection::LosantConnection(WifiConnection& wifi, const char* deviceId, const char* accessKey, const char* accessSecret)
  : m_wifi(wifi)
  , m_device(deviceId)
  , m_accessKey(accessKey)
  , m_accessSecret(accessSecret)
  , m_isConnecting(false)
  , m_lastRetry(0)
{
}

void
LosantConnection::loop()
{
  m_device.loop();

  if (!m_wifi.isConnected()) {
    if (this->isConnected()) {
      LOSANT_CONNECTION_DBG(F("disconnecting due to lost wifi connection"));
      m_device.disconnect();
    }
    m_isConnecting = false;
    return;
  }

  if (this->isConnected()) {
    if (m_isConnecting) {
      m_isConnecting = false;
      LOSANT_CONNECTION_DBG(F("connected in ") << _DEC(millis() - m_lastRetry) << F("ms"));
    }
  }
  else {
    if (m_isConnecting && millis() - m_lastRetry < RETRY_INTERVAL) {
      return;
    }
    LOSANT_CONNECTION_DBG(F("connecting as ") << m_device.getId());
    m_device.connectSecure(m_client, m_accessKey, m_accessSecret);
    m_isConnecting = true;
    m_lastRetry = millis();
  }
}

bool
LosantConnection::isConnected() const
{
  return const_cast<LosantDevice&>(m_device).connected();
}
