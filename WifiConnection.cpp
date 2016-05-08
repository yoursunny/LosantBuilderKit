#include "WifiConnection.hpp"

#define WIFI_CONNECTION_DBG Serial.print

WifiConnection::WifiConnection(const WifiCredential* credentials, size_t nCredentials, int connectTimeout)
  : m_credentials(credentials)
  , m_nCredentials(nCredentials)
  , m_credentialIndex(0)
  , m_wasConnected(true)
  , m_connectTimeout(connectTimeout)
  , m_lastChangeNetwork(millis())
{
  WiFi.disconnect();
}

void
WifiConnection::loop()
{
  if (this->isConnected()) {
    if (!m_wasConnected) {
      m_wasConnected = true;
      WIFI_CONNECTION_DBG("[WifiConnection] connected to ");
      WIFI_CONNECTION_DBG(WiFi.SSID());
      WIFI_CONNECTION_DBG(", ");
      WIFI_CONNECTION_DBG(WiFi.localIP());
      WIFI_CONNECTION_DBG("/");
      WIFI_CONNECTION_DBG(WiFi.subnetMask());
      WIFI_CONNECTION_DBG("/");
      WIFI_CONNECTION_DBG(WiFi.gatewayIP());
      WIFI_CONNECTION_DBG("\n");
    }
  }
  else {
    if (m_wasConnected) {
      m_wasConnected = false;
      WIFI_CONNECTION_DBG("[WifiConnection] connecting to ");
      WIFI_CONNECTION_DBG(m_credentials[m_credentialIndex].first);
      WIFI_CONNECTION_DBG("\n");
    }
    else if (m_connectTimeout > 0 && millis() - m_lastChangeNetwork > m_connectTimeout) {
      this->changeNetwork();
    }

    if (m_credentials[m_credentialIndex].second == nullptr) {
      WiFi.begin(m_credentials[m_credentialIndex].first);
    }
    else {
      WiFi.begin(m_credentials[m_credentialIndex].first, m_credentials[m_credentialIndex].second);
    }
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
  WIFI_CONNECTION_DBG("[WifiConnection] changing network\n");
  m_lastChangeNetwork = millis();
}

bool
WifiConnection::isConnected() const
{
  return WiFi.status() == WL_CONNECTED;
}
