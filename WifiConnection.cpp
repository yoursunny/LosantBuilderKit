#include "WifiConnection.hpp"
#include "logger.hpp"

#define WIFI_CONNECTION_DBG(...) DBG(WifiConnection, __VA_ARGS__)

WifiConnection::WifiConnection(const WifiCredential* credentials, size_t nCredentials, int connectTimeout)
  : m_credentials(credentials)
  , m_nCredentials(nCredentials)
  , m_credentialIndex(0)
  , m_wasConnected(true)
  , m_connectTimeout(connectTimeout)
  , m_lastChangeNetwork(millis())
{
  WiFi.disconnect();
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setSleepMode(WIFI_MODEM_SLEEP);
}

void
WifiConnection::loop()
{
  if (this->isConnected()) {
    if (!m_wasConnected) {
      m_wasConnected = true;
      WIFI_CONNECTION_DBG(F("connected to ") << WiFi.SSID() << ", " << WiFi.localIP() << "/" << WiFi.subnetMask() << "/" << WiFi.gatewayIP());
    }
  }
  else {
    if (m_wasConnected) {
      m_wasConnected = false;
      WIFI_CONNECTION_DBG(F("connecting to ") << m_credentials[m_credentialIndex].first);
    }
    else if (m_connectTimeout > 0 && millis() - m_lastChangeNetwork > m_connectTimeout) {
      this->changeNetwork();
    }

    WiFi.begin(m_credentials[m_credentialIndex].first, m_credentials[m_credentialIndex].second);
  }
}

void
WifiConnection::changeNetwork()
{
  if (++m_credentialIndex >= m_nCredentials) {
    m_credentialIndex = 0;
  }
  WiFi.disconnect();
  m_wasConnected = true;
  WIFI_CONNECTION_DBG(F("changing network"));
  m_lastChangeNetwork = millis();
}

bool
WifiConnection::isConnected() const
{
  return WiFi.status() == WL_CONNECTED;
}
