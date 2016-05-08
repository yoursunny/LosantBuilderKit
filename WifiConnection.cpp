#include "WifiConnection.hpp"

WifiConnection::WifiConnection(const char* ssid, const char* password)
  : m_ssid(ssid)
  , m_password(password)
  , m_isConnected(true)
{
}

void
WifiConnection::loop()
{
  if (this->isConnected()) {
    if (!m_isConnected) {
      m_isConnected = true;
      WIFI_CONNECTION_DBG("[WifiConnection] connected to ");
      WIFI_CONNECTION_DBG(m_ssid);
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
    if (m_isConnected) {
      m_isConnected = false;
      WIFI_CONNECTION_DBG("[WifiConnection] connecting to ");
      WIFI_CONNECTION_DBG(m_ssid);
      WIFI_CONNECTION_DBG("\n");
    }

    if (m_password != nullptr) {
      WiFi.begin(m_ssid, m_password);
    }
    else {
      WiFi.begin(m_ssid);
    }
  }
}

bool
WifiConnection::isConnected() const
{
  return WiFi.status() == WL_CONNECTED;
}
