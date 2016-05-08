#include "LosantPingPong.hpp"

#define LOSANT_PINGPONG_DBG Serial.print

LosantPingPong::LosantPingPong(LosantDevice& device, int pingInterval, int pongMissThreshold)
  : m_device(device)
  , m_pingInterval(pingInterval)
  , m_pongMissThreshold(pongMissThreshold)
  , m_lastPing(millis())
  , m_hasPong(true)
  , m_nMissedPongs(0)
{
}

void
LosantPingPong::loop()
{
  if (millis() - m_lastPing < m_pingInterval) {
    return;
  }

  if (!m_device.connected()) {
    m_lastPing = millis();
    m_hasPong = true;
    m_nMissedPongs = 0;
    return;
  }

  if (m_hasPong) {
    m_nMissedPongs = 0;
  }
  else {
    ++m_nMissedPongs;
    LOSANT_PINGPONG_DBG("[LosantPingPong] ");
    LOSANT_PINGPONG_DBG(m_nMissedPongs, DEC);
    LOSANT_PINGPONG_DBG(" pong missed\n");
    if (m_nMissedPongs >= m_pongMissThreshold) {
      m_nMissedPongs = 0;
      LOSANT_PINGPONG_DBG("[LosantPingPong] disconnecting LosantDevice due to too many missed pongs\n");
      m_device.disconnect();
      return;
    }
  }

  LOSANT_PINGPONG_DBG("[LosantPingPong] sending ping\n");
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["act"] = "ping";
  m_device.sendState(root);

  m_lastPing = millis();
  m_hasPong = false;
}

void
LosantPingPong::handlePong(LosantCommand* cmd)
{
  LOSANT_PINGPONG_DBG("[LosantPingPong] pong received\n");
  m_hasPong = true;
}
